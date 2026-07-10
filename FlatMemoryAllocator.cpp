
#include "FlatMemoryAllocator.h"

FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize) :
	maximumsSize(maximumSize),
	allocatedSize(0),
	memory.reserve(maximumSize),
	initializeMemory();
{}

FlatMemoryAllocator::~FlatMemoryAllocator() {
	memory.clear();
}

void FlatMemoryAllocator::allocate(size_t size){
	for (size_t i = 0; i <= maximumsSize - size; ++i) {
		if (!allocationMap[i] && canAllocateAt(1, size)) {
			allocateAt(i, size);
			return &memory[i];
		}
	}

	return nullptr;
}

void FlatMemoryAllocator::deallocate(void* ptr) {
	size_t index = static_cast<char*>(ptr) - &memory[0];
	if (allocationMap[index]) {
		deallocateAt(index);
	}
}


void FlatMemoryAllocator::visualizeMemory(){
	sreturn std::string(memory.begin(), memory.end());
}

void FlatMemoryAllocator::initializeMemory() {
	std::fill(memory.begin(), memory.end(), '.'); //'.' represents free memory
	std::fill(allocationMap.begin(), allocationMap.end(), false);
}

bool FlatMemoryAllocator::canAllocateAt(size_t index, size_t size) const {
	return (index + size <= maximumSize);
}

void FlatMemoryAllocator::allocateAt(size_t index, size_t size) {
	std::fill(allocationMap.begin() + index, allocationMap.begin() + index + size, true);
	allocatedSize += size;
}

void FlatMemoryAllocator::deallocateAt(size_t index) {
	allocationMap[index];
}