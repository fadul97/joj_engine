// main.cpp : Defines the entry point for the application.
//

#include "Joj.h"

#include "window_win32.h"
 
using namespace std;

// Include basic Win32 API types and functions
#define WIN32_LEAN_AND_MEAN

// Window Procedure Prototype
LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    HWND         hwnd;        // Window ID
    MSG          msg;         // Message
    WNDCLASS     wndclass;    // Window class

    // Defining a window class called "AppWindow"
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WinProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = "AppWindow";

    // Registering the "AppWindow" class
    if (!RegisterClass(&wndclass))
    {
        MessageBox(NULL, "Failed to register window", "Application", MB_ICONERROR);
        return 0;
    }

    // Creating a Window Based on the "AppWindow" Class
    hwnd = CreateWindow("AppWindow",	    // Window class
        "Joj Engine",                      // Window title
        WS_OVERLAPPEDWINDOW |
        WS_VISIBLE,                       // Window style
        CW_USEDEFAULT,                    // Initial x position
        CW_USEDEFAULT,                    // Initial y position
        CW_USEDEFAULT,                    // Initial width
        CW_USEDEFAULT,                    // Initial height
        NULL,                             // Parent window ID
        NULL,                             // Menu ID
        hInstance,                        // Application ID
        NULL);                            // Creation parameters

    // Handling messages destined for the application window
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }


    return 0;
}

//-------------------------------------------------------------------------------

// Window procedure
LRESULT CALLBACK WinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        // Window closed
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}
