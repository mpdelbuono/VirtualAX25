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
 */
AX25Adapter::AX25Adapter() noexcept
    :currentVlan(0)
    ,currentPacketFilterMode(0)
    ,joinedMulticastGroups{0}
{
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

/**
 * Calls to NDIS to indicate this miniport adapter's attributes
 * @param miniportDriverHandle the driver handle to use when making an NDIS request
 * @returns NDIS_STATUS_SUCCESS if attribute setting was successful, or an error code otherwise
 */
_IRQL_requires_(PASSIVE_LEVEL)
_Must_inspect_result_
NDIS_STATUS AX25Adapter::SetMiniportAttributes(_In_ NDIS_HANDLE miniportDriverHandle)
{
    // Validate the driver handle
    if (miniportDriverHandle == nullptr)
    {
        // Don't try to call NDIS with a bad pointer - fail out
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_ADAPTER, "Cannot set miniport attributes: driver handle is nullptr");
        return STATUS_INVALID_PARAMETER_1;
    }

    // Call to NDIS - the attributes should already have been set as part of the state of this object
    return NdisMSetMiniportAttributes(miniportDriverHandle, &attributes);
}

/**
 * Initializes the general attributes structure associated with this adapter. This provides general information
 * about the operation of the radio to which this adapter is connected.
 */
void AX25Adapter::initializeGeneralAttributes() noexcept
{
    // Fill out the version information
    attributes.GeneralAttributes.Header.Revision = NDIS_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES_REVISION_2;
    attributes.GeneralAttributes.Header.Size = NDIS_SIZEOF_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES_REVISION_2;
    attributes.GeneralAttributes.Header.Type = NDIS_OBJECT_TYPE_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES;

    
    attributes.GeneralAttributes.MediaType = NdisMedium802_3;                         // Pretend that we're an ethernet network
    attributes.GeneralAttributes.PhysicalMediumType = NdisPhysicalMediumWirelessWan;  // But warn NDIS that we're in a wireless scenario
    
    // Set transmission parameters to something meaningful at 1200 baud, with cap at 9600 baud
    attributes.GeneralAttributes.MtuSize = DEFAULT_MTU_SIZE_BYTES;
    attributes.GeneralAttributes.MaxXmitLinkSpeed = MAX_XMIT_BITS_PER_SECOND;
    attributes.GeneralAttributes.XmitLinkSpeed = DEFAULT_XMIT_BITS_PER_SECOND;
    attributes.GeneralAttributes.MaxRcvLinkSpeed = MAX_RCV_BITS_PER_SECOND;
    attributes.GeneralAttributes.RcvLinkSpeed = DEFAULT_RCV_BITS_PER_SECOND;
    attributes.GeneralAttributes.LookaheadSize = sizeof(inboundBuffer);

    attributes.GeneralAttributes.MediaConnectState = MediaConnectStateDisconnected;   // Start disconnected
    attributes.GeneralAttributes.MediaDuplexState = MediaDuplexStateHalf;             // A radio link is (almost) always half-duplex
    attributes.GeneralAttributes.MacOptions =
        NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA |
        NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |
        NDIS_MAC_OPTION_NO_LOOPBACK |
        NDIS_MAC_OPTION_8021P_PRIORITY |                            // required to be specified
        NDIS_MAC_OPTION_8021Q_VLAN;
    attributes.GeneralAttributes.SupportedPacketFilters =
        NDIS_PACKET_TYPE_DIRECTED |
        NDIS_PACKET_TYPE_MULTICAST |
        NDIS_PACKET_TYPE_ALL_MULTICAST |
        NDIS_PACKET_TYPE_BROADCAST |
        NDIS_PACKET_TYPE_PROMISCUOUS;

    attributes.GeneralAttributes.MaxMulticastListSize = MAX_MULTICAST_GROUPS;

    // Copy in the mac address
    RtlZeroMemory(attributes.GeneralAttributes.CurrentMacAddress, sizeof(attributes.GeneralAttributes.CurrentMacAddress));
    memcpy(attributes.GeneralAttributes.CurrentMacAddress, &DEFAULT_MAC_ADDRESS, sizeof(DEFAULT_MAC_ADDRESS));
    RtlZeroMemory(attributes.GeneralAttributes.PermanentMacAddress, sizeof(attributes.GeneralAttributes.PermanentMacAddress));
    memcpy(attributes.GeneralAttributes.PermanentMacAddress, &DEFAULT_MAC_ADDRESS, sizeof(DEFAULT_MAC_ADDRESS));
    
    attributes.GeneralAttributes.RecvScaleCapabilities = nullptr;                     // No support for receive-side scaling

    // Port type configuration
    attributes.GeneralAttributes.AccessType = NET_IF_ACCESS_BROADCAST;
    attributes.GeneralAttributes.DirectionType = NET_IF_DIRECTION_SENDRECEIVE;
    attributes.GeneralAttributes.ConnectionType = NET_IF_CONNECTION_DEDICATED;
    attributes.GeneralAttributes.IfType = IF_TYPE_ETHERNET_CSMACD;                    // Pretend to be an ethernet device
                                                                                      // TODO: Evaluate if it would be more appropriate to specify X.25 here
                                                                                      // (AX.25 does not have a corresponding IANA number)
    attributes.GeneralAttributes.IfConnectorPresent = TRUE;      // Pretend there is a connector. We will be using this property to
                                                                 // indicate whether or not we're bound to a KISS/AGWPE port.

    // Required to support all statistics
    attributes.GeneralAttributes.SupportedStatistics =
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

    attributes.GeneralAttributes.SupportedPauseFunctions = NdisPauseFunctionsUnsupported;
    attributes.GeneralAttributes.DataBackFillSize = 0;
    attributes.GeneralAttributes.ContextBackFillSize = 0;
    attributes.GeneralAttributes.SupportedOidList = supportedOids;                          
    attributes.GeneralAttributes.SupportedOidListLength = sizeof(supportedOids);
    attributes.GeneralAttributes.AutoNegotiationFlags =                                   // Pretend like we have always auto-negotiated all parameters
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
    attributes.GeneralAttributes.PowerManagementCapabilitiesEx = &powerManagementCapabilities;
}
