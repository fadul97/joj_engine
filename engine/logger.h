#pragma once

#include "defines.h"
#include "error_list.h"

#define LOG_FATAL_ENABLED 1
#define LOG_ERROR_ENABLED 1

#define FRELEASE 0

#if FRELEASE == 1
#define LOG_WARN_ENABLED 0
#define LOG_DEBUG_ENABLED 0
#define LOG_INFO_ENABLED 0
#else
#define LOG_WARN_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_INFO_ENABLED 1
#endif

typedef enum LogLevel
{
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_DEBUG = 3,
    LOG_LEVEL_INFO = 4,
} LogLevel;

void log_output2(LogLevel level, const char* message);


void log_output(LogLevel level, enum Error err, const char* message, ...);
#ifndef FFATAL
#define FFATAL(error, message, ...) log_output(LOG_LEVEL_FATAL, error, message "\nFile: %s\nLine: %d", __FILE__, __LINE__, ##__VA_ARGS__);
#endif

#ifndef FERROR
#define FERROR(error, message, ...) log_output(LOG_LEVEL_ERROR, error, message "\nFile: %s\nLine: %d", __FILE__, __LINE__, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED == 1
#define FWARN(message, ...) log_output(LOG_LEVEL_WARN, OK, message "\nFile: %s\nLine: %d", __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define FWARN(message, ...);
#endif

#if LOG_DEBUG_ENABLED == 1
#define FDEBUG(message, ...) log_output(LOG_LEVEL_DEBUG, OK, message, ##__VA_ARGS__);
#else
#define FDEBUG(message, ...);
#endif

#if LOG_INFO_ENABLED == 1
#define FINFO(message, ...) log_output(LOG_LEVEL_INFO, OK, message, ##__VA_ARGS__);
#else
#define FINFO(message, ...);
#endif
