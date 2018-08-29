#pragma once

#include "Poco/Data/RecordSet.h"
#include "Poco/Data/SessionPool.h"
#include "Poco/Data/ODBC/Connector.h"

using namespace Poco::Data::Keywords;

class db {
private:
	Poco::Data::SessionPool pool_;

public:
	Poco::Data::Session get_session();
	db();
	~db();
};

extern db* sdb;