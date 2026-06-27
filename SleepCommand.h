#pragma once

#include<ICommand.h>

class sleepCommand : public ICommand {
public:
	sleepCommand(int ticks);
	

};