#pragma once

#include "common.h"
#include "cpu.h"

#define FMT_HEADER_ONLY
#include <fmt/core.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

class Logger {
private:
	std::shared_ptr<spdlog::logger> cpuStateLogger;
public:
	void LogCpuState(const CpuState);
    Logger();
	~Logger();
};

