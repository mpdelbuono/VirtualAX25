// This file is part of the KG7UDH Virtual AX.25 NDIS Driver.
//
// The KG7UDH Virtual AX.25 NDIS Driver is free software: 
// you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The KG7UDH Virtual AX.25 NDIS Driver is distributed in 
// the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this software. If not, see <http://www.gnu.org/licenses/>.

/**
 * @file Utility.h
 * Contains a collection of utility templates and macros which are useful
 * throughout the project for ease of writing compile-time constants
 * and behaviors.
 */
#pragma once

/**
 * Creates a 4-byte tag out of the specified null-terminated string.
 * The tag will be stored in memory in big-endian order so that when
 * looking at memory byte-by-byte the tag will be easily visible.
 * @param tag a 4-character string to use as the tag, followed by
 * the null terminator
 * @returns the 4-byte integer representing the specified tag
 */
constexpr unsigned long AX25_CREATE_TAG(char const tag[5])
{
    return (tag[0] << 0)  |
           (tag[1] << 8)  |
           (tag[2] << 16) |
           (tag[3] << 24);
}

/** Specifies a function that should be placed in non-pageable memory */
#define NON_PAGEABLE_FUNCTION __declspec(code_seg(".text"))

/** Specifies a function that can safely be placed in pageable memory */
#define PAGEABLE_FUNCTION _IRQL_requires_(PASSIVE_LEVEL) __declspec(code_seg("PAGE"))
