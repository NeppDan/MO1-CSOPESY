#pragma once

#include <string>

struct Config {
    int numCpu = 1;
    std::string scheduler = "fcfs";
    int quantumCycles = 1;
    int batchProcessFreq = 1;
    int minIns = 1;
    int maxIns = 1;
    int delayPerExec = 0;
};