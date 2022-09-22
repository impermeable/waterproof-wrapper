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

#include "worker.h"

#include <cerrno>
#include <future>

#include "../utils/buffers.h"

namespace wpwrapper {

worker::worker(unsigned int id, const std::string& sertop_path, const std::vector<std::string>& sertop_args,
        std::shared_ptr<wpwrapper::api> api_instance,
        std::vector<wpwrapper::worker::failure_callback> failure_callbacks,
        std::vector<wpwrapper::worker::response_callback> response_callbacks)
        :id_(id), running_(false), api_(std::move(api_instance)),
         on_failure_(std::move(failure_callbacks)), on_response_(std::move(response_callbacks))
{
    logger_ = spdlog::get("main")->clone(fmt::format("worker{}", id));

    // Create interrupt pipe.
    if (api_->pipe(interrupt_fd_) < 0)
    {
        throw api_error("failed to create interrupt pipe", errno);
    }

    // Create pipes from and to sertop.
    if (api_->pipe(stdin_fd_) < 0)
    {
        throw api_error("failed to create pipe to sertop", errno);
    }

    if (api_->pipe(stdout_fd_) < 0)
    {
        api_->close(stdin_fd_[0]);
        api_->close(stdin_fd_[1]);
        throw api_error("failed to create pipe to sertop", errno);
    }

    // Create sertop instance.
    sertop_instance_ = api_->fork();

    if (sertop_instance_ < 0)
    {
        // Fork failed.
        api_->close(stdin_fd_[0]);
        api_->close(stdin_fd_[1]);
        api_->close(stdout_fd_[0]);
        api_->close(stdout_fd_[1]);

        throw api_error("failed to fork", errno);
    }
    else if (sertop_instance_ == 0)
    {
        // We're the child process.

        // We can't / won't read from our stdout nor write to our stdin.
        api_->close(interrupt_fd_[0]);
        api_->close(interrupt_fd_[1]);
        api_->close(stdin_fd_[1]);
        api_->close(stdout_fd_[0]);

        // Replace our stdin/stdout with the read/write ends of the pipes.
        api_->dup2(stdin_fd_[0], STDIN_FILENO);
        api_->dup2(stdout_fd_[1], STDOUT_FILENO);

        // Don't need these copies anymore.
        api_->close(stdin_fd_[0]);
        api_->close(stdout_fd_[1]);

        // 0: path
        // 1: --print0
        // 2..n-1: sertop_params
        // n: \0
        char* params[3 + sertop_args.size()];
        params[0] = const_cast<char*>(sertop_path.c_str());
        params[1] = const_cast<char*>("--print0");

        for (int i = 0; i < sertop_args.size(); ++i)
        {
            params[2 + i] = const_cast<char*>(sertop_args[i].c_str());
        }

        params[2 + sertop_args.size()] = static_cast<char*>(nullptr);

        if (api_->execv(sertop_path.c_str(), params) < 0)
        {
            logger_->critical("could not create sertop process (error code: {})", errno);

            // Exit child process
            api_->_exit(1);
        }
    }

    // We're the parent process.

    // We can't / won't read from sertop's stdin nor write to its stdout.
    api_->close(stdin_fd_[0]);
    api_->close(stdout_fd_[1]);

    // Start the worker threads.
    running_ = true;
    read_thread_ = std::thread(&worker::read_loop, this);
    write_thread_ = std::thread(&worker::write_loop, this);
}

worker::~worker() noexcept
{
    // Signal worker threads to stop.
    if (running_)
    {
        {
            std::lock_guard<std::mutex> guard(message_queue_mutex_);
            running_ = false;
        }
        cv_.notify_one();

        // Closing the write end of the interrupt pipe will cause an POLLUP event in the read thread.
        char c = '\01';
        int written = 0;
        while (written == 0)
        {
            written = api_->write(interrupt_fd_[1], &c, 1);

            if (written < 0)
            {
                logger_->error("unable to write to interrupt pipe (error code: {})", errno);
                break;
            }

            logger_->debug(written);
        }
    }

    // Wait until worker threads have finished execution.
    if (read_thread_.joinable())
    {
        read_thread_.join();
    }

    if (write_thread_.joinable())
    {
        write_thread_.join();
    }

    logger_->debug("threads joined");

    // Close pipe handles to sertop. This will cause the sertop process to shut down gracefully.
    api_->close(stdin_fd_[1]);
    api_->close(stdout_fd_[0]);

    // Verify that the sertop process has shut down. If it is still open after 500ms, terminate it.
    bool should_terminate = false;

    // This future continuously checks if the sertop process has shut down yet.
    std::future<bool> future = std::async(std::launch::async,
            [&logger = logger_, &api = api_, &sertop_instance = sertop_instance_]()
            {
                int status;

                do
                {
                    // This does not block!
                    if (api->waitpid(sertop_instance, &status, 0) < 0)
                    {
                        logger->error("unable to wait for sertop process shutdown (error code: {})", errno);
                        return false;
                    }
                }
                while (!(WIFEXITED(status) || WIFSIGNALED(status)));

                return true;
            });

    switch (future.wait_for(std::chrono::milliseconds(500)))
    {
    case std::future_status::ready:
        logger_->debug("sertop process shut down gracefully");
        break;
    case std::future_status::timeout:
        logger_->warn("timeout while waiting for sertop process shutdown");
        should_terminate = true;
        break;
    case std::future_status::deferred:
        logger_->error("unable to wait for sertop process shutdown");
        should_terminate = true;
        break;
    }

    if (should_terminate)
    {
        logger_->info("terminating sertop process");

        // Send sigterm top
        if (api_->kill(sertop_instance_, SIGTERM) < 0)
        {
            logger_->error("unable to terminate sertop instance (error code: {})", errno);
        }
    }

    // Close remaining open file descriptors.
    api_->close(interrupt_fd_[0]);
    api_->close(interrupt_fd_[1]);
}

void worker::fail(const wpwrapper::api_error& error)
{
    logger_->error("aborting");
    // Notify worker threads.
    {
        std::lock_guard<std::mutex> guard(message_queue_mutex_);
        running_ = false;
    }
    cv_.notify_one();

    // Closing the write end of the interrupt pipe will cause an POLLUP event in the read thread.
    api_->close(interrupt_fd_[1]);

    // Notify subscribers.
    for (const auto& callback: on_failure_)
    {
        callback(id_, error);
    }
}

void worker::write(const std::string& s)
{
    auto data = s.c_str();
    ssize_t written;

    ssize_t to_write = s.length();
    while (to_write > 0)
    {
        written = api_->write(stdin_fd_[1], data, to_write);

        if (written < 0)
        {
            throw api_error("unable to write to sertop", errno);
        }

        to_write -= written;
        // Move pointer to point at next character to write.
        data += written;
    }
}

void worker::read_loop() noexcept
{
    logger_->debug("started read loop");

    std::vector<char> buffer(4096);
    std::string remainder;

    struct pollfd fds[2];
    int result;
    int read;

    // Listen for hangups on the interrupt pipe. This happens by default, the events value is ignored.
    fds[0].fd = interrupt_fd_[0];
    fds[0].events = POLLIN | POLLHUP | POLLERR;

    // Listen for readability on the stdout pipe.
    fds[1].fd = stdout_fd_[0];
    fds[1].events = POLLIN;

    while (running_)
    {
        result = api_->poll(fds, 2, -1);
        if (result < 0)
        {
            fail(api_error("unable to poll pipes", errno, logger_));
            break;
        }
        else if (result == 0)
        {
            // Spurious wakeup, move to next iteration.
            continue;
        }

        if (fds[0].revents & (POLLIN | POLLHUP | POLLERR))
        {
            // Hangup occurred on interrupt pipe, write end has been closed.
            logger_->debug("received interrupt");
            break;
        }

        if (fds[1].revents & POLLIN)
        {
            // There is data to be read on the stdout pipe.

            // Read from the stdout pipe.
            read = api_->read(stdout_fd_[0], buffer.data(), buffer.size());

            if (read < 0)
            {
                fail(api_error("unable to read from sertop", errno, logger_));
                break;
            }

            // Read message strings from the buffer.
            remainder = parse(buffer, read, remainder);
            buffers::clear(buffer);
        }

        if ((fds[0].revents | fds[1].revents) & POLLERR)
        {
            fail(api_error("unknown error occurred in pipes", 0, logger_));
            break;
        }
    }

    logger_->debug("stopped read loop");
}

} // namespace wpwrapper
