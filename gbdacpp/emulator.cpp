#include "emulator.h"
#include <fstream>
#include <filesystem>
#include <string>

#define FMT_HEADER_ONLY
#include <fmt/core.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>


void Emulator::Run()
{
	auto cpuInstructionLogger = spdlog::basic_logger_mt("cpu instruction", "CpuInstructionLog.txt", true);

	cpuInstructionLogger->set_pattern("%v");
	while(1) {
		if (cpu.Step(rom, cpuInstructionLogger) == -1)
			break;
	}
}

Emulator::Emulator(const char* romPath) : rom(romPath, &runnable), bus(&rom), cpu(&bus)
{
	if (!runnable)
		exit(EXIT_FAILURE);
}

Emulator::~Emulator()
{

}