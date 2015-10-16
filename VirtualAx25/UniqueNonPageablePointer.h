/**
 * @file UniqueNonPageablePointer.h
 * Defines the UniqueNonPageablePointer object, which encapsulates a pointer to an object in
 * non-pageable memory. Use of RAII ensures the object will be deleted appropriately when it
 * leaves scope.
 * @author Matthew P. Del Buono (KG7UDH)
 */
#pragma once

/**
 * Representation of a pointer to a non-pageable object. The owner of this object is the owner
 * of the underlying memory. When this object falls out of scope, if ownership has not been passed
 * to another UniqueNonPageablePointer, the underlying memory will be deallocated.
 */
template <class T>
class UniqueNonPageablePointer
{
public:
	/**
	 * Creates a new UniqueNonPageablePointer pointing to nullptr
	 */
	inline UniqueNonPageablePointer() noexcept :pointer(nullptr) {};

	/**
	 * Creates a new UniqueNonPageablePointer pointing to the specified object
	 * @param initialPointer the object to which this newly created UniqueNonPageablePointer is pointing
	 */
	inline UniqueNonPageablePointer(T* initialPointer) noexcept :pointer(initialPointer) {};

	// Copying a unique pointer is forbidden
	UniqueNonPageablePointer(T&) = delete;
    UniqueNonPageablePointer(T const&) = delete;

	/**
	 * Moves the specified pointer into this new object, which takes over ownership.
	 * @param expiringValue the pointer from which this object is taking ownership
	 */
	inline UniqueNonPageablePointer(UniqueNonPageablePointer<T>&& expiringValue) noexcept {
		pointer = expiringValue.pointer;
		expiringValue.pointer = nullptr;
	}
	
	/**
	 * Destroys this UniqueNonPageablePointer, deallocating the underlying memory
	 * if ownership is still retained at time of the call.
	 */
	_IRQL_requires_max_(DISPATCH_LEVEL)
	inline ~UniqueNonPageablePointer()
	{
		destroy();
	}

	/**
	 * Checks to see if this UniqueNojnPageablePointer currently points to nullptr
	 * @returns true if this object points to nullptr, or false otherwise
	 */
	inline bool IsNull() const
	{
		return pointer == nullptr;
	}

	/**
	 * Deallocates the currently-pointed object and replaces it with the specified
	 * pointer.
	 * @param newPointer the address to which this object should point
	 */
	_IRQL_requires_max_(DISPATCH_LEVEL)
	inline UniqueNonPageablePointer<T> const& operator=(T* newPointer)
	{
		// Destroy/deallocate if necessary
		destroy();

		// Switch to the new pointer
		pointer = newPointer;
		
		return *this;
	}

	/**
	 * Deallocates the currently-pointed object and then moves ownership from the specified
	 * pointer to this object
	 * @param newPointer the object from which ownership will be transferred
	 */
	_IRQL_requires_max_(DISPATCH_LEVEL)
	inline UniqueNonPageablePointer<T> const& operator =(UniqueNonPageablePointer<T>&& newPointer)
	{
		// Destroy/deallocate if necessary
		destroy();

		// Move pointer ownership
		pointer = newPointer.pointer;
		newPointer.pointer = nullptr;
		
		return *this;
	}
    
    /**
     * Returns a pointer to the underlying object. This does not transfer ownership.
     * @returns a pointer to the underlying object
     */
    _Must_inspect_result_
    _Ret_maybenull_
    inline operator T*() noexcept
    {
        return pointer;
    }

    /**
     * Returns a pointer to the underlying object. This does not transfer ownership.
     * @returns a pointer to the underlying object
     */
    _Must_inspect_result_
    _Ret_maybenull_
    inline operator T const *() const noexcept
    {
        return pointer;
    }

	/**
	 * Creates a new UniqueNonPageablePointer pointing to a newly allocated
	 * object T which resides in non-pageable memory.
	 * @IRQL_MAX APC_LEVEL
	 * @throws NTSTATUS STATUS_NO_MEMORY if no non-paged NX memory is available to allocate
	 * this object. This is a structured exception.
	 */
	_IRQL_requires_max_(APC_LEVEL)
    _Maybe_raises_SEH_exception_ // when out of memory
	template <class... Args>
	static UniqueNonPageablePointer<T> Allocate(Args... args)
	{
		void* memory = ExAllocatePoolWithTagPriority(NonPagedPoolNx, sizeof(T), TAG, NormalPoolPriority);
		if (memory == NULL)
		{
			// The request failed - out of memory
			ExRaiseStatus(STATUS_NO_MEMORY);
		}

		// Execute the constructor
		__try
		{
            // Use placement new since the memory has already been allocated accordingly
            UniqueNonPageablePointer<T> pointer(new(memory) T(args...));
            return pointer;
		}
		__finally 
		{
            if (AbnormalTermination())
            {
                ExFreePool(memory);
            }
		}
	}
private:
	T* pointer;
	static constexpr ULONG TAG =
		('a' << 0)  |
		('x' << 8)  |
		('U' << 16) |
		('P' << 24);

	/** 
	 * Destroys and deallocates the pointer to which this object is currently pointing
	 */
	_IRQL_requires_max_(DISPATCH_LEVEL)
	inline void destroy()
	{
		// Destroy/deallocate if necessary
		if (pointer != nullptr)
		{
			pointer->~T();
			ExFreePoolWithTag(pointer, TAG);

			// Set to nullptr so we don't try to double-free later
			pointer = nullptr;
		}
	}
};

