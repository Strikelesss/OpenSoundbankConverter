#pragma once
#include <mutex>
#include <queue>
#include <functional>

struct ThreadPool final
{
	explicit ThreadPool(unsigned int numThreads);
	ThreadPool(ThreadPool const&) = delete; ThreadPool& operator=(const ThreadPool&) = delete;
	~ThreadPool() noexcept;

	void initialize(unsigned int numThreads);
	void queueFunc(std::function<void()>&& func);
	void waitForAll() const noexcept;
	void destroyAll();
	int GetNumTasks() const { return m_tasksInProgress.load(); }
private:
    std::condition_variable m_condition;
	std::vector<std::thread> m_workers{};
	std::queue< std::function<void()> > m_tasks{};
	std::mutex m_queueMutex;
	std::atomic<int> m_tasksInProgress = 0;
	unsigned int m_prevNumThreads = 0u;
	bool m_isEnabled = true;
};