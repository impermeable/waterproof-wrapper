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

#include "../utils/buffers.h"

namespace wpwrapper {

void server::enqueue(const wpwrapper::response& response)
{
    {
        std::lock_guard<std::mutex> guard(response_queue_mutex_);
        response_queue_.push(response);
    }
    cv_.notify_one();
}

void server::unmap(unsigned int id, const response& response)
{
    std::lock_guard<std::mutex> guard(clients_mutex_);

    try
    {
        socket client = client_map_[id];
        write(client, response);
    }
    catch (const api_error& e)
    {
        logger_->debug("error occurred in writing final message to {}", id);
        // we are already shutting down so ignore any errors
    }

    client_map_.erase(id);
    logger_->debug("unmapped instance {}", id);
}

void server::fail(const wpwrapper::api_error& error)
{
    logger_->error("aborting");
    // Notify server threads.
    {
        std::lock_guard<std::mutex> guard(response_queue_mutex_);
        running_ = false;
    }
    cv_.notify_one();

    // Causes an POLLHUP event in the accept and read threads.
    close_all(std::vector<socket>{interrupt_[0]});

    // Notify subscribers.
    for (const auto& callback: on_failure_)
    {
        callback(error);
    }
}

void server::invalidate(wpwrapper::server::socket client)
{
    logger_->debug("invalidating socket {}", client);

    std::lock_guard<std::mutex> guard(clients_mutex_);

    // Remove the socket from the clients list.
    auto in_clients = std::find(clients_.begin(), clients_.end(), client);
    if (in_clients != clients_.end())
    {
        clients_.erase(in_clients);
    }

    // Remove all mappings to the invalid socket.
    for (auto it = client_map_.cbegin(); it != client_map_.cend(); /* Do not increment. */)
    {
        if (it->second == client)
        {
            it = client_map_.erase(it);
            logger_->debug("unmapped instance {} from socket {}", it->first, it->second);

            // Notify subscribers.
            for (const auto& callback: on_invalidate_)
            {
                callback(it->first);
            }
            logger_->debug("invalidated instance {}", it->first);
        }
        else
        {
            ++it;
        }
    }

    // Close the invalid socket.
    close_all(std::vector<socket>{client});
}

std::optional<wpwrapper::request> server::read(wpwrapper::server::socket client) const
{
    std::vector<char> buffer(4096, 0);

    // Points to the next character to read into.
    char* data;
    int bytes_to_read;
    int result;

    // First four bytes in a request indicate request length.
    uint32_t length = 0;

    data = buffer.data();
    bytes_to_read = sizeof length;
    while (bytes_to_read > 0)
    {
        result = api_->recv(client, data, bytes_to_read, 0);

        if (result == 0 || (result < 0 && last_error() == WPCONNRESET))
        {
            // Socket closed on other end of the connection.
            return {};
        }
        else if (result < 0)
        {
            throw api_error(fmt::format("unable to read from socket {}", client), last_error(), logger_);
        }
        else
        {
            bytes_to_read -= result;

            // Move the pointer to the next character to read into.
            data += result;
        }
    }

    // Read the request length from the buffer.
    length = buffers::read_uint32(buffer, buffers::endianness::big);
    buffers::clear(buffer);

    logger_->trace("reading {} bytes from socket {}", length, client);

    // Read data from Waterproof.
    std::string raw_request;
    bytes_to_read = length;
    while (bytes_to_read > 0)
    {
        // Read at most the buffer length bytes.
        result = api_->recv(client, buffer.data(), (bytes_to_read > buffer.size()) ? buffer.size() : bytes_to_read, 0);

        if (result == 0 || (result < 0 && last_error() == WPCONNRESET))
        {
            // Socket closed on other end of the connection.
            return {};
        }
        else if (result < 0)
        {
            throw api_error(fmt::format("unable to read from socket {}", client), last_error(), logger_);
        }
        else
        {
            bytes_to_read -= result;

            raw_request.append(buffer.data(), result);
            buffers::clear(buffer);
        }
    }

    logger_->trace("read {} ({} chars) from socket {}", raw_request, raw_request.length(), client);

    // Parse data to request.
    auto j = nlohmann::json::parse(raw_request);
    return j.get<request>();
}

void server::write(wpwrapper::server::socket client, const wpwrapper::response& response) const
{
    std::vector<char> buffer(4, 0);

    // Points to the next character to write.
    const char* data;
    int bytes_to_write;
    int result;

    // Serialize the response.
    nlohmann::json json = response;
    std::string raw = json.dump();

    // Write the response length to Waterproof.
    uint32_t length = raw.length();
    buffers::write_uint32(length, buffer, buffers::endianness::big);

    logger_->trace("writing {:#010x} to socket {}", length, client);

    data = buffer.data();
    bytes_to_write = sizeof length;
    while (bytes_to_write > 0)
    {
        result = api_->send(client, data, bytes_to_write, 0);

        if (result < 0)
        {
            throw api_error(fmt::format("unable to write to socket {}", client), last_error());
        }

        bytes_to_write -= result;

        // Move pointer to next char to write.
        data += result;
    }

    // Write the response itself to Waterproof.
    data = raw.c_str();
    bytes_to_write = length;
    while (bytes_to_write > 0)
    {
        result = api_->send(client, data, bytes_to_write > 4096 ? 4096 : bytes_to_write, 0);

        if (result < 0)
        {
            throw api_error(fmt::format("unable to write to socket {}", client), last_error());
        }

        bytes_to_write -= result;

        // Move pointer to next char to write.
        data += result;
    }

    logger_->trace("wrote '{}' ({} chars) to socket {}", raw, length, client);
}

void server::accept_loop() noexcept
{
    logger_->debug("started accept loop");

    // The ACK char is written to the interrupt pipe when a new client is accepted.
    char ack = '\x06';

    int result;
    int written;

    // Wait for POLLRDBAND on the interrupt socket as macOS does not seem to adequately support waiting on 'nothing'.
    waitfd interrupt = {interrupt_[0], POLLRDBAND};
    waitfd listen = {listen_socket_, POLLRDNORM};

    waitfd waitfds[2] = {interrupt, listen};

    while (running_)
    {
        result = wait(waitfds, 2);

        if (result < 0)
        {
            fail(api_error("unable to wait on interrupt/accept fds", last_error(), logger_));
            break;
        }
        else if (result == 0)
        {
            // Spurious wakeup, continue to next iteration.
            continue;
        }

        if (waitfds[0].revents & (POLLHUP | POLLERR))
        {
            // Peer disconnected.
            logger_->debug("received interrupt on accept loop");
            break;
        }

        if (waitfds[1].revents & POLLRDNORM)
        {
            // Pending connection on listen socket.
            socket client = api_->accept(listen_socket_, nullptr, nullptr);

            if (client < 0)
            {
                fail(api_error("unable to accept a new client", last_error(), logger_));
                break;
            }

            // TODO: mark socket non-inheritable?

            {
                std::lock_guard<std::mutex> guard(clients_mutex_);
                clients_.push_back(client);
                new_clients_.push(client);
            }

            logger_->debug("signalling read thread to refresh");

            // Notify the read thread that a new client has been accepted.
            written = 0;
            while (written == 0)
            {
                written = api_->send(interrupt_[1], &ack, 1, 0);

                if (written < 0)
                {
                    fail(api_error("unable to write to interrupt pipe", last_error(), logger_));
                    break;
                }
            }
        }
    }

    logger_->debug("stopped accept loop");
}

void server::read_loop() noexcept
{
    logger_->debug("started read loop");
    int result;

    std::vector<socket> invalid_sockets;
    waitfd waitfds[256];
    int next = 0;

    waitfd interrupt = {interrupt_[0], POLLRDNORM};
    waitfds[next++] = interrupt;

    while (running_)
    {
        result = wait(waitfds, next);

        if (result < 0)
        {
            fail(api_error("unable to wait on interrupt/client fds", last_error(), logger_));
            break;
        }
        else if (result == 0)
        {
            // Spurious wakeup, continue to next iteration.
            continue;
        }

        // Check if anything happened on the interrupt fd.
        if (waitfds[0].revents & (POLLHUP | POLLERR))
        {
            // Peer disconnected.
            logger_->debug("received interrupt on read loop");
            break;
        }
        else if (waitfds[0].revents & POLLRDNORM)
        {
            // Data available for read on interrupt fd. A new client has been accepted on the accept() thread.
            char ack;
            int read = 0;
            while (read == 0)
            {
                read = api_->recv(interrupt.fd, &ack, 1, 0);

                if (read < 0)
                {
                    fail(api_error("unable to read from interrupt fd", last_error(), logger_));
                }

                if (read == 1 && ack != '\06')
                {
                    logger_->warn("read unexpected char {:#04x} from interrupt fd", ack);
                }
            }

            // Retrieve the last inserted client.
            socket recent;
            {
                std::lock_guard<std::mutex> guard(clients_mutex_);
                recent = new_clients_.front();
                new_clients_.pop();
            }

            logger_->debug("received refresh for new socket {}", recent);

            // Start listening for incoming data on the new client.
            waitfd client = {recent, POLLRDNORM, 0};
            waitfds[next++] = client;

            continue;
        }

        // Check if anything happened on the client sockets.

        for (int j = 1; j < next; ++j)
        {
#ifdef WPWRAPPER_POSIX
//            waitfds[j-1]->fd = array[j].fd;
//            waitfds[j-1]->events = array[j].events;
#endif

            if (waitfds[j].revents & (POLLHUP | POLLERR))
            {
                logger_->debug("received {} shutdown on socket {}", waitfds[j].revents & POLLHUP ? "soft" : "hard",
                        waitfds[j].fd);
                invalid_sockets.push_back(waitfds[j].fd);
                waitfds[j].fd = -1;
            }
            else if (waitfds[j].revents & POLLRDNORM)
            {
                std::optional<request> request;

                try
                {
                    request = read(waitfds[j].fd);
                }
                catch (const api_error& e)
                {
                    // API error is not fatal for server, but is fatal for client.
                    logger_->error(e.what());
                    invalid_sockets.push_back(waitfds[j].fd);
                    waitfds[j].fd = -1;
                    continue;
                }
                catch (const nlohmann::json::parse_error& e)
                {
                    // Parse error is not fatal for either client or server.
                    logger_->warn("json parse error on socket {}: {}", waitfds[j].fd, e.what());
                    continue;
                }

                if (request)
                {
                    // On receiving a create request, we need to assign an instance id and map it to the socket.
                    if (request->verb_ == request::verb::create)
                    {
                        std::lock_guard<std::mutex> guard(clients_mutex_);

                        request->instance_id_ = next_id_++;
                        client_map_.insert(std::make_pair(request->instance_id_, waitfds[j].fd));

                        logger_->debug("mapped instance {} to socket {}", request->instance_id_, waitfds[j].fd);
                    }

                    for (const auto& callback: on_request_)
                    {
                        callback(request.value());
                    }
                }
                else
                {
                    // Read 0 bytes or got WPCONNRESET: connection shut down on other end.
                    logger_->debug("received shutdown on socket {} while reading", waitfds[j].fd);
                    invalid_sockets.push_back(waitfds[j].fd);
                    waitfds[j].fd = -1;
                }
            }
        }

        // Deal with invalid sockets.
        for (const auto& socket: invalid_sockets)
        {
            invalidate(socket);
        }

        invalid_sockets.clear();
    }

    logger_->debug("stopped read loop");
}

void server::write_loop() noexcept
{
    logger_->debug("started write loop");

    while (running_)
    {
        std::unique_lock<std::mutex> lock(response_queue_mutex_);

        // Wait until a new response can be sent or until we're told to stop.
        cv_.wait(lock, [&]
        {
            return !response_queue_.empty() || !running_;
        });

        if (!running_)
        {
            logger_->debug("received interrupt on write loop");
            break;
        }

        response response = response_queue_.top();
        response_queue_.pop();
        socket client = client_map_[response.instance_id_];

        lock.unlock();

        try
        {
            write(client, response);
        }
        catch (const api_error& e)
        {
            // Fatal error for client, but not for server.
            invalidate(client);
        }
    }

    logger_->debug("stopped write loop");
}

} // namespace wpwrapper
