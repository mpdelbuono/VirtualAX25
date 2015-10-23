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
 * Initializes a new AX25Adapter object to default parameters and state
 * @param driverHandle the NDIS driver handle with which to allocate
 * and free resources
 */
_IRQL_requires_(PASSIVE_LEVEL)
NON_PAGEABLE_FUNCTION
AX25Adapter::AX25Adapter(_In_ NDIS_HANDLE driverHandle) noexcept
    :state(Initializing)
    ,currentVlan(0)
    ,currentPacketFilterMode(0)
    ,joinedMulticastGroups{0}
    ,driverHandle(driverHandle)
{
    initializeRegistrationAttributes();
    initializeGeneralAttributes();

    NDIS_OID const oidList[] = {
        OID_GEN_HARDWARE_STATUS,
        OID_GEN_TRANSMIT_BUFFER_SPACE,
        OID_GEN_RECEIVE_BUFFER_SPACE,
        OID_GEN_TRANSMIT_BLOCK_SIZE,
        OID_GEN_RECEIVE_BLOCK_SIZE,
        OID_GEN_VENDOR_ID,
        OID_GEN_VENDOR_DESCRIPTION,
        OID_GEN_VENDOR_DRIVER_VERSION,
        OID_GEN_CURRENT_PACKET_FILTER,
        OID_GEN_CURRENT_LOOKAHEAD,
        OID_GEN_DRIVER_VERSION,
        OID_GEN_MAXIMUM_TOTAL_SIZE,
        OID_GEN_XMIT_OK,
        OID_GEN_RCV_OK,
        OID_GEN_STATISTICS,
        OID_GEN_TRANSMIT_QUEUE_LENGTH,       // Optional
        OID_GEN_LINK_PARAMETERS,
        OID_GEN_INTERRUPT_MODERATION,
        OID_GEN_MEDIA_SUPPORTED,
        OID_GEN_MEDIA_IN_USE,
        OID_GEN_MAXIMUM_SEND_PACKETS,
        OID_GEN_XMIT_ERROR,
        OID_GEN_RCV_ERROR,
        OID_GEN_RCV_NO_BUFFER,
        OID_802_3_PERMANENT_ADDRESS,
        OID_802_3_CURRENT_ADDRESS,
        OID_802_3_MULTICAST_LIST,
        OID_802_3_MAXIMUM_LIST_SIZE,
        OID_802_3_RCV_ERROR_ALIGNMENT,
        OID_802_3_XMIT_ONE_COLLISION,
        OID_802_3_XMIT_MORE_COLLISIONS,
        OID_802_3_XMIT_DEFERRED,             // Optional
        OID_802_3_XMIT_MAX_COLLISIONS,       // Optional
        OID_802_3_RCV_OVERRUN,               // Optional
        OID_802_3_XMIT_UNDERRUN,             // Optional
        OID_802_3_XMIT_HEARTBEAT_FAILURE,    // Optional
        OID_802_3_XMIT_TIMES_CRS_LOST,       // Optional
        OID_802_3_XMIT_LATE_COLLISIONS,      // Optional
        OID_PNP_CAPABILITIES,                // Optional
        OID_RECEIVE_FILTER_ALLOCATE_QUEUE,
        OID_RECEIVE_FILTER_QUEUE_ALLOCATION_COMPLETE,
        OID_RECEIVE_FILTER_FREE_QUEUE,
        OID_RECEIVE_FILTER_CLEAR_FILTER,
        OID_RECEIVE_FILTER_SET_FILTER,
    };

    static_assert(sizeof(oidList) == OID_LIST_LENGTH * sizeof(NDIS_OID), "OID_LIST_LENGTH needs to be set properly");
    RtlCopyMemory(supportedOids, oidList, sizeof(oidList));

    // Want to leave these two the same size to simplify communication with NDIS
    static_assert(sizeof(inboundBuffer) == sizeof(outboundBuffer), "inbound and outbound buffers must be same size");

    // Initialize the receive DPC
    KeInitializeDpc(&receiveDpc, &receiveDpcCallback, this);

    state = Paused;
}

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
_Result_nullonfailure_
NON_PAGEABLE_FUNCTION
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
 * @throws NTSTATUS STATUS_INVALID_PARAMETER_2 if driverHandle is nullptr
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
NON_PAGEABLE_FUNCTION
void AX25Adapter::operator delete(_In_opt_ void* pointer)
{
    if (pointer != nullptr)
    {
        AX25Adapter* thisAdapter = reinterpret_cast<AX25Adapter*>(pointer);
        
        // This should be impossible, but we should check it rather than blindly accessing memory
        // The extra check is not going to significantly harm performance - this is not a hotspot by any means
        if (thisAdapter->driverHandle == nullptr)
        {
            TraceEvents(TRACE_LEVEL_ERROR, TRACE_ADAPTER, "Cannot deallocate AX25Adapter: NDIS driver handle is nullptr");
            // Can't raise an exception here because it would violate IRQL DISPATCH_LEVEL, so we will simply leave an error in the log.
        }
        else
        {
            TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_ADAPTER, "Deallocating AX25Adapter at %p", pointer);
            NdisFreeMemoryWithTagPriority(thisAdapter->driverHandle, pointer, AX25_ADAPTER_TAG);
        }
    }
}

