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

#include "pch.h"
#include "Miniport.h"
#include "Driver.h"
#include "Miniport.tmh"

/**
 * Constructs a new Miniport object with an invalid NDIS handle and driver object pointer
 */
Miniport::Miniport() noexcept 
    :miniportDriverHandle(NULL)
    ,driverObject(NULL)
{
    // Zero out all of the adapters; start with none of them used
    RtlZeroMemory(adapters, sizeof(adapters));
}

/**
 * Allocator function for a Miniport object. This custom allocator always allocates
 * the object in non-pageable memory.
 * @param size the size of the allocation. This value must be equal to 
 * sizeof(Miniport) or nullptr will be returned.
 * @returns a newly-allocated contiguous block of memory for use as a Miniport object,
 * allocated in non-pageable memory, or nullptr if the size requested was not equivalent
 * to the size of a Miniport object.
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
_IRQL_requires_same_
_Must_inspect_result_
_Success_(size == sizeof(Miniport))
_Ret_maybenull_
_Result_nullonfailure_
void * Miniport::operator new(
    _In_range_(sizeof(Miniport), sizeof(Miniport)) size_t size) noexcept
{
    // Check that we're asking for a buffer of a correct size
    if (size != sizeof(Miniport))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "Unexpected request to allocate memory for Miniport with incorrect size %d (expected %d)",
                    static_cast<int>(size), static_cast<int>(sizeof(Miniport)));
        return nullptr;
    }

    __try
    {
        // Allocate the memory
        void* memory = ExAllocatePoolWithTagPriority(static_cast<POOL_TYPE>(NonPagedPoolNx | POOL_RAISE_IF_ALLOCATION_FAILURE), 
                                                     size, 
                                                     MINIPORT_TAG, 
                                                     NormalPoolPriority);
        if (memory == nullptr)
        {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "Request to allocate Miniport failed unexpectedly: nullptr returned");
        }

        return memory;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        // Error occurred while allocating - Log and return nullptr
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "Request to allocate Miniport failed: %!STATUS!", GetExceptionCode());
        return nullptr;
    }
}

/**
 * Deallocates this Miniport object, which was allocated via a call to ExAllocatePoolWithTagPriority.
 * If nullptr is passed in, this function acts as a no-op.
 * @param pointer a pointer to a miniport object, or nullptr
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
_IRQL_requires_same_
void Miniport::operator delete(
    _Inout_updates_bytes_opt_(sizeof(Miniport)) void* pointer) noexcept
{
    // Only do work if there is a pointer here to operate on - delete nullptr; is always legal
    if (pointer != nullptr)
    {
        ExFreePoolWithTag(pointer, MINIPORT_TAG);
    }
}

/**
 * Fills the specified NDIS_MINIPORT_DRIVER_CHARACTERISTICS object with callback functions and then registers 
 * this driver with NDIS.
 * @param object a pointer to the driver object structure which represents this driver
 * @param registryPath a pointer to a unicode string indicating the path to the non-hardware-instance-specific
 * registry key for this driver's settings
 * @returns NDIS_SUCCESS if the driver successfully registered with NDIS, or a failure code otherwise
 */
_Must_inspect_result_
_IRQL_requires_(PASSIVE_LEVEL)
NDIS_STATUS Miniport::RegisterWithNdis(
    _In_  PDRIVER_OBJECT object,
    _In_  PUNICODE_STRING registryPath) noexcept
{
    // Set up miniport driver characteristics to be passed to the registration function
    NDIS_MINIPORT_DRIVER_CHARACTERISTICS characteristics;
    NdisZeroMemory(&characteristics, sizeof(characteristics));
    characteristics.Header.Type = NDIS_OBJECT_TYPE_MINIPORT_DRIVER_CHARACTERISTICS;
    characteristics.Header.Revision = NDIS_MINIPORT_DRIVER_CHARACTERISTICS_REVISION_2;
    characteristics.Header.Size = NDIS_SIZEOF_MINIPORT_DRIVER_CHARACTERISTICS_REVISION_2;
    characteristics.MajorNdisVersion = NDIS_MINIPORT_MAJOR_VERSION;
    characteristics.MinorNdisVersion = NDIS_MINIPORT_MINOR_VERSION;
    characteristics.MajorDriverVersion = DRIVER_MAJOR_VERSION;
    characteristics.MinorDriverVersion = DRIVER_MINOR_VERSION;

    // No need for a MiniportSetOptions callback
    characteristics.SetOptionsHandler = nullptr;

    // Set the callback functions
    /*
    characteristics.InitializeHandlerEx = &miniportInitializeExCallback;
    characteristics.HaltHandlerEx = &miniportHaltExCallback;
    characteristics.UnloadHandler = &miniportDriverUnloadCallback;
    characteristics.PauseHandler = &miniportPauseCallback;
    characteristics.RestartHandler = &miniportRestartCallback;
    characteristics.OidRequestHandler = &miniportOidRequestCallback;
    characteristics.SendNetBufferListsHandler = &miniportSendNetBufferListsCallback;
    characteristics.ReturnNetBufferListsHandler = &miniportReturnNetBufferListsCallback;
    characteristics.CancelSendHandler = &miniportCancelSendCallback;
    characteristics.CheckForHangHandlerEx = &miniportCheckForHangExCallback;
    characteristics.ResetHandlerEx = &miniportResetExCallback;
    characteristics.DevicePnPEventNotifyHandler = &miniportDevicePnpEventNotifyCallback;
    characteristics.ShutdownHandlerEx = &miniportShutdownExCallback;
    characteristics.CancelOidRequestHandler = &miniportCancelOidRequestCallback;
    */
    // Not going to handle direct OID requests
    characteristics.DirectOidRequestHandler = nullptr;
    characteristics.CancelDirectOidRequestHandler = nullptr;

    TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_DRIVER, "Calling to NdisMRegisterMiniportDriver");
    NDIS_STATUS result = NdisMRegisterMiniportDriver(object, registryPath, this, &characteristics, &miniportDriverHandle);
    if (NT_SUCCESS(result))
    {
        // Store the driver object
        driverObject = object;
    }
    else
    {
        // It failed - let's reset the handle to an invalid one
        miniportDriverHandle = NULL;
        driverObject = NULL;
    }

    return result;
}

