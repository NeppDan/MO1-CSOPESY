#include "ForCommand.h"

ForCommand::ForCommand(int pid, std::vector<std::shared_ptr<ICommand>> instructions, int repeats)
	: ICommand(pid, ICommand::FOR), instructions(std::move(instructions)), repeats(repeats)
{
}

void ForCommand::execute(int coreId, const std::string& processName, const std::string& outputFile)
{
	for (int repeatIndex = 0; repeatIndex < repeats; ++repeatIndex) {
		for (const auto& instruction : instructions) {
			if (instruction) {
				instruction->execute(coreId, processName, outputFile);
			}
		}
	}
}