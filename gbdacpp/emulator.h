#pragma once

#include "common.h"
#include "cpu.h"
#include "bus.h"
#include "rom.h"

class Emulator {
private:
	bool runnable = true;
	Cpu cpu;
	Bus bus;
	Rom rom;
public:
	void Run();
	Emulator(const char *);
	~Emulator();
};
