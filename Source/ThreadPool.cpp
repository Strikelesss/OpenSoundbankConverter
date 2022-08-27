#include "Header/ThreadPool.h"

void ThreadPool::initialize(const unsigned int numThreads)
{
	m_prevNumThreads = numThreads;
	m_workers.resize(numThreads);
	for(auto i(0u); i < numThreads; ++i)
	{
		m_workers[i] = std::thread([this]{
			while (true)
			{
				std::unique_lock uLock(m_queueMutex);
				m_condition.wait(uLock, [this] { return !m_isEnabled || !m_tasks.empty(); });

				if (!m_isEnabled && m_tasks.empty()) { break; }

				const auto task(std::move(m_tasks.front()));
				m_tasks.pop();
				uLock.unlock();

				++m_tasksInProgress;
				task();
				--m_tasksInProgress;
			}
		});
	}
}

void ThreadPool::queueFunc(std::function<void()>&& func)
{
	{
		std::lock_guard taskLock(m_queueMutex);
		m_tasks.push(std::move(func));
	}

    m_condition.notify_one();
}

void ThreadPool::waitForAll() const noexcept
{
	while(true) { if(m_tasks.empty() && m_tasksInProgress.load() == 0) { break; } }
}

void ThreadPool::destroyAll()
{
	m_isEnabled = false;
	m_condition.notify_all();
	for(auto& thread : m_workers) { thread.join(); }
	m_workers.clear();
}