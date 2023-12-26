#include "error_list.h"

const char* error_names[] = {
    "OK",
    "Failed",
    "Window error",
    "Input error",
    "Context error",
    "Renderer error",
    "Platform error"
};

const char* error_strings[] = {
    "",
    "[Failed]: ",
    "[Window error]: ",
    "[Input error]: ",
    "[Context error]: ",
    "[Renderer error]: "
    "[Platform error]: "
};