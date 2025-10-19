#ifndef CSTRL_NUKLEAR_UI_H
#define CSTRL_NUKLEAR_UI_H

#include "cstrl/cstrl_platform.h"
#include "cstrl_defines.h"
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_API CSTRL_API
#include <nuklear/nuklear.h>

typedef struct cstrl_nuklear_context {
    struct nk_context nk_context;
} cstrl_nuklear_context;

CSTRL_API void cstrl_nuklear_init(struct nk_context *context, cstrl_platform_state *platform_state);

CSTRL_API void cstrl_nuklear_font_stash_begin(struct nk_font_atlas **atlas);

CSTRL_API void cstrl_nuklear_font_stash_end(struct nk_context *context);

CSTRL_API void cstrl_nuklear_start(struct nk_context *context, cstrl_platform_state *platform_state);

CSTRL_API void cstrl_nuklear_end(struct nk_context *context, cstrl_platform_state *platform_state);

CSTRL_API void cstrl_nuklear_shutdown(struct nk_context *context);

#endif  // CSTRL_NUKLEAR_UI_H
