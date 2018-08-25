#pragma once

// this cryption unit uses for windows only

#include <windows.h>

typedef cdecl void(*DecryptFunc)(unsigned char*, int, int);
typedef cdecl void(*EncryptFunc)(unsigned char*, int, int, unsigned char**, int*);
typedef cdecl void(*FreeMemFunc)(unsigned char**);

class Crypto {
private:
	HINSTANCE hInst;
public:
	Crypto();
	~Crypto();
	DecryptFunc Decrypt = nullptr;
	EncryptFunc Encrypt = nullptr;
	FreeMemFunc _FreeMem = nullptr;
};

extern Crypto* crypt;