#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

inline std::string buildTimestamp()
{
	const auto now = std::chrono::system_clock::now();
	const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

	std::tm localTime{};
	localtime_s(&localTime, &nowTime);

	std::ostringstream builder;
	builder << std::put_time(&localTime, "%m/%d/%Y %I:%M:%S%p");
	return builder.str();
}