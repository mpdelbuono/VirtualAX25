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
public:
    _IRQL_requires_max_(DISPATCH_LEVEL)
    _Must_inspect_result_
    _Success_(size == sizeof(AX25Adapter))
    _Ret_maybenull_
    _Outptr_result_bytebuffer_(size)
    _Result_nullonfailure_
    void* operator new(_In_ size_t size,
                       _In_ NDIS_HANDLE driverHandle) noexcept;

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _When_(driverHandle == nullptr, _Raises_SEH_exception_)
    void operator delete(_In_opt_ void* pointer,
                         _In_ NDIS_HANDLE driverHandle) noexcept;
private:
    /**
     * The tag to use when allocating an AX25Adapter object in the non-pageable pool. In memory
     * this should appear as "axAX", little-endian.
     */
    static const ULONG AX25_ADAPTER_TAG = AX25_CREATE_TAG("axAX");
};

