// This file is part of WaterProof.
// Copyright (C) 2019  The ChefCoq team.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "server.h"
#include <inaddr.h>

namespace wpwrapper {

server::server(std::shared_ptr<wpwrapper::api> api_instance,
        std::vector<wpwrapper::server::failure_callback> failure_callbacks,
        std::vector<wpwrapper::server::request_callback> request_callbacks,
        std::vector<wpwrapper::server::invalidate_callback> invalidate_callbacks)
        :running_(false), next_id_(0), api_(std::move(api_instance)),
         on_failure_(std::move(failure_callbacks)),
         on_request_(std::move(request_callbacks)),
         on_invalidate_(std::move(invalidate_callbacks))
{
    logger_ = spdlog::get("main")->clone(fmt::format("server"));

    int result;

    // Initialize WinSock2 library.
    WSADATA wsadata;
    result = api_->WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (result != 0)
    {
        throw api_error("unable to initialize winsock library", result, logger_);
    }

    // Resolve server address.
    addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    addrinfo* addr;

    result = api_->getaddrinfo("localhost", "0", &hints, &addr);
    if (result != 0)
    {
        api_->WSACleanup();
        throw api_error("unable to resolve server address", result);
    }

    // Create server socket.
    listen_socket_ = api_->socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (listen_socket_ == INVALID_SOCKET)
    {
        int err = api_->WSAGetLastError();
        api_->freeaddrinfo(addr);
        api_->WSACleanup();
        throw api_error("unable to create server socket", err, logger_);
    }

    // Bind server socket to resolved address.
    result = api_->bind(listen_socket_, addr->ai_addr, addr->ai_addrlen);
    if (result != 0)
    {
        int err = api_->WSAGetLastError();
        api_->freeaddrinfo(addr);
        api_->closesocket(listen_socket_);
        api_->WSACleanup();
        throw api_error("unable to bind server socket", err, logger_);
    }

    struct sockaddr_in sin;
    int addrlen = sizeof(sin);
    if (getsockname(listen_socket_, (struct sockaddr *)&sin, &addrlen) != 0 || sin.sin_family != AF_INET || sizeof(sockaddr_in) != addrlen)
    {
        int err = api_->WSAGetLastError();
        api_->freeaddrinfo(addr);
        api_->closesocket(listen_socket_);
        api_->WSACleanup();
        throw api_error("unable to get name of server socket", err, logger_);
    }

    int server_port = htons(sin.sin_port);
    logger_->info("got port {}", server_port);

    // Don't need this anymore.
    api_->freeaddrinfo(addr);

    // Make socket ready for connections.
    if (api_->listen(listen_socket_, SOMAXCONN) == SOCKET_ERROR)
    {
        int err = api_->WSAGetLastError();
        api_->closesocket(listen_socket_);
        api_->WSACleanup();
        throw api_error("unable to listen on server socket", err, logger_);
    }

    // Create UDP sockets which will server as interrupt mechanism for blocking poll() calls.
    interrupt_[0] = api_->socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (interrupt_[0] == INVALID_SOCKET)
    {
        int err = api_->WSAGetLastError();
        api_->closesocket(listen_socket_);
        api_->WSACleanup();
        throw api_error("unable to create read end of interrupt socket", err, logger_);
    }

    interrupt_[1] = api_->socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (interrupt_[1] == INVALID_SOCKET)
    {
        int err = api_->WSAGetLastError();
        close_all(std::vector<socket>{listen_socket_, interrupt_[0]});
        api_->WSACleanup();
        throw api_error("unable to create write end of interrupt socket", err, logger_);
    }

    // Resolve i address.
    hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;

    addrinfo* iaddr;

    result = api_->getaddrinfo("localhost", std::to_string(server_port).c_str(), &hints, &iaddr);
    if (result != 0)
    {
        close_all(std::vector<socket>{listen_socket_, interrupt_[0], interrupt_[1]});
        api_->WSACleanup();
        throw api_error("unable to resolve interrupt address", result, logger_);
    }

    // Bind interrupt socket to resolved address.
    result = api_->bind(interrupt_[0], iaddr->ai_addr, iaddr->ai_addrlen);
    if (result != 0)
    {
        int err = api_->WSAGetLastError();
        api_->freeaddrinfo(iaddr);
        close_all(std::vector<socket>{listen_socket_, interrupt_[0], interrupt_[1]});
        api_->WSACleanup();
        throw api_error("unable to bind interrupt socket", err, logger_);
    }

    // Connect to socket.
    result = connect(interrupt_[1], iaddr->ai_addr, iaddr->ai_addrlen);
    if (result != 0)
    {
        int err = api_->WSAGetLastError();
        api_->freeaddrinfo(iaddr);
        close_all(std::vector<socket>{listen_socket_, interrupt_[0], interrupt_[1]});
        api_->WSACleanup();
        throw api_error("unable to connect interrupt socket", err, logger_);
    }

    // Start server threads.
    // NOTE: Do not change this message, waterproof relies on the wording and extracts port from here.
    logger_->info("started listening on port {}", server_port);
    running_ = true;
    accept_thread_ = std::thread(&server::accept_loop, this);
    read_thread_ = std::thread(&server::read_loop, this);
    write_thread_ = std::thread(&server::write_loop, this);
}

server::~server() noexcept
{
    // Notify server threads that they need to stop.
    if (running_)
    {
        {
            std::lock_guard<std::mutex> guard(response_queue_mutex_);
            running_ = false;
        }
        cv_.notify_one();

        // Close write end of interrupt pipe. This will cause the accept and read threads to finish execution.
        api_->closesocket(interrupt_[1]);
    }

    // Wait until server threads have finished execution.
    if (accept_thread_.joinable())
    {
        accept_thread_.join();
    }

    if (read_thread_.joinable())
    {
        read_thread_.join();
    }

    if (write_thread_.joinable())
    {
        write_thread_.join();
    }

    // Cleanup.
    std::vector<socket> remaining_sockets{listen_socket_, interrupt_[0]};
    remaining_sockets.insert(remaining_sockets.end(), clients_.begin(), clients_.end());

    close_all(remaining_sockets);
    api_->WSACleanup();
}

void server::close_all(const std::vector<wpwrapper::server::socket>& fds)
{
    for (const auto& fd: fds)
    {
        api_->closesocket(fd);
    }
}

int server::last_error() const noexcept
{
    return api_->WSAGetLastError();
}

int server::wait(wpwrapper::server::waitfd fds[], int n) const noexcept
{
    // Timeout -1 to wait indefinitely.
    return api_->WSAPoll(fds, n, -1);
}

} // namespace wpwrapper
