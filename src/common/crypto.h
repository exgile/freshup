#pragma once

// this cryption unit uses for windows only

#include <windows.h>

typedef cdecl void(*DecryptFunc)(unsigned char*, int, int);
typedef cdecl void(*EncryptFunc)(unsigned char*, int, int, unsigned char**, int*);
typedef cdecl void(*FreeMemFunc)(unsigned char**);

void crypt_init();
void crypt_final();

extern DecryptFunc decrypt;
extern EncryptFunc encrypt;
extern FreeMemFunc freemem;