/**
 * Initializes this miniport, allocating initial buffers as necessary and reading configuration so that
 * the driver is prepared for operation.
 * @param ndisMiniportHandle a handle to this miniport driver. This value must be equal to the most recently received
 * handle during registration with NDIS. 
 * @param miniportDriverContext the driver context with which to perform the operation. This must be a pointer to a Miniport object.
 * @param miniportInitParameters the parameters with which to initialize this miniport driver
 * @returns NDIS_SUCCESS if the initialization was successful, or an error code otherwise
 */
_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_ 
NDIS_STATUS Miniport::miniportInitializeExCallback(_In_ NDIS_HANDLE ndisMiniportHandle, 
                                                   _In_ NDIS_HANDLE miniportDriverContext, 
                                                   _In_ PNDIS_MINIPORT_INIT_PARAMETERS miniportInitParameters) noexcept
{
    // Convert context to a Miniport object
    if (miniportDriverContext == nullptr)
    {
        TraceEvents(TRACE_LEVEL_CRITICAL, TRACE_DRIVER, "Cannot initialize miniport: context is nullptr");
        return STATUS_INVALID_PARAMETER_2;
    }
    Miniport* context = static_cast<Miniport*>(miniportDriverContext);

    // Verify the handle is correct. If not, something is seriously wrong.
    if (ndisMiniportHandle != context->miniportDriverHandle)
    {
        TraceEvents(TRACE_LEVEL_CRITICAL, TRACE_DRIVER, "Cannot initialize miniport: Handle value is inconsistent");
        return STATUS_INVALID_PARAMETER_1;
    }
    return context->miniportInitializeEx(miniportInitParameters);
}

/**
 * Initializes this miniport, allocating initial buffers as necessary and reading configuration so that
 * the driver is prepared for operation. See <https://msdn.microsoft.com/en-us/library/windows/hardware/ff559392(v=vs.85).aspx>
 * for a detailed description of the full process of initializing this miniport.
 * @param initParameters the parameters with which to initialize this miniport driver
 * @returns NDIS_STATUS_SUCCESS if the initialization was successful, or an error code otherwise
 */
_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
NDIS_STATUS Miniport::miniportInitializeEx(_In_ PNDIS_MINIPORT_INIT_PARAMETERS initParameters) noexcept
{
    // Figure out where we're going to store the new adapter
    Adapter* thisAdapter = findFirstMatchingAdapter([](Adapter adapter) { return adapter.inUse == false; });
    if (thisAdapter == nullptr)
    {
        // No space left
        TraceEvents(TRACE_LEVEL_CRITICAL, TRACE_DRIVER, "Cannot allocate miniport adapter: all slots are in use");
        NdisWriteErrorLogEntry(miniportDriverHandle, NDIS_ERROR_CODE_OUT_OF_RESOURCES, 1, static_cast<ULONG>(sizeof(adapters) / sizeof(Adapter)));
        return NDIS_STATUS_RESOURCES;
    }

    // Assign the new adapter per the specified interface number
    thisAdapter->inUse = true;
    thisAdapter->adapterNumber = initParameters->IfIndex;
    thisAdapter->adapter = new(miniportDriverHandle) AX25Adapter(miniportDriverHandle);

    if (thisAdapter->adapter == nullptr)
    {
        // Something went wrong with allocation - die off
        TraceEvents(TRACE_LEVEL_CRITICAL, TRACE_DRIVER, "Failed to allocate an AX25Adapter object");
        // Assume it was due to lack of memory when logging
        NdisWriteErrorLogEntry(miniportDriverHandle, NDIS_ERROR_CODE_OUT_OF_RESOURCES, 0);
        return NDIS_STATUS_RESOURCES;
    }

    return thisAdapter->adapter->SetMiniportAttributes(miniportDriverHandle);
}

/**
 * Callback for the Halt operation on an adapter. Provides the adapter context, which is an
 * AX25Adapter object, which is to be destroyed.
 * @param miniportAdapterContext the AX25Adapter context object which is to be destroyed
 * @param haltAction parameters associated with the reason for the halt operation
 */
_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
void Miniport::miniportHaltExCallback(_In_ NDIS_HANDLE miniportAdapterContext,
                                      _In_ NDIS_HALT_ACTION haltAction) noexcept
{
    UNREFERENCED_PARAMETER(haltAction); // no need for this - we deallocate the same way every time
    AX25Adapter* thisAdapter = reinterpret_cast<AX25Adapter*>(miniportAdapterContext);
    thisAdapter->Destroy();
}
