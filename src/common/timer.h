#pragma once
#include "Semaphore.h"
#include <thread>
#include <queue>
#include <chrono>
#include <assert.h>

using Clock = std::chrono::steady_clock;
using ClockHRC = std::chrono::high_resolution_clock;

class TimerQueue {
public:
	TimerQueue();

	~TimerQueue();

	uint64_t add(int64_t milliseconds, bool periodic, std::function<void(bool)> handler);
	size_t cancel(uint64_t id);
	size_t cancelAll();
private:
	TimerQueue(const TimerQueue&) = delete;
	TimerQueue& operator=(const TimerQueue&) = delete;

	void run();

	std::pair<bool, Clock::time_point> calcWaitTime();

	void checkWork();

	Semaphore m_checkWork;
	std::thread m_th;
	bool m_finish = false;
	uint64_t m_idcounter = 0;

	struct WorkItem {
		Clock::time_point end;
		uint64_t id;  // id==0 means it was cancelled
		std::function<void(bool)> handler;
		bool operator>(const WorkItem& other) const {
			return end > other.end;
		}
		std::chrono::milliseconds periodic;
	};

	std::mutex m_mtx;
	// Inheriting from priority_queue, so we can access the internal container
	class Queue : public std::priority_queue<WorkItem, std::vector<WorkItem>,
		std::greater<WorkItem>> {
	public:
		std::vector<WorkItem>& getContainer() {
			return this->c;
		}
	} m_items;
};

namespace Timing {
	static thread_local ClockHRC::time_point ms_previous;
	double now();
	void sleep(unsigned ms);

}

extern TimerQueue* timer;