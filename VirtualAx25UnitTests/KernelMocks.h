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
 * @file KernelMocks.h
 * Provides mocks to replace NDIS/Kernel calls where appropriate to allow for unit testing.
 * This is done by various macros which inject themselves into where
 * NDIS would be located in driver header files. While this is intrusive,
 * this is most straightforward way to get kernel-mode code testable in
 * a user-mode application.
 * @author Matthew P. Del Buono (KG7UDH)
 */

#pragma once

#define NDIS_HANDLE void*

#pragma region Framework Code
/**
 * Forces macro expansion in cases of complex macros
 */
#define FORCE_EXPAND(x) x

/**
 * Works around a Visual Studio compiler bug where __VA_ARGS__ will be passed as a single parameter.
 * To call FOO(__VA_ARGS__) with this macro, call PASS_ARGLIST_TO_MACRO(FOO, (__VA_ARGS__))
 * (Note: the extra parentheses around __VA_ARGS__ are required as it is part of the workaround.)
 *
 * For additional info, see http://connect.microsoft.com/VisualStudio/feedback/details/380090/variadic-macro-replacement
 */
#define PASS_ARGLIST_TO_MACRO(macro, ...) macro __VA_ARGS__

 /**
  * Counts the number of arguments passed in
  */
#define COUNT_ARGS(...) FORCE_EXPAND(COUNT_ARGS_IMPL(0, __VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define COUNT_ARGS_IMPL(arg0, arg16, arg15, arg14, arg13, arg12, arg11, arg10, arg9, arg8, arg7, arg6, arg5, arg4, arg3, arg2, arg1, result, ...) result

/**
 * Enumerates the arguments separated by commas that are specified as variables, per KERNEL_MOCK_DECL
 */
#define KERNEL_MOCK_LIST_ARGUMENTS(...) KERNEL_MOCK_LIST_ARGUMENTS_##COUNT_ARGS(__VA_ARGS__)

// The following macros provide implementation for KERNEL_MOCK_ENUMERATE_ARGUMENTS, KERNEL_MOCK_LIST_ARGUMENTS, and KERNEL_MOCK_ASSIGN_ARGUMENTS
#define KERNEL_MOCK_ENUMERATE_ARGUMENTS_16(type1, var1, ...) type1 var1; PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_ENUMERATE_ARGUMENTS_14, (__VA_ARGS__))
#define KERNEL_MOCK_ENUMERATE_ARGUMENTS_14(type1, var1, ...) type1 var1; PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_ENUMERATE_ARGUMENTS_12, (__VA_ARGS__))
#define KERNEL_MOCK_ENUMERATE_ARGUMENTS_12(type1, var1, ...) type1 var1; PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_ENUMERATE_ARGUMENTS_10, (__VA_ARGS__))
#define KERNEL_MOCK_ENUMERATE_ARGUMENTS_10(type1, var1, ...) type1 var1; PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_ENUMERATE_ARGUMENTS_8, (__VA_ARGS__))
#define KERNEL_MOCK_ENUMERATE_ARGUMENTS_8(type1, var1, ...) type1 var1; PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_ENUMERATE_ARGUMENTS_6, (__VA_ARGS__))
#define KERNEL_MOCK_ENUMERATE_ARGUMENTS_6(type1, var1, ...) type1 var1; PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_ENUMERATE_ARGUMENTS_4, (__VA_ARGS__))
#define KERNEL_MOCK_ENUMERATE_ARGUMENTS_4(type1, var1, ...) type1 var1; PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_ENUMERATE_ARGUMENTS_2, (__VA_ARGS__))
#define KERNEL_MOCK_ENUMERATE_ARGUMENTS_2(type1, var1) type1 var1;
#define KERNEL_MOCK_ENUMERATE_ARGUMENTS_0() /* nothing to do */

#define KERNEL_MOCK_LIST_ARGUMENTS_16(type1, var1, ...) type1 var1, PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_LIST_ARGUMENTS_14, (__VA_ARGS__))
#define KERNEL_MOCK_LIST_ARGUMENTS_14(type1, var1, ...) type1 var1, PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_LIST_ARGUMENTS_12, (__VA_ARGS__))
#define KERNEL_MOCK_LIST_ARGUMENTS_12(type1, var1, ...) type1 var1, PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_LIST_ARGUMENTS_10, (__VA_ARGS__))
#define KERNEL_MOCK_LIST_ARGUMENTS_10(type1, var1, ...) type1 var1, PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_LIST_ARGUMENTS_8, (__VA_ARGS__))
#define KERNEL_MOCK_LIST_ARGUMENTS_8(type1, var1, ...) type1 var1, PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_LIST_ARGUMENTS_6, (__VA_ARGS__))
#define KERNEL_MOCK_LIST_ARGUMENTS_6(type1, var1, ...) type1 var1, PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_LIST_ARGUMENTS_4, (__VA_ARGS__))
#define KERNEL_MOCK_LIST_ARGUMENTS_4(type1, var1, ...) type1 var1, PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_LIST_ARGUMENTS_2, (__VA_ARGS__))
#define KERNEL_MOCK_LIST_ARGUMENTS_2(type1, var1) type1 var1
#define KERNEL_MOCK_LIST_ARGUMENTS_0() /* nothing to do */

