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
* @file AX25Adapter.cpp
* Implementation of the AX25Adapter class, which represents a single virtual adapter presented
* to the operating system.
* @author Matthew P. Del Buono (KG7UDH)
*/

#include "pch.h"
#include "Trace.h"
#include "AX25Adapter.h"
#include "AX25Adapter.tmh"


/**
 * Allocates space for a new AX25Adapter object in non-pageable memory. An NDIS handle
 * must be specified to pass to the allocation function.
 * @param size the size of the memory block to allocate, which must be equal to sizeof(AX25Adapter)
 * @param driverHandle the NDIS miniport handle with which to allocate the memory
 * @return a newly-allocated block of memory of the specified size in non-pageable memory, or
 * nullptr if an error occurred during allocation
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
_Must_inspect_result_
_Success_(size == sizeof(AX25Adapter))
_Ret_maybenull_
_Outptr_result_bytebuffer_(size)
_Result_nullonfailure_
void* AX25Adapter::operator new(_In_ size_t size,
                                _In_ NDIS_HANDLE driverHandle) noexcept
{   
    // Check the size of the allocation makes sense
    if (size != sizeof(AX25Adapter))
    {
        // Invalid size - refuse to handle the request
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_ADAPTER, "Failed to allocate space for AX25Adapter: invalid size %d (expected %d)",
                    static_cast<int>(size), static_cast<int>(sizeof(AX25Adapter)));
        return nullptr;
    }

    // Verify the driver handle was passed in
    if (driverHandle == nullptr)
    {
        // invalid driver handle - refuse to try to pass this on to NDIS
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_ADAPTER, "Failed to allocate space for AX25Adapter: driverHandle is nullptr");
        return nullptr;
    }

    // Allocate the memory
    static_assert(sizeof(AX25Adapter) <= MAXUINT, "static_cast below is invalid for very large objects");
    void* result = NdisAllocateMemoryWithTagPriority(driverHandle,
                                                     static_cast<UINT>(size),
                                                     AX25_ADAPTER_TAG,
                                                     NormalPoolPriority);
    // Log if there was an error
    if (result == nullptr)
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_ADAPTER, "Failed to allocate space for AX25Adapter: NdisAllocateMemoryWithTagPriority returned nullptr");
    }
    else
    {
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_ADAPTER, "Allocated new AX25Adapter at %p", result);
    }

    return result;
}

/**
 * Deallocates space for this AX25Adapter object. The pointer specified must point to nullptr or 
 * a destroyed AX25Adapter object which was allocated via the AX25Adapter::operator new(size_t, NDIS_HANDLE) function.
 * If nullptr is supplied as an argument, this function is a no-op. Otherwise, the memory specified is deallocated.
 * @param pointer a pointer to the destroyed AX25Adapter to deallocate, or nullptr
 * @param driverHandle the driver handle to pass to NDIS during deallocation
 * @throws NTSTATUS STATUS_INVALID_PARAMETER_2 if driverHandle is nullptr
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
_When_(driverHandle == nullptr, _Raises_SEH_exception_)
void AX25Adapter::operator delete(_In_opt_ void* pointer, _In_ NDIS_HANDLE driverHandle)
{

    if (pointer != nullptr)
    {
        // Protect from an invalid call
        if (driverHandle == nullptr)
        {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_ADAPTER, "Cannot deallocate AX25Adapter: NDIS driver handle is nullptr");
            ExRaiseStatus(STATUS_INVALID_PARAMETER_2);
        }
        else
        {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_ADAPTER, "Deallocating AX25Adapter at %p", pointer);
            NdisFreeMemoryWithTagPriority(driverHandle, pointer, AX25_ADAPTER_TAG);
        }
    }
}