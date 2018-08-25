#pragma once
#include <string>

class pc;

class account {
public:
	account();
	~account();

	bool sys_name_validation(std::string& name);
	void sys_send_data(pc* pc);

	void pc_login(pc* pc);
	void pc_checkup_name(pc* pc);
	void pc_name_validation(pc* pc);
	void pc_request_create_char(pc* pc);
	void pc_request_gamekey(pc* pc);
};

extern account* pc_process;