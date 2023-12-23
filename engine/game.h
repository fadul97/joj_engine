#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS
#include "platform_manager.h"
#endif	// PLATFORM_WINDOWS

namespace JojEngine
{
	class Game
	{
	public:
		Game();
		virtual ~Game();

		virtual void init() = 0;		// Initialize game
		virtual void update() = 0;		// Update game
		virtual void shutdown() = 0;	// Shutdown game

		/* These methods have an empty implementation by default
		 * and only one of them must be overridden in the derived class:
		 * - Draw is executed every cycle of the main loop.
		 * - Display is called only once at the start of the application
		 * and must be called manually in Update every time the screen needs
		 * to be redrawn.
		 */

		virtual void draw(){}					// Draw game (every cicle)
		virtual void display(){}				// Display game (call manually when screen needs to be redrawn)
		virtual void on_pause() { Sleep(10); }	// On pause

	protected:
		static JojPlatform::Window* window;
		static JojPlatform::Input* input;
	};
}