#define KERNEL_MOCK_ASSIGN_ARGUMENTS_16(function, type1, var1, ...) function##_Arguments.##var1 = var1; PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_ASSIGN_ARGUMENTS_14, (function, __VA_ARGS__))
#define KERNEL_MOCK_ASSIGN_ARGUMENTS_14(function, type1, var1, ...) function##_Arguments.##var1 = var1; PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_ASSIGN_ARGUMENTS_12, (function, __VA_ARGS__))
#define KERNEL_MOCK_ASSIGN_ARGUMENTS_12(function, type1, var1, ...) function##_Arguments.##var1 = var1; PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_ASSIGN_ARGUMENTS_10, (function, __VA_ARGS__))
#define KERNEL_MOCK_ASSIGN_ARGUMENTS_10(function, type1, var1, ...) function##_Arguments.##var1 = var1; PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_ASSIGN_ARGUMENTS_8, (function, __VA_ARGS__))
#define KERNEL_MOCK_ASSIGN_ARGUMENTS_8(function, type1, var1, ...) function##_Arguments.##var1 = var1; PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_ASSIGN_ARGUMENTS_6, (function, __VA_ARGS__))
#define KERNEL_MOCK_ASSIGN_ARGUMENTS_6(function, type1, var1, ...) function##_Arguments.##var1 = var1; PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_ASSIGN_ARGUMENTS_4, (function, __VA_ARGS__))
#define KERNEL_MOCK_ASSIGN_ARGUMENTS_4(function, type1, var1, ...) function##_Arguments.##var1 = var1; PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_ASSIGN_ARGUMENTS_2, (function, __VA_ARGS__))
#define KERNEL_MOCK_ASSIGN_ARGUMENTS_2(function, type1, var1, ...) function##_Arguments.##var1 = var1; PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_ASSIGN_ARGUMENTS_0, (function, __VA_ARGS__))
#define KERNEL_MOCK_ASSIGN_ARGUMENTS_0(function)  /* nothing to do */

/**
 * Enumerates the arguments separated by semicolons that are specified as variables, per KERNEL_MOCK_DECL
 */
#define KERNEL_MOCK_ENUMERATE_ARGUMENTS_GET_NAME(n) KERNEL_MOCK_ENUMERATE_ARGUMENTS_##n
#define KERNEL_MOCK_ENUMERATE_ARGUMENTS_IMPL(n, ...) PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_ENUMERATE_ARGUMENTS_GET_NAME(n), (__VA_ARGS__))
#define KERNEL_MOCK_ENUMERATE_ARGUMENTS(...) KERNEL_MOCK_ENUMERATE_ARGUMENTS_IMPL(COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)

 /**
  * Enumerates the arguments separated by commas that are specified as variables, per KERNEL_MOCK_DECL
  */
#define KERNEL_MOCK_ENUMERATE_LIST_GET_NAME(n) KERNEL_MOCK_LIST_ARGUMENTS_##n
#define KERNEL_MOCK_ENUMERATE_LIST_IMPL(n, ...) PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_ENUMERATE_LIST_GET_NAME(n), (__VA_ARGS__))
#define KERNEL_MOCK_ENUMERATE_LIST(...) KERNEL_MOCK_ENUMERATE_LIST_IMPL(COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)

#define KERNEL_MOCK_ASSIGN_ARGUMENTS_GET_NAME(n) KERNEL_MOCK_ASSIGN_ARGUMENTS_##n
#define KERNEL_MOCK_ASSIGN_ARGUMENTS_IMPL(function, n, ...) PASS_ARGLIST_TO_MACRO(KERNEL_MOCK_ASSIGN_ARGUMENTS_GET_NAME(n), (function, __VA_ARGS__))
#define KERNEL_MOCK_ASSIGN_ARGUMENTS(function, ...) KERNEL_MOCK_ASSIGN_ARGUMENTS_IMPL(function, COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)

/**
 * Declares a mock implementation of the given kernel function which takes the specified arguments.
 * The mock function will store all of the supplied parameters to a structure
 * KernelMockData::##functionName##_Arguments with each element having the same name as the argument.
 * The return value upon call will be the current value of KernelMockData::##functionName##_Result,
 * which must be copyable.
 * @param returnType the return type of the kernel function
 * @param functionName the name of the function being mocked
 * @param ... a list of arguments to the function. The even numbered arguments (0-based)
 * should be the types of the arguments, and the odd numbered arguments (0-based) should
 * be the names of the arguments
 */
