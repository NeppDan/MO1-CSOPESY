#include "AddCommand.h"

#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <utility>
#include <cstdint>
#include <tuple>

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

// ADD(var1, var2, var3)
AddCommand::AddCommand(int pid, std::string var1, std::string var2, std::string var3, SymbolTable& symbolTable)
    : ICommand(pid, ADD), var1(std::move(var1)), var2(std::move(var2)), var3(std::move(var3)), symbolTable(symbolTable)
{
}

// ADD(var1, var2, val)
AddCommand::AddCommand(int pid, std::string var1, std::string var2, uint16_t val3, SymbolTable& symbolTable)
    : ICommand(pid, ADD), var1(std::move(var1)), var2(std::move(var2)), val3(std::move(val3)), symbolTable(symbolTable)
{
}

// ADD(var1, val, var3)
AddCommand::AddCommand(int pid, std::string var1, uint16_t val2, std::string var3, SymbolTable& symbolTable)
    : ICommand(pid, ADD), var1(std::move(var1)), val2(std::move(val2)), var3(std::move(var3)), symbolTable(symbolTable)
{
}

// ADD(var1, val, val)
AddCommand::AddCommand(int pid, std::string var1, uint16_t val2, uint16_t val3, SymbolTable& symbolTable)
    : ICommand(pid, ADD), var1(std::move(var1)), val2(std::move(val2)), val3(std::move(val3)), symbolTable(symbolTable)
{
}
std::tuple<uint16_t, uint16_t, uint16_t> AddCommand::performOperation()
{
    uint16_t sum, op2, op3;
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

    sum = op2 + op3;
    this->symbolTable.setVariable(var1, sum);

    return {op2, op3, sum};
}

void AddCommand::execute(int coreId, const std::string& processName, const std::string& outputFile)
{  
    auto [op2, op3, sum] = performOperation();

    std::stringstream msg;
    msg << buildTimestamp() << "Core: " << coreId << " Executing ADD command for PID " << this->pid 
    << ": variable " << this->var1 << ", adding values " << op2 << " and " << op3 << " to equal " << sum << std::endl;
    std::cout << msg.str();
}