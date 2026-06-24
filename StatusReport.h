#pragma once

#include <string>

#include "AppState.h"

class StatusReport {
public:
    static std::string buildScreenList(const AppState& appState);
};