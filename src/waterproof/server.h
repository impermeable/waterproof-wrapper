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

#ifndef WPWRAPPER_SERVER_H
#define WPWRAPPER_SERVER_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include <spdlog/spdlog.h>

#include "message.h"
#include "../utils/exceptions.h"

#ifdef WPWRAPPER_WIN

#include "../win/api.h"

#elif WPWRAPPER_POSIX

#include "../posix/api.h"

#endif

namespace wpwrapper {

class server {
public:
    /// \brief A failure callback takes the error that lead to failure as argument.
    using failure_callback = std::function<void(const api_error&)>;
    /// \brief A request callback takes the received request as argument.
    using request_callback = std::function<void(const request&)>;
    /// \brief An invalidate callback takes the invalid worker id as argument.
    using invalidate_callback = std::function<void(unsigned int)>;

#ifdef WPWRAPPER_WIN
    /// \brief Platform-agnostic socket type. On Windows, a socket is a void pointer.
    using socket = SOCKET;
    using waitfd = WSAPOLLFD;
#elif WPWRAPPER_POSIX
    /// \brief Platform-agnostic socket type. On Ubuntu and macOS, a socket is a file descriptor int.
    using socket = int;
    using waitfd = pollfd;
#endif

    /// \brief Constructs a server that listens on port \c port.
    /// \details Creates three server threads: one for accepting new clients, one for reading from these clients and one
    /// for writing to these clients.
    /// \param api_instance The API instance to use.
    /// \param failure_callbacks A list of callbacks to execute when an error occurs in any of the server threads.
    /// \param request_callbacks A list of callbacks to execute when a request is received from Waterproof.
    /// \param invalidate_callbacks A list of callbacks to execute when a worker instance becomes invalid.
    /// \throw api_error If the socket could not be created, or if any other API call fails.
    server(std::shared_ptr<api> api_instance, std::vector<failure_callback> failure_callbacks,
            std::vector<request_callback> request_callbacks, std::vector<invalidate_callback> invalidate_callbacks);

    /// \brief Destructs this worker.
    /// \details Stops the server threads and cleans up open handles/file descriptors.
    ~server() noexcept;

    // Server is non-copyable.
    server(const server& other) = delete;

    // Server is non-movable.
    server(server&& other) = delete;

    // Server is non-copyable.
    server& operator=(const server& other) = delete;

    // Server is non-movable.
    server& operator=(server&& other) = delete;

    /// \brief Add a response to be sent to Waterproof.
    /// \param response The response to add.
    void enqueue(const response& response);

    /// \brief Unmaps a single worker from its socket.
    /// \param id Unique identifier for the worker to unmap.
    void unmap(unsigned int id, const response& response);

private:

    /// \brief Closes a list of sockets (on Windows) or file descriptors (on macOS and Ubuntu).
    /// \param fds A list containing all file descriptors to close.
    void close_all(const std::vector<socket>& fds);

    /// \brief Executes on_failure callbacks and stops all server threads.
    /// \param error The error that lead to failure.
    void fail(const api_error& error);

    /// \brief Unmaps all workers associated with a client. Executes the on_invalidate callbacks.
    /// \param client The socket to invalidate.
    void invalidate(socket client);

    /// \brief Returns the error status for the last failed operation.
    /// \return The error status for the last failed operation.
    int last_error() const noexcept;

    /// \brief Waits on an array of poll file descriptors.
    /// \param fds The poll file descriptors to wait on.
    /// \param n The number of file descriptors to wait on.
    /// \return The number of file descriptors with nonzero revents values.
    int wait(waitfd fds[], int n) const noexcept;

    /// \brief Reads a request from a socket.
    /// \param client The socket to read from.
    /// \return The read request. Empty if the socket was reset or shutdown.
    /// \throw api_error If an API call fails.
    std::optional<request> read(socket client) const;

    /// \brief Writes a response to a socket.
    /// \param client The socket to write to.
    /// \param response The response to write.
    void write(socket client, const response& response) const;

    /// \brief Listens on the listen socket and accepts new clients.
    /// \note Should be executed on a separate thread.
    void accept_loop() noexcept;

    /// \brief Performs subsequent reads from Waterproof.
    /// \note Should be executed on a separate thread.
    void read_loop() noexcept;

    /// \brief Writes messages to Waterproof whenever they become available.
    /// \note Should be executed on a separate thread.
    void write_loop() noexcept;

    /// \brief Indicates whether the server threads should be running or not.
    std::atomic<bool> running_;

    /// \brief The lowest free worker instance id, which will be assigned to a new worker upon a create request.
    unsigned int next_id_;
    /// \brief Socket used to listen for new clients.
    socket listen_socket_;

    /// \brief Logger used in this server.
    std::shared_ptr<spdlog::logger> logger_;
    /// \brief API instance used for all API calls.
    std::shared_ptr<api> api_;

    /// \brief List of callbacks that are executed when an error occurs in one of the worker threads.
    std::vector<failure_callback> on_failure_;
    /// \brief List of callbacks that are executed when a request is received from Waterproof.
    std::vector<request_callback> on_request_;
    /// \brief List of callbacks that are executed when a worker becomes invalid.
    std::vector<invalidate_callback> on_invalidate_;

    /// \brief Priority queue containing all responses that still need to be sent.
    std::priority_queue<response> response_queue_;
    /// \brief Guards the response queue.
    mutable std::mutex response_queue_mutex_;
    /// \brief Notified when a new response is available or when the server threads need to finish execution.
    std::condition_variable cv_;

    /// \brief Maps worker instances to their corresponding sockets. Used to route responses to the correct destination.
    std::map<int, socket> client_map_;
    /// \brief List of all accepted client sockets.
    std::vector<socket> clients_;
    /// \brief Queue of all clients that have been accepted but not yet marked as readable by the read thread.
    std::queue<socket> new_clients_;
    /// \brief Guards the client collections.
    mutable std::mutex clients_mutex_;

    /// \brief Thread on which the accept loop is executed.
    std::thread accept_thread_;
    /// \brief Thread on which the read loop is executed.
    std::thread read_thread_;
    /// \brief Thread on which the write loop is executed.
    std::thread write_thread_;

    /// \brief Used to interrupt blocking poll() calls when certain events occur. Written to when the client list has
    /// been refreshed, closed when the server threads need to finish execution.
    /// \details On Windows, this is an UDP socket. On macOS or Ubuntu, this is a self-pipe.
    socket interrupt_[2];
};

} // namespace wpwrapper

#endif // WPWRAPPER_SERVER_H
