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