/**
 * Calls to NDIS to indicate this miniport adapter's attributes
 * @returns NDIS_STATUS_SUCCESS if attribute setting was successful, or an error code otherwise
 */
_IRQL_requires_(PASSIVE_LEVEL)
_Must_inspect_result_
PAGEABLE_FUNCTION
NDIS_STATUS AX25Adapter::SetMiniportAttributes()
{
    ASSERT(driverHandle != nullptr);
    // Validate the driver handle
    if (driverHandle == nullptr)
    {
        // Don't try to call NDIS with a bad pointer - fail out
        TraceEvents(TRACE_LEVEL_CRITICAL, TRACE_ADAPTER, "Cannot set miniport attributes: driver handle is nullptr");
        return STATUS_INVALID_ADDRESS;
    }

    // Call to NDIS - the attributes should already have been set as part of the state of this object
    NDIS_STATUS status = NdisMSetMiniportAttributes(driverHandle, registrationAttributes);
    if (NDIS_STATUS_SUCCESS != status)
    {
        // Something went wrong - report it and give up
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_ADAPTER, "Failed to set miniport adapter registration attributes: %!STATUS!", status);
        return status;
    }
    status = NdisMSetMiniportAttributes(driverHandle, generalAttributes);
    if (NDIS_STATUS_SUCCESS != status)
    {
        // Report the error
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_ADAPTER, "Failed to set miniport adapter general attributes: %!STATUS!", status);
        return status;
    }


    return status;
}

/**
 * Initializes the general attributes structure associated with this adapter. This provides general information
 * about the operation of the radio to which this adapter is connected.
 */
