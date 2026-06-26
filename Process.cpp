#include "Process.h"

#include <fstream>

Process::Process(const std::string& processName, int processId, int numInstructions)
    : pid(processId),
      name(processName),
      totalInstructions(numInstructions),
      remainingInstructions(numInstructions),
      instructionCounter(0),
      outputFileName(processName + ".txt")
      
{
    std::ofstream file(outputFileName, std::ios::trunc);
    if (file.is_open()) {
        file << "Process name: " << name << '\n';
        file << "Logs:\n\n";
    }
}

void Process::addInstruction(const std::shared_ptr<ICommand>& command)
{
    instructionList.push_back(command);
}

void Process::executeCurrentInstruction(int coreId)
{
    const int currentInstruction = instructionCounter.load();
    if (hasFinished() || currentInstruction < 0 || currentInstruction >= static_cast<int>(instructionList.size())) {
        return;
    }

    auto& instruction = instructionList[currentInstruction];
    if (instruction) {
        instruction->execute(coreId, name, outputFileName);
    }
}

void Process::moveToNextInstruction()
{
    if (remainingInstructions.load() > 0) {
        remainingInstructions.fetch_sub(1);
    }

    if (instructionCounter.load() < static_cast<int>(instructionList.size())) {
        instructionCounter.fetch_add(1);
    }
}

int Process::getRemainingInstructions() const
{
    return remainingInstructions.load();
}

bool Process::hasFinished() const
{
    return remainingInstructions.load() <= 0 || instructionCounter.load() >= static_cast<int>(instructionList.size());
}

int Process::getPID() const
{
    return pid;
}

void Process::setPID(int newPid)
{
    pid = newPid;
}

const std::string& Process::getName() const
{
    return name;
}

int Process::getTotalInstructions() const
{
    return totalInstructions;
}

int Process::getCompletedInstructions() const
{
    return totalInstructions - remainingInstructions.load();
}

const std::string& Process::getOutputFileName() const
{
    return outputFileName;
}

SymbolTable& Process::getSymbolTable()
{
    return symbolTable;
}
