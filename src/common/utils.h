#pragma once
#include "typedef.h"
#include <Poco/Datetime.h>
#include <Poco/Timespan.h>
#include <random>

#define STRUCT_PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )

#define COUNTOF(x) (sizeof(x)/sizeof(*x))

#define CREATE_SHARED(type) ( std::make_shared<type>() )
#define CREATE_UNIQUE(type) ( std::make_unique<type>() )

#define WTHEAD(p, val) ( (p)->write<uint16>(val) )

#define WTIU08(p, val) ( (p)->write<uint8>(val) )
#define WTIU16(p, val) ( (p)->write<uint16>(val) )
#define WTIU32(p, val) ( (p)->write<uint32>(val) )
#define WTIU64(p, val) ( (p)->write<uint64>(val) )
#define WTI08(p, val) ( (p)->write<int8>(val) )
#define WTI16(p, val) ( (p)->write<int16>(val) )
#define WTI32(p, val) ( (p)->write<int32>(val) )
#define WTI64(p, val) ( (p)->write<int64>(val) )
#define WTFLO(p, val) ( (p)->write<float>(val) )
#define WTPHEX(p, o, s) ( (p)->write_hex(o, s) )
#define WTCSTR(p, s) ( (p)->write<std::string>(s) )
#define WTFSTR(p, s, c) ( (p)->write_string(s, c) )
#define WTDATETIME(p) ( (p)->write_datetime() )
#define WTPOINTER(p, pt, s) ( (p)->write((char*)pt, s) )
#define WTZERO(p, c) ( (p)->write_null(c) )

#define WRESET(p) ( (p)->reset() )

#define RTIU08(p) ( (p)->read<uint8>() )
#define RTIU16(p) ( (p)->read<uint16>() )
#define RTIU32(p) ( (p)->read<uint32>() )
#define RTIU64(p) ( (p)->read<uint64>() )
#define RTI08(p) ( (p)->read<int32>() )
#define RTI16(p) ( (p)->read<int16>() )
#define RTI32(p) ( (p)->read<int32>() )
#define RTI64(p) ( (p)->read<int64>() )
#define RTFLO(p) ( (p)->read<float>() )
#define RTSTR(p) ( (p)->read<std::string>() )
#define RTPOINTER(p, pt, s) ( (p)->read((char*)pt, s) )
#define RSKIP(p, c) ( (p)->skip(c) )

#define STRCMP(l, r) ( l.compare(r) == 0 )

// use to free for array of pointers
#define NULL_POINTER(p)	\
do {	\
	delete p;	\
	p = nullptr;	\
} while (0)	\

void rnd_init(void);
int32 rnd(void);
uint32 rnd_uint32(void);
int32 rnd_value(int32 min, int32 max);
std::string rnd_str(int max_length, std::string possible_chars = "abcdef1234567890");

uint8 itemdb_type(uint32 id);

Poco::DateTime localtime(Poco::DateTime& dt);
Poco::DateTime localtime();
uint32 timestamp();
uint32 timestamp(const Poco::DateTime& dt);

int rnd_weight(const std::vector<int>& list);