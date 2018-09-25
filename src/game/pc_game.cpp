#include "pc.h"
#include "gameplay.h"
#include "../common/packet.h"

void pc::change_equipment() {
	Packet p;
	uint8 action = read<uint8>();
	uint32 value = read<uint32>();

	p.write<uint16>(0x4b);
	p.write<uint32>(0);
	p.write<uint8>(action);
	p.write<uint32>(connection_id_);

	switch (action) {
	case e_char:
		warehouse->write_current_char(&p);
		break;
	}

	if (action == e_char) {
		if (game == nullptr) return;
		game->send(&p);
	}
	else {
		send_packet(&p);
	}
}