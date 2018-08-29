#pragma once
#include <string>
#include "typedef.h"

class pc;

struct mail_data {
	uint32 mail_id;
	std::string sender;
	std::string subject;
	std::string message;
};

void pc_loadmail(pc* pc);
void pc_readmail(pc* pc);