//
// Created by 12105 on 11/23/2024.
//

#include "cstrl/cstrl_types.h"

#include <cstrl/cstrl_platform.h>

#if defined(CSTRL_PLATFORM_WINDOWS)

#include "log.c/log.h"
#include "platform_internal.h"

#include <windows.h>
#include <windowsx.h>

static double g_clock_frequency;
static LARGE_INTEGER g_start_time;

static bool g_should_exit = false;

short g_win32_key_to_cstrl_key[512];

void win32_key_to_cstrl_key_init()
{
    g_win32_key_to_cstrl_key[VK_LCONTROL] = CSTRL_KEY_LEFT_CONTROL;
    g_win32_key_to_cstrl_key[VK_RCONTROL] = CSTRL_KEY_RIGHT_CONTROL;
    g_win32_key_to_cstrl_key[VK_LSHIFT] = CSTRL_KEY_LEFT_SHIFT;
    g_win32_key_to_cstrl_key[VK_RSHIFT] = CSTRL_KEY_RIGHT_SHIFT;
    g_win32_key_to_cstrl_key[VK_BACK] = CSTRL_KEY_BACKSPACE;

    g_win32_key_to_cstrl_key[VK_TAB] = CSTRL_KEY_TAB;

    g_win32_key_to_cstrl_key[VK_ESCAPE] = CSTRL_KEY_ESCAPE;

    g_win32_key_to_cstrl_key[0x30] = CSTRL_KEY_0;
    g_win32_key_to_cstrl_key[0x31] = CSTRL_KEY_1;
    g_win32_key_to_cstrl_key[0x32] = CSTRL_KEY_2;
    g_win32_key_to_cstrl_key[0x33] = CSTRL_KEY_3;
    g_win32_key_to_cstrl_key[0x34] = CSTRL_KEY_4;
    g_win32_key_to_cstrl_key[0x35] = CSTRL_KEY_5;
    g_win32_key_to_cstrl_key[0x36] = CSTRL_KEY_6;
    g_win32_key_to_cstrl_key[0x37] = CSTRL_KEY_7;
    g_win32_key_to_cstrl_key[0x38] = CSTRL_KEY_8;
    g_win32_key_to_cstrl_key[0x39] = CSTRL_KEY_9;

    g_win32_key_to_cstrl_key[VK_OEM_PERIOD] = CSTRL_KEY_PERIOD;

    g_win32_key_to_cstrl_key[0x41] = CSTRL_KEY_A;
    g_win32_key_to_cstrl_key[0x42] = CSTRL_KEY_B;
    g_win32_key_to_cstrl_key[0x43] = CSTRL_KEY_C;
    g_win32_key_to_cstrl_key[0x44] = CSTRL_KEY_D;
    g_win32_key_to_cstrl_key[0x45] = CSTRL_KEY_E;
    g_win32_key_to_cstrl_key[0x46] = CSTRL_KEY_F;
    g_win32_key_to_cstrl_key[0x47] = CSTRL_KEY_G;
    g_win32_key_to_cstrl_key[0x48] = CSTRL_KEY_H;
    g_win32_key_to_cstrl_key[0x49] = CSTRL_KEY_I;
    g_win32_key_to_cstrl_key[0x4A] = CSTRL_KEY_J;
    g_win32_key_to_cstrl_key[0x4B] = CSTRL_KEY_K;
    g_win32_key_to_cstrl_key[0x4C] = CSTRL_KEY_L;
    g_win32_key_to_cstrl_key[0x4D] = CSTRL_KEY_M;
    g_win32_key_to_cstrl_key[0x4E] = CSTRL_KEY_N;
    g_win32_key_to_cstrl_key[0x4F] = CSTRL_KEY_O;
    g_win32_key_to_cstrl_key[0x50] = CSTRL_KEY_P;
    g_win32_key_to_cstrl_key[0x51] = CSTRL_KEY_Q;
    g_win32_key_to_cstrl_key[0x52] = CSTRL_KEY_R;
    g_win32_key_to_cstrl_key[0x53] = CSTRL_KEY_S;
    g_win32_key_to_cstrl_key[0x54] = CSTRL_KEY_T;
    g_win32_key_to_cstrl_key[0x55] = CSTRL_KEY_U;
    g_win32_key_to_cstrl_key[0x56] = CSTRL_KEY_V;
    g_win32_key_to_cstrl_key[0x57] = CSTRL_KEY_W;
    g_win32_key_to_cstrl_key[0x58] = CSTRL_KEY_X;
    g_win32_key_to_cstrl_key[0x59] = CSTRL_KEY_Y;
    g_win32_key_to_cstrl_key[0x5A] = CSTRL_KEY_Z;
}

