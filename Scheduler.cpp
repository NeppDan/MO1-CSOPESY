#include "Scheduler.h"

#include "AppState.h"
#include "Config.h"
#include "ProcessFactory.h"
#include "ProcessRegistry.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <sstream>

Scheduler::Scheduler(int cores)
	: numCores(cores), acceptingWork(true), stopRequested(false)
{
	workers.reserve(numCores);
	for (int coreId = 0; coreId < numCores; ++coreId) {
		workers.push_back(std::make_unique<CoreWorker>());
	}
}

void Scheduler::setRegistry(ProcessRegistry* reg)
{
	registry = reg;
}

void Scheduler::setAppState(AppState* state)
{
	appState = state;
}

void Scheduler::setBatchGenerationEnabled(bool enabled)
{
	batchGenerationEnabled = enabled;
}

void Scheduler::addProcess(const std::shared_ptr<Process>& process)
{
	if (!process) {
		return;
	}

	{
		std::lock_guard<std::mutex> lock(queueMutex);
		if (!acceptingWork) {
			return;
		}
		readyQueue.push_back(process);
	}

	{
		std::lock_guard<std::mutex> lock(stateMutex);
		snapshots[process->getPID()] = { process, ProcessState::Waiting, -1, buildTimestamp() };
	}

	queueCv.notify_all();
}

void Scheduler::start()
{
	if (appState != nullptr && !tickThread.joinable()) {
		tickThread = std::thread(&Scheduler::tickLoop, this);
	}

	for (int coreId = 0; coreId < numCores; ++coreId) {
		workers[coreId]->thread = std::thread(&Scheduler::workerLoop, this, coreId);
	}

	schedulerThread = std::thread(&Scheduler::schedulerLoop, this);
}

void Scheduler::stop()
{
	{
		std::lock_guard<std::mutex> lock(queueMutex);
		stopRequested = true;
		acceptingWork = false;
	}
	queueCv.notify_all();

	for (auto& worker : workers) {
		{
			std::lock_guard<std::mutex> workerLock(worker->mutex);
			worker->stopRequested = true;
		}
		worker->cv.notify_all();
	}

	if (schedulerThread.joinable()) {
		schedulerThread.join();
	}

	if (tickThread.joinable()) {
		tickThread.join();
	}

	for (auto& worker : workers) {
		if (worker->thread.joinable()) {
			worker->thread.join();
		}
	}
}

bool Scheduler::isFinished() const
{
	std::lock_guard<std::mutex> lock(stateMutex);
	if (snapshots.empty()) {
		return true;
	}

	for (const auto& entry : snapshots) {
		if (entry.second.state != ProcessState::Finished) {
			return false;
		}
	}

	return true;
}

std::string Scheduler::screenLs() const
{
	std::ostringstream output;
	std::lock_guard<std::mutex> lock(stateMutex);

	output << "----------------------------------------\n";
	output << "Running processes:\n";
	for (const auto& entry : snapshots) {
		const auto& snapshot = entry.second;
		if (snapshot.state == ProcessState::Running) {
			output << snapshot.process->getName()
				   << "  (" << snapshot.stateTimestamp << ")"
				   << "    Core: " << snapshot.coreId
				   << "    " << snapshot.process->getCompletedInstructions()
				   << " / " << snapshot.process->getTotalInstructions() << '\n';
		}
	}

	output << "\nFinished processes:\n";
	for (const auto& entry : snapshots) {
		const auto& snapshot = entry.second;
		if (snapshot.state == ProcessState::Finished) {
			output << snapshot.process->getName()
				   << "  (" << snapshot.stateTimestamp << ")"
				   << "    Finished"
				   << "    " << snapshot.process->getCompletedInstructions()
				   << " / " << snapshot.process->getTotalInstructions() << '\n';
		}
	}
	output << "----------------------------------------\n";
	return output.str();
}

