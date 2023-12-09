#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include <windows.h>

namespace JojPlatform
{
	class Win32Timer
	{
	public:
		Win32Timer();
		~Win32Timer();

		void start();				// Start/resume counting time
		void stop();				// Stop counting time
		f32 reset();				// Restarts counter and returns elapsed time
		f32 elapsed();				// Returns elapsed time in seconds	
		b8 was_elapsed(f32 secs);	// Checks if "secs" seconds have passed

	private:
		LARGE_INTEGER counter_start;	// Start of counter 
		LARGE_INTEGER end;				// End of counter
		LARGE_INTEGER freq;				// Counter frequency
		b8 stopped;						// Counter state
	};

	inline b8 Win32Timer::was_elapsed(f32 secs)
	{ return (elapsed() >= secs ? true : false); }
}

#endif  // PLATFORM_WINDOWS