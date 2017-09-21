#include "StackAllocator.h"
#include <inttypes.h>
#include <stdlib.h>
namespace SE
{
	namespace Utilz
	{
		StackAllocator::StackAllocator()
		{

		}

		StackAllocator::~StackAllocator()
		{
			free(stackBeg);
		}

		void StackAllocator::InitStackAlloc(size_t amountOfMemory)
		{
			stackBeg = malloc(amountOfMemory);
			stackCurrent = stackBeg;
			stackSize = amountOfMemory;
		}

		void* StackAllocator::GetMemoryAligned(size_t requestedMemSize, size_t alignment)
		{
			if (!(alignment >= 1))
			{
				return nullptr;
			}
			if (!(alignment <= 128))
			{
				return nullptr;
			}
			if (!((alignment & (alignment - 1)) == 0))
			{
				return nullptr;
			}

			//tot size to allocate
			size_t expandSize_bytes = requestedMemSize + alignment;

			//alloct unaligned, convert to uintptr_t
			void* rawVoidptr = GetMemUnaligned(expandSize_bytes);
			if (rawVoidptr == nullptr)
			{
				return nullptr;
			}
			uintptr_t rawAddress = reinterpret_cast<uintptr_t>(rawVoidptr);
			
			//determine how misaligned it is, (by masking of lower bits)
			size_t mask = (alignment - 1);
			uintptr_t misalignmnet = (rawAddress & mask);
			ptrdiff_t adjustment = alignment - misalignmnet;

			//calc adjusted address
			uintptr_t alignedAddress = rawAddress + adjustment;
			
			//store adjustment in byte before the aligned ptr
			if (!(adjustment < 256))
			{
				return nullptr;
			}
			uint8_t* pAlignedMem = reinterpret_cast<uint8_t*>(alignedAddress);
			pAlignedMem[-1] = static_cast<uint8_t>(adjustment);
			
			return static_cast<void*>(pAlignedMem);
		}

		void StackAllocator::ClearStackAlloc()
		{
			stackCurrent = stackBeg;
			occupiedSize = 0;
		}

		void* StackAllocator::GetMemUnaligned(size_t requestedMemSize)
		{
			if (occupiedSize + requestedMemSize > stackSize)
			{
				return nullptr;
			}
			occupiedSize += requestedMemSize;
			void* newMemPtr = stackCurrent;
			stackCurrent = (char*)stackCurrent + requestedMemSize;
			return newMemPtr;
		}
	}
}