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

#include "config.h"

namespace wpwrapper {

const uint16_t config::port = 51613;

config::config(std::string raw_create_options) {
    #ifdef WPWRAPPER_WIN

    sertop_path = "C:\\ProgramData\\waterproof\\vendor\\opam\\ocaml-variants.4.07.1+mingw64c\\bin\\sertop.exe";

    #elif WPWRAPPER_POSIX

    sertop_path = "/opt/waterproof/vendor/opam/default/bin/sertop";

    #endif

    sertop_args = {"--implicit"};

    // For now if statement to ensure backwards compatibility with earlier waterproof versions
    if(raw_create_options != "") {
        auto j = nlohmann::json::parse(raw_create_options);
        std::string entered_path = j.at("path").get<std::string>();
        if(entered_path != "") {
            sertop_path = entered_path;
        }
        sertop_args = j.at("args").get<std::vector<std::string>>();
    }
}

} // namespace wpwrapper::config