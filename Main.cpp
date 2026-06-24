//#include "Scheduler.h"
//#include "PrintCommand.h"

#include <iostream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

#include <fstream>
#include <sstream>
#include <map>
/*
* 
* namespace {
    std::shared_ptr<Process> buildProcess(int index)
    {
        std::ostringstream nameBuilder;
        nameBuilder << "process_" << std::setfill('0') << std::setw(2) << (index + 1);

        auto process = std::make_shared<Process>(nameBuilder.str(), index + 1, 100);
        for (int commandIndex = 1; commandIndex <= 100; ++commandIndex) {
            process->addInstruction(std::make_shared<PrintCommand>(process->getPID(), "print command " + std::to_string(commandIndex)));
        }

        return process;
    }

*/



    // Helper function to trim whitespace
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t");
        size_t last = str.find_last_not_of(" \t");
        return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
    }

    std::map<std::string, std::string> parseConfig(const std::string& fileName) {
        std::ifstream file(fileName);
        std::map<std::string, std::string> config;
        std::string line;

        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file: " + fileName);
        }

        while (std::getline(file, line)) {
            // Ignore empty lines or comments
            if (line.empty() || line[0] == '#') continue;

            // Split line into key and value
            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = trim(line.substr(0, delimiterPos));
                std::string value = trim(line.substr(delimiterPos + 1));
                config[key] = value;
            }
        }


    }


int main()
{
    int cores;


    //Scheduler scheduler(4);

    for (int processIndex = 0; processIndex < 10; ++processIndex) {
        //scheduler.addProcess(buildProcess(processIndex));
    }


    std::cout << "Scheduler emulator ready. Type 'screen -ls' to inspect processes or 'exit' to close.\n";

    std::string command;
    while (std::getline(std::cin, command)) {
        if (command == "screen -ls") {
            //std::cout << scheduler.screenLs();
        }
        else if (command == "scheduler -start") {
            //scheduler.start();
        }
        else if (command == "scheduler -end") {
            //scheduler.stop();
        }
        else if (command == "report -util") {
            break;
        }
        else if (command == "exit") {
            break;
        }
        else if (!command.empty()) {
            std::cout << "Unknown command. Use 'screen -ls' or 'exit'.\n";
        }
        /*
                if (scheduler.isFinished()) {
            std::cout << "All processes have finished. You can type 'screen -ls' once more or 'exit'.\n";
        }
        */

    }


    return 0;
}

