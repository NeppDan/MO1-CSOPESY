#include "SubtractCommand.h"
#include "Timestamp.h"

#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <utility>
#include <cstdint>
#include <tuple>


// SUBTRACT(var1, var2, var3)
SubtractCommand::SubtractCommand(int pid, std::string var1, std::string var2, std::string var3, SymbolTable& symbolTable)
    : ICommand(pid, SUBTRACT), var1(std::move(var1)), var2(std::move(var2)), var3(std::move(var3)), symbolTable(symbolTable)
{
}

// SUBTRACT(var1, var2, val)
SubtractCommand::SubtractCommand(int pid, std::string var1, std::string var2, uint16_t val3, SymbolTable& symbolTable)
    : ICommand(pid, ADD), var1(std::move(var1)), var2(std::move(var2)), val3(std::move(val3)), symbolTable(symbolTable)
{
}

// SUBTRACT(var1, val, var3)
SubtractCommand::SubtractCommand(int pid, std::string var1, uint16_t val2, std::string var3, SymbolTable& symbolTable)
    : ICommand(pid, ADD), var1(std::move(var1)), val2(std::move(val2)), var3(std::move(var3)), symbolTable(symbolTable)
{
}

// SUBTRACT(var1, val, val)
SubtractCommand::SubtractCommand(int pid, std::string var1, uint16_t val2, uint16_t val3, SymbolTable& symbolTable)
    : ICommand(pid, ADD), var1(std::move(var1)), val2(std::move(val2)), val3(std::move(val3)), symbolTable(symbolTable)
{
}
std::tuple<uint16_t, uint16_t, uint16_t> SubtractCommand::performOperation()
{
    uint16_t diff, op2, op3;
    if (this->var2 != ""){
        if (!this->symbolTable.hasVariable(var2)){ // var2 is not declared
            this->symbolTable.setVariable(var2, 0); // declared with value 0
        }
        op2 = this->symbolTable.getVariable(this->var2); 
    }

    if (this->var3 != ""){
        if (!this->symbolTable.hasVariable(var3)){ // var3 is not declared
            this->symbolTable.setVariable(var3, 0); // declared with value 0
        }
        op3 = this->symbolTable.getVariable(this->var3);
    }

    if (this->var2 == ""){
        op2 = this->val2;
    }

    if (this->var3 == ""){
        op3 = this->val3;
    }

    int16_t neg_check = static_cast<int16_t>(op2) - static_cast<int16_t>(op3);
    if (neg_check < 0){
        diff = 0;
    } else {
        diff = static_cast<uint16_t>(neg_check);
    }

    this->symbolTable.setVariable(var1, diff);

    return {op2, op3, diff};
}

void SubtractCommand::execute(int coreId, const std::string& processName, const std::string& outputFile)
{  
    auto [op2, op3, diff] = performOperation();

    std::ofstream out(outputFile, std::ios::app);
    if (out.is_open()) {
        out << "(" << buildTimestamp() << ") Core:" << coreId << " Executing SUBTRACT command for PID " << this->pid
            << ": variable " << this->var1 << ", subtracting values " << op2 << " and " << op3 << " to equal " << diff << "\n";
    }
}