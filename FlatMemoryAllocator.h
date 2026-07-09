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
}