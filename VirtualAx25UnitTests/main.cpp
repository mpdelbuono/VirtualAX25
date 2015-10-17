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
 * @file main.cpp
 * Defines the unit test driver entry point
 * @author Matthew P. Del Buono
 */
#include "pch.h"
#include "VS2015Printer.h"

/**
 * Entry point for the unit test driver application
 * @param argc the number of arguments passed to this application
 * @param argv an array of size argc+2, where element 0 is this
 * executable's name, the next argc arguments are the arguments passed in
 * along the command line, and argument argc+1 is nullptr
 * @returns 0 if all unit tests passed, or a non-zero exit code
 * otherwise
 */
int _tmain(int argc, TCHAR *argv[])
{
    // Set up gtest
    testing::InitGoogleTest(&argc, argv);
    
    // Remove the default listener and substitute our own
    testing::TestEventListeners& listeners = testing::UnitTest::GetInstance()->listeners();
    delete listeners.Release(listeners.default_result_printer());
    listeners.Append(new VS2015Printer);    // cleaned up by the framework

    // Execute the tests
    return RUN_ALL_TESTS();
}
