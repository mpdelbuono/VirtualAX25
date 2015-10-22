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

class AX25AdapterFixture : public testing::Test
{
protected:
    // Required to use malloc() here so that we don't call the constructor.
    // Constructor operations are part of the test cases, and a placement new is being called accordingly.
    AX25AdapterFixture() :
        memory(malloc(sizeof(AX25Adapter)))
    {
        
    }

    ~AX25AdapterFixture()
    {
        free(memory);
    }

    void* memory;
    static constexpr void* DRIVER_HANDLE = reinterpret_cast<void*>(0x10203040A0B0C0D0ULL);


};

TEST_F(AX25AdapterFixture, InvalidAllocations)
{
    EXPECT_EQ(nullptr, new(nullptr) AX25Adapter(DRIVER_HANDLE));
}

TEST_F(AX25AdapterFixture, ValidAllocations)
{
    KernelMockData::NdisAllocateMemoryWithTagPriority_Result = memory;
    EXPECT_EQ(memory, new(DRIVER_HANDLE) AX25Adapter(DRIVER_HANDLE));
    EXPECT_EQ(DRIVER_HANDLE, KernelMockData::NdisAllocateMemoryWithTagPriority_Arguments.NdisHandle);
    EXPECT_EQ(sizeof(AX25Adapter), KernelMockData::NdisAllocateMemoryWithTagPriority_Arguments.Length);
    EXPECT_EQ(EX_POOL_PRIORITY::NormalPoolPriority, KernelMockData::NdisAllocateMemoryWithTagPriority_Arguments.Priority);
}

TEST_F(AX25AdapterFixture, DeleteNullptr)
{
    AX25Adapter* ptr = nullptr;
    EXPECT_NO_THROW(delete ptr);
}

TEST_F(AX25AdapterFixture, ValidDeallocation)
{
    AX25Adapter* ptr = new(DRIVER_HANDLE) AX25Adapter(DRIVER_HANDLE);
    KernelMockData::NdisFreeMemoryWithTagPriority_CallCount = 0;

    EXPECT_NO_THROW(delete ptr);
    EXPECT_EQ(1, KernelMockData::NdisFreeMemoryWithTagPriority_CallCount);
    EXPECT_EQ(DRIVER_HANDLE, KernelMockData::NdisFreeMemoryWithTagPriority_Arguments.NdisHandle);
    EXPECT_EQ(ptr, KernelMockData::NdisFreeMemoryWithTagPriority_Arguments.VirtualAddress);
}