#include "db.h"

db* sdb = nullptr;

db::db() : pool_("ODBC", "DRIVER={SQL Server};Server=DESKTOP-B9HNC4F;Database=PANGYA;User ID=sa;Password=123456", 1, 32) {
	Poco::Data::ODBC::Connector::registerConnector();
}

db::~db() {
	pool_.shutdown();
}

Poco::Data::Session db::get_session() {
	Poco::Data::Session sess = pool_.get();
	sess.setFeature("forceEmptyString", true);
	return sess;
}