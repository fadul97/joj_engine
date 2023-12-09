#pragma once

#include <sstream>

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                                \
{                                                                       \
    HRESULT hr = (x);                                                   \
    if(FAILED(hr)) {                                                    \
        std::stringstream text;                                         \
         text << __func__                                               \
            << " failed in " << __FILE__                                \
            << ", line " << std::to_string(__LINE__)                    \
            << ":\n";                                                   \
}                                                                       
#endif