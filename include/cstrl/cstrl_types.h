//
// Created by 12105 on 11/23/2024.
//

#ifndef CSTRL_TYPES_H
#define CSTRL_TYPES_H

#include <stddef.h>

typedef struct da_int
{
    int *array;
    size_t size;
    size_t capacity;
} da_int;

typedef struct da_float
{
    float *array;
    size_t size;
    size_t capacity;
} da_float;

typedef struct da_char
{
    char *array;
    size_t size;
    size_t capacity;
} da_char;

typedef da_char string;

typedef struct da_string
{
    string *array;
    size_t size;
    size_t capacity;
} da_string;

typedef enum cstrl_mouse_buttons
{
    CSTRL_MOUSE_BUTTON_LEFT,
    CSTRL_MOUSE_BUTTON_RIGHT,
    CSTRL_MOUSE_BUTTON_MIDDLE,
    CSTRL_MOUSE_BUTTON_MAX
} cstrl_mouse_buttons;

#define DEFINE_KEY(name, code) CSTRL_KEY_##name = code
typedef enum cstrl_keys
{
    DEFINE_KEY(LEFT_CONTROL, 17),
    DEFINE_KEY(RIGHT_CONTROL, 18),
    DEFINE_KEY(LEFT_SHIFT, 19),
    DEFINE_KEY(RIGHT_SHIFT, 20),

    DEFINE_KEY(TAB, 21),

    DEFINE_KEY(ESCAPE, 27),

    DEFINE_KEY(A, 65),
    DEFINE_KEY(B, 66),
    DEFINE_KEY(C, 67),
    DEFINE_KEY(D, 68),
    DEFINE_KEY(E, 69),
    DEFINE_KEY(F, 70),
    DEFINE_KEY(G, 71),
    DEFINE_KEY(H, 72),
    DEFINE_KEY(I, 73),
    DEFINE_KEY(J, 74),
    DEFINE_KEY(K, 75),
    DEFINE_KEY(L, 76),
    DEFINE_KEY(M, 77),
    DEFINE_KEY(N, 78),
    DEFINE_KEY(O, 79),
    DEFINE_KEY(P, 80),
    DEFINE_KEY(Q, 81),
    DEFINE_KEY(R, 82),
    DEFINE_KEY(S, 83),
    DEFINE_KEY(T, 84),
    DEFINE_KEY(U, 85),
    DEFINE_KEY(V, 86),
    DEFINE_KEY(W, 87),
    DEFINE_KEY(X, 88),
    DEFINE_KEY(Y, 89),
    DEFINE_KEY(Z, 90),

    DEFINE_KEY(MAX, 192)
} cstrl_keys;

typedef enum cstrl_key_modifiers
{
    CSTRL_KEY_MOD_SHIFT = 0x0001,
    CSTRL_KEY_MOD_CONTROL = 0x0002,
    CSTRL_KEY_MOD_ALT = 0x0004,
    CSTRL_KEY_MOD_SUPER = 0x0008,
    CSTRL_KEY_MOD_CAPS_LOCK = 0x0010,
    CSTRL_KEY_MOD_NUM_LOCK = 0x0020
} cstrl_key_modifiers;

typedef enum cstrl_key_states
{
    CSTRL_PRESS_KEY,
    CSTRL_RELEASE_KEY,
    CSTRL_REPEAT_KEY
} cstrl_key_states;

typedef enum cstrl_mouse_modes
{
    CSTRL_MOUSE_NORMAL,
    CSTRL_MOUSE_DISABLED
} cstrl_mouse_mode;

#endif // CSTRL_TYPES_H
