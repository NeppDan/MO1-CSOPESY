#pragma once

#include "ICommand.h"
#include "SymbolTable.h"

#include <string>

class PrintCommand : public ICommand
{
public:
    // PRINT("literal message") - e.g. default "Hello world from <process_name>!"
    PrintCommand(int pid, std::string message, SymbolTable& symbolTable);

    // PRINT("prefix" + var) - e.g. PRINT("Value from: " + x)
    PrintCommand(int pid, std::string message, std::string varName, SymbolTable& symbolTable);

    void execute(int coreId, const std::string& processName, const std::string& outputFile) override;

private:
    std::string message;
    std::string varName = "";
    SymbolTable& symbolTable;
};