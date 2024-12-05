#pragma once
#include <list>
#include <windows.h>
#include <intrin.h>

typedef struct RS_MATRIXPAGE
{
	uint64_t d_0;

	uint8_t pad[56];
	BYTE data[4032];

} MATRIXPAGE;

#define IS_MATRIXPAGE_FULL(A) ((A->d_0 & 0x7FFFFFFFFFFFFFFF) == 0)

namespace rasticore
{

	LPVOID RsAlloc(SIZE_T size);

	class RsMatrixAllocator
	{
	public:
		std::list<LPVOID> actPagePool;
		//std::list<LPVOID> fulPagePool;

		MATRIXPAGE* cPage;
		DWORD pgAllocSize;

		RsMatrixAllocator() = default;
		RsMatrixAllocator(DWORD s_alloc);
		void inline InitMatrixPage(MATRIXPAGE* mp);

		LPVOID AllocMatrix();
		void FreeMatrix(LPVOID mat);
		void AllocNewMatrixPage(DWORD cnt);

	};
}

template<typename T>
class STACK
{
public:

	size_t c_size;
	T* base_ptr;

	STACK() {}

	STACK(size_t e_size)
	{
		base_ptr = new T[e_size];
		c_size = 0;
	}

	void put(T o)
	{
		*(base_ptr + c_size) = o;
		c_size++;
	}

	T* alloc()
	{
		c_size++;
		return base_ptr + (c_size - 1);
	}

	T fget()
	{
		if (c_size == 0)
			return *base_ptr;
		return *(base_ptr + (c_size - 1));
	}

	void pop()
	{
		c_size--;
	}

	size_t getSize()
	{
		return c_size;
	}

};

template<typename T>
class DynamicList
{
public:
	T* base_ptr;
	uint32_t c_size;
	uint32_t reserved;

	DynamicList() {}

	DynamicList(uint32_t _size)
	{
		c_size = 0;
		reserved = _size;
		base_ptr = new T[_size];
	}

	void push_back(T obj)
	{
		if (c_size == reserved)
		{
			reserve(reserved * 2);
		}
		base_ptr[c_size] = obj;
		c_size++;
	}

	T at(uint32_t index)
	{
		return base_ptr[index];
	}

	T* atp(uint32_t index)
	{
		return base_ptr + index;
	}

	void reserve(uint32_t n_size)
	{
		T* o_base_ptr = base_ptr;
		base_ptr = new T[n_size];
		memcpy(base_ptr, o_base_ptr, c_size * sizeof(T));
		delete[] o_base_ptr;
	}

	void del_last()
	{
		c_size--;
	}
};
