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
 * @file Miniport.h
 * Defines the Miniport class, which provides the NDIS Miniport handler function interface
 * @author Matthew P. Del Buono (KG7UDH)
 */
#pragma once
#include "AX25Adapter.h"

/**
 * Provides the NDIS Miniport handler function interface. 
 * 
 * This class is a singleton
 * which will be allocated in non-pageable memory. Accordingly, access of this memory is
 * valid at any IRQL. However, various function calls themselves may not be valid at a given
 * IRQL, so check each function's documentation before calling it.
 */
class Miniport
{
    // Friendship for unit tests
    friend class Miniport_DestroyOnHalt_Test;
public:
    Miniport() noexcept;
    ~Miniport() noexcept;

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _IRQL_requires_same_
    _Must_inspect_result_
    _Success_(size == sizeof(Miniport))
    _Ret_maybenull_
    _Result_nullonfailure_
    void* operator new(
        _In_range_(sizeof(Miniport), sizeof(Miniport)) size_t size) noexcept;

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _IRQL_requires_same_
    void operator delete(
         _Inout_updates_bytes_opt_(sizeof(Miniport)) void* pointer) noexcept;

    _Must_inspect_result_
    _IRQL_requires_(PASSIVE_LEVEL)
    NDIS_STATUS RegisterWithNdis(
        _In_  PDRIVER_OBJECT driverObject,
        _In_  PUNICODE_STRING registryPath) noexcept;

private:
    /**
     * Handle to this miniport driver as granted by NDIS
     */
    NDIS_HANDLE miniportDriverHandle;

    /**
     * Pointer to the driver object as given by the operating system
     */
    PDRIVER_OBJECT driverObject;

    #pragma region Miniport Handler Callbacks
    _IRQL_requires_(PASSIVE_LEVEL)
    _IRQL_requires_same_
    static NDIS_STATUS miniportInitializeExCallback(
        _In_ NDIS_HANDLE                    ndisMiniportHandle,
        _In_ NDIS_HANDLE                    miniportDriverContext,
        _In_ PNDIS_MINIPORT_INIT_PARAMETERS miniportInitParameters) noexcept;

    _IRQL_requires_(PASSIVE_LEVEL)
    _IRQL_requires_same_
    static void miniportHaltExCallback(
        _In_ NDIS_HANDLE        miniportAdapterContext,
        _In_ NDIS_HALT_ACTION   haltAction) noexcept;

    _IRQL_requires_(PASSIVE_LEVEL)
    _IRQL_requires_same_
    static NDIS_STATUS miniportPauseCallback(
        _In_ NDIS_HANDLE                     miniportAdapterContext,
        _In_ PNDIS_MINIPORT_PAUSE_PARAMETERS miniportPauseParameters);

    _IRQL_requires_(PASSIVE_LEVEL)
    _IRQL_requires_same_
    static NDIS_STATUS miniportRestartCallback(
        _In_ NDIS_HANDLE                       miniportAdapterContext,
        _In_ PNDIS_MINIPORT_RESTART_PARAMETERS miniportRestartParameters);

    _IRQL_requires_(PASSIVE_LEVEL)
    _IRQL_requires_same_
    static NDIS_STATUS miniportOidRequestCallback(
        _In_ NDIS_HANDLE            miniportAdapterContext,
        _In_ PNDIS_OID_REQUEST      oidRequest);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _IRQL_requires_same_
    static void miniportSendNetBufferListsCallback(
        _In_ NDIS_HANDLE        miniportAdapterContext,
        _In_ PNET_BUFFER_LIST   netBufferList,
        _In_ NDIS_PORT_NUMBER   portNumber,
        _In_ ULONG              sendFlags);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _IRQL_requires_same_
    static void miniportReturnNetBufferListsCallback(
        _In_ NDIS_HANDLE        miniportAdapterContext,
        _In_ PNET_BUFFER_LIST   netBufferLists,
        _In_ ULONG              returnFlags);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _IRQL_requires_same_
    static void miniportCancelSendCallback(
        _In_ NDIS_HANDLE miniportAdapterContext,
        _In_ PVOID       cancelId);

    _IRQL_requires_(PASSIVE_LEVEL)
    _IRQL_requires_same_
    static BOOLEAN miniportCheckForHangExCallback(
        _In_ NDIS_HANDLE miniportAdapterContext);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _IRQL_requires_same_
    static NDIS_STATUS miniportResetExCallback(
        _In_  NDIS_HANDLE miniportAdapterContext,
        _Out_ PBOOLEAN    addressingReset);

    _IRQL_requires_(PASSIVE_LEVEL)
    _IRQL_requires_same_
    static void miniportDevicePnpEventNotifyCallback(
        _In_ NDIS_HANDLE            miniportAdapterContext,
        _In_ PNET_DEVICE_PNP_EVENT  netDevicePnpEvent);

    _When_(shutdownAction == NdisShutdownPowerOff, _IRQL_requires_(PASSIVE_LEVEL))
    _When_(shutdownAction == NdisShutdownBugCheck, _IRQL_requires_(HIGH_LEVEL))
    _IRQL_requires_same_
    static void miniportShutdownExCallback(
        _In_ NDIS_HANDLE            miniportAdapterContext,
        _In_ NDIS_SHUTDOWN_ACTION   shutdownAction);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _IRQL_requires_same_
    static void miniportCancelOidRequestCallback(
        _In_ NDIS_HANDLE miniportAdapterContext,
        _In_ PVOID       requestId);

    _IRQL_requires_(PASSIVE_LEVEL)
    _IRQL_requires_same_
    static void miniportDriverUnloadCallback(
        _In_ PDRIVER_OBJECT driverObject) noexcept;

    #pragma endregion
    #pragma region Miniport Handler Member Functions
    _IRQL_requires_(PASSIVE_LEVEL)
    _IRQL_requires_same_
    NDIS_STATUS miniportInitializeEx(
        _In_ PNDIS_MINIPORT_INIT_PARAMETERS initParameters) noexcept;
    #pragma endregion

    /**
     * Structure which maps adapter objects to their interface numbers. All
     * adapters should be present in non-paged memory.
     */
    struct Adapter
    {
        AX25Adapter* adapter;
        NET_IFINDEX adapterNumber;
        bool inUse;
    } adapters[16];

    /**
     * Finds the first adapter in the adapter list that matches the criterion specified. If
     * no matching adapter is found, nullptr is returned.
     * @param criterion a function or function-object which will be called on each adapter in
     * order. This function should return true for a match, or false otherwise.
     * @returns the first adapter that matches the criterion specified, or nullptr if no adapter
     * matches the criterion.
     */
    template <class Function>
    _Ret_maybenull_
    _Must_inspect_result_
    inline Adapter* findFirstMatchingAdapter(_In_ Function criterion) noexcept
    {
        for (Adapter& adapter : adapters)
        {
            if (criterion(adapter))
            {
                return &adapter;
            }
        }

        // Couldn't find any matching adapter - return nullptr
        return nullptr;
    }


    /**
     * The tag to use when allocating a Miniport object in the non-pageable pool. In memory
     * this should appear as "axMP", little-endian.
     */
    static const ULONG MINIPORT_TAG =
        ('a' << 0 ) |
        ('x' << 8 ) |
        ('M' << 16) |
        ('P' << 24);

    /**
     * The currently active Miniport object that has been allocated. Only one Miniport
     * object can currently be active at a time, as it represents the global state of the
     * driver. Preferably this would not be required, but various Miniport functions
     * do not send along the context, most notably MiniportDriverUnload.
     */
    static Miniport* activeContext;

};



