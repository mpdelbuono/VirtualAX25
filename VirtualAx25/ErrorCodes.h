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
 * @file ErrorCodes.h
 * Definition of globally-used error codes for the AX.25 NDIS Driver
 * @author Matthew P. Del Buono (KG7UDH)
 */

#pragma once

static constexpr ULONG AX25_CRITICAL_ERROR_NDIS_LEAK_BUFFER_LISTS = 0x00001000; //<! Error code for indicating NDIS leaked memory by passing a bad context to MiniportReceiveNetBufferLists