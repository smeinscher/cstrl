#include "cstrl/cstrl_util.h"
#include <stdio.h>

static void *game_dl = NULL;
static time_t g_last_edit_ts = 0;

#ifdef CSTRL_PLATFORM_WINDOWS
#include "windows.h"
#include <libloaderapi.h>
#include <minwindef.h>
#include <processenv.h>
bool (*g_game_init_func_ptr)(void);
PROC g_game_update_func_ptr;
PROC g_game_render_func_ptr;
PROC g_game_shutdown_func_ptr;
PROC g_game_is_running_func_ptr;

DWORD WINAPI hot_reload_thread(LPVOID vargp)
{
    bool *done_reloading = (bool *)vargp;
    printf("Hot Reload!\n");
    if (game_dl)
    {
        FreeLibrary(game_dl);
        game_dl = NULL;
    }
    if (cstrl_copy_file("build-debug\\cstrl-rpg-game.dll", "build-debug\\cstrl-rpg-game-load.dll"))
    {
        printf("Failed to copy library\n");
        return 0;
    }
    game_dl = LoadLibrary("build-debug\\cstrl-rpg-game-load.dll");
    *done_reloading = true;
    printf("Finished\n");
    return 0;
}

void reload_game()
{
    time_t current_ts = cstrl_get_file_timestamp("build-debug\\cstrl-rpg-game.dll");
    char cwd[2048];
    GetCurrentDirectoryA(2048, cwd);
    printf("%s\n", cwd);
    if (current_ts > g_last_edit_ts)
    {
        bool done_reloading = false;
        HANDLE loading_thread_id = CreateThread(NULL, 0, hot_reload_thread, &done_reloading, 0, NULL);
        WaitForSingleObject(hot_reload_thread, INFINITE);
        CloseHandle(loading_thread_id);
        g_game_init_func_ptr = (bool (*)(void))GetProcAddress(game_dl, "rpg_game_init");
        g_game_update_func_ptr = GetProcAddress(game_dl, "rpg_game_update");
        g_game_render_func_ptr = GetProcAddress(game_dl, "rpg_game_render");
        g_game_shutdown_func_ptr = GetProcAddress(game_dl, "rpg_game_shutdown");
        g_game_is_running_func_ptr = GetProcAddress(game_dl, "rpg_game_is_running");
        g_last_edit_ts = current_ts;
    }
}
#endif

int main()
{
    reload_game();
    if (!g_game_init_func_ptr())
    {
        printf("Error initializing game\n");
        return 1;
    }

    while (g_game_is_running_func_ptr())
    {
        reload_game();
        g_game_update_func_ptr();
        g_game_render_func_ptr();
    }

    g_game_shutdown_func_ptr();
    return 0;
}
