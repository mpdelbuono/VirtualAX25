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
* @file KernelMocks.cpp
* Provides mocks to replace NDIS/Kernel calls where appropriate to allow for unit testing.
* This is done by various macros which inject themselves into where
* NDIS would be located in driver header files and by forcibly linking
* to a different symbol which matches. While this is intrusive,
* this is most straightforward way to get kernel-mode code testable in
* a user-mode application.
* @author Matthew P. Del Buono (KG7UDH)
*/
#include "pch.h"
#include "KernelMocks.h"


extern "C"
{
    KERNEL_MOCK_DEF(void*, NdisAllocateMemoryWithTagPriority,
                     NDIS_HANDLE, NdisHandle,
                     UINT, Length,
                     ULONG, Tag,
                     EX_POOL_PRIORITY, Priority)
    {

    }

    KERNEL_MOCK_DEF(void, NdisFreeMemoryWithTagPriority,
                    NDIS_HANDLE, NdisHandle,
                    void*, VirtualAddress,
                    ULONG, Tag)
    {

    }

    KERNEL_MOCK_DEF(void, ExRaiseStatus,
                    NTSTATUS, Status)
    {
        // Desirable to actually raise the exception in most cases because we can catch that
        RaiseException(args.Status, 0, 0, NULL);
    }

    KERNEL_MOCK_DEF(NDIS_STATUS, NdisMSetMiniportAttributes,
                    NDIS_HANDLE, NDisMiniportAdapterHandle,
                    NDIS_MINIPORT_ADAPTER_ATTRIBUTES*, MiniportAttributes)
    {

    }
    
    KERNEL_MOCK_DEF(void, __imp_KeInitializeDpc,
                    KDPC*, Dpc,
                    void*, DeferredRoutine,
                    void*, DeferredContext)
    {

    }
    
    // This is an actual kernel API, so it's a little weird. We do this raw.
    // Goal is to implement __stdcall for the __imp_ExRaiseStatus function, but
    // __stdcall doesn't follow that naming convention. Fortunately, the calling convention
    // doesn't matter significantly because we immediately jump out due to the exception.
    void __imp_ExRaiseStatus(DWORD status)
    {
        // Argument passing convention: By value
        // Stack-maintenance repsonsibility: Called function pops own arguments
        
        // Don't need to do anything special here. The goal is to raise an exception, so just do it
        RaiseException(status, 0, 0, NULL);
    }

    // Same kind of problem as __imp_ExRaiseStatus
    void __imp_KeBugCheckEx(DWORD code,
                            void* p1, void* p2, void* p3, void* p4)
    {
        // Raising an exception is the closest we can do to simulating a blue screen
        RaiseException(code, 0, 0, NULL);
    }

    // WppAutoLogTrace is a debug-only function that is not of interest to the unit test system.
    // Because it is __cdecl, we can just discard it by linking it to nothing.
    void WppAutoLogTrace() {}



}