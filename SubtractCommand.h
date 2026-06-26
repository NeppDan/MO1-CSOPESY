#pragma once

#include "ICommand.h"
#include "SymbolTable.h"

#include <string>
#include <cstdint>

class SubtractCommand : public ICommand
{
public:
    // SUBTRACT(var1, var2, var3)
    SubtractCommand(int pid, std::string var1, std::string var2, std::string var3, SymbolTable& symbolTable);

    // SUBTRACT(var1, var2, val)
    SubtractCommand(int pid, std::string var1, std::string var2, uint16_t val3, SymbolTable& symbolTable);

    // SUBTRACT(var1, val, var3)
    SubtractCommand(int pid, std::string var1, uint16_t val2, std::string var3, SymbolTable& symbolTable);

    // SUBTRACT(var1, val, val)
    SubtractCommand(int pid, std::string var1, uint16_t val2, uint16_t val3, SymbolTable& symbolTable);
    
    void execute(int coreId, const std::string& processName, const std::string& outputFile) override;
    std::tuple<uint16_t, uint16_t, uint16_t> performOperation();
private:
    std::string var1;
    std::string var2 = "";
    std::string var3 = "";
    uint16_t val2 = 0;
    uint16_t val3 = 0;
    SymbolTable& symbolTable;
};