#define KERNEL_MOCK_DECL(returnType, functionName, ...)                                 \
    namespace KernelMockData {                                                          \
        struct functionName##_ArgumentData {                                            \
            KERNEL_MOCK_ENUMERATE_ARGUMENTS(__VA_ARGS__)                                \
        };                                                                              \
        extern functionName##_ArgumentData functionName##_Arguments;                    \
        extern KernelMock::KernelMockReturnType<returnType> functionName##_Result;      \
    }

/**
 * Implements a mock implementation of the given kernel function which takes the specified arguments.
 * The mock function will store all of the supplied parameters to a structure
 * KernelMockData::##functionName##_Arguments with each element having the same name as the argument.
 * The return value upon call will be the current value of KernelMockData::##functionName##_Result,
 * which must be copyable.
 *
 * Custom additional implementation can be added with the scope block that follows
 * KERNEL_MOCK_IMPL. This is required in all uses, even if it is blank. The implementation
 * can access the variable 'args' which will be a structure containing all of the 
 * parameters. This will be called after all arguments have been recorded and before the return
 * value is determined, so that information can be used and modified as necessary. Use
 * as follows:
 *
 * KERNEL_MOCK_IMPL(void*, KernelAlloc, size_t, size)
 * {
 *     // some custom implementation
 *     if (args.size > 4 * 1024 * 1024 * 1024) { 
 *       throw "test case illegal: beyond the limits of 32-bit systems"; 
 *     }
 * }
 * @param returnType the return type of the kernel function
 */
#define KERNEL_MOCK_DEF_IMPL(returnType, functionName, argumentList, argumentAssignments)           \
    static void functionName##MockImpl(const KernelMockData::##functionName##_ArgumentData& args);  \
    KernelMock::KernelMockReturnType<returnType> KernelMockData::##functionName##_Result;           \
    namespace KernelMockImpl {                                                                      \
        static KernelMockData::##functionName##_ArgumentData functionName##_Arguments;              \
        returnType functionName(argumentList)                                                       \
        {                                                                                           \
            argumentAssignments                                                                     \
            ::functionName##MockImpl(functionName##_Arguments);                                     \
            return KernelMockData::##functionName##_Result.get();                                   \
        }                                                                                           \
    }                                                                                               \
    static void functionName##MockImpl(const KernelMockData::##functionName##_ArgumentData& args)
                           
        

#define KERNEL_MOCK_DEF(returnType, functionName, ...) KERNEL_MOCK_DEF_IMPL(returnType, functionName, KERNEL_MOCK_ENUMERATE_LIST(__VA_ARGS__), KERNEL_MOCK_ASSIGN_ARGUMENTS(functionName, __VA_ARGS__))

namespace KernelMock
{
    /**
     * Represents a value which will be returned by a kernel function. This object
     * can be used as if it were an object of type T, with the exception that if
     * members of type T are desired, then operator -> should be used instead of operator .
     * @tparam T the type of the object to return
     */
    template <class T>
    class KernelMockReturnType
    {
    public:
        /**
         * Creates a new KernelMockReturnType<T> object with a default-initialized value
         */
        KernelMockReturnType() :value() {}

        /**
         * Creates a new KernelMockReturnType<T> object with the specified value
         */
        KernelMockReturnType(T const& initialValue) :value(initialValue) {}

        /**
         * Gets the value currently assigned to this return value object 
         * @returns the value currently assigned to this object
         */
        T get() const { return value; };

        /**
         * Assigns the return value to be the specified value
         * @param initialValue the value to return from the mock function
         */
        T const& operator =(T const& initialValue) { value = initialValue; return initialValue; };
    private:
        T value;
    };

    /**
     * Template specialization of KernelMockReturnType for void types. In these cases, 
     * no value is stored, so nothing can be assigned to this type. However, the mock
     * function expects a value to return. The get() function is declared as returning
     * type void because returning the result of a function of type void is legal, thus
     * allowing the KERNEL_MOCK_IMPL function to remain consistent for all functions.
     */
    template <>
    class KernelMockReturnType<void>
    {
    public:
        /** Does nothing but return, as there is no value to return */
        void get() {}
    };
    
}
#pragma endregion

enum EX_POOL_PRIORITY
{
    LowPoolPriority,
    LowPoolPrioritySpecialPoolOverrun = 8,
    LowPoolPrioritySpecialPoolUnderrun = 9,
    NormalPoolPriority = 16,
    NormalPoolPrioritySpecialPoolOverrun = 24,
    NormalPoolPrioritySpecialPoolUnderrun = 25,
    HighPoolPriority = 32,
    HighPoolPrioritySpecialPoolOverrun = 40,
    HighPoolPrioritySpecialPoolUnderrun = 41
};

KERNEL_MOCK_DECL(void*, NdisAllocateMemoryWithTagPriority,
                 NDIS_HANDLE, NdisHandle,
                 UINT, Length,
                 ULONG, Tag,
                 EX_POOL_PRIORITY, Priority);
KERNEL_MOCK_DECL(void, NdisFreeMemoryWithTagPriority,
                 NDIS_HANDLE, NdisHandle,
                 void*, VirtualAddress,
                 ULONG, Tag)
