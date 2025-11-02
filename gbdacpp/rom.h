#pragma once

#include "common.h"
#include <array>
#include <string>

typedef struct RomHeader {
	std::string title;
	u8 romType;
	u64 romSize;
	u32 ramSize;
} RomHeader;

class Rom {
private:
	RomHeader header;
	u8* data = nullptr;
	bool disableBootROM = false;

	u8 BootRomRead(u16);
public:
	int ParseHeader();
	void UnlockBootROM();
	bool IsBootROMUnlocked() const;
	u8 Read(u16);
	void Write(u16, u8);
	Rom(const char *, bool *);
	~Rom();
};