void Scheduler::updateProcessState(const std::shared_ptr<Process>& process, ProcessState state, int coreId)
{
	if (!process) {
		return;
	}

	std::lock_guard<std::mutex> lock(stateMutex);

	if (registry) {
		if (state == ProcessState::Running) {
			registry->updateRunning(process->getPID(), coreId, process->getCompletedInstructions());
		} else if (state == ProcessState::Finished) {
			registry->markFinished(process->getPID(), process->getCompletedInstructions());
		}
	}
	auto& snapshot = snapshots[process->getPID()];
	snapshot.process = process;
	snapshot.state = state;
	snapshot.coreId = coreId;
	snapshot.stateTimestamp = buildTimestamp();
}

void Scheduler::schedulerLoop()
{
	while (true) {
		std::shared_ptr<Process> nextProcess;
		int targetCore = -1;

		{
			std::unique_lock<std::mutex> queueLock(queueMutex);
			queueCv.wait(queueLock, [&]() {
				if (stopRequested) {
					return true;
				}

				if (readyQueue.empty()) {
					return false;
				}

				for (const auto& worker : workers) {
					std::lock_guard<std::mutex> workerLock(worker->mutex);
					if (!worker->hasWork && !worker->stopRequested) {
						return true;
					}
				}
				return false;
			});

			if (stopRequested) {
				break;
			}

			for (int coreId = 0; coreId < numCores && !readyQueue.empty(); ++coreId) {
				auto& worker = workers[coreId];
				std::lock_guard<std::mutex> workerLock(worker->mutex);
				if (!worker->hasWork && !worker->stopRequested) {
					nextProcess = readyQueue.front();
					readyQueue.pop_front();
					targetCore = coreId;
					worker->assignedProcess = nextProcess;
					worker->hasWork = true;
					break;
				}
			}
		}

		if (nextProcess && targetCore >= 0) {
			updateProcessState(nextProcess, ProcessState::Running, targetCore);
			workers[targetCore]->cv.notify_one();
		}

		if (isFinished()) {
			break;
		}
	}
}

void Scheduler::tickLoop()
{
	while (true) {
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			if (stopRequested) {
				break;
			}
		}

		if (appState != nullptr) {
			const long long currentTick = appState->cpuCycles.fetch_add(1) + 1;
			maybeSpawnBatchProcess(currentTick);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void Scheduler::maybeSpawnBatchProcess(long long currentTick)
{
	if (appState == nullptr || appState->config == nullptr || appState->registry == nullptr) {
		return;
	}

	if (!batchGenerationEnabled) {
		return;
	}

	const Config& config = *appState->config;
	if (config.batchProcessFreq <= 0) {
		return;
	}

	if (currentTick < config.batchProcessFreq) {
		return;
	}

	if (currentTick % config.batchProcessFreq != 0) {
		return;
	}

	if (lastBatchSpawnTick == currentTick) {
		return;
	}

	lastBatchSpawnTick = currentTick;

	const auto existingProcesses = appState->registry->getAllProcesses();
	for (const auto& process : existingProcesses) {
		if (process.name.size() >= 2 && process.name[0] == 'p') {
			const std::string suffix = process.name.substr(1);
			if (!suffix.empty() && std::all_of(suffix.begin(), suffix.end(), [](unsigned char ch) { return std::isdigit(ch) != 0; })) {
				const int numericId = std::stoi(suffix);
				if (numericId >= nextGeneratedProcessIndex) {
					nextGeneratedProcessIndex = numericId + 1;
				}
			}
		}
	}

	std::ostringstream nameBuilder;
	nameBuilder << "p" << std::setfill('0') << std::setw(2) << nextGeneratedProcessIndex++;
	auto process = ProcessFactory::createDummyProcess(nameBuilder.str(), nextGeneratedProcessIndex, config);
	int pid = appState->registry->addProcess(process, nameBuilder.str());
	process->setPID(pid);
	addProcess(process);
}