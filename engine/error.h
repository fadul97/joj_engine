#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include <windows.h>
#include <string>

namespace JojEngine
{
	class Error
	{
	public:
		Error();
		Error(HRESULT hr, const std::string& func, const std::string& file, int line);
		~Error();

		std::string to_string() const;

	private:
		HRESULT hres_code;
		std::string func_name;
		std::string file_name;
		i32 line_num;
	};
}


#ifndef ThrowIfFailed
#define ThrowIfFailed(x)															\
{																					\
    HRESULT hr = (x);																\
    if(FAILED(hr)) { throw JojEngine::Error(hr, __func__, __FILE__, __LINE__); }	\
}
#endif

#endif	// PLATFORM_WINDOWS