#include "pch.h"
#include "Miniport.h"
#include "Driver.h"
#include "driver.tmh"

/**
 * Constructs a new Miniport object with an invalid NDIS handle and driver object pointer
 */
Miniport::Miniport() noexcept 
    :miniportDriverHandle(NULL)
    ,driverObject(NULL)
{

}

/**
* Gets an instance of the Miniport handler function interface. The pointer returned is a non-owner
* pointer of the allocated Miniport. The pointer is guaranteed to be valid for the lifetime of the driver
* and point to memory in non-pageable space.
*
* This function can be called at any IRQL if the Miniport driver has already been allocated, but only at
* APC_LEVEL or lower if the driver might not have been allocated yet. Care must be taken to ensure
* that the IRQL is appropriate for the given conditions. If executing above APC_LEVEL, then use @seealso TryGetInstance()
* instead to prevent an allocation if the state is unknown.
*
* If allocation fails, nullptr is returned.
* @returns an instance of the Miniport handler function interface, or nullptr if the system is out of memory
*/
_Must_inspect_result_
_Ret_maybenull_
Miniport * Miniport::GetInstance()
{
	// If we haven't yet allocated an instance, do so
	if (instance.IsNull())
	{
        __try
        {
            instance = UniqueNonPageablePointer<Miniport>::Allocate();
        }
        __except ((GetExceptionCode() == STATUS_NO_MEMORY) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            // going to leave the instance as a nullptr; it will be returned as such
        }
	}

	// Return the instance
	return instance;
}


/**
* Gets an instance of the Miniport handler function interface if it has already been allocated. The pointer returned
* is a non-owner pointer of the allocated Miniport. The pointer is guaranteed to be valid for the lifetime of the driver
* and point to memory in non-pageable space if it has already been allocated. If the Miniport object has not yet been
* allocated, then nullptr is returned.
*
* This function can be called at any IRQL, but will not allocate memory if the Miniport object has not yet been allocated.
* If the object needs to be allocated, then call @seealso GetInstance(). However, note that @seealso GetInstance() can only
* allocate memory if executing at or below APC_LEVEL.
*/
_Must_inspect_result_
_Ret_maybenull_
Miniport * Miniport::TryGetInstance() noexcept
{
    return instance;
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

    // Set the callback functions
    characteristics.InitializeHandlerEx = &miniportInitializeEx;
    characteristics.HaltHandlerEx = &miniportHaltEx;
    characteristics.UnloadHandler = &miniportDriverUnload;
    characteristics.PauseHandler = &miniportPause;
    characteristics.RestartHandler = &miniportRestart;
    characteristics.OidRequestHandler = &miniportOidRequest;
    characteristics.SendNetBufferListsHandler = &miniportSendNetBufferLists;
    characteristics.ReturnNetBufferListsHandler = &miniportReturnNetBufferLists;
    characteristics.CancelSendHandler = &miniportCancelSend;
    characteristics.CheckForHangHandlerEx = &miniportCheckForHangEx;
    characteristics.ResetHandlerEx = &miniportResetEx;
    characteristics.DevicePnPEventNotifyHandler = &miniportDevicePnpEventNotify;
    characteristics.ShutdownHandlerEx = &miniportShutdownEx;
    characteristics.CancelOidRequestHandler = &miniportCancelOidRequest;

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
