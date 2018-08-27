#include "mail.h"
#include "pc.h"
#include "utils.h"
#include "itemdb.h"
#include "../common/packet.h"
#include "../common/db.h"

using namespace Poco::Data::Keywords;

void pc_loadmail(pc* pc) {
	int page_select = pc->read<uint32>();
	int page_total = 0;

	Poco::Data::Session sess = sdb->get_session();
	Poco::Data::Statement stm(sess);

	/* select mail total from sql */
	stm << "SELECT COUNT(mail_id) AS MAIL_TOTAL FROM mail WHERE account_id = ?", into(page_total), use(pc->account_id_), now;

	/* mail page total */
	page_total = (uint32)ceil(page_total * 1.0 / 20);

	/* select mail data */
	Poco::Data::Statement stm2(sess);
	stm2 << "SELECT A.mail_id, B.name, C.item_count,"
		<< "typeid = ISNULL(D.typeid, 0), set_typeid = ISNULL(D.set_typeid, 0), amount = ISNULL(D.amount, 0), day = ISNULL(D.day, 0), ucc, "
		<< "(CASE WHEN A.read_date IS NULL THEN 0 ELSE 1 END) AS mail_read "
		<< "FROM mail A "
		<< "LEFT JOIN account B ON A.account_id = B.account_id "
		/* item mail total */
		<< "OUTER APPLY ( "
		<< "SELECT item_count = COUNT(mail_id) FROM mail_item WHERE mail_id = A.mail_id AND release_date IS NULL "
		<< ") C "
		/* first item */
		<< "OUTER APPLY ( "
		<< "SELECT TOP 1 typeid, set_typeid, amount, day, ucc FROM mail_item WHERE mail_id = A.mail_id AND release_date IS NULL "
		<< ") D "
		<< "WHERE A.account_id = ? AND delete_date IS NULL", use(pc->account_id_), now;

	Poco::Data::RecordSet rs(stm2);

	bool done = rs.moveFirst();

	Packet packet;
	packet.write<uint16>(0x211);
	packet.write<uint32>(0);
	packet.write<uint32>(page_select);
	packet.write<uint32>(page_total);
	packet.write<uint32>(rs.rowCount());

	while (done) {
		packet.write<uint32>(rs["mail_id"]);
		packet.write_string(rs["name"], 16);
		packet.write_null(0x74);
		packet.write<uint8>(rs["mail_read"]); // is read?
		packet.write<uint32>(rs["item_count"]); /// item count
		packet.write<int>(-1);
		packet.write<uint32>(rs["set_typeid"] > 0 ? rs["set_typeid"] : rs["typeid"]); // item typeid
		packet.write<uint8>(rs["day"] > 0 ? 1 : 0); // timer?
		packet.write<uint32>(rs["amount"]); // amount
		packet.write <uint32>(rs["day"]); // day
		packet.write_null(0x10);
		packet.write<int>(-1);
		packet.write<uint32>(0);
		packet.write_string(rs["ucc"], 8); // ucc
		packet.write_null(6);
		done = rs.moveNext();
	}

	pc->send_packet(&packet);
}