PAGEABLE_FUNCTION
void AX25Adapter::initializeGeneralAttributes() noexcept
{
    // Fill out the version information
    generalAttributes->Header.Revision = NDIS_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES_REVISION_2;
    generalAttributes->Header.Size = NDIS_SIZEOF_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES_REVISION_2;
    generalAttributes->Header.Type = NDIS_OBJECT_TYPE_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES;

    
    generalAttributes->MediaType = NdisMedium802_3;                         // Pretend that we're an ethernet network
    generalAttributes->PhysicalMediumType = NdisPhysicalMediumWirelessWan;  // But warn NDIS that we're in a wireless scenario
    
    // Set transmission parameters to something meaningful at 1200 baud, with cap at 9600 baud
    generalAttributes->MtuSize = DEFAULT_MTU_SIZE_BYTES;
    generalAttributes->MaxXmitLinkSpeed = MAX_XMIT_BITS_PER_SECOND;
    generalAttributes->XmitLinkSpeed = DEFAULT_XMIT_BITS_PER_SECOND;
    generalAttributes->MaxRcvLinkSpeed = MAX_RCV_BITS_PER_SECOND;
    generalAttributes->RcvLinkSpeed = DEFAULT_RCV_BITS_PER_SECOND;
    generalAttributes->LookaheadSize = sizeof(inboundBuffer);

    generalAttributes->MediaConnectState = MediaConnectStateDisconnected;   // Start disconnected
    generalAttributes->MediaDuplexState = MediaDuplexStateHalf;             // A radio link is (almost) always half-duplex
    generalAttributes->MacOptions =
        NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA |
        NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |
        NDIS_MAC_OPTION_NO_LOOPBACK |
        NDIS_MAC_OPTION_8021P_PRIORITY |                            // required to be specified
        NDIS_MAC_OPTION_8021Q_VLAN;
    generalAttributes->SupportedPacketFilters =
        NDIS_PACKET_TYPE_DIRECTED |
        NDIS_PACKET_TYPE_MULTICAST |
        NDIS_PACKET_TYPE_ALL_MULTICAST |
        NDIS_PACKET_TYPE_BROADCAST |
        NDIS_PACKET_TYPE_PROMISCUOUS;

    generalAttributes->MaxMulticastListSize = MAX_MULTICAST_GROUPS;

    // Copy in the mac address
    RtlZeroMemory(generalAttributes->CurrentMacAddress, sizeof(generalAttributes->CurrentMacAddress));
    memcpy(generalAttributes->CurrentMacAddress, &DEFAULT_MAC_ADDRESS, sizeof(DEFAULT_MAC_ADDRESS));
    RtlZeroMemory(generalAttributes->PermanentMacAddress, sizeof(generalAttributes->PermanentMacAddress));
    memcpy(generalAttributes->PermanentMacAddress, &DEFAULT_MAC_ADDRESS, sizeof(DEFAULT_MAC_ADDRESS));
    
    generalAttributes->RecvScaleCapabilities = nullptr;                     // No support for receive-side scaling

    // Port type configuration
    generalAttributes->AccessType = NET_IF_ACCESS_BROADCAST;
    generalAttributes->DirectionType = NET_IF_DIRECTION_SENDRECEIVE;
    generalAttributes->ConnectionType = NET_IF_CONNECTION_DEDICATED;
    generalAttributes->IfType = IF_TYPE_ETHERNET_CSMACD;                    // Pretend to be an ethernet device
                                                                                      // TODO: Evaluate if it would be more appropriate to specify X.25 here
                                                                                      // (AX.25 does not have a corresponding IANA number)
    generalAttributes->IfConnectorPresent = TRUE;      // Pretend there is a connector. We will be using this property to
                                                                 // indicate whether or not we're bound to a KISS/AGWPE port.

    // Required to support all statistics
    generalAttributes->SupportedStatistics =
        NDIS_STATISTICS_DIRECTED_FRAMES_RCV_SUPPORTED |
        NDIS_STATISTICS_MULTICAST_FRAMES_RCV_SUPPORTED |
        NDIS_STATISTICS_BROADCAST_FRAMES_RCV_SUPPORTED |
        NDIS_STATISTICS_BYTES_RCV_SUPPORTED |
        NDIS_STATISTICS_RCV_DISCARDS_SUPPORTED |
        NDIS_STATISTICS_RCV_ERROR_SUPPORTED |
        NDIS_STATISTICS_DIRECTED_FRAMES_XMIT_SUPPORTED |
        NDIS_STATISTICS_MULTICAST_FRAMES_XMIT_SUPPORTED |
        NDIS_STATISTICS_BROADCAST_FRAMES_XMIT_SUPPORTED |
        NDIS_STATISTICS_BYTES_XMIT_SUPPORTED |
        NDIS_STATISTICS_XMIT_ERROR_SUPPORTED |
        NDIS_STATISTICS_XMIT_DISCARDS_SUPPORTED |
        NDIS_STATISTICS_DIRECTED_BYTES_RCV_SUPPORTED |
        NDIS_STATISTICS_MULTICAST_BYTES_RCV_SUPPORTED |
        NDIS_STATISTICS_BROADCAST_BYTES_RCV_SUPPORTED |
        NDIS_STATISTICS_DIRECTED_BYTES_XMIT_SUPPORTED |
        NDIS_STATISTICS_MULTICAST_BYTES_XMIT_SUPPORTED |
        NDIS_STATISTICS_BROADCAST_BYTES_XMIT_SUPPORTED;

    generalAttributes->SupportedPauseFunctions = NdisPauseFunctionsUnsupported;
    generalAttributes->DataBackFillSize = 0;
    generalAttributes->ContextBackFillSize = 0;
    generalAttributes->SupportedOidList = supportedOids;                          
    generalAttributes->SupportedOidListLength = sizeof(supportedOids);
    generalAttributes->AutoNegotiationFlags =                                   // Pretend like we have always auto-negotiated all parameters
        NDIS_LINK_STATE_XMIT_LINK_SPEED_AUTO_NEGOTIATED                   |               // since the link will always be the same
        NDIS_LINK_STATE_RCV_LINK_SPEED_AUTO_NEGOTIATED |
        NDIS_LINK_STATE_DUPLEX_AUTO_NEGOTIATED;

    // Set up power management capabilities. Note that this never changes, so it can safely be a static member here.
    static NDIS_PM_CAPABILITIES powerManagementCapabilities;
    powerManagementCapabilities.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
    powerManagementCapabilities.Header.Revision = NDIS_PM_CAPABILITIES_REVISION_1;
    powerManagementCapabilities.Header.Size = NDIS_SIZEOF_NDIS_PM_CAPABILITIES_REVISION_1;
    powerManagementCapabilities.Flags = 0;
    powerManagementCapabilities.SupportedWoLPacketPatterns = 0;     // No Wake on LAN over AX.25 for now :)
    powerManagementCapabilities.NumTotalWoLPatterns = 0;
    powerManagementCapabilities.MaxWoLPatternSize = 0;
    powerManagementCapabilities.MaxWoLPatternOffset = 0;
    powerManagementCapabilities.MaxWoLPacketSaveBuffer = 0;
    powerManagementCapabilities.SupportedProtocolOffloads = 0;      // No low-power usage of the radio for now, though this may be beneficial later
    powerManagementCapabilities.NumArpOffloadIPv4Addresses = 0;
    powerManagementCapabilities.NumNSOffloadIPv6Addresses = 0;
    powerManagementCapabilities.MinMagicPacketWakeUp = NdisDeviceStateUnspecified;
    powerManagementCapabilities.MinPatternWakeUp = NdisDeviceStateUnspecified;
    powerManagementCapabilities.MinLinkChangeWakeUp = NdisDeviceStateUnspecified;    
    generalAttributes->PowerManagementCapabilitiesEx = &powerManagementCapabilities;
}

