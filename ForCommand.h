#pragma once

#include "ICommand.h"

#include <memory>
#include <vector>

class ForCommand : public ICommand
{
public:
	ForCommand(int pid, std::vector<std::shared_ptr<ICommand>> instructions, int repeats);

	void execute(int coreId, const std::string& processName, const std::string& outputFile) override;

private:
	std::vector<std::shared_ptr<ICommand>> instructions;
	int repeats;
};