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
 * @file VS2015Printer.h
 * Defines a test listener that matches Visual Studio's output format
 * @author Matthew P. Del Buono (KG7UDH)
 */
#pragma once

/**
 * Implementation of an EmptyTestEventListener which overrides appropriate functions
 * according to the following desired output behavior:
 *      - Passing test cases are not displayed
 *      - A failure that occurs provides output in the output window which
 *        can be double clicked to take the developer to the line at which it failed
 *      - The first line indicates the random seed being used for test shuffling
 *      - The final line is a judgement of pass or failure and the number of unit tests
 *        executed, total failures, and the amount of time they took to run
 */
class VS2015Printer :
    public testing::EmptyTestEventListener
{
public:
    void OnTestPartResult(const testing::TestPartResult& test) override;
    void OnTestProgramEnd(const testing::UnitTest& unitTest) override;
};

