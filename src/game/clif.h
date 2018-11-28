#pragma once
struct pc;

void clif_pc_opencard_failed(pc *pc);
void clif_bongdari_not_enoughtmoney(pc *pc);
void clif_bongdari_not_enoughtmoney_big(pc *pc);
void clif_bongdari_normal_begin(pc *pc);

void clif_scratch_not_enough_item(pc *pc);