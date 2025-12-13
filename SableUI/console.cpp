#ifdef _WIN32
#include <windows.h>
#endif

#include <SableUI/console.h>
#include <SableUI/SableUI.h>
#include <filesystem>
#include <corecrt.h>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <cstdarg>
#include <cstdio>
#include <chrono>
#include <string>
#include <vector>
#include <ctime>


static void SetConsoleColour(SableUI::LogColourANSI colour)
{
	std::cout << "\033[" << colour << "m";
}

static void ResetColour()
{
	std::cout << "\033[" << SableUI::RESET << "m";
}

static SableUI::Console* s_ConsoleInstance = nullptr;

static std::tm getLocalTime() {
	std::time_t now_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::tm now_tm;

#ifdef _WIN32
	localtime_s(&now_tm, &now_time_t);
#else
	localtime_r(&now_time_t, &now_tm);
#endif

	return now_tm;
}

void SableUI::Console::Init()
{
    s_ConsoleInstance = new Console();
}

void SableUI::Console::Shutdown()
{
    delete s_ConsoleInstance;
	s_ConsoleInstance = nullptr;
}

std::string SableUI::Console::EnumToString(SableUI::LogType type)
{
	switch (type)
	{
	case LogType::SBUI_LOG:
		return "[LOG]\t";

	case LogType::SBUI_WARNING:
		return "[WARN]\t";

	case LogType::SBUI_ERROR:
		return "[ERROR]\t";
	default:
		return "[UNKNOWN LOGTYPE]";
	}
}

SableUI::Console::Console() {};

static std::string GetTime()
{
	auto now = std::chrono::system_clock::now();
	std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
	std::tm now_tm;

#ifdef _WIN32
	localtime_s(&now_tm, &now_time_t);
#else
	localtime_r(&now_time_t, &now_tm);
#endif

	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

	std::ostringstream oss;
	oss << std::setfill('0')
		<< std::setw(2) << now_tm.tm_min << ':'
		<< std::setw(2) << now_tm.tm_sec << '.'
		<< std::setw(3) << milliseconds.count();

	return "[" + oss.str() + "] ";
}

static std::string GetFileLine(const char* file, int line)
{
#ifdef _DEBUG
	std::filesystem::path path(file);
	
	return path.filename().string() + ':' + std::to_string(line) + ' ';
#else
	return "";
#endif
}

void SableUI::Console::Log(const char* format, const char* file, int line, const char* func, const char* subsystem, ...)
{
    std::string time = GetTime();

    char buffer[1024];
    va_list args;
    va_start(args, subsystem);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    std::string finalMessage = time + '[' + std::string(subsystem) + "] " + GetFileLine(file, line) + buffer;

    SetConsoleColour(GREEN);
    std::cout << finalMessage << std::endl;
    m_Logs.push_back({ LogType::SBUI_LOG, finalMessage, file, line, func, subsystem });
    ResetColour();

    SableUI::PostEmptyEvent();
}

void SableUI::Console::Warn(const char* format, const char* file, int line, const char* func, const char* subsystem, ...)
{
    std::string time = GetTime();

    char buffer[1024];
    va_list args;
    va_start(args, subsystem);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    std::string finalMessage = "[" + std::string(subsystem) + "] " + GetFileLine(file, line) + buffer;

    SetConsoleColour(YELLOW);
    std::cout << finalMessage << std::endl;
    m_Logs.push_back({ LogType::SBUI_WARNING, finalMessage, file, line, func, subsystem });
    ResetColour();

    SableUI::PostEmptyEvent();
}

void SableUI::Console::Error(const char* format, const char* file, int line, const char* func, const char* subsystem, ...)
{
    std::string time = GetTime();

    char buffer[1024];
    va_list args;
    va_start(args, subsystem);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    std::string finalMessage = "[" + std::string(subsystem) + "] " + GetFileLine(file, line) + buffer;

    SetConsoleColour(RED);
    std::cout << finalMessage << std::endl;
    m_Logs.push_back({ LogType::SBUI_ERROR, finalMessage, file, line, func, subsystem });
    ResetColour();

    SableUI::PostEmptyEvent();
}

void SableUI::Console::NotifyError(const char* format, const char* file, int line, const char* func, const char* subsystem, ...)
{
    std::string time = GetTime();

    char buffer[1024];
    va_list args;
    va_start(args, subsystem);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    std::string finalMessage = time + "[" + std::string(subsystem) + "] " + GetFileLine(file, line) + buffer;

    SetConsoleColour(RED);
    std::cout << EnumToString(LogType::SBUI_ERROR) << finalMessage << std::endl;
    m_Logs.push_back({ LogType::SBUI_ERROR, finalMessage, file, line, func, subsystem });
    ResetColour();

    SableUI::PostEmptyEvent();

#ifdef _WIN32
    MessageBoxA(nullptr, finalMessage.c_str(), "Runtime Error", MB_OK | MB_ICONERROR);
#endif
}

void SableUI::Console::RuntimeError(const char* format, const char* file, int line, const char* func, const char* subsystem, ...)
{
    std::string time = GetTime();

    char buffer[1024];
    va_list args;
    va_start(args, subsystem);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    std::string finalMessage = time + "[" + std::string(subsystem) + "] " + GetFileLine(file, line) + buffer;

    SetConsoleColour(RED);
    std::cout << EnumToString(LogType::SBUI_ERROR) << finalMessage << std::endl;
    m_Logs.push_back({ LogType::SBUI_ERROR, finalMessage, file, line, func, subsystem });
    ResetColour();

#ifdef _WIN32
    MessageBoxA(nullptr, finalMessage.c_str(), "Runtime Error", MB_OK | MB_ICONERROR);
    exit(1);
#else
    throw std::runtime_error(finalMessage);
#endif

    SableUI::PostEmptyEvent();
}

void SableUI::Console::Clear()
{
	m_Logs.clear();
}

std::vector<SableUI::LogData>& SableUI::Console::GetLogs()
{
	return m_Logs;
}