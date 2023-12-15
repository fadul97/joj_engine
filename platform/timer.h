#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include <windows.h>

namespace JojPlatform
{
	class Timer
	{
	public:
		Timer();
		~Timer();

		void start();				// Start/resume counting time
		void stop();				// Stop counting time
		f32 reset();				// Restarts counter and returns elapsed time
		f32 elapsed();				// Returns elapsed time in seconds	
		b8 was_elapsed(f32 secs);	// Checks if "secs" seconds have passed
		void time_begin_period();	// Adjust sleep resolution to 1 millisecond
		void time_end_period();		// Return sleep resolution to original value

	private:
		LARGE_INTEGER counter_start;	// Start of counter 
		LARGE_INTEGER end;				// End of counter
		LARGE_INTEGER freq;				// Counter frequency
		b8 stopped;						// Counter state
	};
	inline b8 Timer::was_elapsed(f32 secs)
	{ return (elapsed() >= secs ? true : false); }

	inline void Timer::time_begin_period()
	{ timeBeginPeriod(1); }

	inline void Timer::time_end_period()
	{ timeEndPeriod(1); }
}

#endif  // PLATFORM_WINDOWS