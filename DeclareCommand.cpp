#include "DeclareCommand.h"

#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <utility>
#include <cstdint>

namespace {
std::string buildTimestamp()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};
    localtime_s(&localTime, &nowTime);

    std::ostringstream builder;
    builder << std::put_time(&localTime, "%m/%d/%Y %I:%M:%S%p");
    return builder.str();
}
}

DeclareCommand::DeclareCommand(int pid, std::string varName, uint16_t value, SymbolTable& symbolTable)
    : ICommand(pid, DECLARE), varName(std::move(varName)), value(std::move(value)), symbolTable(symbolTable)
{
}

void DeclareCommand::performDeclaration()
{
    this->symbolTable.setVariable(varName, value);
}

void DeclareCommand::execute(int coreId, const std::string& processName, const std::string& outputFile)
{
    performDeclaration();

    std::stringstream msg;
    msg << buildTimestamp() << "Core: " << coreId << " Executing DECLARE command for PID " << this->pid 
    << ": variable " << this->varName << " with value " << this->value << std::endl;
    std::cout << msg.str();
}