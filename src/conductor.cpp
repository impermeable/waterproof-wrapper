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

#include "conductor.h"

#include <atomic>

#include <spdlog/spdlog.h>

#include "utils/config.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace wpwrapper {

conductor::conductor()
        :next_id_(0), server_failed_(false), signal_received_(false)
{
    logger_ = spdlog::get("main")->clone("conductor");

    api_ = std::make_shared<api_wrapper>();

    server::failure_callback on_failure = [&](const api_error& error)
    {
        {
            std::lock_guard<std::mutex> guard(queue_m_);
            server_failed_ = true;
        }
        queue_cv_.notify_one();
    };

    server::request_callback on_request = [&](const request& request)
    {
        {
            std::lock_guard<std::mutex> guard(queue_m_);
            in_queue_.push(request);
        }
        queue_cv_.notify_one();
    };

    server::invalidate_callback on_invalidate = [&](unsigned int id)
    {
        {
            std::lock_guard<std::mutex> guard(queue_m_);
            workers_.erase(id);
            logger_->debug("destroyed worker {}", id);
        }
        queue_cv_.notify_one();
    };

    server_ = std::make_unique<server>(config::port, api_,
            std::vector<server::failure_callback>{on_failure},
            std::vector<server::request_callback>{on_request},
            std::vector<server::invalidate_callback>{on_invalidate});

    run_thread_ = std::thread(&conductor::run, this);
}

conductor::~conductor()
{
    if (!signal_received_ && !server_failed_)
    {
        signal_received_ = true;
        queue_cv_.notify_all();
    }

    if (run_thread_.joinable())
    {
        run_thread_.join();
    }
}

void conductor::notify()
{
    signal_received_ = true;

    queue_cv_.notify_all();
}

bool conductor::has_failed() const noexcept
{
    return server_failed_ || signal_received_;
}

void conductor::run()
{
    logger_->debug("started");

    while (!signal_received_ && !server_failed_)
    {
        std::unique_lock<std::mutex> lock(queue_m_);

        queue_cv_.wait_for(lock, std::chrono::milliseconds(500), [&]
        {
            return !in_queue_.empty() || !out_queue_.empty() || server_failed_ || signal_received_;
        });

        if (server_failed_)
        {
            logger_->debug("server failed, stopping");
            break;
        }

        if (signal_received_)
        {
            logger_->debug("flag set, stopping");
            break;
        }

        while (!in_queue_.empty())
        {
            // Potentially expensive, so done on this thread instead of the callback thread
            handle_request(in_queue_.front());
            in_queue_.pop();
        }

        if (signal_received_)
        {
            logger_->debug("flag set, stopping");
            break;
        }

        while (!out_queue_.empty())
        {
            server_->enqueue(out_queue_.front());
            out_queue_.pop();
        }

        lock.unlock();
    }

    logger_->debug("stopped");
}

void conductor::handle_response(unsigned int instance_id, const std::string& response)
{
    {
        std::lock_guard<std::mutex> guard(queue_m_);

        wpwrapper::response rsp = create_empty_response(instance_id);
        rsp.content_ = response;
        rsp.verb_ = request::verb::forward;

        out_queue_.push(rsp);
    }
    queue_cv_.notify_all();
}

void conductor::handle_request(const wpwrapper::request& request)
{
    switch (request.verb_)
    {
    case request::verb::create:
    { // Open a new scope here because we declare variables.
        response response = create_empty_response(request.instance_id_, 1);
        response.verb_ = request::verb::create;

        auto on_response = std::bind(&conductor::handle_response, this, std::placeholders::_1, std::placeholders::_2);
        auto on_failure = std::bind(&conductor::handle_worker_failure, this, std::placeholders::_1,
                std::placeholders::_2);

        try
        {
            config conf(request.content_);
            logger_->info("start sertop at: {}", conf.sertop_path);
            auto w = std::make_unique<worker>(request.instance_id_,
                    conf.sertop_path,
                    conf.sertop_args, api_,
                    std::vector<worker::failure_callback>{on_failure},
                    std::vector<worker::response_callback>{on_response});
            workers_.insert(std::make_pair(request.instance_id_, std::move(w)));
            response.status_ = response::status::success;
        }
        catch (const api_error& e)
        {
            response.status_ = response::status::failure;
            response.content_ = e.what();
        }

        logger_->debug("created worker {}", request.instance_id_);

        out_queue_.push(response);
        break;
    }
    case request::verb::destroy:
    { // Open a new scope here because we declare variables.
        workers_.erase(request.instance_id_);
        logger_->debug("destroyed worker {}", request.instance_id_);

        response response = create_empty_response(request.instance_id_, 1);
        response.verb_ = request::verb::destroy;
        response.content_ = "";
        server_->unmap(request.instance_id_, response);
        break;
    }
    case request::verb::forward:
        workers_[request.instance_id_]->enqueue(request.content_);
        break;
    case request::verb::stop:
        logger_->debug("received stop signal");
        signal_received_ = true;
        break;
    }
}

void conductor::handle_worker_failure(unsigned int instance_id, const wpwrapper::api_error& error)
{
    std::lock_guard<std::mutex> guard(queue_m_);

    // Fatal error occurred, delete worker and inform Waterproof.
    workers_.erase(instance_id);

    response response = create_empty_response(instance_id, 1, wpwrapper::response::status::failure);
    response.verb_ = request::verb::destroy;
    response.content_ = error.what();

    out_queue_.push(response);
}

response conductor::create_empty_response(
        const unsigned int instance_id, const int priority,
        const wpwrapper::response::status status)
{
    // this function is mostly here so the id_ gets handled the same
    response response;
    response.id_ = next_id_++;
    response.priority_ = priority;
    response.instance_id_ = instance_id;
    response.status_ = status;
    return response;
}

} // namespace wpwrapper
