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

namespace wpwrapper {

worker::worker(unsigned int id, const std::string& sertop_path, const std::vector<std::string>& sertop_args,
        std::shared_ptr<wpwrapper::api> api_instance,
        std::vector<wpwrapper::worker::failure_callback> failure_callbacks,
        std::vector<wpwrapper::worker::response_callback> response_callbacks)
        :id_(id), running_(false), api_(std::move(api_instance)),
         on_failure_(std::move(failure_callbacks)), on_response_(std::move(response_callbacks))
{
    logger_ = spdlog::get("main")->clone(fmt::format("worker{}", id));

    // Pipe name should be unique, so contains the process id and worker id.
    auto name = fmt::format(R"(\\.\Pipe\WaterproofWrapper.{}.{})", GetCurrentProcessId(), id);

    // Create this worker's end of the pipe. This pipe is duplex and in overlapped mode: it supports asynchronous reads
    // and writes on the same handle.
    pipe_worker_end_ = api_->CreateNamedPipeA(name.c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS, 1, 0, 0, 0, nullptr);

    if (INVALID_HANDLE_VALUE == pipe_worker_end_)
    {
        throw api_error("failed to create worker end of pipe", api_->GetLastError(), logger_);
    }

    // Sertop's end of the pipe needs to be inheritable by the sertop process.
    SECURITY_ATTRIBUTES sa = {sizeof(sa), nullptr, TRUE};
    pipe_sertop_end_ = api_->CreateFileA(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, &sa, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, nullptr);

    if (INVALID_HANDLE_VALUE == pipe_sertop_end_)
    {
        DWORD error = api_->GetLastError();
        api_->CloseHandle(pipe_worker_end_);
        throw api_error("failed to create sertop end of pipe", error, logger_);
    }

    // Concatenate all arguments into a single string.
    std::string command;
    for (const auto& arg: sertop_args)
    {
        command += ' ' + arg;
    }

    // Always require sertop to end responses with the null terminator
    command += " --print0";

    logger_->debug("starting sertop instance with arguments{}", command);

    command = sertop_path + command;

    // Configure sertop process to inherit the pipe_sertop_end_ handle and use it as its standard handles.
    STARTUPINFO si = {sizeof(si)};
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = pipe_sertop_end_;
    si.hStdOutput = pipe_sertop_end_;
    si.hStdError = pipe_sertop_end_;

    PROCESS_INFORMATION pi = {};

    // Start a sertop instance as a windowless child process.
    if (!api_->CreateProcessA(nullptr, const_cast<LPSTR>(command.c_str()), nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
            nullptr, nullptr, &si, &pi))
    {
        DWORD error = api_->GetLastError();
        close_all(std::vector<HANDLE>{pipe_sertop_end_, pipe_worker_end_});
        throw api_error("unable to create sertop process", error, logger_);
    }

    logger_->debug("created sertop process with id {}", pi.dwProcessId);
    api_->CloseHandle(pi.hThread);
    sertop_instance_ = pi.hProcess;

    // Create events and overlapped structures used to synchronize the pipe communication. The events all need to be
    // reset manually.
    interrupt_event_ = api_->CreateEventA(nullptr, TRUE, FALSE, nullptr);
    if (nullptr == interrupt_event_)
    {
        DWORD error = api_->GetLastError();
        close_all(std::vector<HANDLE>{pipe_sertop_end_, pipe_worker_end_, pi.hThread, pi.hProcess});
        throw api_error("unable to create interrupt event", error, logger_);
    }

    read_event_ = api_->CreateEventA(nullptr, TRUE, FALSE, nullptr);
    if (nullptr == read_event_)
    {
        DWORD error = api_->GetLastError();
        close_all(std::vector<HANDLE>{pipe_sertop_end_, pipe_worker_end_, pi.hThread, pi.hProcess, interrupt_event_});
        throw api_error("unable to create read event", error, logger_);
    }

    write_event_ = api_->CreateEventA(nullptr, TRUE, FALSE, nullptr);
    if (nullptr == write_event_)
    {
        DWORD error = api_->GetLastError();
        close_all(std::vector<HANDLE>{pipe_sertop_end_, pipe_worker_end_, pi.hThread, pi.hProcess, interrupt_event_,
                                      read_event_});
        throw api_error("unable to create write event", error, logger_);
    }

    read_overlapped_ = {};
    read_overlapped_.hEvent = read_event_;
    write_overlapped_ = {};
    write_overlapped_.hEvent = write_event_;

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

        api_->SetEvent(interrupt_event_);
        api_->CancelIoEx(pipe_worker_end_, nullptr);
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

    // Close pipe handles. This will cause the sertop process to shut down gracefully.
    api_->CloseHandle(pipe_sertop_end_);
    api_->CloseHandle(pipe_worker_end_);

    // Verify that the sertop process has shut down. If it is still open after 500ms, terminate it.
    bool should_terminate = false;
    switch (api_->WaitForSingleObject(sertop_instance_, 500))
    {
    case WAIT_OBJECT_0:
        logger_->debug("sertop process shut down gracefully");
        break;
    case WAIT_TIMEOUT:
        logger_->warn("timeout while waiting for sertop process shutdown");
        should_terminate = true;
        break;
    case WAIT_FAILED:
        logger_->error("unable to wait for sertop process shutdown (error code: {})", api_->GetLastError());
        should_terminate = true;
        break;
    }

    if (should_terminate)
    {
        logger_->debug("terminating sertop process");
        if (!api_->TerminateProcess(sertop_instance_, 0))
        {
            logger_->error("unable to terminate sertop process (error code: {})", api_->GetLastError());
        }
    }

    // Close remaining open handles.
    api_->CloseHandle(sertop_instance_);
}

void worker::close_all(const std::vector<HANDLE>& handles) noexcept
{
    for (const auto& handle: handles)
    {
        if (nullptr != handle)
        {
            api_->CloseHandle(handle);
        }
    }
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

    api_->SetEvent(interrupt_event_);
    api_->CancelIoEx(pipe_worker_end_, nullptr);

    // Notify subscribers.
    for (const auto& callback: on_failure_)
    {
        callback(id_, error);
    }
}

void worker::write(const std::string& s)
{
    auto data = s.c_str();
    DWORD error;
    DWORD written;

    int to_write = s.length();
    while (to_write > 0)
    {
        // Start a new write operation.
        if (!api_->WriteFile(pipe_worker_end_, LPVOID(data), to_write, nullptr, &write_overlapped_)
                && (error = api_->GetLastError()) != ERROR_IO_PENDING)
        {
            throw api_error("could not start write to sertop", error);
        }

        // Block until write operation has finished.
        if (!api_->GetOverlappedResult(pipe_worker_end_, &write_overlapped_, &written, TRUE))
        {
            throw api_error("could not finish write to sertop", api_->GetLastError());
        }

        to_write -= written;
        // Increase pointer to point at next character to write.
        data += written;
    }
}

void worker::read_loop() noexcept
{
    logger_->debug("started read loop");

    std::vector<char> buffer(4096);
    std::string remainder;

    DWORD error;
    DWORD read;
    DWORD result;

    // This order is important: if both become signalled, WaitForMultipleObjects returns the index of the first handle
    // in the array whose object was signalled.
    HANDLE events[2] = {interrupt_event_, read_event_};

    while (running_)
    {
        // Begin a new asynchronous read operation.
        if (!api_->ReadFile(pipe_worker_end_, buffer.data(), buffer.size(), nullptr, &read_overlapped_)
                && (error = api_->GetLastError()) != ERROR_IO_PENDING)
        {
            fail(api_error("unable to start reading from sertop", error, logger_));
        }

        // Wait until the interrupt signal is set or until the read operation has finished.
        result = api_->WaitForMultipleObjects(2, events, FALSE, INFINITE);

        if (result == WAIT_FAILED)
        {
            fail(api_error("unable to wait on read and interrupt events", api_->GetLastError(), logger_));
            break;
        }

        if (result == WAIT_OBJECT_0)
        {
            // Interrupt event was signalled.
            logger_->debug("received interrupt event");
            break;
        }
        else if (result == WAIT_OBJECT_0 + 1)
        {
            // Read event was signalled.
            if (!api_->GetOverlappedResult(pipe_worker_end_, &read_overlapped_, &read, TRUE))
            {
                fail(api_error("unable to finish reading from sertop", api_->GetLastError(), logger_));
                break;
            }

            // Read message strings from the buffer.
            remainder = parse(buffer, read, remainder);

            // Reset the read signal.
            if (!api_->ResetEvent(read_event_))
            {
                fail(api_error("unable to reset read event", api_->GetLastError(), logger_));
                break;
            }
        }
    }

    logger_->debug("stopped read loop");
}

} // namespace wpwrapper
