#include <iostream>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>

#include "Config.h"
#include "AppState.h"
#include "StatusReport.h"
#include "ProcessRegistry.h"
#include "ProcessFactory.h"
#include "FCFSScheduler.h"
#include "RoundRobinScheduler.h"
#include "FlatMemoryAllocator.h"
namespace {

std::string trim(const std::string& str)
{
    const size_t first = str.find_first_not_of(" \t\r\n");
    const size_t last = str.find_last_not_of(" \t\r\n");
    return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
}

std::string stripQuotes(const std::string& value)
{
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        return value.substr(1, value.size() - 2);
    }
    return value;
}

std::string toLower(std::string value)
{
    for (char& ch : value) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return value;
}

bool parseConfig(const std::string& fileName, Config& config, std::string& errorMessage)
{
    std::ifstream file(fileName);
    if (!file.is_open()) {
        errorMessage = "Unable to open file: " + fileName;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line.front() == '#') {
            continue;
        }

        std::istringstream parser(line);
        std::string key;
        std::string value;
        parser >> key >> value;

        if (key.empty() || value.empty()) {
            continue;
        }

        key = toLower(key);
        value = stripQuotes(value);

        try {
            if (key == "num-cpu") {
                config.numCpu = std::stoi(value);
            } else if (key == "scheduler") {
                config.scheduler = toLower(value);
            } else if (key == "quantum-cycles") {
                config.quantumCycles = std::stoi(value);
            } else if (key == "batch-process-freq") {
                config.batchProcessFreq = std::stoi(value);
            } else if (key == "min-ins") {
                config.minIns = std::stoi(value);
            } else if (key == "max-ins") {
                config.maxIns = std::stoi(value);
            } else if (key == "delay-per-exec" || key == "delays-per-exec") {
                config.delayPerExec = std::stoi(value);
            } else if (key == "max-overall-mem"){
                config.maxOverallMem = std::stoull(value);
            } else if (key == "mem-per-frame"){
                config.memPerFrame = std::stoull(value);
            } else if (key == "mem-per-proc"){
                config.memPerProc = std::stoull(value);
            }
        } catch (const std::exception&) {
            errorMessage = "Invalid value in config.txt for key: " + key;
            return false;
        }
    }

    if (config.numCpu < 1 || config.numCpu > 128) {
        errorMessage = "num-cpu must be between 1 and 128.";
        return false;
    }
    if (config.scheduler != "fcfs" && config.scheduler != "rr") {
        errorMessage = "scheduler must be either fcfs or rr.";
        return false;
    }
    if (config.quantumCycles < 1) {
        errorMessage = "quantum-cycles must be at least 1.";
        return false;
    }
    if (config.batchProcessFreq < 1) {
        errorMessage = "batch-process-freq must be at least 1.";
        return false;
    }
    if (config.minIns < 1 || config.maxIns < config.minIns) {
        errorMessage = "Instruction bounds in config.txt are invalid.";
        return false;
    }
    if (config.delayPerExec < 0) {
        errorMessage = "delay-per-exec must be 0 or greater.";
        return false;
    }
    if (config.maxOverallMem < 1) {
        errorMessage = "max-overall-mem must be at least 1.";
        return false;
    }
    if (config.memPerFrame < 1) {
        errorMessage = "mem-per-frame must be at least 1.";
        return false;
    }
    if (config.memPerProc < 1 || config.memPerProc > config.maxOverallMem) {
        errorMessage = "mem-per-proc must be between 1 and max-overall-mem.";
        return false;
    }

    return true;
}

void printBanner()
{
    std::cout << "            .-')                   _ (`-.    ('-.    .-')                \n"
    << "           ( OO ).                ( (OO  ) _(  OO)  ( OO ).              \n"
    << "   .-----.(_)---\\_) .-'),-----.  _.`     \\(,------.(_)---\\_)  ,--.   ,--.\n"
    << "  '  .--.//    _ | ( OO'  .-.  '(__...--'' |  .---'/    _ |    \\  `.'  / \n"
    << "  |  |('-.\\  :` `. /   |  | |  | |  /  | | |  |    \\  :` `.  .-')     /  \n"
    << " /_) |OO  )'..`''.)\\_) |  |\\|  | |  |_.' |(|  '--.  '..`''.)(OO  \\   /   \n"
    << " ||  |`-'|.-._)   \\  \\ |  | |  | |  .___.' |  .--' .-._)   \\ |   /  /\\_  \n"
    << "(_'  '--'\\\\       /   `'  '-'  ' |  |      |  `---.\\       / `-./  /.__) \n"
    << "   `-----' `-----'      `-----'  `--'      `------' `-----'    `--'      \n";

    std::cout << "-----------------------------------------------------------------------\n";
    std::cout << "Welcome to the CSOPESY Emulator!\n\n";
    std::cout << "Developers: \n";
    std::cout << "Casio, Johann\n";
    std::cout << "Mabeza, Noreen\n";
    std::cout << "Namuag, Leigh\n";
    std::cout << "Tiopes, Matthew\n\n";
    std::cout << "Last updated: March 24, 2026\n";
    std::cout << "-----------------------------------------------------------------------\n";
}

