#include "timer.h"
#include <map>
#include <memory>

std::map<int, std::unique_ptr<Worker>> timers;

void timer_init() {
	// Create timer
	for (int i = 0; i < MAX_TIMER; ++i) {
		timers[i] = std::make_unique<Worker>();
		timers[i]->enabled = false;
	}
}

int aquire_timerid() {
	for (int i = 0; i < MAX_TIMER; ++i) {
		if (timers[i]->enabled == false) {
			return i;
		}
	}

	return -1; // No Slot available for timer.
}

// -1 = Fail to add timer
int addtimer(int ms, std::function<void(void)> handler, bool periodic) {
	int timerid = aquire_timerid();

	if (timerid == -1) {
		throw "Timer : No Slot available for timer.";
	}

	timers[timerid]->enabled = true;
	timers[timerid]->end = std::chrono::system_clock::now() + std::chrono::milliseconds(ms);
	timers[timerid]->ms = std::chrono::milliseconds(ms);
	timers[timerid]->periodic = periodic;
	timers[timerid]->handler = handler;

	return timerid;
}

bool deltimer(int timerid) {
	if (timers[timerid]->enabled == true) {
		timers[timerid]->enabled = false;
		timers[timerid]->handler = nullptr;
		return true;
	}
	return false;
}

void do_timer() {
	for (auto const &timer : timers) {
		if (timer.second->enabled == true && timer.second->end <= std::chrono::system_clock::now()) {
			
			if (timer.second->handler)
				timer.second->handler(); // Execute the function

			if (!timer.second->periodic) {
				timer.second->enabled = false;
				timer.second->handler = nullptr;
			}

			timer.second->end = std::chrono::system_clock::now() + std::chrono::milliseconds(timer.second->ms);
		}
	}
}

void timer_final() {
	timers.clear();
}