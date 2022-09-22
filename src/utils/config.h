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

#ifndef WPWRAPPER_CONFIG_H
#define WPWRAPPER_CONFIG_H

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace wpwrapper {

class config {

public:

static const uint16_t port;

config(std::string raw_create_options);

std::string sertop_path;

std::vector<std::string> sertop_args;

};

} // namespace wpwrapper::config

#endif // WPWRAPPER_CONFIG_H
