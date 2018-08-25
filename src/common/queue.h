#pragma once

#include "TimerQueue.h"

class Queue {
public:
	TimerQueue* task;
	Queue();
	~Queue();
};

extern Queue* queue;