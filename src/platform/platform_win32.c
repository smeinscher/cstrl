//
// Created by 12105 on 11/23/2024.
//

#include <cstrl/cstrl_platform.h>

#ifdef CSTRL_PLATFORM_WINDOWS

#include "glad/glad.h"
#include "log.c/log.h"
#include "platform_internal.h"

#include <windows.h>

static double clock_frequency;
static LARGE_INTEGER start_time;

static bool should_exit = false;

LRESULT CALLBACK win32_process_messages(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_CREATE: {
        return 0;
    }
    case WM_CLOSE:
    case WM_DESTROY:
        should_exit = true;
        PostQuitMessage(0);
        return 0;
    // case WM_PAINT: {
    //     PAINTSTRUCT ps;
    //     hdc = BeginPaint(hwnd, &ps);
    //
    //     FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
    //
    //     EndPaint(hwnd, &ps);
    //     return 0;
    // }
    default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}

bool cstrl_platform_init(cstrl_platform_state *platform_state, const char *application_name, int x, int y, int width,
                         int height)
{
    platform_state->internal_state = malloc(sizeof(internal_state));
    internal_state *state = (internal_state *)platform_state->internal_state;

    state->h_instance = GetModuleHandleA(0);

    HICON icon = LoadIcon(state->h_instance, IDI_APPLICATION);
    WNDCLASSA wc = {0};
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = win32_process_messages;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = state->h_instance;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = "cstrl_window_class";

    if (!RegisterClassA(&wc))
    {
        MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    unsigned int window_style =
        WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
    unsigned int window_ex_style = WS_EX_APPWINDOW;

    RECT border_rect = {0};
    AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

    int window_x = x + border_rect.left;
    int window_y = y + border_rect.top;
    int window_width = width + border_rect.right - border_rect.left;
    int window_height = height + border_rect.bottom - border_rect.top;

    HWND hwnd = CreateWindowExA(window_ex_style, "cstrl_window_class", application_name, window_style, window_x,
                                window_y, window_width, window_height, 0, 0, state->h_instance, 0);

    if (hwnd == 0)
    {
        MessageBoxA(NULL, "Window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        log_fatal("Window creation failed");
        return false;
    }
    state->hwnd = hwnd;

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock_frequency = 1.0 / (double)frequency.QuadPart;
    QueryPerformanceCounter(&start_time);

    return true;
}

void cstrl_platform_destroy(cstrl_platform_state *platform_state)
{
    internal_state *state = (internal_state *)platform_state->internal_state;

    if (state->hwnd)
    {
        DestroyWindow(state->hwnd);
        state->hwnd = 0;
    }
}

void cstrl_platform_pump_messages(cstrl_platform_state *platform_state)
{
    MSG msg = {};
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

double cstrl_platform_get_absolute_time()
{
    LARGE_INTEGER current_time;
    QueryPerformanceCounter(&current_time);
    return (double)current_time.QuadPart * clock_frequency;
}

void cstrl_platform_sleep(unsigned long long ms)
{
    Sleep(ms);
}

bool cstrl_platform_should_exit(cstrl_platform_state *platform_state)
{
    return should_exit;
}

#endif