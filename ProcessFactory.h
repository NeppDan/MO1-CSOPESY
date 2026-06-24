#pragma once

#include <memory>
#include <string>

#include "Config.h"

class Process;

class ProcessFactory {
public:
    static std::shared_ptr<Process> createDummyProcess(
        const std::string& name,
        int index,
        const Config& config);
};
