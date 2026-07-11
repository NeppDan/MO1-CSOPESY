#include "RoundRobinScheduler.h"

#include <chrono>
#include <thread>

RoundRobinScheduler::RoundRobinScheduler(int cores, int quantumCycles)
	: Scheduler(cores), quantum(quantumCycles)
{
}

RoundRobinScheduler::~RoundRobinScheduler()
{
	stop();
}

void RoundRobinScheduler::workerLoop(int coreId)
{
	auto& worker = workers[coreId];
	while (true) {
		std::shared_ptr<Process> process;
		{
			std::unique_lock<std::mutex> workerLock(worker->mutex);
			worker->cv.wait(workerLock, [&]() {
				return worker->hasWork || worker->stopRequested;
				});

			if (worker->stopRequested) {
				break;
			}

			process = worker->assignedProcess;
		}

		if (process) {
			int executionDelayMs = 0;
			if (appState != nullptr && appState->config != nullptr) {
				executionDelayMs = appState->config->delayPerExec;
			}
			if (executionDelayMs <= 0) {
				executionDelayMs = 20;
			}

			int executed = 0;
			while (executed < quantum && !process->hasFinished()) {
				process->executeCurrentInstruction(coreId);
				process->moveToNextInstruction();
				updateProcessState(process, ProcessState::Running, coreId);
				std::this_thread::sleep_for(std::chrono::milliseconds(executionDelayMs));
				++executed;
			}

			if (process->hasFinished()) {
				updateProcessState(process, ProcessState::Finished, coreId);
				releaseProcessMemory(process->getPID());
			}
			else {
				updateProcessState(process, ProcessState::Waiting, -1);
				std::lock_guard<std::mutex> queueLock(queueMutex);
				readyQueue.push_back(process); // back of the line
			}

			// A quantum cycle has just elapsed on this core; snapshot the
			// overall memory state per the memory manager spec.
			writeMemorySnapshot();
		}

		{
			std::lock_guard<std::mutex> workerLock(worker->mutex);
			worker->assignedProcess.reset();
			worker->hasWork = false;
		}
		queueCv.notify_all();
	}
}