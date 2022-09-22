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

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include "conductor.h"
#include "utils/config.h"

#ifdef WPWRAPPER_WIN

#include <windows.h>

#elif WPWRAPPER_POSIX

#include <csignal>

#endif

// Guaranteed to be lock-free.
// Volatile to prevent compiler optimization.
volatile std::atomic_flag keep_running = ATOMIC_FLAG_INIT;

#ifdef WPWRAPPER_WIN

BOOL WINAPI console_handler(DWORD signal)
{

    switch (signal)
    {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
        keep_running.clear();
        return TRUE;
    default:
        break;
    }

    return FALSE;
}

#elif WPWRAPPER_POSIX

extern "C" void signal_handler(int signum)
{
    keep_running.clear();
}

#endif

void configure_logger()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);
    console_sink->set_pattern("[%^%8l%$] %n: %v");

    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("wpwrapper.log", 5 * 1024 * 1024, 5, true);
    file_sink->set_level(spdlog::level::trace);

    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};

    auto logger = std::make_shared<spdlog::logger>("main", sinks.begin(), sinks.end());
    logger->flush_on(spdlog::level::debug);
    logger->set_level(spdlog::level::trace);

    spdlog::register_logger(logger);
}

int main(int argc, char** argv)
{
#ifdef WPWRAPPER_WIN

    SetConsoleCtrlHandler(console_handler, TRUE);

#elif WPWRAPPER_POSIX

    struct sigaction action{};
    memset(&action, 0, sizeof action);
    action.sa_handler = signal_handler;

    sigaction(SIGINT, &action, nullptr);
    sigaction(SIGTERM, &action, nullptr);

#endif

    try
    {
        configure_logger();
    }
    catch (const spdlog::spdlog_ex& e)
    {
    }

    spdlog::get("main")->info("Started wpwrapper with {} arguments", argc - 1);

    keep_running.test_and_set();

    std::optional<wpwrapper::conductor> conductor;

    try
    {
        conductor.emplace();
    }
    catch (const wpwrapper::api_error& e)
    {
        return e.error_number_;
    }

    while (keep_running.test_and_set() && !conductor->has_failed())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    conductor->notify();

    if (!conductor->has_failed())
    {
        spdlog::get("main")->info("received SIGINT/SIGTERM");
    }

    spdlog::get("main")->info("Exiting...");

    return 0;
}
