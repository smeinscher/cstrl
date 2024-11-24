//
// Created by 12105 on 11/23/2024.
//

#ifndef PLATFORM_INTERNAL_H
#define PLATFORM_INTERNAL_H
#include <windows.h>

typedef struct internal_state
{
    HINSTANCE h_instance;
    HWND hwnd;
} internal_state;

#endif // PLATFORM_INTERNAL_H