void printMenuPrompt()
{
    std::cout << "root:> ";
}

void clearConsole()
{
    std::cout << "\x1B[2J\x1B[H";
}

void enterAlternateScreen()
{
    std::cout << "\x1B[?1049h\x1B[2J\x1B[H";
}

void exitAlternateScreen()
{
    std::cout << "\x1B[?1049l";
}

std::string readFileContents(const std::string& fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open()) {
        return "";
    }

    std::ostringstream output;
    output << file.rdbuf();
    return output.str();
}

std::string readProcessLogs(const std::string& fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open()) {
        return "";
    }

    std::string line;
    int lineNumber = 0;
    std::ostringstream output;
    while (std::getline(file, line)) {
        ++lineNumber;
        if (lineNumber <= 2) {
            continue;
        }
        output << line << '\n';
    }

    return output.str();
}

void syncProcessCounter(ProcessRegistry& registry, int& processCounter)
{
    const auto processes = registry.getAllProcesses();
    for (const auto& process : processes) {
        if (process.name.size() >= 2 && process.name[0] == 'p') {
            const std::string suffix = process.name.substr(1);
            bool numeric = !suffix.empty();
            for (char ch : suffix) {
                if (!std::isdigit(static_cast<unsigned char>(ch))) {
                    numeric = false;
                    break;
                }
            }

            if (numeric) {
                const int numericId = std::stoi(suffix);
                if (numericId >= processCounter) {
                    processCounter = numericId + 1;
                }
            }
        }
    }
}

void printProcessSmi(const ProcessStatusEntry& entry)
{
    const std::string logFile = entry.name + ".txt";
    const std::string logs = readProcessLogs(logFile);

    std::cout << "Process name: " << entry.name << '\n';
    std::cout << "ID: " << entry.pid << '\n';
    std::cout << "Logs:\n";
    if (!logs.empty()) {
        std::cout << logs;
    }

    std::cout << "Current instruction line: " << entry.completedInstructions << '\n';
    std::cout << "Lines of code: " << entry.totalInstructions << '\n';
    if (entry.finished) {
        std::cout << "Finished!\n";
    }
}

void writeReportUtil(const AppState& appState)
{
    std::ofstream file("csopesy-log.txt", std::ios::trunc);
    if (!file.is_open()) {
        std::cout << "Unable to write csopesy-log.txt\n";
        return;
    }

    file << StatusReport::buildScreenList(appState);
    std::cout << "Report written to csopesy-log.txt\n";
}

void runProcessScreen(const std::string& processName, ProcessRegistry& registry, bool allowFinishedAccess)
{
    ProcessStatusEntry entry;
    if (!registry.findByName(processName, entry) || (!allowFinishedAccess && entry.finished)) {
        std::cout << "Process " << processName << " not found.\n";
        return;
    }

    enterAlternateScreen();
    std::cout << "Process screen: " << processName << "\n";

    std::string command;
    while (true) {
        printMenuPrompt();
        if (!std::getline(std::cin, command)) {
            break;
        }

        command = trim(command);
        if (command.empty()) {
            continue;
        }

        if (command == "exit") {
            exitAlternateScreen();
            break;
        }

        if (command == "process-smi") {
            ProcessStatusEntry currentEntry;
            if (!registry.findByName(processName, currentEntry)) {
                std::cout << "Process " << processName << " not found.\n";
                continue;
            }

            printProcessSmi(currentEntry);
        } else {
            std::cout << "Unrecognized command.\n";
        }
    }
}
}

namespace {
int processCounter = 1;
}

