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

#ifndef WPWRAPPER_WORKER_H
#define WPWRAPPER_WORKER_H

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include <spdlog/spdlog.h>

#include "../utils/exceptions.h"

#ifdef WPWRAPPER_WIN

#include "../win/api.h"

#elif WPWRAPPER_POSIX

#include "../posix/api.h"

#endif

namespace wpwrapper {

/// \brief A worker starts, stops, reads from and writes to a sertop instance.
class worker {
public:
    /// \brief A failure callback takes the notifying worker's id and the error that lead to failure as arguments.
    using failure_callback = std::function<void(unsigned int, const api_error&)>;
    /// \brief A response callback takes the notifying worker's is and the received message as argument.
    using response_callback = std::function<void(unsigned int, const std::string&)>;

    /// \brief Constructs a worker with an unique identifier \c id.
    /// \details A child process will be created, running a binary \c sertop_path with arguments \c sertop_args.
    /// Subsequently, two worker threads will be started. If an error occurs in one of those worker threads, the
    /// \c failure_callbacks will be called. If this worker receives a message from sertop, the \c response_callbacks
    /// will be called.
    /// \param id An unique identifier for this worker.
    /// \param sertop_path The path where the sertop binary is located.
    /// \param sertop_args A list of arguments to pass to the sertop binary.
    /// \param api_instance The API instance to use.
    /// \param failure_callbacks A list of callbacks to execute when an error occurs in any of the worker threads.
    /// \param response_callbacks A list of callbacks to execute when a message is received from sertop.
    /// \throw api_error If the child process, or the pipe to it, could not be created.
    worker(unsigned int id, const std::string& sertop_path, const std::vector<std::string>& sertop_args,
            std::shared_ptr<api> api_instance, std::vector<failure_callback> failure_callbacks,
            std::vector<response_callback> response_callbacks);

    /// \brief Destructs this worker.
    /// \details Stops the worker threads. Attempts to gracefully close the sertop process. If that fails, the process
    /// will be terminated.
    ~worker() noexcept;

    // Worker is non-copyable.
    worker(const worker& other) = delete;

    // Worker is non-movable.
    worker(worker&& other) = delete;

    // Worker is non-copyable.
    worker& operator=(const worker& other) = delete;

    // Worker is non-movable.
    worker& operator=(worker&& other) = delete;

    /// \brief Add a message to be sent to the sertop instance.
    /// \param message The message to add.
    void enqueue(const std::string& message);

    /// \brief Interrupt the worker by sending a SIGINT/CTRL^C signal
    void interrupt();

private:

#ifdef WPWRAPPER_WIN

    /// \brief Closes all HANDLEs in \c handles.
    /// \param handles The HANDLEs to close.
    void close_all(const std::vector<HANDLE>& handles) noexcept;

#endif

    /// \brief Executes on_failure callbacks and stops all worker threads.
    /// \param error The error that lead to failure.
    void fail(const api_error& error);

    /// \brief Parses a \c buffer containing \c read chars into a list of received messages, and executes the
    /// on_response callbacks on them. The first message will be prefixed with the supplied \c prefix.
    /// \details All messages from sertop are terminated with the null-terminator char. It is possible that the buffer
    /// contains multiple messages after a read operation. These will all be split into separate messages before the
    /// callbacks are executed. It may also be the case that the last (or only) message in the buffer is incomplete.
    /// In this case, the part of this message that is in the buffer will be returned, and should be passed as prefix to
    /// the next call to parse. In this next call to parse, the first chars in the buffer will belong to this incomplete
    /// message, so they can be combined with the prefix to restore the full message.
    /// \param buffer The buffer to parse.
    /// \param read The number of bytes read into the buffer.
    /// \param prefix The first part of a message which was not fully read in the previous parse call.
    /// \return The first part of a not-null-terminated, and hence incomplete, message.
    std::string parse(std::vector<char> buffer, int read, const std::string& prefix);

    /// \brief Writes a string to sertop.
    /// \param s The string to write.
    /// \throw api_error If the string could not be written to sertop.
    void write(const std::string& s);

    /// \brief Performs subsequent reads from sertop.
    /// \note Should be executed on a separate thread.
    void read_loop() noexcept;

    /// \brief Writes messages to sertop whenever they become available.
    /// \note Should be executed on a separate thread.
    void write_loop() noexcept;

    /// \brief An unique identifier for this worker.
    unsigned int id_;
    /// \brief \c true if the worker threads should be running, \c false if they should not be.
    std::atomic<bool> running_;

    /// \brief Logger used in this worker.
    std::shared_ptr<spdlog::logger> logger_;
    /// \brief API instance used for all API calls.
    std::shared_ptr<api> api_;

    /// \brief List of callbacks that are executed when an error occurs in one of the worker threads.
    std::vector<failure_callback> on_failure_;
    /// \brief List of callbacks that are executed when a message is received from sertop.
    std::vector<response_callback> on_response_;

    /// \brief FIFO queue containing all messages that have been added but not sent.
    std::queue<std::string> message_queue_;
    /// \brief Guards the message queue.
    mutable std::mutex message_queue_mutex_;

    /// \brief Notified whenever a new message is added to the queue or whenever the worker needs to stop.
    std::condition_variable cv_;

    /// \brief Thread on which the read loop is executed.
    std::thread read_thread_;
    /// \brief Thread on which the write loop is executed.
    std::thread write_thread_;

#ifdef WPWRAPPER_WIN
    /// \brief Handle to sertop's end of the pipe.
    HANDLE pipe_worker_end_;
    /// \brief Handle to the worker's end of the pipe.
    HANDLE pipe_sertop_end_;
    /// \brief Handle to the sertop process.
    HANDLE sertop_instance_;
    /// \brief Process id of sertop process
    DWORD sertop_pid_;

    /// \brief Event that is set when the read loop needs to be interrupted.
    HANDLE interrupt_event_;
    /// \brief Event that is set when a read operations has finished.
    HANDLE read_event_;
    /// \brief Event that is set when a write operation has finished.
    HANDLE write_event_;

    /// \brief Used to synchronize read operations.
    OVERLAPPED read_overlapped_;
    /// \brief Used to synchronize write operations.
    OVERLAPPED write_overlapped_;
#elif WPWRAPPER_POSIX
    /// \brief File descriptors for the read and write ends of the pipe to sertop.
    int stdin_fd_[2];
    /// \brief File descriptors for the read and write ends of the pipe from sertop.
    int stdout_fd_[2];
    /// \brief File descriptors for the read and write ends of the interrupt pipe.
    int interrupt_fd_[2];
    /// \brief Sertop process id.
    pid_t sertop_instance_;
#endif
};

} // namespace wpwrapper

#endif // WPWRAPPER_WORKER_H
