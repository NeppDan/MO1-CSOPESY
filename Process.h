#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "ICommand.h"
#include "IMemoryAllocator.h"
#include "SymbolTable.h"

class Process {
private:
	int pid;
	std::string name;
	int totalInstructions;
	std::atomic<int> remainingInstructions;
	std::atomic<int> instructionCounter;
	std::string outputFileName;

	SymbolTable symbolTable;
	std::vector<std::shared_ptr<ICommand>> instructionList;

	size_t memoryRequired;


public:
	Process(const std::string& processName, int processId, int numInstructions, size_t memoryRequired);

	void addInstruction(const std::shared_ptr<ICommand>& command);
	void executeCurrentInstruction(int coreId);
	void moveToNextInstruction();
	int getRemainingInstructions() const; 
	bool hasFinished() const;
	int getPID() const;
	void setPID(int newPid);
	const std::string& getName() const;
	int getTotalInstructions() const;
	int getCompletedInstructions() const;
	const std::string& getOutputFileName() const;
	void setTotalInstructions(int totalInstructionsCount);

	int getCurrentInstructionSleepTicks() const;
	size_t getMemoryRequired() const;

	SymbolTable& getSymbolTable();
};


