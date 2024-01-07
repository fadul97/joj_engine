#pragma once

#include "defines.h"

#ifdef PLATFORM_WINDOWS

#include <windows.h>	// Include Win32 functions
#include <windowsx.h>	// Include Win32 extensions
#include <string>

namespace JojPlatform
{
	enum class WindowMode { FULLSCREEN, WINDOWED };

	class Window
	{
	public:
		Window();
		~Window();

		HWND get_id() const;            // Return window ID
		HDC get_device_context() const;	// Return device context
		i32 get_width() const;          // Return window width
		i32 get_height() const;         // Return window height
		WindowMode get_mode() const;	// Return window mode (Fullscreen, windowed or borderless mode)
		i32 get_xcenter() const;        // Return center position in x
		i32 get_ycenter() const;        // Return center position in y
		std::string get_title() const;  // Return window title
		COLORREF get_color() const;     // Return window background color
		f32 get_aspect_ratio() const;   // Return window aspect ratio

		void set_icon(const u32 icon);              // Set window icon
		void set_cursor(const u32 cursor);          // Set window cursor
		void set_title(const std::string title);    // Set window title
		void set_size(i32 width, i32 height);       // Set window width and height
		void set_mode(WindowMode mode);             // Set window mode (Full-screen, windowed or borderless mode)
		void set_color(u32 r, u32 g, u32 b);        // Set window background color

		void hide_cursor(b8 hide);                  // Enable or disable cursor display
		
		/*
		 *	@brief Displays the text at the (x,y) position on the screen using the specified color,
		 *	it uses Windows GDI (slow) and should only be used for debugging.
		 */
		void print_on_window(std::string text, i16 x, i16 y, COLORREF color);
		
		void close();
		void clear();
		b8 create();

		void set_on_focus(void(*func)());	// Set function to be executed when regaining focus
		void set_lost_focus(void(*func)());	// Set function to be executed when losing focus

		// Handle Window events
		static LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		HWND id;            // Window ID
		HDC hdc;            // Device context
		i32 width;          // Window width
		i32 height;         // Window heigh
		std::string title;  // Window title
		RECT rect;          // Window client area
		HICON icon;         // Window icon
		HCURSOR cursor;     // Window cursor
		COLORREF color;     // Window background color
		DWORD style;        // Window style 
		WindowMode mode;    // Full-screen, windowed or borderless mode
		i32 xpos;           // Initial window position on the x-axis
		i32 ypos;           // Initial window position on the y-axis
		i32 xcenter;        // Window center on the x-axis
		i32 ycenter;        // Window center on the y-axis

		static void (*on_focus)();      // Run when window regains focus
		static void (*lost_focus)();    // Run when window loses focus
	};

	// Return window ID
	inline HWND Window::get_id() const
	{ return id; }

	// Return device context
	inline HDC Window::get_device_context() const
	{ return hdc; }

	// Return window width
	inline i32 Window::get_width() const
	{ return width; }

	// Return window height
	inline i32 Window::get_height() const
	{ return height; }

	// Return window mode (Fullscreen, windowed, or borderless mode)
	inline WindowMode Window::get_mode() const
	{ return mode; }

	// Return center position in x
	inline i32 Window::get_xcenter() const
	{return xcenter; }

	// Return center position in y
	inline i32 Window::get_ycenter() const
	{ return ycenter; }

	// Return window title
	inline std::string Window::get_title() const
	{ return title; }

	// Return window background color
	inline COLORREF Window::get_color() const
	{ return color; }

	// Return window aspect ratio
	inline f32 Window::get_aspect_ratio() const
	{ return width / f32(height); }

	// Set window icon
	inline void Window::set_icon(const u32 icon)
	{ this->icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(icon)); }

	// Set window cursor
	inline void Window::set_cursor(const u32 cursor)
	{ this->cursor = LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(cursor)); }

	// Set window title
	inline void Window::set_title(const std::string title)
	{ this->title = title; }

	// Set window background color
	inline void Window::set_color(u32 r, u32 g, u32 b)
	{ this->color = RGB(r, g, b); }

	// Enable or disable cursor display
	inline void Window::hide_cursor(b8 hide)
	{ ShowCursor(!hide); }

	// Close window
	inline void Window::close()
	{ PostMessage(id, WM_DESTROY, 0, 0); }

	// Clear client area
	inline void Window::clear()
	{ FillRect(hdc, &rect, CreateSolidBrush(get_color())); }

	// Set function to be executed when regaining focus
	inline void Window::set_on_focus(void(*func)())
	{ on_focus = func; }

	// // Set function to be executed when losing focus
	inline void Window::set_lost_focus(void(*func)())
	{ lost_focus = func; }

} // namespace JojPlatform

#endif // PLATFORM_WINDOWS
