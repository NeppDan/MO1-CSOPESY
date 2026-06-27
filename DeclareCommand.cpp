#include "DeclareCommand.h"
#include "Timestamp.h"

#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <utility>
#include <cstdint>



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

    std::ofstream out(outputFile, std::ios::app);
    if (out.is_open()) {
        out << "(" << buildTimestamp() << ") Core:" << coreId << " Executing DECLARE command for PID " << this->pid
            << ": variable " << this->varName << " with value " << this->value << "\n";
    }
}