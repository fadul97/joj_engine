#include "logger.h"

#include <stdio.h>
#include <stdarg.h>

#if PLATFORM_WINDOWS
#include <Windows.h>

void log_output2(LogLevel level, const char* message)
{
    printf("PRINTEI\n");
}

void log_output(LogLevel level, enum Error err, const char* message, ...)
{
    // Setup for sprintf and then printf
    char out_msg[1000];
    char out_msg2[1000];
    memset(out_msg, 0, sizeof(out_msg));

    va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(out_msg, 1000, message, arg_ptr);

    // Colors (Windows)
    WORD color_attributes[5] = {
        FOREGROUND_RED | FOREGROUND_INTENSITY,  // Bold red
        FOREGROUND_RED,                         // Red
        FOREGROUND_RED | FOREGROUND_GREEN,      // Orange
        FOREGROUND_BLUE,                        // Blue
        FOREGROUND_GREEN                        // Green
    };

    // Get the console handle
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // Bold red, Red, Orange, Blue, Green
    const char* color_strings[5] = {
        "\033[1;31m", "\033[0;31m", "\033[0;33m", "\033[0;34m", "\033[0;32m"
    };

    const char* level_strings[5] = {
        "[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[DEBUG]: ", "[INFO]: "
    };

    // FATAL log
    if (level == LOG_LEVEL_FATAL)
    {
        sprintf(out_msg2, "%s%s%s", level_strings[level], error_strings[err], out_msg);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | BACKGROUND_RED);
        printf("%s\n", out_msg2);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default
        return;
    }
    // ERROR log
    else if (level == LOG_LEVEL_ERROR)
    {
        sprintf(out_msg2, "%s%s%s", level_strings[level], error_strings[err], out_msg);
        SetConsoleTextAttribute(hConsole, color_attributes[level]);
        printf("%s\n", out_msg2);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default
        return;
    }

    sprintf(out_msg2, "%s%s", level_strings[level], out_msg);
    SetConsoleTextAttribute(hConsole, color_attributes[level]);
    printf("%s\n", out_msg2);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default
}

#else // PLATFORM_WINDOWS


void log_output(LogLevel level, enum Error err, const char* message, ...)
{
    // Setup for sprintf and then printf
    char out_msg[1000];
    char out_msg2[1000];
    memset(out_msg, 0, sizeof(out_msg));

    va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(out_msg, 1000, message, arg_ptr);

    /* Colors(Linux only) */
    const char* default_color = "\033[0m";

    // Bold red, Red, Orange, Blue, Green
    const char* color_strings[5] = {
        "\033[1;31m", "\033[0;31m", "\033[0;33m", "\033[0;34m", "\033[0;32m"
    };

    const char* level_strings[5] = {
        "[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[DEBUG]: ", "[INFO]: "
    };

    // FATAL or ERROR log
    if (level < 2)
    {
        sprintf(out_msg2, "%s%s%s", level_strings[level], error_strings[err], out_msg);
        printf("%s%s%s\n", color_strings[level], out_msg2, default_color);
        return;
    }

    sprintf(out_msg2, "%s%s", level_strings[level], out_msg);
    printf("%s%s%s\n", color_strings[level], out_msg2, default_color);
}

#endif