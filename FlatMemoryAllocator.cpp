
#include "FlatMemoryAllocator.h"

FlatMemoryAllocator::FlatMemoryAllocator(size_t maximumSize) :
	maximumsSize(maximumSize),
	memory.reserve(maximumSize),
	initializeMemory();
}

~FlatMemoryAllocator() {
	memory.clear();
}

void* allocate(size_t size) override {
	for (size_t i = 0; i <= maximumsSize - size; ++i) {
		if (!allocationMap[i] && canAllocateAt(1, size)) {
			allocateAt(i, size);
			return &memory[i];
		}
	}

	return nullptr;
}

void deallocate(void* ptr) override {
	size_t index = static_cast<char*>(ptr) - &memory[0];
	if (allocationMap[index]) {
		deallocateAt(index);
	}
}


std::string visualizeMemory() override {
	sreturn std::string(memory.begin(), memory.end());
}