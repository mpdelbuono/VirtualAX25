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
 * @file AX25AdapterTests.cpp
 * Unit tests for the Virtual AX.25 NDIS Driver AX25Adapter class
 * @author Matthew P. Del Buono (KG7UDH)
 */
#include "pch.h"
#include "KernelMocks.h"
#include "AX25Adapter.h"

TEST(AX25Adapter, InvalidAllocations)
{
    EXPECT_EQ(nullptr, new(nullptr) AX25Adapter);
}

TEST(AX25Adapter, ValidAllocations)
{
    void* const DRIVER_HANDLE = reinterpret_cast<void*>(0x10203040A0B0C0D0ULL);
    void* const MEMORY_PTR = reinterpret_cast<void*>(0x010203040A0B0C0DULL);
    KernelMockData::NdisAllocateMemoryWithTagPriority_Result = MEMORY_PTR;
    EXPECT_EQ(MEMORY_PTR, new(DRIVER_HANDLE) AX25Adapter);
    EXPECT_EQ(DRIVER_HANDLE, KernelMockData::NdisAllocateMemoryWithTagPriority_Arguments.NdisHandle);
}

TEST(AX25Adapter, DeleteNullptr)
{
    AX25Adapter* ptr = nullptr;
    void* const DRIVER_HANDLE = reinterpret_cast<void*>(0x10203040A0B0C0D0ULL);
    EXPECT_NO_THROW(ptr->operator delete(ptr, DRIVER_HANDLE));
}

TEST(AX25Adapter, ThrowIfBadDriverHandle)
{   
    AX25Adapter* ptr = reinterpret_cast<AX25Adapter*>(0x010203040A0B0C0DULL);
    void* const DRIVER_HANDLE = reinterpret_cast<void*>(0x10203040A0B0C0D0ULL);
    KernelMockData::NdisFreeMemoryWithTagPriority_CallCount = 0;
    
    // There should be no call to deallocate, and a throw should occur
    EXPECT_DEATH(ptr->operator delete(ptr, nullptr), "") << "deletion of AX25Adapter with nullptr as driver handle should cause a structured exception";
    EXPECT_EQ(0, KernelMockData::NdisFreeMemoryWithTagPriority_CallCount);
}

TEST(AX25Adapter, ValidDeallocation)
{
    AX25Adapter* ptr = reinterpret_cast<AX25Adapter*>(0x010203040A0B0C0DULL);
    void* const DRIVER_HANDLE = reinterpret_cast<void*>(0x10203040A0B0C0D0ULL);
    KernelMockData::NdisFreeMemoryWithTagPriority_CallCount = 0;

    EXPECT_NO_THROW(ptr->operator delete(ptr, DRIVER_HANDLE));
    EXPECT_EQ(1, KernelMockData::NdisFreeMemoryWithTagPriority_CallCount);
    EXPECT_EQ(DRIVER_HANDLE, KernelMockData::NdisFreeMemoryWithTagPriority_Arguments.NdisHandle);
}