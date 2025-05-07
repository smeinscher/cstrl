#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#if defined(CSTRL_RPG_GAME_EXPORT)
#if defined(_MSC_VER)
#define GAME_API _declspec(dllexport)
#else
#define GAME_API __attribute__((visibility("default")))
#endif
#else
#if defined(_MSC_VER)
#define GAME_API __declspec(dllimport)
#else
#define GAME_API
#endif
#endif

GAME_API bool rpg_game_init();

GAME_API void rpg_game_update();

GAME_API void rpg_game_render();

GAME_API void rpg_game_shutdown();

GAME_API bool rpg_game_is_running();

#endif
