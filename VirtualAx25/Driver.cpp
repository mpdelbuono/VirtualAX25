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

/*++

Module Name:

    driver.c

Abstract:

    This file contains the driver entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/

#include "pch.h"
#include "driver.h"
#include "driver.tmh"
#include "Miniport.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#endif

Miniport *pMiniportObject = nullptr;



/**
 * Entry point for the driver from the KMDF framework.
 * 
 * This is the first function called by the OS. This function needs to set up
 * the various callbacks and set up the NDIS miniport.
 * @param driverObject the DRIVER_OBJECT structure representing this driver
 * @param registryPath the path to the driver's registry key. Note that this
 * parameter is not unique to a specific hardware instance.
 * @returns STATUS_SUCCESS if the driver was initialized successfully, or
 * an error code otherwise
 * @IRQL PASSIVE_LEVEL
 */
extern "C" 
NDIS_STATUS DriverEntry(
    _In_ PDRIVER_OBJECT  driverObject,
    _In_ PUNICODE_STRING registryPath
    )
{
    //
    // Initialize WPP Tracing
    //
    WPP_INIT_TRACING( driverObject, registryPath );
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    // Create the NDIS driver context object, which also will provide the various NDIS handlers
    Miniport* miniport = new Miniport();
    if (miniport == NULL)
    {
        TraceEvents(TRACE_LEVEL_CRITICAL, TRACE_DRIVER, "Failed to allocate a miniport object! Failing driver entry.");
        return STATUS_NO_MEMORY;
    }
    

    // Register with NDIS
    NDIS_STATUS result = miniport->RegisterWithNdis(driverObject, registryPath);
    if (NT_SUCCESS(result) == false)
    {
        // Error registering with NDIS.
        TraceEvents(TRACE_LEVEL_CRITICAL, TRACE_DRIVER, "NDIS registration failed with status %!STATUS!", result);
        WPP_CLEANUP(driverObject);
        return result;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
    return result;
}