int main()
{
    Config config;
    ProcessRegistry registry;
    AppState appState;
    appState.config = &config;
    appState.registry = &registry;

    std::unique_ptr<FCFSScheduler> fcfsScheduler;
    std::unique_ptr<RoundRobinScheduler> rrScheduler;
    std::unique_ptr<IMemoryAllocator> memoryAllocator;
    bool useRoundRobin = false;
    bool initialized = false;
    bool schedulerRunning = false;

    auto startActiveScheduler = [&]() {
        if (schedulerRunning) {
            return;
        }
        if (useRoundRobin) {
            rrScheduler->start();
        } else {
            fcfsScheduler->start();
        }
        schedulerRunning = true;
    };

    auto stopActiveScheduler = [&]() {
        if (!schedulerRunning) {
            return;
        }
        if (useRoundRobin) {
            rrScheduler->stop();
        } else {
            fcfsScheduler->stop();
        }
        schedulerRunning = false;
    };

    auto addProcessToActiveScheduler = [&](const std::shared_ptr<Process>& process) {
        if (useRoundRobin) {
            rrScheduler->addProcess(process);
        } else {
            fcfsScheduler->addProcess(process);
        }
    };

    auto setBatchGenerationEnabled = [&](bool enabled) {
        if (useRoundRobin) {
            rrScheduler->setBatchGenerationEnabled(enabled);
        } else {
            fcfsScheduler->setBatchGenerationEnabled(enabled);
        }
    };

    printBanner();

    std::string command;
    while (true) {
        printMenuPrompt();
        if (!std::getline(std::cin, command)) {
            break;
        }

        command = trim(command);
        if (command.empty()) {
            continue;
        }

        if (command == "exit") {
            break;
        }

        if (command == "initialize") {
            std::string errorMessage;
            if (!parseConfig("config.txt", config, errorMessage)) {
                std::cout << "Initialization failed: " << errorMessage << '\n';
                continue;
            }

            initialized = true;
            appState.initialized = true;
            useRoundRobin = (config.scheduler == "rr");
            memoryAllocator = std::make_unique<FlatMemoryAllocator>(config.maxOverallMem);
            if (useRoundRobin) {
                rrScheduler = std::make_unique<RoundRobinScheduler>(config.numCpu, config.quantumCycles);
                rrScheduler->setRegistry(&registry);
                rrScheduler->setAppState(&appState);
                rrScheduler->setBatchGenerationEnabled(false);
                rrScheduler->setMemoryAllocator(memoryAllocator.get());
                fcfsScheduler.reset();
            } else {
                fcfsScheduler = std::make_unique<FCFSScheduler>(config.numCpu);
                fcfsScheduler->setRegistry(&registry);
                fcfsScheduler->setAppState(&appState);
                fcfsScheduler->setBatchGenerationEnabled(false);
                fcfsScheduler->setMemoryAllocator(memoryAllocator.get());
                rrScheduler.reset();
            }
            processCounter = 1;
            std::cout << "Initialized with " << config.numCpu << " CPU(s), scheduler " << config.scheduler << ".\n";
            continue;
        }

        if (!initialized) {
            std::cout << "Please run initialize first.\n";
            continue;
        }

        if (command == "screen -ls") {
            std::cout << StatusReport::buildScreenList(appState);
        } else if (command == "scheduler-start") {
            if ((useRoundRobin && !rrScheduler) || (!useRoundRobin && !fcfsScheduler)) {
                std::cout << "Scheduler not initialized.\n";
                continue;
            }
            setBatchGenerationEnabled(true);
            startActiveScheduler();
        } else if (command == "scheduler-stop") {
            if ((useRoundRobin && !rrScheduler) || (!useRoundRobin && !fcfsScheduler)) {
                std::cout << "Scheduler not initialized.\n";
                continue;
            }
            stopActiveScheduler();
            std::cout << "Scheduler stopped.\n";
        } else if (command == "report-util") {
            writeReportUtil(appState);
        } else if (command == "screen -s") {
            std::cout << "Use: screen -s <process name>\n";
        } else if (command.rfind("screen -s ", 0) == 0) {
            const std::string processName = trim(command.substr(10));
            if (processName.empty()) {
                std::cout << "Use: screen -s <process name>\n";
            } else {
                startActiveScheduler();
                syncProcessCounter(registry, processCounter);
                ProcessStatusEntry existingEntry;
                if (registry.findByName(processName, existingEntry)) {
                    std::cout << "Process " << processName << " already exists.\n";
                    continue;
                }

                auto process = ProcessFactory::createDummyProcess(processName, processCounter++, config);
                const int pid = registry.addProcess(process, processName);
                process->setPID(pid);
                addProcessToActiveScheduler(process);
                runProcessScreen(processName, registry, true);
            }
        } else if (command == "screen -r") {
            std::cout << "Use: screen -r <process name>\n";
        } else if (command.rfind("screen -r ", 0) == 0) {
            const std::string processName = trim(command.substr(10));
            if (processName.empty()) {
                std::cout << "Use: screen -r <process name>\n";
            } else {
                runProcessScreen(processName, registry, false);
            }
        } else {
            std::cout << "Unrecognized command.\n";
        }
    }

    return 0;
}

