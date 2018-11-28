#pragma once

#include "../common/ini_reader.h"

void config_init();
void config_final();

extern INIReader *config;