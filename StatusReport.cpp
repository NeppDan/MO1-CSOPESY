#include "StatusReport.h"

#include "ProcessRegistry.h"

#include <iomanip>
#include <sstream>

namespace {
std::string yesNo(bool value)
{
    return value ? "Finished" : "Running";
}
}

std::string StatusReport::buildScreenList(const AppState& appState)
{
    std::ostringstream output;

    output << "----------------------------------------\n";
    output << "root:> screen -ls\n\n";

    if (!appState.initialized || appState.config == nullptr || appState.registry == nullptr) {
        output << "Please run initialize first.\n";
        output << "----------------------------------------\n";
        return output.str();
    }

    output << "CPU ticks: " << appState.cpuCycles.load() << "\n";

    const auto processes = appState.registry->getAllProcesses();
    const int cpuCount = appState.config->numCpu;

    int runningCount = 0;
    for (const auto& p : processes) {
        if (!p.finished) {
            runningCount++;
        }
    }

    const int usedCores = runningCount > cpuCount ? cpuCount : runningCount;
    const int availableCores = cpuCount - usedCores;
    const int cpuUtilization = cpuCount == 0 ? 0 : (usedCores * 100 / cpuCount);

    output << "CPU utilization: " << cpuUtilization << "%\n";
    output << "Cores used: " << usedCores << "\n";
    output << "Cores available: " << availableCores << "\n\n";

    output << "Running processes:\n";
    bool hasRunning = false;
    for (const auto& process : processes) {
        if (!process.finished) {
            hasRunning = true;
            output << process.name
                   << "  (" << process.timestamp << ")"
                   << "    Core: " << process.coreId
                   << "    " << process.completedInstructions
                   << " / " << process.totalInstructions << '\n';
        }
    }
    if (!hasRunning) {
        output << "None\n";
    }

    output << "\nFinished processes:\n";
    bool foundFinished = false;
    for (const auto& process : processes) {
        if (process.finished) {
            foundFinished = true;
            output << process.name
                   << "  (" << process.timestamp << ")"
                   << "    " << yesNo(process.finished)
                   << "    " << process.completedInstructions
                   << " / " << process.totalInstructions << '\n';
        }
    }
    if (!foundFinished) {
        output << "None\n";
    }

    output << "----------------------------------------\n";
    return output.str();
}