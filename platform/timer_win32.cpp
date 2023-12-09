#include "timer_win32.h"

#if PLATFORM_WINDOWS

JojPlatform::Win32Timer::Win32Timer()
{
	// Get frequency from high-resolution counter
	QueryPerformanceFrequency(&freq);

	// Reset the start and end values of the counter
	ZeroMemory(&counter_start, sizeof(counter_start));
	ZeroMemory(&end, sizeof(end));

	// Timer working
	stopped = false;
}

void JojPlatform::Win32Timer::start()
{
	if (stopped)
	{
		// Resume count time
		//
		//      <--- elapsed ---->
		// ----|------------------|------------> time
		//    start               end     
		//

		// Time elapsed before stopping
		i64 elapsed = end.QuadPart - counter_start.QuadPart;

		// Takes into account time already elapsed before the stop
		QueryPerformanceCounter(&counter_start);
		counter_start.QuadPart -= elapsed;

		// Resume normal counting
		stopped = false;
	}
	else
	{
		// Start counting time
		QueryPerformanceCounter(&counter_start);
	}
}

void JojPlatform::Win32Timer::stop()
{
	if (!stopped)
	{
		// Mark the stopping point of time
		QueryPerformanceCounter(&end);
		stopped = true;
	}
}

f32 JojPlatform::Win32Timer::reset()
{
	i64 elapsed;

	if (stopped)
	{
		// Get time elapsed before stopping
		elapsed = end.QuadPart - counter_start.QuadPart;

		// Reset time count
		QueryPerformanceCounter(&counter_start);

		// Count reactivated
		stopped = false;
	}
	else
	{
		// End time counting
		QueryPerformanceCounter(&end);

		// Calculate elapsed time (in cycles)
		elapsed = end.QuadPart - counter_start.QuadPart;

		// Reset counter
		counter_start = end;
	}

	// Convert time to seconds
	return f32(elapsed / f64(freq.QuadPart));
}

f32 JojPlatform::Win32Timer::elapsed()
{
	i64 elapsed;

	if (stopped)
	{
		// Take time elapsed until the stop
		elapsed = end.QuadPart - counter_start.QuadPart;

	}
	else
	{
		// End time counting
		QueryPerformanceCounter(&end);

		// Calculate elapsed time (in cycles)
		elapsed = end.QuadPart - counter_start.QuadPart;
	}

	// Convert time to seconds
	return float(elapsed / f64(freq.QuadPart));
}

#endif	// PLATFORM_WINDOWS