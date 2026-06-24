#include "ProcessFactory.h"

#include "Process.h"
#include "ICommand.h"

#include <thread>
#include <chrono>
#include <fstream>

#include <cstdlib>
#include <ctime>
#include <sstream>

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
        DummyInstruction(int pid, int delay)
            : ICommand(pid, ICommand::PRINT), delayMs(delay) {}

        void execute(int coreId, const std::string& /*processName*/, const std::string& outputFile) override {
            if (delayMs > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
            }
            // Append a tiny log entry so process output exists
            std::ofstream out(outputFile, std::ios::app);
            if (out.is_open()) {
                out << "Executed on core " << coreId << "\n";
            }
        }
    };

    for (int i = 0; i < instructionCount; ++i) {
        process->addInstruction(std::make_shared<DummyInstruction>(index, config.delayPerExec));
    }

    return process;
}
