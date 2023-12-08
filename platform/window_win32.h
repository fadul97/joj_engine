#pragma once

#include "defines.h"

#include <windows.h>	// Include Win32 functions
#include <windowsx.h>	// Include Win32 extensions
#include <string>

void print_window_win32(u32 n);

namespace JojPlatform
{
	enum WindowModes { FULLSCREEN, WINDOWED };

	class Win32Window
	{
	public:
		Win32Window();
		~Win32Window();

		HWND get_id();								// Return window ID
		i16 get_width();							// Return window width
		i16 get_height();							// Return window height
		u32 get_mode() const;						// Return window mode (Full-screen, windowed or borderless mode)
		i16 get_xcenter() const;					// Return center position in x
		i16 get_ycenter() const;					// Return center position in y
		std::string get_title() const;				// Return window title
		COLORREF get_color();						// Return window background color

		void set_icon(const u32 icon);				// Set window icon
		void set_cursor(const u32 cursor);			// Set window cursor
		void set_title(const std::string title);	// Set window title
		void set_size(i16 width, i16 height);		// Set window width and height
		void set_mode(u32 mode);					// Set window mode (Full-screen, windowed or borderless mode)
		void set_color(u32 r, u32 g, u32 b);		// Set window background color

		void hide_cursor(b8 hide);					// Enable or disable cursor display
		
		/*
		 *	@brief Displays the text at the (x,y) position on the screen using the specified color,
		 *	it uses Windows GDI (slow) and should only be used for debugging.
		 */
		void print_on_window(std::string text, i16 x, i16 y, COLORREF color);
		
		void close();
		void clear();
		b8 create();

		void set_on_focus(void(*func)());			// Set function to be executed when regaining focus
		void set_lost_focus(void(*func)());			// Set function to be executed when losing focus

		// Handle Window events
		static LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		HDC			hdc;    						// Device context
		RECT		rect;							// Window client area
		HWND		id;								// Window ID
		i16			width;							// Window width
		i16			height;							// Window heigh
		HICON		icon;							// Window icon
		HCURSOR		cursor;							// Window cursor
		COLORREF	color;							// Window background color
		std::string	title;							// Window title
		DWORD		style;							// Window style 
		u32			mode;							// Full-screen, windowed or borderless mode
		i16			xpos;							// Initial window position on the x-axis
		i16			ypos;							// Initial window position on the y-axis
		i16			xcenter;						// Window center on the x-axis
		i16			ycenter;						// Window center on the y-axis

		static void (*on_focus)();					// Run when window regains focus
		static void (*lost_focus)();				// Run when window loses focus
	};

	// Return window ID
	inline HWND Win32Window::get_id()
	{ return id; }

	// Return window width
	inline i16 Win32Window::get_width()
	{ return width; }

	// Return window height
	inline i16 Win32Window::get_height()
	{ return height; }

	// Return window mode (Full-screen, windowed, or borderless mode)
	inline u32 Win32Window::get_mode() const
	{ return mode; }

	// Return center position in x
	inline i16 Win32Window::get_xcenter() const
	{return xcenter; }

	// Return center position in y
	inline i16 Win32Window::get_ycenter() const
	{ return ycenter; }

	// Return window title
	inline std::string Win32Window::get_title() const
	{ return title; }

	// Return window background color
	inline COLORREF Win32Window::get_color()
	{ return color; }

	// Set window icon
	inline void Win32Window::set_icon(const u32 icon)
	{ this->icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(icon)); }

	// Set window cursor
	inline void Win32Window::set_cursor(const u32 cursor)
	{ this->cursor = LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(cursor)); }

	// Set window title
	inline void Win32Window::set_title(const std::string title)
	{ this->title = title; }

	// Set window background color
	inline void Win32Window::set_color(u32 r, u32 g, u32 b)
	{ this->color = RGB(r, g, b); }

	// Enable or disable cursor display
	inline void Win32Window::hide_cursor(b8 hide)
	{ ShowCursor(!hide); }

	// Close window
	inline void Win32Window::close()
	{ PostMessage(id, WM_DESTROY, 0, 0); }

	// Clear client area
	inline void Win32Window::clear()
	{ FillRect(hdc, &rect, CreateSolidBrush(get_color())); }

	// Set function to be executed when regaining focus
	inline void Win32Window::set_on_focus(void(*func)())
	{ on_focus = func; }

	// // Set function to be executed when losing focus
	inline void Win32Window::set_lost_focus(void(*func)())
	{ lost_focus = func; }

}	// namespace JojPlatform