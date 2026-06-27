#include <iostream>
#include <chrono>
#include <thread>

#include "SleepCommand.h"

SleepCommand::SleepCommand(int pid, uint8_t ticks)
	: ICommand(pid, ICommand::SLEEP), ticks(ticks)
{
}

void SleepCommand::execute(int coreId, const std::string& processName, const std::string& outputFile)
{
}

int SleepCommand::getSleepTicks() const
{
	return static_cast<int>(ticks);
}