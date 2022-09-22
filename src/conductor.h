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

#ifndef WPWRAPPER_CONDUCTOR_H
#define WPWRAPPER_CONDUCTOR_H

#include <spdlog/logger.h>

#include "sertop/worker.h"
#include "waterproof/server.h"

#ifdef WPWRAPPER_WIN

#include "win/api_wrapper.h"

#elif WPWRAPPER_POSIX

#include "posix/api_wrapper.h"

#endif

namespace wpwrapper {

/// \brief A conductor manages interaction between a server and all associated workers.
class conductor {

public:

    /// \brief
    conductor();

    ~conductor();

    void notify();

    bool has_failed() const noexcept;

private:
    std::shared_ptr<spdlog::logger> logger_;

    uint64_t next_id_;

    std::atomic<bool> server_failed_;
    std::atomic<bool> signal_received_;

    std::shared_ptr<api_wrapper> api_;

    std::unique_ptr<server> server_;
    std::map<unsigned int, std::unique_ptr<worker>> workers_;

    std::queue<request> in_queue_;
    std::queue<response> out_queue_;

    std::thread run_thread_;

    mutable std::mutex queue_m_;
    std::condition_variable queue_cv_;

    void run();

    void handle_request(const wpwrapper::request& request);

    void handle_response(unsigned int instance_id, const std::string& response);

    void handle_worker_failure(unsigned int instance_id, const api_error& error);

    response create_empty_response(unsigned int instance_id, int priority = 0,
                                   wpwrapper::response::status status = wpwrapper::response::status::success);
};

} // namespace wpwrapper

#endif // WPWRAPPER_CONDUCTOR_H
