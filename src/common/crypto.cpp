#include "crypto.h"

Crypto* crypt = nullptr;

Crypto::Crypto(){
	hInst = LoadLibraryA("Project1.dll");

	if (!hInst) {
		return;
	}

	Decrypt = (DecryptFunc)GetProcAddress(hInst, "__Decrypt");
	Encrypt = (EncryptFunc)GetProcAddress(hInst, "__Encrypt");
	_FreeMem = (FreeMemFunc)GetProcAddress(hInst, "__FreeMem");

	if (!Decrypt) {
		return;
	}

	if (!Encrypt) {
		return;
	}

	if (!_FreeMem) {
		return;
	}
}

Crypto::~Crypto() {
	FreeLibrary(hInst);
}