LRESULT CALLBACK win32_process_messages(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_CREATE: {
        return 0;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP: {
        cstrl_platform_state *state = GetPropW(hwnd, L"cstrl_platform_state");
        if (state == NULL)
        {
            log_trace("State is NULL, skipping keyboard input");
            return 0;
        }
        internal_state *internal_state = state->internal_state;
        const int action = (HIWORD(lparam) & KF_UP) ? CSTRL_ACTION_RELEASE : CSTRL_ACTION_PRESS;

        int scancode = HIWORD(lparam) & (KF_EXTENDED | 0xff);

        if (internal_state->state_common.callbacks.key != NULL)
        {
            internal_state->state_common.callbacks.key(state, g_win32_key_to_cstrl_key[wparam], scancode, action, 0);
        }
        if (action == CSTRL_ACTION_PRESS)
        {
            internal_state->state_common.input.most_recent_key_pressed = g_win32_key_to_cstrl_key[wparam];
        }
        else
        {
            internal_state->state_common.input.most_recent_key_pressed = CSTRL_KEY_NONE;
        }
        return 0;
    }
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_XBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP: {
        cstrl_mouse_button button;
        if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP)
        {
            button = CSTRL_MOUSE_BUTTON_LEFT;
        }
        else if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP)
        {
            button = CSTRL_MOUSE_BUTTON_RIGHT;
        }
        else if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP)
        {
            button = CSTRL_MOUSE_BUTTON_MIDDLE;
        }
        else
        {
            button = CSTRL_MOUSE_BUTTON_UNKNOWN;
        }

        cstrl_action action;
        if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_XBUTTONDOWN)
        {
            action = CSTRL_ACTION_PRESS;
        }
        else
        {
            action = CSTRL_ACTION_RELEASE;
        }

        cstrl_platform_state *state = GetPropW(hwnd, L"cstrl_platform_state");
        if (state == NULL)
        {
            log_trace("State is NULL, skipping mouse button input");
            return 0;
        }
        internal_state *internal_state = state->internal_state;

        internal_state->state_common.input.mouse_buttons[button] = action;

        if (internal_state->state_common.callbacks.mouse_button != NULL)
        {
            // TODO: support all modifiers windows supports
            int mods = 0;
            if (wparam & MK_SHIFT)
            {
                mods |= CSTRL_KEY_MOD_SHIFT;
            }
            if (wparam & MK_CONTROL)
            {
                mods |= CSTRL_KEY_MOD_CONTROL;
            }
            if (HIBYTE(GetKeyState(VK_MENU)) & 0x80)
            {
                mods |= CSTRL_KEY_MOD_ALT;
            }
            internal_state->state_common.callbacks.mouse_button(state, button, action, mods);
        }
        return 0;
    }
    case WM_MOUSEMOVE: {
        cstrl_platform_state *state = GetPropW(hwnd, L"cstrl_platform_state");
        if (state == NULL)
        {
            log_trace("State is NULL, skipping mouse move input");
            return 0;
        }
        internal_state *internal_state = state->internal_state;

        int x = GET_X_LPARAM(lparam);
        int y = GET_Y_LPARAM(lparam);

        if (internal_state->state_common.callbacks.mouse_position != NULL)
        {
            internal_state->state_common.callbacks.mouse_position(state, x, y);
        }
        if (internal_state->state_common.input.mouse_mode == CSTRL_MOUSE_DISABLED)
        {
            // TODO: base this on screen dimensions
            SetCursorPos(1920 / 2, 1080 / 2);
        }
        else
        {
            internal_state->state_common.input.last_mouse_x = x;
            internal_state->state_common.input.last_mouse_y = y;
        }

        return 0;
    }
    case WM_SIZE: {
        const int width = LOWORD(lparam);
        const int height = HIWORD(lparam);
        cstrl_platform_state *state = GetPropW(hwnd, L"cstrl_platform_state");
        if (state == NULL)
        {
            log_trace("State is NULL, skipping size update");
            return 0;
        }
        internal_state *internal_state = state->internal_state;

        if (internal_state->state_common.window_width != width || internal_state->state_common.window_height != height)
        {
            internal_state->state_common.window_width = width;
            internal_state->state_common.window_height = height;

            if (internal_state->state_common.callbacks.framebuffer_size != NULL)
            {
                internal_state->state_common.callbacks.framebuffer_size(state, width, height);
            }
        }
        return 0;
    }
    case WM_MOUSEWHEEL: {
        short delta = GET_WHEEL_DELTA_WPARAM(wparam);
        short fw_keys = GET_KEYSTATE_WPARAM(wparam);
        cstrl_platform_state *state = GetPropW(hwnd, L"cstrl_platform_state");
        if (state == NULL)
        {
            log_trace("State is NULL, skipping mouse wheel update");
            return 0;
        }
        internal_state *internal_state = state->internal_state;
        if (internal_state->state_common.callbacks.mouse_wheel != NULL)
        {
            internal_state->state_common.callbacks.mouse_wheel(state, 0, delta, fw_keys);
        }
        return 0;
    }
    case WM_MOUSEHWHEEL: {
        short delta = GET_WHEEL_DELTA_WPARAM(wparam);
        short fw_keys = GET_KEYSTATE_WPARAM(wparam);
        cstrl_platform_state *state = GetPropW(hwnd, L"cstrl_platform_state");
        if (state == NULL)
        {
            log_trace("State is NULL, skipping mouse wheel update");
            return 0;
        }
        internal_state *internal_state = state->internal_state;

        if (internal_state->state_common.callbacks.mouse_wheel != NULL)
        {
            internal_state->state_common.callbacks.mouse_wheel(state, delta, 0, fw_keys);
        }
        return 0;
    }
    case WM_CLOSE:
    case WM_DESTROY:
        g_should_exit = true;
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}

