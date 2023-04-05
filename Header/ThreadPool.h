#pragma once
#include <mutex>
#include <queue>
#include <functional>

struct ThreadPool final
{
	explicit ThreadPool(const uint32_t numThreads) { initialize(numThreads); }
	ThreadPool(ThreadPool const&) = delete; ThreadPool& operator=(const ThreadPool&) = delete;
	~ThreadPool() noexcept { waitForAll(); destroyAll(); }

	void initialize(uint32_t numThreads);
	void queueFunc(std::function<void()>&& func);
	void waitForAll() const noexcept;
	void destroyAll();
	size_t GetNumTasks() const { return m_tasks.size(); }
private:
    std::condition_variable m_condition;
	std::vector<std::thread> m_workers{};
	std::queue< std::function<void()> > m_tasks{};
	std::mutex m_queueMutex;
	uint32_t m_prevNumThreads = 0u;
	bool m_isEnabled = true;
};