/**
 * Initializes the NDIS registration attributes associated with this adapter.
 */
PAGEABLE_FUNCTION
void AX25Adapter::initializeRegistrationAttributes() noexcept
{
    // Set the header
    registrationAttributes->Header.Type = NDIS_OBJECT_TYPE_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES;
    registrationAttributes->Header.Revision = NDIS_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES_REVISION_1;
    registrationAttributes->Header.Size = NDIS_SIZEOF_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES_REVISION_1;

    // We ARE the adapter context, so set that parameter accordingly
    registrationAttributes->MiniportAdapterContext = this;

    registrationAttributes->AttributeFlags =
        NDIS_MINIPORT_ATTRIBUTES_NDIS_WDM |            // This is a virtual device, and as such does not require HW allocation
        NDIS_MINIPORT_ATTRIBUTES_SURPRISE_REMOVE_OK;   // Can safely handle this event, since this is not a real device

    registrationAttributes->CheckForHangTimeInSeconds = 0;  // Use default hang check time, since everything we're doing
                                                            // should not be slowed by hardware

    // No meaning to an "interface type" here. Just claim it's a "host-specific internal interface".
    registrationAttributes->InterfaceType = NdisInterfaceInternal;

}

/**
 * Callback DPC for received packets. When a new packet comes in, this DPC is scheduled for processing.
 * @param dpc the KDPC object representing this DPC
 * @param adapterContext the context associated with this adapter
 */
