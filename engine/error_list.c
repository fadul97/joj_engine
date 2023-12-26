#include "error_list.h"

const char* error_names[] = {
    "OK",
    "Failed",
    "Window error",
    "Context error",
    "Input error",
    "Platform error"
};

const char* error_strings[] = {
    "",
    "[Failed]: ",
    "[Window error]: ",
    "[Context error]: ",
    "[Input error]: ",
    "[Platform error]: "
};