CSTRL_API bool cstrl_platform_init(cstrl_platform_state *platform_state, const char *application_name, int x, int y,
                                   int width, int height, bool fullscreen)
{
    platform_state->internal_state = malloc(sizeof(internal_state));
    internal_state *state = platform_state->internal_state;

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

    unsigned int window_style;
    unsigned int window_ex_style;
    if (fullscreen)
    {
        window_style = WS_POPUP | WS_MAXIMIZE;
        window_ex_style = 0;
    }
    else
    {
        window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
        window_ex_style = WS_EX_APPWINDOW;
    }

    RECT border_rect = {0};
    AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

    int window_x;
    int window_y;
    int window_width;
    int window_height;
    if (fullscreen)
    {
        window_x = 0;
        window_y = 0;
        window_width = 1920;
        window_height = 1080;
    }
    else
    {
        window_x = x + border_rect.left;
        window_y = y + border_rect.top;
        window_width = width + border_rect.right - border_rect.left;
        window_height = height + border_rect.bottom - border_rect.top;
    }

    HWND hwnd = CreateWindowExA(window_ex_style, "cstrl_window_class", application_name, window_style, window_x,
                                window_y, window_width, window_height, 0, 0, state->h_instance, 0);

    if (hwnd == 0)
    {
        MessageBoxA(NULL, "Window creation failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        log_fatal("Window creation failed");
        return false;
    }

    if (!SetPropA(hwnd, "cstrl_platform_state", platform_state))
    {
        log_fatal("Failed to set prop 'cstrl_platform_state'");
        return false;
    }

    state->hwnd = hwnd;

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    g_clock_frequency = 1.0 / (double)frequency.QuadPart;
    QueryPerformanceCounter(&g_start_time);

    state->state_common.callbacks.key = NULL;
    state->state_common.callbacks.mouse_position = NULL;
    state->state_common.callbacks.framebuffer_size = NULL;
    state->state_common.callbacks.mouse_button = NULL;
    state->state_common.callbacks.mouse_wheel = NULL;

    win32_key_to_cstrl_key_init();

    state->state_common.input.mouse_mode = CSTRL_MOUSE_NORMAL;

    // TODO: get screen dimensions
    SetCursorPos(1920 / 2, 1080 / 2);

    state->state_common.input.last_mouse_x = width / 2;
    state->state_common.input.last_mouse_y = height / 2;

    state->state_common.input.cursor_shown = true;

    state->state_common.window_width = window_width;
    state->state_common.window_height = window_height;

    ShowWindow(state->hwnd, SW_SHOW);

    return true;
}

CSTRL_API void cstrl_platform_shutdown(cstrl_platform_state *platform_state)
{
    internal_state *state = platform_state->internal_state;

    if (state->hwnd)
    {
        DestroyWindow(state->hwnd);
        state->hwnd = 0;
    }
}

CSTRL_API void cstrl_platform_pump_messages(cstrl_platform_state *platform_state)
{
    MSG msg = {};
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

CSTRL_API double cstrl_platform_get_absolute_time()
{
    LARGE_INTEGER current_time;
    QueryPerformanceCounter(&current_time);
    return (double)current_time.QuadPart * g_clock_frequency;
}

CSTRL_API void cstrl_platform_sleep(unsigned long long ms)
{
    Sleep(ms);
}

CSTRL_API bool cstrl_platform_should_exit()
{
    return g_should_exit;
}

CSTRL_API void cstrl_platform_set_should_exit(bool should_exit)
{
    g_should_exit = should_exit;
}

CSTRL_API void cstrl_platform_set_show_cursor(cstrl_platform_state *platform_state, bool show_cursor)
{
    internal_state *state = platform_state->internal_state;
    state->state_common.input.cursor_shown = show_cursor;
    ShowCursor(show_cursor);
}

#endif
