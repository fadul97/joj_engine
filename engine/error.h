#pragma once

namespace JojEngine
{
#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                               \
{                                                                      \
    HRESULT hr = (x);                                                  \
    if(FAILED(hr)) { throw Error(hr, __func__, __FILE__, __LINE__); }  \
}
#endif
}