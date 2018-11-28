#pragma once
#include <chrono>
#include <functional>

#define MAX_TIMER 9999

struct Worker {
	bool enabled;
	std::chrono::system_clock::time_point end;
	std::chrono::milliseconds ms;
	std::function<void(void)> handler;
	bool periodic;
};

void timer_init();
int aquire_timerid();
int addtimer(int ms, std::function<void(void)> handler, bool periodic);
bool deltimer(int timerid);
void do_timer();
void timer_final();