#pragma once

enum Error
{
    OK,
    FAILED,
    ERR_WINDOW,
    ERR_INPUT,
    ERR_CONTEXT,
    ERR_RENDERER,
    ERR_PLATFORM
};

extern const char* error_names[];
extern const char* error_strings[];
