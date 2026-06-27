#pragma once

#include <condition_variable>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "Process.h"
#include "Config.h"
#include "Scheduler.h"

class ProcessRegistry;
struct AppState;

class FCFSScheduler : public Scheduler {

public:
	explicit FCFSScheduler(int cores) ;
	~FCFSScheduler() override;

protected:
	void workerLoop(int coreId) override;
};