_Use_decl_annotations_
NON_PAGEABLE_FUNCTION
void AX25Adapter::receiveDpcCallback(_In_ KDPC* dpc,
                                     _In_opt_ void* adapterContext,
                                     _In_opt_ void* systemArgument1,
                                     _In_opt_ void* systemArgument2)
{
    // Normally I would call ExRaiseStatus(STATUS_NOT_IMPLEMENTED) here, but that is illegal
    // at DISPATCH_LEVEL, so we're just going to issue a bugcheck instead.
    UNREFERENCED_PARAMETER(dpc);
    UNREFERENCED_PARAMETER(adapterContext);
    UNREFERENCED_PARAMETER(systemArgument1);
    UNREFERENCED_PARAMETER(systemArgument2);
    KeBugCheckEx(KMODE_EXCEPTION_NOT_HANDLED,
                 static_cast<ULONG_PTR>(STATUS_NOT_IMPLEMENTED),
                 reinterpret_cast<ULONG_PTR>(__FUNCTION__),
                 NULL, NULL);
}

/**
 * Destroys and deallocates this object. After calling this function, the object is no longer valid and
 * points to invalid memory.
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
NON_PAGEABLE_FUNCTION
void AX25Adapter::Destroy() noexcept
{
    this->~AX25Adapter();   // doesn't currently do anything interesting, but called for completeness/futureproofing
    delete this;            // calls to AX25Adapter::operator delete
}

/**
 * Pauses this adapter, typically in preparation for a sleep operation. The adapter must:
 * - Complete all active transmissions
 * - Stop accepting new IRPs for transmission
 * - Stop accepting inbound packets
 *
 * There is no need for a graceful disconnect of the media, though it may be desirable for some
 * connectors to be disconnected cleanly. However, no spurious AX.25 packets should be sent
 * to close an AX.25 connection.
 * 
 * If the transmissions cannot be completed immediately, this function can return a status of pending,
 * indicating that a future call to NdisMPauseComplete will indicate that pausing has been completed.
 *
 * The state of this adapter after a successful call is either 'Pausing' or 'Paused', and if the state
 * was 'Pausing', then the state is 'Paused' after a call to NdisMPauseComplete.
 * @param driverHandle the NDIS driver handle with which to perform allocations/deallocations
 * @returns NDIS_STATUS_SUCCESS if the adapter successfully completed pausing and the adapter is in the
 * 'Paused' state, or NDIS_STATUS_PENDING if the adapter is in the 'Pausing' state and a future call to
 * NdisMPauseComplete will be made.
 */
_IRQL_requires_(PASSIVE_LEVEL)
_Must_inspect_result_
PAGEABLE_FUNCTION
NDIS_STATUS AX25Adapter::Pause() noexcept
{
    // TODO: Complete active transmissions
    state = Paused;
    return NDIS_STATUS_SUCCESS;
}


/**
 * Restarts this adapter, shifting it back into the running state. After this function is called,
 * the adapter is ready to again receive and transmit packets.
 */
_IRQL_requires_(PASSIVE_LEVEL)
_Must_inspect_result_
PAGEABLE_FUNCTION
NDIS_STATUS AX25Adapter::Restart(const NDIS_MINIPORT_RESTART_PARAMETERS& restartParameters) noexcept
{
    UNREFERENCED_PARAMETER(restartParameters);
    // TODO: Connect to connector as appropriate
    state = Running;
    return NDIS_STATUS_SUCCESS;
}

/**
 * Handles an OID request to query or set information for this adapter. The OID request
 * specifies the behavior expected.
 * @param oidRequest the request to enact. Depending on the request, this parameter may also
 * be used to provide information back to the requestor.
 * @returns NDIS_STATUS_INVALID_OID if the OID was not recognized
 * @returns NDIS_STATUS_NOT_SUPPORTED if the OID was recognized, but not supported by this driver
 * @returns NDIS_STATUS_BUFFER_TOO_SHORT if the request cannot fit in the buffer supplied by the 
 * request
 * @returns NDIS_STATUS_NOT_ACCEPTED if the adapter is in the Halted state
 * @returns NDIS_STATUS_PENDING if the request was accepted, and is in progress. A future call
 * to NdisMOidRequestComplete will be made when the request is complete
 * @returns NDIS_STATUS_SUCCESS if the OID request was completed in its entirety
 */
