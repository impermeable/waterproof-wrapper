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

#include "message.h"

namespace wpwrapper {

bool response::operator<(const wpwrapper::response& rhs) const
{
    // The idea is to allow for 'emergency' responses with higher priority, but still ensure that responses from sertop
    // will be sent to Waterproof in the order in which they are received.
    //
    // We define a weak ordering on the set of responses as follows:
    //      We say request A < request B if:
    //      * A has lower priority than B, or,
    //      * A and B have equal priority and B has lower id than B (A has higher id than B).
    //
    // If we store responses in some max-heap, we will obtain our desired properties: 'emergency' responses have higher
    // priority so they are always at (near) the top of the heap, and as responses get ids in the order in which
    // they arrive, responses with lower ids will be nearer to the top as well.
    return std::tie(priority_, rhs.id_) < std::tie(rhs.priority_, id_);
}

bool response::operator>(const wpwrapper::response& rhs) const
{
    return rhs < *this;
}

void from_json(const json& j, request& r)
{
    j.at("verb").get_to(r.verb_);
    j.at("instance_id").get_to(r.instance_id_);
    j.at("content").get_to(r.content_);
}

void from_json(const json& j, response& r)
{
    j.at("status").get_to(r.status_);
    j.at("verb").get_to(r.verb_);
    j.at("instance_id").get_to(r.instance_id_);
    j.at("content").get_to(r.content_);
}

void to_json(json& j, const request& r)
{
    j = json{
            {"verb",        r.verb_},
            {"instance_id", r.instance_id_},
            {"content",     r.content_},
    };
}

void to_json(json& j, const response& r)
{
    // The id and priority are for internal use only, and are not sent back.
    j = json{
            {"status",      r.status_},
            {"verb",        r.verb_},
            {"instance_id", r.instance_id_},
            {"content",     r.content_},
    };
}

} // namespace wpwrapper
