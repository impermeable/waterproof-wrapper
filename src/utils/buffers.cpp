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

#include "buffers.h"

#include <cstring>

namespace wpwrapper::buffers {

void clear(std::vector<char>& buffer) noexcept
{
    std::fill(buffer.begin(), buffer.end(), 0);
}

uint32_t read_uint32(const std::vector<char>& buffer, endianness source_endianness, int offset) noexcept
{
    uint32_t result = 0;

    if (source_endianness == endianness::big)
    {
        // Big endian, most significant bits first.
        result |= static_cast<uint8_t>(buffer[offset]) << 24u;
        result |= static_cast<uint8_t>(buffer[offset + 1]) << 16u;
        result |= static_cast<uint8_t>(buffer[offset + 2]) << 8u;
        result |= static_cast<uint8_t>(buffer[offset + 3]);
    }
    else
    {
        // Little endian, least significant bits first.
        result |= static_cast<uint8_t>(buffer[offset]);
        result |= static_cast<uint8_t>(buffer[offset + 1]) << 8u;
        result |= static_cast<uint8_t>(buffer[offset + 2]) << 16u;
        result |= static_cast<uint8_t>(buffer[offset + 3]) << 24u;
    }

    return result;
}

void write_uint32(uint32_t value, std::vector<char>& buffer, endianness target_endianness, int offset) noexcept
{
    if (target_endianness == endianness::big)
    {
        // Big endian, most significant bits first.
        buffer[offset] = (value >> 24u);
        buffer[offset + 1] = ((value >> 16u) & 0xFFu);
        buffer[offset + 2] = ((value >> 8u) & 0xFFu);
        buffer[offset + 3] = (value & 0xFFu);
    }
    else
    {
        // Little endian, least significant bits first.
        buffer[offset] = (value & 0xFFu);
        buffer[offset + 1] = ((value >> 8u) & 0xFFu);
        buffer[offset + 2] = ((value >> 16u) & 0xFFu);
        buffer[offset + 3] = (value >> 24u);
    }
}

} // namespace wpwarpper::buffer
