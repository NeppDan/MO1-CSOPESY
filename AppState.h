#pragma once

#include <atomic>
#include <string>
#include <vector>

#include "Config.h"

class ProcessRegistry;

struct ProcessStatusEntry {
    std::string name;
    int pid = 0;
    int coreId = -1;
    int completedInstructions = 0;
    int totalInstructions = 0;
    bool finished = false;
    std::string timestamp;
};

struct AppState {
    bool initialized = false;
    Config* config = nullptr;
    ProcessRegistry* registry = nullptr;
    std::atomic<long long> cpuCycles{0};
};
