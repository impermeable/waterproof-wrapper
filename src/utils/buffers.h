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

#ifndef WPWRAPPER_BUFFERS_H
#define WPWRAPPER_BUFFERS_H

#include <cstdint>
#include <vector>

namespace wpwrapper::buffers {

/// \brief Indicates the endianness of all scalar types.
/// \see https://en.cppreference.com/w/cpp/types/endian
enum class endianness {
#ifdef WPWRAPPER_WIN
    little = 0,
    big = 1,
    native = little
#else
    little = __ORDER_LITTLE_ENDIAN__,
    big = __ORDER_BIG_ENDIAN__,
    native = __BYTE_ORDER__
#endif
};

/// \brief Zeroes out a buffer.
/// \param buffer The buffer to zero out.
void clear(std::vector<char>& buffer) noexcept;

/// \brief Reads four subsequent bytes with endianness \c source_endianness from a vector \c buffer, starting with the
/// element at position \c offset, into a 32-bit unsigned integer.
/// \param buffer The buffer the read from.
/// \param source_endianness The endianness of the buffer.
/// \param offset The index of the first buffer element to read from. Defaults to 0.
/// \return The value stored in the four elements specified.
uint32_t read_uint32(const std::vector<char>& buffer, endianness source_endianness, int offset = 0) noexcept;

/// \brief Writes an unsigned 32-bit integer \c value to four elements of a buffer \c buffer with endianness
/// \c target_endianness, starting at the element with index \c offset.
/// \param value The value to write.
/// \param buffer The buffer to write to.
/// \param target_endianness The endianness of the buffer.
/// \param offset The index of the first buffer element to write to. Defaults to 0.
void write_uint32(uint32_t value, std::vector<char>& buffer, endianness target_endianness, int offset = 0) noexcept;

} // namespace wpwarpper::buffers

#endif // WPWRAPPER_BUFFERS_H
