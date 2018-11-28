#include "crypto.h"

DecryptFunc decrypt = nullptr;
EncryptFunc encrypt = nullptr;
FreeMemFunc freemem = nullptr;
HINSTANCE hInst = nullptr;

void crypt_init() {
	hInst = LoadLibraryA("Project1.dll");

	if (!hInst) {
		return;
	}

	decrypt = (DecryptFunc)GetProcAddress(hInst, "__Decrypt");
	encrypt = (EncryptFunc)GetProcAddress(hInst, "__Encrypt");
	freemem = (FreeMemFunc)GetProcAddress(hInst, "__FreeMem");

	if (!decrypt) {
		return;
	}

	if (!encrypt) {
		return;
	}

	if (!freemem) {
		return;
	}
}

void crypt_final() {
	FreeLibrary(hInst);
}