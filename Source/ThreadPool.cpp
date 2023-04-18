#include "Header/ThreadPool.h"

void ThreadPool::initialize(const uint32_t numThreads)
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
				task();
				
				m_tasks.pop();
				uLock.unlock();
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
	while(!m_tasks.empty()) { }
}

void ThreadPool::destroyAll()
{
	m_isEnabled = false;
	m_condition.notify_all();
	for(auto& thread : m_workers) { thread.join(); }
	m_workers.clear();
}