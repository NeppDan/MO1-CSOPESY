#pragma once
#include <string>

class FlatMemoryAllocator
{
private:
	int id;
	string name;
	uint8_t size;

public:
	void allocate();
	void deallocate();

}