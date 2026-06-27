#pragma once


#include "Scheduler.h"

class FCFSScheduler : public Scheduler {
public:
	explicit FCFSScheduler(int cores);
	~FCFSScheduler() override;

protected:
	void workerLoop(int coreID) override;
};