#pragma once

#include "common.h"
#include "rom.h"

class Bus {
private:
	Rom* rom;
	u8 memory[0x10000];
public:
	void Write(const u16, const u8);
	u8 Read(const u16);
	Bus(Rom *);
	~Bus();
};
