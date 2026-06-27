#pragma once

#include "ICommand.h"


class SleepCommand : public ICommand {
public:
	SleepCommand(int pid, uint8_t ticks);

	void execute(int coreId, const std::string& processName, const std::string& outputFile) override;
	int getSleepTicks() const override;

private:
	uint8_t ticks;
};