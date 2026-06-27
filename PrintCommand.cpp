#include "PrintCommand.h"
#include "Timestamp.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>



PrintCommand::PrintCommand(int pid, std::string message, SymbolTable& symbolTable)
    : ICommand(pid, ICommand::PRINT), message(std::move(message)), symbolTable(symbolTable)
{
}

PrintCommand::PrintCommand(int pid, std::string message, std::string varName, SymbolTable& symbolTable)
    : ICommand(pid, ICommand::PRINT), message(std::move(message)), varName(std::move(varName)), symbolTable(symbolTable)
{
}

void PrintCommand::execute(int coreId, const std::string& processName, const std::string& outputFile)
{
    std::string fullMessage = message;

    if (!varName.empty()) {
        if (!symbolTable.hasVariable(varName)) {
            symbolTable.setVariable(varName, 0);
        }
        fullMessage += std::to_string(symbolTable.getVariable(varName));
    }

    std::ofstream out(outputFile, std::ios::app);
    if (out.is_open()) {
        out << "(" << buildTimestamp() << ") Core:" << coreId << " \"" << fullMessage << "\"\n";
    }
}