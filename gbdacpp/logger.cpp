#include "logger.h"

void Logger::LogCpuState(const CpuState state)
{	
	/* 
		using the boot rom's log file from https://github.com/wheremyfoodat/Gameboy-logs
		to see if something goes wrong 
	 */
	cpuStateLogger->info("A: {:02X} F: {:02X} B: {:02X} C: {:02X} D: {:02X} E: {:02X} H: {:02X} L: {:02X} SP: {:04X} PC: 00:{:04X} ({:02X} {:02X} {:02X} {:02X})", 
			state.AF.A, state.AF.F, MSB(state.BC), LSB(state.BC), MSB(state.DE), LSB(state.DE),
			MSB(state.HL), LSB(state.HL), state.SP, state.PC,
			state.romData[0], state.romData[1], state.romData[2], state.romData[3]);
}

Logger::Logger()
{
	cpuStateLogger = spdlog::basic_logger_mt("cpu instruction", "Log/CpuInstructionLog.txt", true);
	cpuStateLogger->set_pattern("%v");
}

Logger::~Logger()
{
}