#pragma once

struct pc;

void sys_send_jerk(pc* pc);
void sys_send_pc_data(pc* pc);
void pc_req_login(pc* pc);

void pc_req_savedata(pc *pc);