#pragma once
#include <mutex>
#include <queue>
#include <functional>

struct ThreadPool final
{
	explicit ThreadPool(unsigned int numThreads);
	ThreadPool(ThreadPool const&) = delete; ThreadPool& operator=(const ThreadPool&) = delete;
	~ThreadPool() noexcept;

	/*
	\brief Initialize x threads
	*/
	void initialize(unsigned int numThreads);

	/*
	\brief Queue a task for a thread to take
	*/
	void queueFunc(std::function<void()>&& func);

	/*
	\brief Waits for all threads to finish tasks up
	*/
	void waitForAll() const noexcept;

	/*
	\brief Destroys all threads, generally never needs to be called finished
		with the thread pool
	*/
	void destroyAll();

	/*
	\brief Resets all threads, generally never needs to be called unless
		all threads are destroyed
	*/
	void resetAll(unsigned int numThreads = 0u);
private:
    std::condition_variable m_condition;
	std::vector<std::thread> m_workers{};
	std::queue< std::function<void()> > m_tasks{};
	std::mutex m_queueMutex;
	std::atomic<int> m_tasksInProgress = 0;
	unsigned int m_prevNumThreads = 0u;
	bool m_isEnabled = true;
};