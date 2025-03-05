#ifndef UI_INTERNAL_H
#define UI_INTERNAL_H

#include "cstrl/cstrl_platform.h"
#include "stb/stb_image_write.h"
#include "stb/stb_truetype.h"

void *cstrl_ui_renderer_init(cstrl_platform_state *platform_state);

stbtt_packedchar *cstrl_ui_renderer_get_char_data(void *internal_render_state);

void cstrl_ui_renderer_new_frame(void *internal_render_state);

void cstrl_ui_renderer_add_rect_position(void *internal_render_state, int x0, int y0, int x1, int y1);

void cstrl_ui_renderer_add_rect_uv(void *internal_render_state, float u0, float v0, float u1, float v1);

void cstrl_ui_renderer_add_rect_color(void *internal_render_state, float r, float g, float b, float a);

void cstrl_ui_renderer_draw_rects(void *internal_render_state);

void cstrl_ui_renderer_add_font_position(void *internal_render_state, int x0, int y0, int x1, int y1);

void cstrl_ui_renderer_add_font_uv(void *internal_render_state, float u0, float v0, float u1, float v1);

void cstrl_ui_renderer_add_font_color(void *internal_render_state, float r, float g, float b, float a);

void cstrl_ui_renderer_draw_font(void *internal_render_state);

void cstrl_ui_renderer_shutdown(void *internal_render_state);

#endif // UI_INTERNAL_H
