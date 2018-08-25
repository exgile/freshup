#pragma once
#include <string>

class pc;

class account {
public:
	account();
	~account();
	
	void sys_send_jerk(pc* pc);
	void sys_send_pc_data(pc* pc);

	void pc_login(pc* pc);
};

extern account* pc_process;