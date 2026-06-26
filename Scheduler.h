#pragma once


#include "AppState.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstreaS>

class Scheduler {
public:
	Scheduler(int cores);

private: 
	void setRegistry();
	void setAppState();
	
	void addProcess();
	void start();
	void stop();

	void isFinished();
	void screenLS();
	void updateProcessState();

	void schedulerLoop();
	void tickLoop();

public: 
	int cores;

}

	