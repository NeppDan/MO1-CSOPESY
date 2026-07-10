#pragma once
#include <string>
#include "IMemoryAllocator.h"

class FlatMemoryAllocator : public IMemoryAllocator
{
public:
	FlatMemoryAllocator(size_t totalSize);
	~FlatMemoryAllocator();
	void* allocate(size_t size) override;
	void deallocate(void* ptr) override;
	std::string visualizeMemory() override;

private:
	size_t maximumSize;
	size_t allocatedSize;
	std::vector<char>memory;
	std::fill(allocationMap.begin(), allocationMap.end(), false);

	bool canAlocateAt(size_t index, size_t size) const;
	void allocateAt(size_t index, size_t size);
	void deallocateAt(size_t index);
}