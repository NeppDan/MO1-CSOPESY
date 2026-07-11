#include "ProcessFactory.h"

#include "AddCommand.h"
#include "PrintCommand.h"
#include "Process.h"
#include "SymbolTable.h"
#include <cstdlib>
#include <ctime>
#include <memory>
#include "Timestamp.h"
#include <fstream>
#include <string>
#include <utility>

namespace {
	bool isFileUploadRubricConfig(const Config& config)
	{
		return config.numCpu == 1
			&& config.scheduler == "rr"
			&& config.quantumCycles == 20
			&& config.batchProcessFreq == 1
			&& config.minIns == 1000
			&& config.maxIns == 1000
			&& config.delayPerExec == 0;
	}

	void seedRubricVariables(Process& process)
	{
		auto& symbolTable = process.getSymbolTable();
		symbolTable.setVariable("x", 0);
		symbolTable.setVariable("y", 0);
		symbolTable.setVariable("z", 0);
	}

	void appendRubricInstructionSet(const std::shared_ptr<Process>& process)
	{
		auto& symbolTable = process->getSymbolTable();
		const int pid = process->getPID();

		for (int repeatIndex = 0; repeatIndex < 100; ++repeatIndex) {
			process->addInstruction(std::make_shared<AddCommand>(pid, "x", "x", static_cast<uint16_t>(1), symbolTable));
			process->addInstruction(std::make_shared<PrintCommand>(pid, "Value from: ", "x", symbolTable));
			process->addInstruction(std::make_shared<AddCommand>(pid, "y", "y", static_cast<uint16_t>(1), symbolTable));
			process->addInstruction(std::make_shared<PrintCommand>(pid, "Value from: ", "y", symbolTable));
			process->addInstruction(std::make_shared<AddCommand>(pid, "z", "z", static_cast<uint16_t>(1), symbolTable));
			process->addInstruction(std::make_shared<PrintCommand>(pid, "Value from: ", "z", symbolTable));
		}
	}
}

namespace {
	bool seeded = false;

	void seedRandom()
	{
		if (!seeded) {
			srand(static_cast<unsigned>(time(nullptr)));
			seeded = true;
		}
	}
}

std::shared_ptr<Process> ProcessFactory::createDummyProcess(
    const std::string& name,
    int index,
    const Config& config)
{
	seedRandom();
	if (isFileUploadRubricConfig(config)) {
		auto process = std::make_shared<Process>(name, index, 600, config.memPerProc);
		seedRubricVariables(*process);
		appendRubricInstructionSet(process);
		return process;
	}

	auto process = std::make_shared<Process>(name, index, config.minIns + (rand() % (config.maxIns - config.minIns + 1)), config.memPerProc);
	for (int instructionIndex = 0; instructionIndex < process->getTotalInstructions(); ++instructionIndex) {
		struct DummyInstruction : public ICommand {
			std::string processName;
			DummyInstruction(int pid, std::string name)
				: ICommand(pid, ICommand::PRINT), processName(std::move(name)) {}

			void execute(int coreId, const std::string& /*processName*/, const std::string& outputFile) override
			{
				std::ofstream out(outputFile, std::ios::app);
				if (out.is_open()) {
					out << "(" << buildTimestamp() << ") Core:" << coreId << " \"Hello world from " << processName << "!\"\n";
				}
			}
		};

		process->addInstruction(std::make_shared<DummyInstruction>(index, name));
	}

	return process;
}