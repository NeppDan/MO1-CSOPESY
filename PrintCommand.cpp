#include "PrintCommand.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>

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