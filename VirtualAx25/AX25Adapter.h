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
 * @file AX25Adapter.h
 * Definition of the AX25Adapter class, which represents a single virtual adapter presented 
 * to the operating system.
 * @author Matthew P. Del Buono (KG7UDH)
 */

#pragma once
#include "Utility.h"

/**
 * Represents a single AX.25 adapter. In scenarios where multiple AX.25 adapters have been
 * added to the system, there will be exactly one Miniport object for shared use by all adapters, but
 * each individual adapter will have its own AX25Adapter object. The Miniport object will call
 * into the appropriate AX25Adapter object as needed.
 */
class AX25Adapter
{
    // Some friendships defined to assist with unit testing without interfering with the class operation
    friend class AX25AdapterFixture_DeleteNullptr_Test;
    friend class AX25AdapterFixture_ValidDeallocation_Test;
public:
    _IRQL_requires_max_(DISPATCH_LEVEL)
    _Must_inspect_result_
    _Success_(size == sizeof(AX25Adapter))
    _Ret_maybenull_
    _Result_nullonfailure_
    NON_PAGEABLE_FUNCTION
    void* operator new(_In_ size_t size,
                       _In_ NDIS_HANDLE driverHandle) noexcept;

    _IRQL_requires_(PASSIVE_LEVEL)
    NON_PAGEABLE_FUNCTION
    AX25Adapter(_In_ NDIS_HANDLE driverHandle) noexcept;

    _IRQL_requires_(PASSIVE_LEVEL)
    _Must_inspect_result_
    PAGEABLE_FUNCTION
    NDIS_STATUS SetMiniportAttributes();

    _IRQL_requires_max_(DISPATCH_LEVEL)
    NON_PAGEABLE_FUNCTION
    virtual void Destroy() noexcept;

    _IRQL_requires_(PASSIVE_LEVEL)
    _Must_inspect_result_
    PAGEABLE_FUNCTION
    virtual NDIS_STATUS Pause() noexcept;

    _IRQL_requires_(PASSIVE_LEVEL)
    _Must_inspect_result_
    PAGEABLE_FUNCTION
    virtual NDIS_STATUS Restart(_In_ const NDIS_MINIPORT_RESTART_PARAMETERS& restartParameters) noexcept;

    _IRQL_requires_(PASSIVE_LEVEL)
    _Must_inspect_result_
    PAGEABLE_FUNCTION 
    virtual NDIS_STATUS HandleOidRequest(_In_ NDIS_OID_REQUEST& oidRequest) noexcept;

    _When_((sendFlags & NDIS_SEND_FLAGS_DISPATCH_LEVEL), _IRQL_requires_(DISPATCH_LEVEL))
    _When_((!(sendFlags & NDIS_SEND_FLAGS_DISPATCH_LEVEL)), _IRQL_requires_max_(APC_LEVEL))
    _IRQL_requires_same_
    NON_PAGEABLE_FUNCTION
    void SendNetBufferLists(
        _In_ NET_BUFFER_LIST& netBufferList, 
        _In_ ULONG sendFlags) noexcept;

    _When_(returnFlags & NDIS_RETURN_FLAGS_DISPATCH_LEVEL, _IRQL_requires_(DISPATCH_LEVEL))
    _When_(!(returnFlags & NDIS_RETURN_FLAGS_DISPATCH_LEVEL), _IRQL_requires_max_(APC_LEVEL))
    _IRQL_requires_same_
    NON_PAGEABLE_FUNCTION
    void ReturnNetBufferLists(
        _In_ NET_BUFFER_LIST& netBufferLists,
        _In_ ULONG returnFlags) noexcept;
private:
    /**
     * Represents the current state of this adapter 
     */
    enum State
    {
        Halted,         //<! Not used, because this object is deallocated while halted
        Initializing,   //<! State of the adapter while it is being constructed
        Paused,         //<! Idle state of the adapter where no send/receive operations are possible
        Restarting,     //<! State of the adapter while it is being transitioned to the Running state
        Running,        //<! Normal state of the adapter where it is processing send/receive operations
        Pausing,        //<! State of the adapter while it is completing pending send/receive operations in preparation for pausing
        Shutdown        //<! Not used, because this object is deallocated while the driver is shut down
    } state; //<! current state of the adapter

    /**
     * The tag to use when allocating an AX25Adapter object in the non-pageable pool. In memory
     * this should appear as "axAX", little-endian.
     */
    static constexpr ULONG AX25_ADAPTER_TAG = AX25_CREATE_TAG("axAX");

    static constexpr ULONG DEFAULT_MTU_SIZE_BYTES = 512;        //<! Default MTU for an AX.25 link
    static constexpr ULONG DEFAULT_XMIT_BITS_PER_SECOND = 1200; //<! Default transmit speed for an AX.25 link on VHF
    static constexpr ULONG MAX_XMIT_BITS_PER_SECOND = 9600;     //<! Maximum transmit speed for an AX.25 link on VHF

    // The link is bidirectional in nature, using the same modulation typically
    static constexpr ULONG DEFAULT_RCV_BITS_PER_SECOND = DEFAULT_XMIT_BITS_PER_SECOND;  //<! Default receive speed for an AX.25 link on VHF
    static constexpr ULONG MAX_RCV_BITS_PER_SECOND = MAX_XMIT_BITS_PER_SECOND;          //<! Maximum receive speed for an AX.25 link on VHF

