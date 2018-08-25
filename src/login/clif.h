#pragma once
#include <string>

class pc;

enum loginmsg {
	INCORRECT_PASSWORD,
	USER_NOT_FOUND,
	USER_STATE_BAN,
	MAINTENANCE_MODE
};

class clif {
public:
	clif();
	~clif();
	void login_msg(pc* pc, loginmsg type);
	void send_accountid(pc* pc);
	void send_firstset(pc* pc);
	void send_name_taken(pc* pc);
	void send_name_available(pc* pc, std::string& name);
	void send_name_validate_true(pc* pc);
	void send_name_validate_failed(pc* pc);
	void send_auth_login(pc* pc);
	void send_pc_data(pc* pc);
	void send_game_server(pc* pc);
	void send_hotkey(pc* pc);
};

extern clif* sclif;