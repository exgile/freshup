#include "clif.h"
#include "pc.h" // pc Class
#include "../common/utils.h"
#include "../common/packet.h" // Packet class

void clif_pc_opencard_failed(pc *pc) {
	Packet p;
	WTHEAD(&p, 0x154);
	WTIU32(&p, 1);
	pc->send(&p);
}

void clif_bongdari_not_enoughtmoney(pc *pc) {
	Packet p;
	WTHEAD(&p, 0x21b);
	WTIU32(&p, 2651002);
	pc->send(&p);
}

void clif_bongdari_normal_begin(pc *pc) {
	Packet p;
	WTHEAD(&p, 0xfb);
	WTI32(&p, -1);
	WTI32(&p, -3);
	pc->send(&p);
}

void clif_bongdari_not_enoughtmoney_big(pc *pc) {
	Packet p;
	WTHEAD(&p, 0x26c);
	WTIU32(&p, 2651002);
	pc->send(&p);
}

void clif_scratch_not_enough_item(pc *pc) {
	Packet p;
	WTHEAD(&p, 0xDD);
	WTIU32(&p, 3000036);
	pc->send(&p);
}