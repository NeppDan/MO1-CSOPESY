#include "ProcessRegistry.h"

#include "Process.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace {
std::string buildTimestamp()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm localTime{};
    localtime_s(&localTime, &time);

    std::ostringstream builder;
    builder << std::put_time(&localTime, "%m/%d/%Y %I:%M:%S%p");
    return builder.str();
}
}

int ProcessRegistry::addProcess(const std::shared_ptr<Process>& process, const std::string& name)
{
    std::lock_guard<std::mutex> lock(mutex);

    ProcessStatusEntry entry;
    entry.name = name;
    entry.pid = nextPid++;
    entry.coreId = -1;
    entry.totalInstructions = process->getTotalInstructions();
    entry.completedInstructions = 0;
    entry.finished = false;
    entry.timestamp = buildTimestamp();

    processes.push_back(entry);
    return entry.pid;
}

void ProcessRegistry::updateRunning(int pid, int coreId, int completedInstructions)
{
    std::lock_guard<std::mutex> lock(mutex);

    for (auto& entry : processes) {
        if (entry.pid == pid) {
            entry.coreId = coreId;
            entry.finished = false;
            entry.completedInstructions = completedInstructions;
            entry.timestamp = buildTimestamp();
            break;
        }
    }
}

void ProcessRegistry::markFinished(int pid, int completedInstructions)
{
    std::lock_guard<std::mutex> lock(mutex);

    for (auto& entry : processes) {
        if (entry.pid == pid) {
            entry.finished = true;
            entry.coreId = -1;
            entry.completedInstructions = completedInstructions;
            entry.timestamp = buildTimestamp();
            break;
        }
    }
}

std::vector<ProcessStatusEntry> ProcessRegistry::getAllProcesses() const
{
    std::lock_guard<std::mutex> lock(mutex);
    return processes;
}

bool ProcessRegistry::findByName(const std::string& name, ProcessStatusEntry& entry) const
{
    std::lock_guard<std::mutex> lock(mutex);

    for (const auto& process : processes) {
        if (process.name == name) {
            entry = process;
            return true;
        }
    }
    return false;
}
