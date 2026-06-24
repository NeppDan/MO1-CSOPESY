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

    return true;
}

void printBanner()
{
    std::cout << "            .-')     _ (`-.                 ('-.    .-')                \n"
    << "           ( OO ).  ( (OO  )              _(  OO)  ( OO ).              \n"
    << "   .-----.(_)---\\_)_.`     \\ .-'),-----. (,------.(_)---\\_)  ,--.   ,--.\n"
    << "  '  .--.//    _ |(__...--''( OO'  .-.  ' |  .---'/    _ |    \\  `.'  / \n"
    << "  |  |('-.\\  :` `. |  /  | |/   |  | |  | |  |    \\  :` `.  .-')     /  \n"
    << " /_) |OO  )'..`''.)|  |_.' |\\_) |  |\\|  |(|  '--.  '..`''.)(OO  \\   /  \n"
    << " ||  |`-'|.-._)   \\|  .___.'  \\ |  | |  | |  .--' .-._)   \\ |   /  /\\_  \n"
    << "(_'  '--'\\\\       /|  |        `'  '-'  ' |  `---.\\       / `-./  /.__) \n"
    << "   `-----' `-----' `--'          `-----'  `------' `-----'    `--'      \n";

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

    std::unique_ptr<FCFSScheduler> scheduler;
    bool initialized = false;

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
            scheduler = std::make_unique<FCFSScheduler>(config.numCpu);
            scheduler->setRegistry(&registry);
            scheduler->setAppState(&appState);
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
            if (!scheduler) {
                std::cout << "Scheduler not initialized.\n";
                continue;
            }
            scheduler->start();
            for (int i = 0; i < 3; ++i) {
                std::ostringstream nameBuilder;
                nameBuilder << "p" << std::setfill('0') << std::setw(2) << processCounter++;
                auto process = ProcessFactory::createDummyProcess(nameBuilder.str(), i, config);
                int pid = registry.addProcess(process, nameBuilder.str());
                process->setPID(pid);
                scheduler->addProcess(process);
            }
            std::cout << "Scheduler started. Processes generated.\n";
        } else if (command == "scheduler-stop") {
            if (!scheduler) {
                std::cout << "Scheduler not initialized.\n";
                continue;
            }
            scheduler->stop();
            std::cout << "Scheduler stopped.\n";
        } else if (command == "report-util") {
            std::cout << "report-util command will be implemented soon.\n";
        } else if (command == "screen -s") {
            std::cout << "Use: screen -s <process name>\n";
        } else if (command.rfind("screen -s ", 0) == 0) {
            std::cout << "screen session creation will be added soon.\n";
        } else if (command == "screen -r") {
            std::cout << "Use: screen -r <process name>\n";
        } else if (command.rfind("screen -r ", 0) == 0) {
            std::cout << "screen session resume will be added soon.\n";
        } else {
            std::cout << "Unrecognized command.\n";
        }
    }

    return 0;
}

