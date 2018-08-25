#include <iostream>
#include "reader.h"

Config* config = nullptr;

Config::Config()  {
	read = new INIReader("config/config.txt");
}

Config::~Config() {
	delete read;
}