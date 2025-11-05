#pragma once

#include "common.h"
#include "cpu.h"
#include "bus.h"
#include "rom.h"
#include "logger.h"

class Emulator {
private:
	Cpu cpu;
	Bus bus;
	Rom rom;
	Logger logger;
public:
	void Run();
	int Load(const char *);
	Emulator(const char *);
	~Emulator();
};
