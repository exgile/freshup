#include <iostream>
#include "reader.h"

INIReader *config = nullptr;

void config_init() {
	config = new INIReader("config/config.txt");
}

void config_final() {
	delete config;
}