#include "FlatMemoryAllocator.h"

FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize) : maximumSize(maximumSize), allocatedSize(0)
{
	memory.resize(maximumSize);
	allocationMap.resize(maximumSize);
	initializeMemory();
}

FlatMemoryAllocator::~FlatMemoryAllocator() {
	memory.clear();
}

void* FlatMemoryAllocator::allocate(size_t size) {
	if (size == 0 || size > maximumSize) {
		return nullptr;
	}

	for (size_t i = 0; i <= maximumSize - size; ++i) {
		if (!allocationMap[i] && canAllocateAt(i, size)) {
			allocateAt(i, size);
			return &memory[i];
		}
	}

	return nullptr;
}

void FlatMemoryAllocator::deallocate(void* ptr) {
	size_t index = static_cast<char*>(ptr) - &memory[0];
	if (index < allocationMap.size() && allocationMap[index]) {
		deallocateAt(index);
	}
}

std::string FlatMemoryAllocator::visualizeMemory() {
	return std::string(memory.begin(), memory.end());
}

void FlatMemoryAllocator::initializeMemory() {
	std::fill(memory.begin(), memory.end(), '.'); //'.' represents free memory
	std::fill(allocationMap.begin(), allocationMap.end(), false);
}

bool FlatMemoryAllocator::canAllocateAt(size_t index, size_t size) const {
	return (index + size <= maximumSize);
}

size_t FlatMemoryAllocator::getMaxSize() const {
	return maximumSize;
}

size_t FlatMemoryAllocator::addressOf(void* ptr) const {
	return static_cast<size_t>(static_cast<char*>(ptr) - &memory[0]);
}

void FlatMemoryAllocator::allocateAt(size_t index, size_t size) {
	std::fill(allocationMap.begin() + index, allocationMap.begin() + index + size, true);
	std::fill(memory.begin() + index, memory.begin() + index + size, '#'); // '#' represents used memory
	allocationSizes[index] = size;
	allocatedSize += size;
}

void FlatMemoryAllocator::deallocateAt(size_t index) {
	auto it = allocationSizes.find(index);
	if (it == allocationSizes.end()) {
		return;
	}
	size_t size = it->second;

	std::fill(allocationMap.begin() + index, allocationMap.begin() + index + size, false);
	std::fill(memory.begin() + index, memory.begin() + index + size, '.');
	allocatedSize -= size;
	allocationSizes.erase(it);
}