_IRQL_requires_(PASSIVE_LEVEL)
_Must_inspect_result_
PAGEABLE_FUNCTION
NDIS_STATUS AX25Adapter::HandleOidRequest(_In_ NDIS_OID_REQUEST& oidRequest) noexcept
{
    // Check to make sure we're in a valid state
    if (state == Halted)
    {
        TraceEvents(TRACE_LEVEL_WARNING, TRACE_ADAPTER, "OID request not accepted: adapter is halted");
        return NDIS_STATUS_NOT_ACCEPTED;
    }

    // TODO: Handle OID requests
    UNREFERENCED_PARAMETER(oidRequest);
    return STATUS_NOT_IMPLEMENTED;
}

/**
 * Sends the given network data along this adapter. This adapter must be in the running state or the request
 * will be rejected. This function may return before the data has been transmitted. After completion 
 * (whether success or failure), NdisMSendNetBufferListsComplete will be called. Until NdisMSendNetBufferListsComplete
 * is called, this adapter owns the netBufferList object provided. After NdisMSendNetBufferListsComplete is called,
 * ownership is released and the memory is treated as inaccessible. Note that a call to NdisMSendNetBufferListsComplete
 * does not necessarily mean the data has been transmitted, but rather that it has at least been queued for transmission
 * and the buffer is no longer needed.
 * @param netBufferList a linked list of NET_BUFFER_LIST objects indicating the data to be transmitted, in order
 * @param sendFlags the flags associted with this send operation. If NDIS_SEND_FLAGS_CHECK_FOR_LOOPBACK is set, then
 * this function will check to see if the data should loop back. If NDIS_SEND_FLAGS_DISPATCH_LEVEL is set, then the current
 * IRQL is DISPATCH_LEVEL.
 */
_When_(sendFlags & NDIS_SEND_FLAGS_DISPATCH_LEVEL, _IRQL_requires_(DISPATCH_LEVEL))
_When_(!(sendFlags & NDIS_SEND_FLAGS_DISPATCH_LEVEL), _IRQL_requires_max_(APC_LEVEL))
_IRQL_requires_same_
NON_PAGEABLE_FUNCTION
void AX25Adapter::SendNetBufferLists(
    _In_ NET_BUFFER_LIST& netBufferList, 
    _In_ ULONG sendFlags) noexcept
{
   // TODO: Enqueue the data 
    UNREFERENCED_PARAMETER(netBufferList);
    UNREFERENCED_PARAMETER(sendFlags);
}

/**
 * Returns the specified NET_BUFFER_LIST objects to being owned by this object so that they
 * can be reused in future receive operations.
 * @param netBufferLists a linked list of NET_BUFFER_LIST objects that are to be returned to ownership
 * by this object
 * @param returnFlags a flag field with bit NDIS_RETURN_FLAGS_DISPATCH_LEVEL set if the current IRQL
 * is DISPATCH_LEVEL.
 */
_When_(returnFlags & NDIS_RETURN_FLAGS_DISPATCH_LEVEL, _IRQL_requires_(DISPATCH_LEVEL))
_When_(!(returnFlags & NDIS_RETURN_FLAGS_DISPATCH_LEVEL), _IRQL_requires_max_(APC_LEVEL))
_IRQL_requires_same_
NON_PAGEABLE_FUNCTION
void ReturnNetBufferLists(
    _In_ NET_BUFFER_LIST& netBufferLists,
    _In_ ULONG returnFlags) noexcept
{
    // TODO: Return the net buffer lists
    UNREFERENCED_PARAMETER(netBufferLists);
    UNREFERENCED_PARAMETER(returnFlags);
}
