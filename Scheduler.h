#pragma once


#include "AppState.h"
#include "Process.h"

#include <condition_variable>
#include <deque>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class ProcessRegistry;
struct AppState;

class Scheduler {
protected: 
	enum class ProcessState {
		Waiting,
		Finished,
		Running
	};

	struct ProcessSnapshot {
		std::shared_ptr<Process> process;
		ProcessState state;
		int coreId;
		std::string stateTimestamp;
	};

	struct CoreWorker {
		std::thread thread;
		std::mutex mutex;
		std::condition_variable cv;
		std::shared_ptr<Process> assignedProcess;
		bool hasWork = false;
		bool stopRequested = false;
	};

	int numCores;
	std::deque<std::shared_ptr<Process>> readyQueue;
	std::vector<std::unique_ptr<CoreWorker>> workers;
	std::thread schedulerThread;
	std::thread tickThread;
	std::mutex queueMutex;
	std::condition_variable queueCv;
	mutable std::mutex stateMutex;
	std::map<int, ProcessSnapshot> snapshots;
	bool acceptingWork;
	bool stopRequested;
	ProcessRegistry* registry = nullptr;
	AppState* appState = nullptr;

	void schedulerLoop();
	void tickLoop();
	void updateProcessState(const std::shared_ptr<Process>& process, ProcessState state, int coreId);
	static std::string buildTimestamp();

	// For FCFS
	virtual void workerLoop(int coreId) = 0;


public:
	explicit Scheduler(int cores);
	virtual ~Scheduler() = default; 

	void setRegistry(ProcessRegistry* reg);
	void setAppState(AppState* state);
	void addProcess(const std::shared_ptr<Process>& process);
	void start();
	void stop();
	bool isFinished() const;
	std::string screenLs() const;
};


	