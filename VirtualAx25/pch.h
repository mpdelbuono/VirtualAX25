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
 * @file pch.h
 * Precompiled header for the Virtual AX.25 Driver project
 * @author Matthew P. Del Buono (KG7UDH)
 */
#pragma once

extern "C" {

#pragma warning(push)
#pragma warning(disable:4201) // nonstandard extension used: nameless struct/union
#include <ndis.h>
#pragma warning(pop)
}

#include <ntddk.h>
#include <wdf.h>
#include <cstddef>

// Because <new> cannot be included because of conflicts with VS's /kernel option,
// we define global placement new here
inline void* operator new(size_t, void* pointer)
{
    return pointer;
}

// Because <utility> is not available in the DDK, we implement std::move here
template<class T>
struct RemoveReference
{   
    typedef T Type;
};

template<class T>
struct RemoveReference<T&>
{  
    typedef T Type;
};

template<class T>
struct RemoveReference<T&&>
{ 
    typedef T Type;
};

template<class T> 
inline typename RemoveReference<T>::Type&& std_move(T&& arg)
{   
    return (typename RemoveReference<T>::Type&&) arg;
}
