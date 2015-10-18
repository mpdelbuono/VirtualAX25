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
 * @file Driver.h
 * Declares global-level driver functions, including the driver entry point.
 * @author Matthew P. Del Buono (KG7UDH)
 */

#define INITGUID
#include "pch.h"


#include "trace.h"

EXTERN_C_START

//
// WDFDRIVER Events
//

extern "C" NDIS_STATUS DriverEntry(_In_ PDRIVER_OBJECT, _In_ PUNICODE_STRING);

EXTERN_C_END

#define DRIVER_MAJOR_VERSION 0
#define DRIVER_MINOR_VERSION 1