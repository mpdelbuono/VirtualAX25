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
#include "pch.h"
#include "UniqueNonPageablePointer.h"

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
public:

    _Must_inspect_result_
    _IRQL_requires_(PASSIVE_LEVEL)
    NDIS_STATUS RegisterWithNdis(
        _In_  PDRIVER_OBJECT driverObject,
        _In_  PUNICODE_STRING registryPath) noexcept;

    _Must_inspect_result_
    _Ret_maybenull_
    static Miniport* GetInstance();

    _Must_inspect_result_
    _Ret_maybenull_
    static Miniport* TryGetInstance() noexcept;
private:
    friend class UniqueNonPageablePointer<Miniport>; // Allow UniqueNonPageablePointer to call the constructor for allocation
    Miniport() noexcept;

    /**
     * Singleton instance of this Miniport. The memory will be automatically freed upon shutdown as necessary.
     */
    static UniqueNonPageablePointer<Miniport> instance;

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
    static NDIS_STATUS miniportInitializeEx(
        _In_ NDIS_HANDLE                    ndisMiniportHandle,
        _In_ NDIS_HANDLE                    miniportDriverContext,
        _In_ PNDIS_MINIPORT_INIT_PARAMETERS miniportInitParameters);

    _IRQL_requires_(PASSIVE_LEVEL)
    _IRQL_requires_same_
    static void miniportHaltEx(
        _In_ NDIS_HANDLE        miniportAdapterContext,
        _In_ NDIS_HALT_ACTION   haltAction);

    _IRQL_requires_(PASSIVE_LEVEL)
    _IRQL_requires_same_
    static NDIS_STATUS miniportPause(
        _In_ NDIS_HANDLE                     miniportAdapterContext,
        _In_ PNDIS_MINIPORT_PAUSE_PARAMETERS miniportPauseParameters);

    _IRQL_requires_(PASSIVE_LEVEL)
    _IRQL_requires_same_
    static NDIS_STATUS miniportRestart(
        _In_ NDIS_HANDLE                       miniportAdapterContext,
        _In_ PNDIS_MINIPORT_RESTART_PARAMETERS miniportRestartParameters);

    _IRQL_requires_(PASSIVE_LEVEL)
    _IRQL_requires_same_
    static NDIS_STATUS miniportOidRequest(
        _In_ NDIS_HANDLE            miniportAdapterContext,
        _In_ PNDIS_OID_REQUEST      oidRequest);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _IRQL_requires_same_
    static void miniportSendNetBufferLists(
        _In_ NDIS_HANDLE        miniportAdapterContext,
        _In_ PNET_BUFFER_LIST   netBufferList,
        _In_ NDIS_PORT_NUMBER   portNumber,
        _In_ ULONG              sendFlags);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _IRQL_requires_same_
    static void miniportReturnNetBufferLists(
        _In_ NDIS_HANDLE        miniportAdapterContext,
        _In_ PNET_BUFFER_LIST   netBufferLists,
        _In_ ULONG              returnFlags);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _IRQL_requires_same_
    static void miniportCancelSend(
        _In_ NDIS_HANDLE miniportAdapterContext,
        _In_ PVOID       cancelId);

    _IRQL_requires_(PASSIVE_LEVEL)
    _IRQL_requires_same_
    static BOOLEAN miniportCheckForHangEx(
        _In_ NDIS_HANDLE miniportAdapterContext);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _IRQL_requires_same_
    static NDIS_STATUS miniportResetEx(
        _In_  NDIS_HANDLE miniportAdapterContext,
        _Out_ PBOOLEAN    addressingReset);

    _IRQL_requires_(PASSIVE_LEVEL)
    _IRQL_requires_same_
    static void miniportDevicePnpEventNotify(
        _In_ NDIS_HANDLE            miniportAdapterContext,
        _In_ PNET_DEVICE_PNP_EVENT  netDevicePnpEvent);

    _When_(shutdownAction == NdisShutdownPowerOff, _IRQL_requires_(PASSIVE_LEVEL))
    _When_(shutdownAction == NdisShutdownBugCheck, _IRQL_requires_(HIGH_LEVEL))
    _IRQL_requires_same_
    static void miniportShutdownEx(
        _In_ NDIS_HANDLE            miniportAdapterContext,
        _In_ NDIS_SHUTDOWN_ACTION   shutdownAction);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _IRQL_requires_same_
    static void miniportCancelOidRequest(
        _In_ NDIS_HANDLE miniportAdapterContext,
        _In_ PVOID       requestId);

    _IRQL_requires_(PASSIVE_LEVEL)
    _IRQL_requires_same_
    static void miniportDriverUnload(
        _In_ PDRIVER_OBJECT driverObject);

#pragma endregion

};



