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

#include <optional>

namespace wpwrapper {

void worker::enqueue(const std::string& message)
{
    {
        std::lock_guard<std::mutex> guard(message_queue_mutex_);
        message_queue_.push(message);
    }
    cv_.notify_one();
}

std::string worker::parse(std::vector<char> buffer, int read, const std::string& prefix)
{
    std::vector<std::string> result;
    std::string raw_result = prefix;

    // Read buffer into a raw string clear it.
    raw_result.append(buffer.data(), read);
    std::fill(buffer.begin(), buffer.end(), 0);

    // Fixes search issue
    read += prefix.length();

    // Every response is terminated with a null-terminator, so we can count the number of full messages in the buffer.
    int count = std::count(raw_result.begin(), raw_result.end(), '\0');

    if (count > 0)
    {
        // Worker read at least one full message. We will now read all full messages into the result vector.
        const char* ptr = raw_result.c_str();
        for (int i = 0; i < count; ++i)
        {
            result.emplace_back(std::string(ptr));

            // Move the pointer to the start of the next message.
            ptr += result.back().size() + 1;
            read -= result.back().size() + 1;
        }

        for (const auto& response: result)
        {
            for (const auto& callback: on_response_)
            {
                callback(id_, response);
            }
        }

        // Return the remaining chars, which will be prefixed the next time parse() is called.
        return read == 0 ? "" : std::string(ptr, read);
    }
    else
    {
        // Worker did not read any full messages. This happens if the message is longer than the buffer. We return
        // the full buffer as a prefix for the next parse() call.
        return raw_result;
    }
}

void worker::write_loop() noexcept
{
    logger_->debug("started write loop");

    while (running_)
    {
        std::optional<std::unique_lock<std::mutex>> lock;

        try
        {
            lock.emplace(message_queue_mutex_);
        }
        catch (const std::system_error& e)
        {
            fail(api_error("failed to acquire lock", 0, logger_));
            break;
        }

        // Wait until a new message can be written or we're told to stop.
        cv_.wait(*lock, [&message_queue = message_queue_, &running = running_]
        {
            return !message_queue.empty() || !running;
        });

        if (!running_)
        {
            break;
        }

        auto message = message_queue_.front();
        message_queue_.pop();

        try
        {
            lock->unlock();
        }
        catch (const std::system_error& e)
        {
            fail(api_error("failed to unlock lock", 0, logger_));
            break;
        }

        try
        {
            write(message);
        }
        catch (const api_error& e)
        {
            logger_->error(e.what());
            fail(e);
            break;
        }
    }

    logger_->debug("stopped write loop");
}

} // namespace wpwrapper
