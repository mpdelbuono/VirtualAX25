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
 * @file AX25AdapterMock.h
 * Provides a mock implementation of an AX25Adapter object
 * @author Matthew P. Del Buono (KG7UDH)
 */

#pragma once

#include "gmock/gmock.h"
#include "AX25Adapter.h"

namespace Mocks
{
    /**
     * Mock implementation of an ::AX25Adapter object
     */
    class AX25Adapter : public ::AX25Adapter
    {
    public:
        inline AX25Adapter() noexcept :
            ::AX25Adapter(nullptr) {}

        // Need to use GMOCK_METHOD0_ here because we need to specify noexcept
        GMOCK_METHOD0_(, noexcept, , Destroy, void());
    };
}
