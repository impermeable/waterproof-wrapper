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

#ifndef WPWRAPPER_MESSAGE_H
#define WPWRAPPER_MESSAGE_H

#include <string>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace wpwrapper {

/// \brief A request received from Waterproof.
struct request {
    /// \brief An action that can be performed by the wrapper.
    enum class verb {
        /// \brief Create a new worker.
                create,
        /// \brief Destroy a worker.
                destroy,
        /// \brief Forward the request content to the worker.
                forward,
      /// \brief Interrupt the worker.
                interrupt,
        /// \brief Stop the wrapper.
                stop
    };

    /// \brief The action that should be performed by the wrapper.
    verb verb_;

    /// \brief The identifier of the worker which should be destroyed or to which the request content should be
    /// forwarded. Ignored in create and stop requests.
    unsigned int instance_id_;

    /// \brief The request content. In forward requests, the content is what will be forwarded to the worker. Ignored in
    /// all other requests.
    std::string content_;
};

/// \brief A response sent back to Waterproof.
struct response {
    /// \brief Indicates if the request was executing successfully or not.
    enum class status {
        /// \brief An error occurred while executing the request.
                failure,
        /// \brief The request was executing successfully.
                success
    };

    /// \brief Unique identifier for this response.
    /// \note For internal use only, is not (de)serialized.
    uint64_t id_;

    /// \brief Indicates this response's priority. The server will send responses with higher priority first.
    /// \note For internal use only, is not (de)serialized.
    int priority_;

    /// \brief Indicates if the request to which this response corresponds was executed successfully.
    /// \warning Does not say anything about what a sertop instance thinks about forwarded content. I.e. even if a
    /// request content contains syntax errors, the response status will be success if the request was delivered to a
    /// sertop instance successfully.
    status status_;

    /// \brief The verb of the request to which this response corresponds.
    request::verb verb_;

    /// \brief Identifies the worker that executed the request to which this response corresponds.
    unsigned int instance_id_;

    /// \brief The response content.
    /// \details In failure responses, this will contain some error message. In success responses, this will contain
    /// sertop's responses for forward requests and be empty for other requests.
    std::string content_;

    /// \brief Defines a weak ordering on the set of responses.
    /// \details  We say that a request A is smaller than some other request B if A has lower priority than B or if A
    /// and B have equal priority and A has a higher id than B.
    /// \param rhs The request to compare to.
    /// \return \c true if \c this is 'smaller' than \c rhs, \c false if otherwise.
    bool operator<(const response& rhs) const;

    /// \brief Equivalent to \c rhs less than \c this.
    /// \param rhs The request to comare to.
    /// \return \c true if \c this is 'larger' than \c rhs, \c false if otherwise.
    bool operator>(const response& rhs) const;
};

// Define how a request::verb enum should be (de)serialized.
NLOHMANN_JSON_SERIALIZE_ENUM(request::verb, {
    { request::verb::create, "create" },
    { request::verb::destroy, "destroy" },
    { request::verb::forward, "forward" },
    { request::verb::interrupt, "interrupt" },
    { request::verb::stop, "stop" },
})

// Define how a response::status enum should be (de)serialized.
NLOHMANN_JSON_SERIALIZE_ENUM(response::status, {
    { response::status::failure, "failure" },
    { response::status::success, "success" },
})

/// \brief Deserializes a JSON object \c j into a request \c r.
/// \param j The JSON object to deserialize.
/// \param r The request to deserialize into.
void from_json(const json& j, request& r);

/// \brief Deserializes a JSON object \c j into a response \c r.
/// \param j The JSON object to deserialize.
/// \param r The response to deserialize into.
void from_json(const json& j, response& r);

/// \brief Serializes a request \c r into a JSON object \c j.
/// \param j The JSON object to serialize into.
/// \param r The request to serialize.
void to_json(json& j, const request& r);

/// \brief Serializes a response \c r into a JSON object \c j.
/// \param j The JSON object to serialize into.
/// \param r The response to serialize.
void to_json(json& j, const response& r);

} // namespace wpwrapper

#endif // WPWRAPPER_MESSAGE_H
