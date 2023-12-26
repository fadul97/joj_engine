#pragma once

enum Error
{
    OK,
    FAILED,
    ERR_WINDOW,
    ERR_CONTEXT,
    ERR_INPUT,
    ERR_PLATFORM
};

extern const char* error_names[];
extern const char* error_strings[];
