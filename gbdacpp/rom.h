#pragma once

#include "common.h"
#include <array>
#include <memory>
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
	std::unique_ptr<u8[]> data = nullptr;
	bool disableBootROM = false;
	u8 BootRomRead(u16);
public:
	int Load(const char*);
	int ParseHeader();
	void UnlockBootROM();
	bool IsBootROMUnlocked() const;
	u8 Read(u16);
	void Write(u16, u8);
	Rom();
	~Rom();
};
