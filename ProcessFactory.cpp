#include "ProcessFactory.h"

#include "Process.h"
#include "ICommand.h"

#include <ctime>
#include <iomanip>
#include <sstream>
#include <thread>
#include <chrono>
#include <fstream>

#include <cstdlib>
#include <utility>

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

std::shared_ptr<Process> ProcessFactory::createDummyProcess(
    const std::string& name,
    int index,
    const Config& config)
{
    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned>(time(nullptr)));
        seeded = true;
    }

    const int minIns = config.minIns;
    const int maxIns = config.maxIns;
    const int instructionCount = minIns + (rand() % (maxIns - minIns + 1));

    auto process = std::make_shared<Process>(name, index, instructionCount);

    // Create simple dummy instructions so the process has an instruction list
    struct DummyInstruction : public ICommand {
        int delayMs;
        std::string processName;

        DummyInstruction(int pid, int delay, std::string name)
            : ICommand(pid, ICommand::PRINT), delayMs(delay), processName(std::move(name)) {}

        void execute(int coreId, const std::string& /*processName*/, const std::string& outputFile) override {
            if (delayMs > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
            }
            std::ofstream out(outputFile, std::ios::app);
            if (out.is_open()) {
                out << "(" << buildTimestamp() << ") Core:" << coreId << " \"Hello world from " << processName << "!\"\n";
            }
        }
    };

    for (int i = 0; i < instructionCount; ++i) {
        process->addInstruction(std::make_shared<DummyInstruction>(index, config.delayPerExec, name));
    }

    return process;
}
