#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "AppState.h"

class Process;

class ProcessRegistry {
private:
    mutable std::mutex mutex;
    std::vector<ProcessStatusEntry> processes;
    int nextPid = 1;

public:
    ProcessRegistry() = default;

    int addProcess(const std::shared_ptr<Process>& process, const std::string& name);
    void updateRunning(int pid, int coreId, int completedInstructions);
    void markFinished(int pid, int completedInstructions);
    std::vector<ProcessStatusEntry> getAllProcesses() const;
    bool findByName(const std::string& name, ProcessStatusEntry& entry) const;
};