    static constexpr size_t MAX_MULTICAST_GROUPS = 16;          //<! Maximum number of multicast groups supported simultaneously
    static constexpr size_t MAC_ADDRESS_LENGTH_BITS = 8 * 7;    //<! Number of bits in an AX.25 address

    /** The default MAC address used to allocate to this adapter */
    static constexpr unsigned long long DEFAULT_MAC_ADDRESS =   // Set default address to KG7UDH-0 for now
        (static_cast<unsigned long long>('K') << 0 ) |
        (static_cast<unsigned long long>('G') << 8 ) |
        (static_cast<unsigned long long>('7') << 16) |
        (static_cast<unsigned long long>('U') << 24) |
        (static_cast<unsigned long long>('D') << 32) | 
        (static_cast<unsigned long long>('H') << 40) |
                                       (0ULL  << 48);

    /**
     * Buffer for storing inbound data (received from the radio)
     */
    BYTE inboundBuffer[DEFAULT_MTU_SIZE_BYTES];

    /**
     * Buffer for storing outbound data (to send to the radio)
     */
    BYTE outboundBuffer[DEFAULT_MTU_SIZE_BYTES];

   


    /**
     * The VLAN to which this adapter is assigned. At startup, the VLAN is set to 0. NDIS 
     * can change this at any time. If it is changed, the adadpter will begin filtering to the
     * specified VLAN.
     */
    int currentVlan;

    /**
     * The current packet filtering mode of this adapter. At startup, this is set to 0 (no
     * packets are processed). NDIS will reconfigure the filter mode depending on what is
     * needed from the adapter. 
     * @seealso https://msdn.microsoft.com/en-us/library/windows/hardware/ff569575(v=vs.85).aspx
     */
    ULONG currentPacketFilterMode;

    /**
     * Multicast groups that have been joined on this adapter, as MAC addresses. An address of
     * 0 indicates an unused slot. At startup, this list is initialized to all zeroes. Note that
     * AX.25 MAC addresses are 56 bits long (6 callsign characters and an SSID) so a 64-bit
     * integer must be used here.
     */
    UINT64 joinedMulticastGroups[MAX_MULTICAST_GROUPS];
    static_assert(sizeof(UINT64) * 8 >= MAC_ADDRESS_LENGTH_BITS, "joinedMulticastGroups is not large enough to store an AX.25 MAC Address");

    /** The number of supported OIDs in the supportedOids field */
    static constexpr size_t OID_LIST_LENGTH = 44;

    /**
     * The OIDs that this AX25 Adapter supports. This is not unique to a given adapter; all adapters
     * will support the same OIDs. However, it is allocated as part of the AX25Adapter to keep it in non-pageable memory.
     */
    NDIS_OID supportedOids[OID_LIST_LENGTH];

    /**
     * Representation of an entity stored in an NDIS_MINIPORT_ADAPTER_ATTRIBUTES object.
     * That classifier is a union, and so it represents one of several possible types. To
     * improve type safety, this template class is introduced to allow for accessing it as 
     * its defined type, but to also get it as an NDIS_MINIPORT_ADAPTER_ATTRIBUTES object 
     * as necessary.
     * @tparam T the type of the member
     * @tparam Member a pointer to the member of the attribute being referenced.
     * members of NDIS_MINIPORT_ADAPTER_ATTRIBUTES
     */
    template <class T, T NDIS_MINIPORT_ADAPTER_ATTRIBUTES::*Member>
    class AdapterAttributes
    {
    public:
        NON_PAGEABLE_FUNCTION
        inline T* operator->() noexcept { return &(attributes.*Member); }
        NON_PAGEABLE_FUNCTION
        inline operator NDIS_MINIPORT_ADAPTER_ATTRIBUTES*() noexcept { return &attributes; }
    private:
        NDIS_MINIPORT_ADAPTER_ATTRIBUTES attributes;
    };

    /**
     * The NDIS general attributes associated with this adapter. This structure must be allocated in
     * non-pageable memory and live for the duration of the adapter, as NDIS will periodically
     * reference it and does not make its own copy.
     */
    AdapterAttributes<NDIS_MINIPORT_ADAPTER_GENERAL_ATTRIBUTES, &NDIS_MINIPORT_ADAPTER_ATTRIBUTES::GeneralAttributes> generalAttributes;

    /**
     * The NDIS registration attributes associated with this adapter. This structure must be allocated in
     * non-pageable memory and live for the duration of the adapter, as NDIS will periodically 
     * reference it and does not make its own copy.
     */
    AdapterAttributes<NDIS_MINIPORT_ADAPTER_REGISTRATION_ATTRIBUTES, &NDIS_MINIPORT_ADAPTER_ATTRIBUTES::RegistrationAttributes> registrationAttributes;

    PAGEABLE_FUNCTION
    void initializeGeneralAttributes() noexcept;

    PAGEABLE_FUNCTION
    void initializeRegistrationAttributes() noexcept;

    /**
     * DPC structure which represents the receive action DPC. When a new packet comes in, this DPC
     * will be used as a callback for processing the received information.
     */
    KDPC receiveDpc;

    NON_PAGEABLE_FUNCTION 
    static KDEFERRED_ROUTINE receiveDpcCallback;


    /**
     * The NDIS driver handle assigned to this driver, which was supplied during allocation
     * and construction.
     */
    const NDIS_HANDLE driverHandle;

    _IRQL_requires_max_(DISPATCH_LEVEL)
    NON_PAGEABLE_FUNCTION
    void operator delete(_In_opt_ void* pointer) noexcept;

};

