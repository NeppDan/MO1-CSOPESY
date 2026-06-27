#pragma once

#include "Scheduler.h"

class RoundRobinScheduler : public Scheduler {
private:
	int quantum;

public:
	RoundRobinScheduler(int cores, int quantumCycles);
	~RoundRobinScheduler() override;

protected:
	void workerLoop(int coreId) override;
};