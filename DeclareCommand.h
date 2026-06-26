#pragma once

#include "ICommand.h"
#include "SymbolTable.h"

#include <string>
#include <cstdint>

class DeclareCommand : public ICommand
{
public:
    DeclareCommand(int pid, std::string varName, uint16_t value, SymbolTable& symbolTable);
    void execute(int coreId, const std::string& processName, const std::string& outputFile) override;
    void performDeclaration();
private:
    std::string varName;
    uint16_t value;
    SymbolTable& symbolTable;
};