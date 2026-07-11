#include "FCFSScheduler.h"

#include <chrono>
#include <thread>


FCFSScheduler::FCFSScheduler(int cores)
	: Scheduler(cores)
{
}

FCFSScheduler::~FCFSScheduler()
{
	stop();
}
void FCFSScheduler::workerLoop(int coreId)
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

			while (!process->hasFinished()) {
				process->executeCurrentInstruction(coreId);
				process->moveToNextInstruction();
				updateProcessState(process, ProcessState::Running, coreId);
				std::this_thread::sleep_for(std::chrono::milliseconds(executionDelayMs));
			}

			updateProcessState(process, ProcessState::Finished, coreId);
			releaseProcessMemory(process->getPID());
		}

		{
			std::lock_guard<std::mutex> workerLock(worker->mutex);
			worker->assignedProcess.reset();
			worker->hasWork = false;
		}
		queueCv.notify_all();
	}
}