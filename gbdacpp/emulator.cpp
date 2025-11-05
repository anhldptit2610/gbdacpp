#include <fstream>
#include <filesystem>
#include <string>
#include "emulator.h"
#include "logger.h"


int Emulator::Load(const char* romPath)
{
	return rom.Load(romPath);
}

void Emulator::Run()
{
	while(1) {
#ifdef LOGGER_ENABLE
		logger.LogCpuState(cpu.GetCpuState());
#endif
		if (cpu.Step(rom) == -1)
			break;
	}
}

Emulator::Emulator(const char *romPath) : rom(), bus(&rom), cpu(&bus), logger()
{

}

Emulator::~Emulator()
{

}