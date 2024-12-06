//
// Created by 12105 on 11/23/2024.
//

#include "cstrl/cstrl_types.h"

#include <cstrl/cstrl_platform.h>

#ifdef CSTRL_PLATFORM_WINDOWS

#include "glad/glad.h"
#include "log.c/log.h"
#include "platform_internal.h"

#include <windows.h>
#include <windowsx.h>

static double clock_frequency;
static LARGE_INTEGER start_time;

static bool g_should_exit = false;

short win32_mouse_button_to_cstrl_mouse_button[6];
short win32_key_to_cstrl_key[512];

void win32_mouse_button_to_cstrl_mouse_button_init()
{
    win32_mouse_button_to_cstrl_mouse_button[VK_LBUTTON] = CSTRL_MOUSE_BUTTON_LEFT;
    win32_mouse_button_to_cstrl_mouse_button[VK_RBUTTON] = CSTRL_MOUSE_BUTTON_RIGHT;
    win32_mouse_button_to_cstrl_mouse_button[VK_MBUTTON] = CSTRL_MOUSE_BUTTON_MIDDLE;
}

void win32_key_to_cstrl_key_init()
{
    win32_key_to_cstrl_key[VK_LCONTROL] = CSTRL_KEY_LEFT_CONTROL;
    win32_key_to_cstrl_key[VK_RCONTROL] = CSTRL_KEY_RIGHT_CONTROL;
    win32_key_to_cstrl_key[VK_LSHIFT] = CSTRL_KEY_LEFT_SHIFT;
    win32_key_to_cstrl_key[VK_RSHIFT] = CSTRL_KEY_RIGHT_SHIFT;

    win32_key_to_cstrl_key[VK_TAB] = CSTRL_KEY_TAB;

    win32_key_to_cstrl_key[VK_ESCAPE] = CSTRL_KEY_ESCAPE;

    win32_key_to_cstrl_key[0x41] = CSTRL_KEY_A;
    win32_key_to_cstrl_key[0x42] = CSTRL_KEY_B;
    win32_key_to_cstrl_key[0x43] = CSTRL_KEY_C;
    win32_key_to_cstrl_key[0x44] = CSTRL_KEY_D;
    win32_key_to_cstrl_key[0x45] = CSTRL_KEY_E;
    win32_key_to_cstrl_key[0x46] = CSTRL_KEY_F;
    win32_key_to_cstrl_key[0x47] = CSTRL_KEY_G;
    win32_key_to_cstrl_key[0x48] = CSTRL_KEY_H;
    win32_key_to_cstrl_key[0x49] = CSTRL_KEY_I;
    win32_key_to_cstrl_key[0x4A] = CSTRL_KEY_J;
    win32_key_to_cstrl_key[0x4B] = CSTRL_KEY_K;
    win32_key_to_cstrl_key[0x4C] = CSTRL_KEY_L;
    win32_key_to_cstrl_key[0x4D] = CSTRL_KEY_M;
    win32_key_to_cstrl_key[0x4E] = CSTRL_KEY_N;
    win32_key_to_cstrl_key[0x4F] = CSTRL_KEY_O;
    win32_key_to_cstrl_key[0x50] = CSTRL_KEY_P;
    win32_key_to_cstrl_key[0x51] = CSTRL_KEY_Q;
    win32_key_to_cstrl_key[0x52] = CSTRL_KEY_R;
    win32_key_to_cstrl_key[0x53] = CSTRL_KEY_S;
    win32_key_to_cstrl_key[0x54] = CSTRL_KEY_T;
    win32_key_to_cstrl_key[0x55] = CSTRL_KEY_U;
    win32_key_to_cstrl_key[0x56] = CSTRL_KEY_V;
    win32_key_to_cstrl_key[0x57] = CSTRL_KEY_W;
    win32_key_to_cstrl_key[0x58] = CSTRL_KEY_X;
    win32_key_to_cstrl_key[0x59] = CSTRL_KEY_Y;
    win32_key_to_cstrl_key[0x5A] = CSTRL_KEY_Z;
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
        const int action = (HIWORD(lparam) & KF_UP) ? CSTRL_RELEASE_KEY : CSTRL_PRESS_KEY;

        int scancode = HIWORD(lparam) & (KF_EXTENDED | 0xff);

        if (internal_state->state_common.callbacks.key != NULL)
        {
            internal_state->state_common.callbacks.key(state, win32_key_to_cstrl_key[wparam], scancode, action, 0);
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
            SetCursorPos(1920 / 2, 1080 / 2);
        }
        else
        {
            internal_state->state_common.input.last_mouse_x = x;
            internal_state->state_common.input.last_mouse_y = y;
        }

        return 0;
    }
    case WM_CLOSE:
    case WM_DESTROY:
        g_should_exit = true;
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

    if (!SetPropA(hwnd, "cstrl_platform_state", platform_state))
    {
        log_fatal("Failed to set prop 'cstrl_platform_state'");
        return false;
    }

    state->hwnd = hwnd;

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock_frequency = 1.0 / (double)frequency.QuadPart;
    QueryPerformanceCounter(&start_time);

    state->state_common.callbacks.key = NULL;
    state->state_common.callbacks.mouse_position = NULL;

    win32_mouse_button_to_cstrl_mouse_button_init();
    win32_key_to_cstrl_key_init();

    state->state_common.input.mouse_mode = CSTRL_MOUSE_NORMAL;

    SetCursorPos(1920 / 2, 1080 / 2);

    state->state_common.input.last_mouse_x = 400;
    state->state_common.input.last_mouse_y = 300;

    state->state_common.input.cursor_shown = true;

    return true;
}

void cstrl_platform_shutdown(cstrl_platform_state *platform_state)
{
    internal_state *state = platform_state->internal_state;

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

bool cstrl_platform_should_exit()
{
    return g_should_exit;
}

void cstrl_platform_set_should_exit(bool should_exit)
{
    g_should_exit = should_exit;
}

void cstrl_platform_set_show_cursor(cstrl_platform_state *platform_state, bool show_cursor)
{
    internal_state *state = platform_state->internal_state;
    state->state_common.input.cursor_shown = show_cursor;
    ShowCursor(show_cursor);
}

#endif