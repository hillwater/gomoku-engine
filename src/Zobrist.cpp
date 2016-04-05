#include "../includes/Zobrist.h"

// initial RC4 Generator with empty password
void RC4Struct::InitZero(void) {
	int i, j;
	int8 uc;

	x = y = j = 0;
	for (i = 0; i < 256; i ++) {
		s[i] = i;
	}
	for (i = 0; i < 256; i ++) {
		j = (j + s[i]) & 255;
		uc = s[i];
		s[i] = s[j];
		s[j] = uc;
	}
}

// generate next RC4
int8 RC4Struct::NextByte(void) {
	int8 uc;
	x = (x + 1) & 255;
	y = (y + s[x]) & 255;
	uc = s[x];
	s[x] = s[y];
	s[y] = uc;
	return s[(s[x] + s[y]) & 255];
}

// generate next 4 bytes
int32 RC4Struct::NextLong(void) {
	int8 uc0, uc1, uc2, uc3;
	uc0 = NextByte();
	uc1 = NextByte();
	uc2 = NextByte();
	uc3 = NextByte();
	return uc0 + (uc1 << 8) + (uc2 << 16) + (uc3 << 24);
}

ZobristStruct::ZobristStruct(void)
{
	dwKey = dwLock0 = dwLock1 = 0;
}


ZobristStruct::~ZobristStruct(void)
{
}

void ZobristStruct::InitZero()
{
	dwKey = dwLock0 = dwLock1 = 0;
}

void ZobristStruct::InitRC4(RC4Struct &rc4) {        // fill with RC4
	dwKey = rc4.NextLong();
	dwLock0 = rc4.NextLong();
	dwLock1 = rc4.NextLong();
}

void ZobristStruct::Xor(const ZobristStruct &zobr1, const ZobristStruct &zobr2) {
	dwKey ^= zobr1.dwKey ^ zobr2.dwKey;
	dwLock0 ^= zobr1.dwLock0 ^ zobr2.dwLock0;
	dwLock1 ^= zobr1.dwLock1 ^ zobr2.dwLock1;
}

// initial Zobrist table
void Zobrist::initial(void) {
	int i, j;
	RC4Struct rc4;

	rc4.InitZero();
	player.InitRC4(rc4);

	curZobr.InitZero();

	for (i = 0; i < 2; i ++) {
		for (j = 0; j < 256; j ++) {
			table[i][j].InitRC4(rc4);
		}
	}
}
