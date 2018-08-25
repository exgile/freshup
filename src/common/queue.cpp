#include "queue.h"

extern Queue* queue = nullptr;

Queue::Queue() : task(new TimerQueue()){}

Queue::~Queue() {
	task->cancelAll();
	delete task;
};