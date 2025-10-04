#pragma once

#include <vector>
#include <string>
#include <cstdarg>
#include <cstdio>

#undef SABLEUI_SUBSYSTEM
#define SABLEUI_SUBSYSTEM "SableUI"

namespace SableUI
{
    enum LogColourANSI
    {
        RESET = 0,
        RED = 31,
        GREEN = 32,
        YELLOW = 33
    };

    enum class LogType
    {
        SBUI_LOG = 0,
        SBUI_WARNING = 1,
        SBUI_ERROR = 2
    };

    struct LogData
    {
        LogType type;
        std::string message;
        std::string file;
        int line;
        std::string func;
        std::string subsystem;
    };

    class Console
    {
    public:
        Console(const Console&) = delete;
        Console& operator=(const Console&) = delete;

        static void Init();
        static void Shutdown();

        static std::vector<LogData>& GetLogs();

        static void Clear();

        static void Log(const char* format, const char* file, int line, const char* func, const char* subsystem, ...);
        static void Warn(const char* format, const char* file, int line, const char* func, const char* subsystem, ...);
        static void Error(const char* format, const char* file, int line, const char* func, const char* subsystem, ...);
        static void NotifyError(const char* format, const char* file, int line, const char* func, const char* subsystem, ...);
        static void RuntimeError(const char* format, const char* file, int line, const char* func, const char* subsystem, ...);

    private:
        Console();

        static std::string EnumToString(LogType type);
        static inline std::vector<LogData> m_Logs;
    };
}

#ifdef _DEBUG
#define SableUI_Log(format, ...)           SableUI::Console::Log(format, __FILE__, __LINE__, __func__, SABLEUI_SUBSYSTEM, ##__VA_ARGS__)
#define SableUI_Warn(format, ...)          SableUI::Console::Warn(format, __FILE__, __LINE__, __func__, SABLEUI_SUBSYSTEM, ##__VA_ARGS__)
#define SableUI_Error(format, ...)         SableUI::Console::Error(format, __FILE__, __LINE__, __func__, SABLEUI_SUBSYSTEM, ##__VA_ARGS__)
#define SableUI_Notify_Error(format, ...)  SableUI::Console::NotifyError(format, __FILE__, __LINE__, __func__, SABLEUI_SUBSYSTEM, ##__VA_ARGS__)
#define SableUI_Runtime_Error(format, ...) SableUI::Console::RuntimeError(format, __FILE__, __LINE__, __func__, SABLEUI_SUBSYSTEM, ##__VA_ARGS__)
#else
#define SableUI_Log(format, ...)           SableUI::Console::Log(format, "", 0, "", SABLEUI_SUBSYSTEM, ##__VA_ARGS__)
#define SableUI_Warn(format, ...)          SableUI::Console::Warn(format, "", 0, "", SABLEUI_SUBSYSTEM, ##__VA_ARGS__)
#define SableUI_Error(format, ...)         SableUI::Console::Error(format, "", 0, "", SABLEUI_SUBSYSTEM, ##__VA_ARGS__)
#define SableUI_Notify_Error(format, ...)  SableUI::Console::NotifyError(format, "", 0, "", SABLEUI_SUBSYSTEM, ##__VA_ARGS__)
#define SableUI_Runtime_Error(format, ...) SableUI::Console::RuntimeError(format, "", 0, "", SABLEUI_SUBSYSTEM, ##__VA_ARGS__)
#endif