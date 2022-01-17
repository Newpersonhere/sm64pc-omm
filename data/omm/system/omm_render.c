#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS
extern f32 omm_get_life_up_gauge_position(s32 hp);
extern f32 omm_get_life_gauge_rumble_strength(struct MarioState *m);

//
//       COORDS
//
//   |         
//   |   _    _
//   |  | \  / |
//   |  ||\\//||
// y--- || \/ ||
//   |
//   +--|---------
//      x
///////////////////

s32 omm_render_get_string_width(s32 font, const u8 *str64) {
    s32 width = 0;
    for (; *str64 != 0xFF; str64++) {
        switch (font) {
            case OMM_RENDER_FONT_GENERIC: width += gDialogCharWidths[*str64]; break;
            case OMM_RENDER_FONT_MENU:    width += ((*str64 == DIALOG_CHAR_SPACE) ? 4 : gDialogCharWidths[*str64 - (26 * (*str64 >= 0x24 && *str64 <= 0x3D))]); break;
            case OMM_RENDER_FONT_HUD:     width += ((*str64 == DIALOG_CHAR_SPACE) ? 8 : 12); break;
        }
    }
    return width;
}

void omm_render_string(s32 font, const u8 *str64, s16 x, s16 y, u8 red, u8 green, u8 blue, u8 alpha, bool shadow) {
    if (shadow) {
        omm_render_string(font, str64, x + 1, y - 1, 0, 0, 0, alpha, false);
    }
    omm_render_create_dl_ortho_matrix();
    switch (font) {
        case OMM_RENDER_FONT_GENERIC: {
            gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
            gDPSetEnvColor(gDisplayListHead++, red, green, blue, alpha);
            print_generic_string(x, y, str64);
            gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, 0xFF);
            gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
        } break;

        case OMM_RENDER_FONT_MENU: {
            u8 *temp = omm_text_capitalize(omm_text_copy(str64, false));
            gSPDisplayList(gDisplayListHead++, dl_menu_ia8_text_begin);
            gDPSetEnvColor(gDisplayListHead++, red, green, blue, alpha);
            print_menu_generic_string(x, SCREEN_HEIGHT - 8 - y, temp);
            gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, 0xFF);
            gSPDisplayList(gDisplayListHead++, dl_menu_ia8_text_end);
        } break;

        case OMM_RENDER_FONT_HUD: {
            gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
            gDPSetEnvColor(gDisplayListHead++, red, green, blue, alpha);
            print_hud_lut_string(HUD_LUT_GLOBAL, x, SCREEN_HEIGHT - 16 - y, str64);
            gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, 0xFF);
            gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
        } break;
    }
}

void omm_render_texrect(s16 x, s16 y, s16 w, s16 h, s32 fmt, s32 siz, s16 texw, s16 texh, u8 r, u8 g, u8 b, u8 a, const void *texp) {
    omm_render_create_dl_ortho_matrix();
    gDPPipeSync(gDisplayListHead++);
    gDPSetTexturePersp(gDisplayListHead++, G_TP_NONE);
    gDPSetRenderMode(gDisplayListHead++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
    gDPSetCombineLERP(gDisplayListHead++, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0);
    gDPSetEnvColor(gDisplayListHead++, r, g, b, a);
    gDPSetTextureFilter(gDisplayListHead++, G_TF_POINT);
    gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPPipeSync(gDisplayListHead++);
    switch (siz) {
        case G_IM_SIZ_4b:  gDPLoadTextureBlock(gDisplayListHead++, texp, fmt, G_IM_SIZ_4b,  texw, texh, 0, G_TX_CLAMP, G_TX_CLAMP, 0, 0, 0, 0); break;
        case G_IM_SIZ_8b:  gDPLoadTextureBlock(gDisplayListHead++, texp, fmt, G_IM_SIZ_8b,  texw, texh, 0, G_TX_CLAMP, G_TX_CLAMP, 0, 0, 0, 0); break;
        case G_IM_SIZ_16b: gDPLoadTextureBlock(gDisplayListHead++, texp, fmt, G_IM_SIZ_16b, texw, texh, 0, G_TX_CLAMP, G_TX_CLAMP, 0, 0, 0, 0); break;
        case G_IM_SIZ_32b: gDPLoadTextureBlock(gDisplayListHead++, texp, fmt, G_IM_SIZ_32b, texw, texh, 0, G_TX_CLAMP, G_TX_CLAMP, 0, 0, 0, 0); break;
    }
    gSPTextureRectangle(gDisplayListHead++, (x) << 2, (SCREEN_HEIGHT - h - y) << 2, (x + w) << 2, (SCREEN_HEIGHT - y) << 2, G_TX_RENDERTILE, 0, 0, ((0x400 * texw) / w), ((0x400 * texh) / h));
    gDPPipeSync(gDisplayListHead++);
    gDPSetTexturePersp(gDisplayListHead++, G_TP_PERSP);
    gDPSetRenderMode(gDisplayListHead++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
    gDPSetCombineLERP(gDisplayListHead++, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE);
    gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, 0xFF);
    gDPSetTextureFilter(gDisplayListHead++, G_TF_BILERP);
    gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
}

void omm_render_glyph(s16 x, s16 y, s16 w, s16 h, u8 red, u8 green, u8 blue, u8 alpha, const void *texture, bool shadow) {
    if (shadow) {
        omm_render_glyph(x + 1, y - 1, w, h, 0, 0, 0, alpha, texture, false);
    }
    omm_render_create_dl_ortho_matrix();
    gDPPipeSync(gDisplayListHead++);
    gDPSetTexturePersp(gDisplayListHead++, G_TP_NONE);
    gDPSetRenderMode(gDisplayListHead++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
    gDPSetCombineLERP(gDisplayListHead++, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0);
    gDPSetEnvColor(gDisplayListHead++, red, green, blue, alpha);
    gDPSetTextureFilter(gDisplayListHead++, G_TF_POINT);
    gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPPipeSync(gDisplayListHead++);
    gDPLoadTextureBlock(gDisplayListHead++, texture, G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 16, 0, G_TX_CLAMP, G_TX_CLAMP, 0, 0, 0, 0);
    gSPTextureRectangle(gDisplayListHead++, (x) << 2, (SCREEN_HEIGHT - h - y) << 2, (x + w) << 2, (SCREEN_HEIGHT - y) << 2, G_TX_RENDERTILE, 0, 0, (0x4000 / w), (0x4000 / h));
    gDPPipeSync(gDisplayListHead++);
    gDPSetTexturePersp(gDisplayListHead++, G_TP_PERSP);
    gDPSetRenderMode(gDisplayListHead++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
    gDPSetCombineLERP(gDisplayListHead++, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE);
    gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, 0xFF);
    gDPSetTextureFilter(gDisplayListHead++, G_TF_BILERP);
    gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
}

void omm_render_small_glyph(s16 x, s16 y, s16 w, s16 h, u8 red, u8 green, u8 blue, u8 alpha, const void *texture, bool shadow) {
    if (shadow) {
        omm_render_small_glyph(x + 1, y - 1, w, h, 0, 0, 0, alpha, texture, false);
    }
    omm_render_create_dl_ortho_matrix();
    gDPPipeSync(gDisplayListHead++);
    gDPSetTexturePersp(gDisplayListHead++, G_TP_NONE);
    gDPSetRenderMode(gDisplayListHead++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
    gDPSetCombineLERP(gDisplayListHead++, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0);
    gDPSetEnvColor(gDisplayListHead++, red, green, blue, alpha);
    gDPSetTextureFilter(gDisplayListHead++, G_TF_POINT);
    gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPPipeSync(gDisplayListHead++);
    gDPLoadTextureBlock(gDisplayListHead++, texture, G_IM_FMT_RGBA, G_IM_SIZ_16b, 8, 8, 0, G_TX_CLAMP, G_TX_CLAMP, 0, 0, 0, 0);
    gSPTextureRectangle(gDisplayListHead++, (x) << 2, (SCREEN_HEIGHT - h - y) << 2, (x + w) << 2, (SCREEN_HEIGHT - y) << 2, G_TX_RENDERTILE, 0, 0, (0x2000 / w), (0x2000 / h));
    gDPPipeSync(gDisplayListHead++);
    gDPSetTexturePersp(gDisplayListHead++, G_TP_PERSP);
    gDPSetRenderMode(gDisplayListHead++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
    gDPSetCombineLERP(gDisplayListHead++, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE);
    gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, 0xFF);
    gDPSetTextureFilter(gDisplayListHead++, G_TF_BILERP);
    gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
}

void omm_render_number(s32 number, s32 digits, s16 x, s16 y, s16 w, s16 h, s16 xStep, u8 alpha, bool makeZerosTransparent, bool shadow) {
    static const char *sOmmNumberGlyphs[] = {
        OMM_TEXTURE_HUD_0,
        OMM_TEXTURE_HUD_1,
        OMM_TEXTURE_HUD_2,
        OMM_TEXTURE_HUD_3,
        OMM_TEXTURE_HUD_4,
        OMM_TEXTURE_HUD_5,
        OMM_TEXTURE_HUD_6,
        OMM_TEXTURE_HUD_7,
        OMM_TEXTURE_HUD_8,
        OMM_TEXTURE_HUD_9,
    };

    s32 glyphs[16];
    number = omm_clamp_s(number, 0, 0x7FFFFFFF);
    digits = omm_clamp_s(digits, 1, 15);
    for (s32 i = 0; i != digits; ++i) {
        glyphs[i] = (number % 10);
        number /= 10;
    }
    for (s32 i = digits, j = 0; i != 0; --i, x += xStep) {
        s32 n = glyphs[i - 1];
        j = omm_max_s(j, n);
        omm_render_glyph(x, y, w, h, 0xFF, 0xFF, 0xFF, (alpha * (0xFF - 0xAA * (makeZerosTransparent && ((digits == 1) || (i > 1)) && (j == 0)))) / 0xFF, sOmmNumberGlyphs[n], shadow);
    }
}

// create_dl_ortho_matrix(), but with already-computed and hardcoded matrices, to reduce computation time and GFX pool usage by 128 bytes every call
void omm_render_create_dl_ortho_matrix() {
    static const Mtx sIdentMatrix = { {
        { 1.f, 0.f, 0.f, 0.f },
        { 0.f, 1.f, 0.f, 0.f },
        { 0.f, 0.f, 1.f, 0.f },
        { 0.f, 0.f, 0.f, 1.f },
    } };
    static const Mtx sOrthoMatrix = { {
        { 2.f / SCREEN_WIDTH, 0.f, 0.f, 0.f },
        { 0.f, 2.f / SCREEN_HEIGHT, 0.f, 0.f },
        { 0.f, 0.f, -0.1f, 0.f },
        { -1.f, -1.f, 0.f, 1.f },
    } };
    gSPPerspNormalize(gDisplayListHead++, 0xFFFF);
    gSPMatrix(gDisplayListHead++, &sIdentMatrix, G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gDisplayListHead++, &sIdentMatrix, G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(gDisplayListHead++, &sOrthoMatrix, G_MTX_PROJECTION | G_MTX_MUL | G_MTX_NOPUSH);
}

//
// Constants
//

// Commons
#define OMM_INVSQRT2                                        0.70710678118
#define OMM_HUD_DISPLAY_FLAG_POWER_UP                       HUD_DISPLAY_FLAG_STAR_COUNT
#define OMM_HUD_DISPLAY_FLAG_VIBE_GAUGE                     HUD_DISPLAY_FLAG_CAMERA_AND_POWER
#define OMM_HUD_DISPLAY_FLAG_O2_METER                       HUD_DISPLAY_FLAG_CAMERA_AND_POWER
#define OMM_HUD_DISPLAY_FLAG_RED_COINS_RADAR                HUD_DISPLAY_FLAG_STAR_COUNT
#define OMM_HUD_DISPLAY_FLAG_SPARKLY_STARS_CONDITIONS       HUD_DISPLAY_FLAG_STAR_COUNT
#define OMM_RENDER_GLYPH_SIZE                               (12)
#define OMM_RENDER_LEFT_X                                   GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(16)
#define OMM_RENDER_RIGHT_X                                  GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(16)
#define OMM_RENDER_TOP_Y                                    (SCREEN_HEIGHT - 16)
#define OMM_RENDER_BOTTOM_Y                                 (16)
#define OMM_RENDER_NUMBER_OFFSET_X                          ((OMM_RENDER_GLYPH_SIZE * 3) / 4)
#define OMM_RENDER_STAR_OFFSET_X                            ((OMM_RENDER_GLYPH_SIZE * 9) / 8)
#define OMM_RENDER_OFFSET_Y                                 ((OMM_RENDER_GLYPH_SIZE * 5) / 4)
#define OMM_RENDER_ENABLE_ALPHA                             gSPSetOtherMode(gDisplayListHead++, G_SETOTHERMODE_L, 18, 1, G_BL_A_MEM)

// Values
#define OMM_RENDER_VALUE_GLYPH_X                            (OMM_RENDER_LEFT_X)
#define OMM_RENDER_VALUE_NUMBER_X                           (OMM_RENDER_VALUE_GLYPH_X + ((OMM_RENDER_GLYPH_SIZE * 5) / 4))
#define OMM_RENDER_VALUE_Y                                  (OMM_RENDER_TOP_Y - OMM_RENDER_GLYPH_SIZE)

// Coin sparkles (Vanishing HUD only)
#define OMM_RENDER_HUD_COIN_SPARKLE_SIZE                    ((OMM_RENDER_GLYPH_SIZE * 3) / 2)
#define OMM_RENDER_HUD_COIN_SPARKLE_X                       (OMM_RENDER_VALUE_GLYPH_X - ((OMM_RENDER_HUD_COIN_SPARKLE_SIZE - OMM_RENDER_GLYPH_SIZE) / 2))
#define OMM_RENDER_HUD_COIN_SPARKLE_Y                       (y - ((OMM_RENDER_HUD_COIN_SPARKLE_SIZE - OMM_RENDER_GLYPH_SIZE) / 2))
#define OMM_RENDER_HUD_COIN_SPARKLE_FRAMES                  (6)
#define OMM_RENDER_HUD_COIN_SPARKLE_DELAY                   (2)
#define OMM_RENDER_HUD_COIN_SPARKLE_COUNT                   (8)

// Collectibles
#define OMM_RENDER_COLLECTIBLE_X                            (OMM_RENDER_LEFT_X)
#define OMM_RENDER_COLLECTIBLE_Y                            (OMM_RENDER_BOTTOM_Y)

// Dark mode
#define OMM_RENDER_DARK_MODE_NUM_POINTS                     (256)
#define OMM_RENDER_DARK_MODE_RADIUS_MIN                     (0.f)
#define OMM_RENDER_DARK_MODE_RADIUS_MAX                     (0.3f)
#define OMM_RENDER_DARK_MODE_RADIUS_OUT                     (1.5f)
#define OMM_RENDER_DARK_MODE_HALF_WIDTH                     ((GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(0) - GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(0)) / 2.f)
#define OMM_RENDER_DARK_MODE_HALF_HEIGHT                    (SCREEN_HEIGHT / 2.f)
#define OMM_RENDER_DARK_MODE_CENTER_X                       (SCREEN_WIDTH / 2.f)
#define OMM_RENDER_DARK_MODE_CENTER_Y                       (SCREEN_HEIGHT / 2.f)

// Power meter
#define OMM_RENDER_POWER_X                                  (OMM_RENDER_RIGHT_X - ((OMM_RENDER_GLYPH_SIZE * 11) / 8))
#define OMM_RENDER_POWER_Y                                  (OMM_RENDER_TOP_Y - ((OMM_RENDER_GLYPH_SIZE * 11) / 8))
#define OMM_RENDER_POWER_BACKGROUND_NUM_TRIS                (64)
#define OMM_RENDER_POWER_BACKGROUND_R                       (0x30)
#define OMM_RENDER_POWER_BACKGROUND_G                       (0x30)
#define OMM_RENDER_POWER_BACKGROUND_B                       (0x30)
#define OMM_RENDER_POWER_SEGMENT_NUM_QUADS                  (32)
#define OMM_RENDER_POWER_SEGMENT_RADIUS_0                   (OMM_RENDER_GLYPH_SIZE * 1.000f)
#define OMM_RENDER_POWER_SEGMENT_RADIUS_1                   (OMM_RENDER_GLYPH_SIZE * 1.125f)
#define OMM_RENDER_POWER_SEGMENT_RADIUS_2                   (OMM_RENDER_GLYPH_SIZE * 1.250f)
#define OMM_RENDER_POWER_SEGMENT_RADIUS_3                   (OMM_RENDER_GLYPH_SIZE * 1.375f)
#define OMM_RENDER_POWER_SEGMENT_CENTER_R                   ((OMM_ARRAY_OF(const u8) { 0xFF, 0xFF, 0xFF, 0x00, 0x00 })[index])
#define OMM_RENDER_POWER_SEGMENT_CENTER_G                   ((OMM_ARRAY_OF(const u8) { 0xFF, 0x00, 0xC8, 0xE0, 0xC8 })[index])
#define OMM_RENDER_POWER_SEGMENT_CENTER_B                   ((OMM_ARRAY_OF(const u8) { 0xFF, 0x00, 0x00, 0x00, 0xFF })[index])
#define OMM_RENDER_POWER_SEGMENT_BORDER_R                   ((OMM_ARRAY_OF(const u8) { 0xFF, 0xD0, 0xD0, 0x00, 0x00 })[index])
#define OMM_RENDER_POWER_SEGMENT_BORDER_G                   ((OMM_ARRAY_OF(const u8) { 0xFF, 0x00, 0xA0, 0xB8, 0xA0 })[index])
#define OMM_RENDER_POWER_SEGMENT_BORDER_B                   ((OMM_ARRAY_OF(const u8) { 0xFF, 0x00, 0x00, 0x00, 0xD0 })[index])
#define OMM_RENDER_POWER_SEGMENT_DELTA                      (OMM_RENDER_GLYPH_SIZE / 16.f)
#define OMM_RENDER_POWER_HEART_NUM_PIECES                   (64)
#define OMM_RENDER_POWER_HEART_RADIUS_1_X                   (OMM_RENDER_GLYPH_SIZE * 1.250f)
#define OMM_RENDER_POWER_HEART_RADIUS_1_Y                   (OMM_RENDER_GLYPH_SIZE * 1.125f)
#define OMM_RENDER_POWER_HEART_RADIUS_2_X                   (OMM_RENDER_GLYPH_SIZE * 1.375f)
#define OMM_RENDER_POWER_HEART_RADIUS_2_Y                   (OMM_RENDER_GLYPH_SIZE * 1.250f)
#define OMM_RENDER_POWER_HEART_CENTER_R                     (0xFF)
#define OMM_RENDER_POWER_HEART_CENTER_G                     (0x3F + index * 0x60)
#define OMM_RENDER_POWER_HEART_CENTER_B                     (0x3F + index * 0x60)
#define OMM_RENDER_POWER_HEART_BORDER_R                     (0xD0)
#define OMM_RENDER_POWER_HEART_BORDER_G                     (0x30 + index * 0x50)
#define OMM_RENDER_POWER_HEART_BORDER_B                     (0x30 + index * 0x50)
#define OMM_RENDER_POWER_HEART_PULSE                        ((OMM_ARRAY_OF(const s32) { 2, 1, 0, 0, 0, 1, 2 })[hp < OMM_RENDER_POWER_HP_CRITICAL ? omm_min_s(gGlobalTimer % 30, 6) : 0])
#define OMM_RENDER_POWER_HEART_OFFSET_Y                     (OMM_RENDER_GLYPH_SIZE / 8.f)
#define OMM_RENDER_POWER_LIFE_UP_OFFSET_X                   (OMM_RENDER_GLYPH_SIZE / 2.f)
#define OMM_RENDER_POWER_NUMBER_OFFSET_X                    (OMM_RENDER_GLYPH_SIZE / 2)
#define OMM_RENDER_POWER_NUMBER_OFFSET_Y                    ((OMM_RENDER_GLYPH_SIZE * 11) / 16)
#define OMM_RENDER_POWER_HP_FULL_SEGMENTS                   ((OMM_ARRAY_OF(const s32) { 8,  3,  6 })[gOmmMovesetType])
#define OMM_RENDER_POWER_HP_PER_SEGMENT                     ((OMM_ARRAY_OF(const s32) { 1, 10, 10 })[gOmmMovesetType])
#define OMM_RENDER_POWER_HP_NORMAL                          ((OMM_ARRAY_OF(const s32) { 7, 31, 61 })[gOmmMovesetType])
#define OMM_RENDER_POWER_HP_LOW                             ((OMM_ARRAY_OF(const s32) { 5, 25, 25 })[gOmmMovesetType])
#define OMM_RENDER_POWER_HP_CRITICAL                        ((OMM_ARRAY_OF(const s32) { 3, 15, 15 })[gOmmMovesetType])
#define OMM_RENDER_POWER_HP_VALUE                           ((hp + OMM_RENDER_POWER_HP_PER_SEGMENT / 2) / OMM_RENDER_POWER_HP_PER_SEGMENT)

// Oxygen meter
#define OMM_RENDER_O2_NUM_QUADS                             (180)
#define OMM_RENDER_O2_COLOR_R                               ((i >= j) ? omm_min_s((      (t) * 0x1FF), 0xFF) :          (0xFF))
#define OMM_RENDER_O2_COLOR_G                               ((i >= j) ? omm_min_s(((1.f - t) * 0x1FF), 0xFF) : (!blink * 0xFF))
#define OMM_RENDER_O2_COLOR_B                               ((i >= j) ?                               (0x00) : (!blink * 0xFF))
#define OMM_RENDER_O2_RADIUS_IN                             (3.f)
#define OMM_RENDER_O2_RADIUS_OUT                            (5.f)

// Camera
#define OMM_RENDER_CAMERA_X                                 (OMM_RENDER_RIGHT_X - (2 * OMM_RENDER_GLYPH_SIZE))
#define OMM_RENDER_CAMERA_Y                                 (19)
#define OMM_RENDER_CAMERA_OFFSET_X                          (OMM_RENDER_GLYPH_SIZE)
#define OMM_RENDER_CAMERA_OFFSET_Y                          ((OMM_RENDER_GLYPH_SIZE * 3) / 4)
#define OMM_RENDER_CAMERA_STEP_Y                            (OMM_RENDER_GLYPH_SIZE / 4)

// Red coins radar
#define OMM_RENDER_RADAR_X                                  (OMM_RENDER_LEFT_X + OMM_RENDER_RADAR_RADIUS + OMM_RENDER_RADAR_ARROW_SIZE / 2)
#define OMM_RENDER_RADAR_Y                                  (19 + OMM_RENDER_RADAR_RADIUS)
#define OMM_RENDER_RADAR_COIN_SIZE                          (OMM_RENDER_GLYPH_SIZE)
#define OMM_RENDER_RADAR_STAR_SIZE                          ((OMM_RENDER_GLYPH_SIZE * 5) / 4)
#define OMM_RENDER_RADAR_ARROW_SIZE                         ((OMM_RENDER_GLYPH_SIZE * 7) / 8)
#define OMM_RENDER_RADAR_RADIUS                             (((OMM_RENDER_RADAR_COIN_SIZE + OMM_RENDER_RADAR_ARROW_SIZE) * 7) / 10)

// Red coins
#define OMM_RENDER_RED_COINS_X                              (OMM_RENDER_RIGHT_X - (3 * OMM_RENDER_GLYPH_SIZE) - (4 * OMM_RENDER_NUMBER_OFFSET_X))
#define OMM_RENDER_RED_COINS_Y                              (19)

// Timer
#define OMM_RENDER_TIMER_OFFSET_X                           (OMM_RENDER_NUMBER_OFFSET_X)
#define OMM_RENDER_TIMER_X                                  ((SCREEN_WIDTH - 8 * OMM_RENDER_TIMER_OFFSET_X) / 2)
#define OMM_RENDER_TIMER_Y                                  (OMM_RENDER_TOP_Y - OMM_RENDER_GLYPH_SIZE - (OMM_RENDER_OFFSET_Y / 2))

// Sparkly Stars timer
#define OMM_RENDER_SPARKLY_TIMER_GLYPH_SIZE                 (10)
#define OMM_RENDER_SPARKLY_TIMER_OFFSET_X(glyphSize)        (((glyphSize * 3) + 3) / 4)
#define OMM_RENDER_SPARKLY_TIMER_X(glyphSize)               ((SCREEN_WIDTH - 8 * OMM_RENDER_SPARKLY_TIMER_OFFSET_X(glyphSize)) / 2)
#define OMM_RENDER_SPARKLY_TIMER_Y                          (8)

// Pause (Course)
#define OMM_RENDER_PAUSE_COURSE_LEFT_ALIGN_X                ((SCREEN_WIDTH * 2) / 5)
#define OMM_RENDER_PAUSE_COURSE_RIGHT_ALIGN_X               (OMM_RENDER_PAUSE_COURSE_LEFT_ALIGN_X - 16)
#define OMM_RENDER_PAUSE_COURSE_OFFSET_Y                    (16)
#define OMM_RENDER_PAUSE_COURSE_OPTIONS_Y                   (SCREEN_HEIGHT / 2 - 16)
#define OMM_RENDER_PAUSE_COURSE_INFO_Y                      (OMM_RENDER_PAUSE_COURSE_OPTIONS_Y + OMM_RENDER_PAUSE_COURSE_OFFSET_Y * 5)

// Pause (Castle)
#define OMM_RENDER_PAUSE_CASTLE_PAUSE_Y                     (SCREEN_HEIGHT / 2)
#define OMM_RENDER_PAUSE_CASTLE_BOX_W                       (180)
#define OMM_RENDER_PAUSE_CASTLE_BOX_H                       (48)
#define OMM_RENDER_PAUSE_CASTLE_BOX_Y                       (SCREEN_HEIGHT / 4)
#define OMM_RENDER_PAUSE_CASTLE_BOX_TRIANGLE_UP_Y           (OMM_RENDER_PAUSE_CASTLE_BOX_Y + OMM_RENDER_PAUSE_CASTLE_BOX_H / 2 + 4)
#define OMM_RENDER_PAUSE_CASTLE_BOX_TRIANGLE_DOWN_Y         (OMM_RENDER_PAUSE_CASTLE_BOX_Y - OMM_RENDER_PAUSE_CASTLE_BOX_H / 2 - 4)
#define OMM_RENDER_PAUSE_CASTLE_BOX_LINE_1_Y                (OMM_RENDER_PAUSE_CASTLE_BOX_Y + 10)
#define OMM_RENDER_PAUSE_CASTLE_BOX_LINE_2_Y                (OMM_RENDER_PAUSE_CASTLE_BOX_Y -  4)
#define OMM_RENDER_PAUSE_CASTLE_BOX_LINE_3_Y                (OMM_RENDER_PAUSE_CASTLE_BOX_Y - 18)

// Course complete
#define OMM_RENDER_COURSE_COMPLETE_LEFT_ALIGN_X             ((SCREEN_WIDTH * 2) / 5)
#define OMM_RENDER_COURSE_COMPLETE_RIGHT_ALIGN_X            (OMM_RENDER_PAUSE_COURSE_LEFT_ALIGN_X - 16)
#define OMM_RENDER_COURSE_COMPLETE_HI_SCORE_X               (OMM_RENDER_COURSE_COMPLETE_LEFT_ALIGN_X + ((OMM_RENDER_GLYPH_SIZE * 9) / 2))
#define OMM_RENDER_COURSE_COMPLETE_OFFSET_Y                 (16)

// Star select
#define OMM_RENDER_STAR_SELECT_J                            (0x13)
#define OMM_RENDER_STAR_SELECT_Q                            (0x1A)
#define OMM_RENDER_STAR_SELECT_V                            (0x1F)
#define OMM_RENDER_STAR_SELECT_X                            (0x21)
#define OMM_RENDER_STAR_SELECT_Z                            (0x23)
#define OMM_RENDER_STAR_SELECT_SPACE                        (DIALOG_CHAR_SPACE)
#define OMM_RENDER_STAR_SELECT_COMMA                        (DIALOG_CHAR_COMMA)
#define OMM_RENDER_STAR_SELECT_HYPHEN                       (0x9F)
#define OMM_RENDER_STAR_SELECT_APOSTROPHE                   (0x3E)
#define OMM_RENDER_STAR_SELECT_BACKGROUND_BOTTOM_Y0         (0)
#define OMM_RENDER_STAR_SELECT_BACKGROUND_BOTTOM_Y1         (90)
#define OMM_RENDER_STAR_SELECT_BACKGROUND_TOP_Y0            (SCREEN_HEIGHT - 70)
#define OMM_RENDER_STAR_SELECT_BACKGROUND_TOP_Y1            (SCREEN_HEIGHT)
#define OMM_RENDER_STAR_SELECT_COURSE_NAME_Y                (SCREEN_HEIGHT - 56)
#define OMM_RENDER_STAR_SELECT_ACT_STAR_X                   (0)
#define OMM_RENDER_STAR_SELECT_ACT_STAR_Y                   (25)
#define OMM_RENDER_STAR_SELECT_ACT_STAR_Z                   (-300)
#define OMM_RENDER_STAR_SELECT_ACT_STAR_DX                  (300)
#define OMM_RENDER_STAR_SELECT_ACT_STAR_DZ                  (600)
#define OMM_RENDER_STAR_SELECT_ACT_NAME_Y                   (64)
#define OMM_RENDER_STAR_SELECT_SCORE_Y                      (24)
#define OMM_RENDER_STAR_SELECT_100_COINS_STAR_X             ((SCREEN_WIDTH / 2) + 44)
#define OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE          (16)

// You got a Star!
#define OMM_RENDER_YOU_GOT_A_STAR_Y                         (32)
#define OMM_RENDER_YOU_GOT_A_STAR_W                         (200)
#define OMM_RENDER_YOU_GOT_A_STAR_H                         (64)
#define OMM_RENDER_YOU_GOT_A_STAR_LINE_1_Y                  (OMM_RENDER_YOU_GOT_A_STAR_Y + 46)
#define OMM_RENDER_YOU_GOT_A_STAR_LINE_2_Y                  (OMM_RENDER_YOU_GOT_A_STAR_Y + 28)
#define OMM_RENDER_YOU_GOT_A_STAR_LINE_3_Y                  (OMM_RENDER_YOU_GOT_A_STAR_Y + 10)

// Shortcuts
#define omm_render_string_left_align(...)                   omm_render_string(OMM_RENDER_FONT_MENU, __VA_ARGS__)
#define omm_render_string_right_align(str, x, ...)          omm_render_string(OMM_RENDER_FONT_MENU, str, x - omm_render_get_string_width(OMM_RENDER_FONT_MENU, str), __VA_ARGS__)
#define omm_render_string_centered(str, ...)                omm_render_string(OMM_RENDER_FONT_MENU, str, ((SCREEN_WIDTH - omm_render_get_string_width(OMM_RENDER_FONT_MENU, str)) / 2), __VA_ARGS__)
#define omm_render_string_hud(...)                          omm_render_string(OMM_RENDER_FONT_HUD, __VA_ARGS__)
#define omm_render_string_hud_centered(str, ...)            omm_render_string(OMM_RENDER_FONT_HUD, str, ((SCREEN_WIDTH - omm_render_get_string_width(OMM_RENDER_FONT_HUD, str)) / 2), __VA_ARGS__)
#define omm_render_glyph_hud(x, y, ...)                     omm_render_glyph(x, y, OMM_RENDER_GLYPH_SIZE, OMM_RENDER_GLYPH_SIZE, __VA_ARGS__)
#define omm_render_number_hud(n, d, x, y, ...)              omm_render_number(n, d, x, y, OMM_RENDER_GLYPH_SIZE, OMM_RENDER_GLYPH_SIZE, OMM_RENDER_NUMBER_OFFSET_X, __VA_ARGS__)
#define omm_render_counter_hud(c, n, y, r, g, b, a)         { const u8 textX[] = { c, 0xFB, 0xFF }; OMM_STRING(textCount, 8, "%d", n); omm_render_string_left_align(textX, (SCREEN_WIDTH / 2) - 2 - omm_render_get_string_width(OMM_RENDER_FONT_MENU, textX), y, r, g, b, a, false); omm_render_string_left_align(omm_text_convert(textCount, false), (SCREEN_WIDTH / 2) + 2, y, r, g, b, a, false); }

//
// Data
//

static s32 sOmmCourseCompleteCoins = 0;
static s32 sOmmCourseCompleteTimer = 0;
static s32 sOmmDialogBoxState = 0;
static s32 sOmmDialogTextAlpha = 0;

static u8 sOmmYouGotAStarText[3][64];
static s32 sOmmYouGotAStarTimer = 0;
static bool sOmmYouGotAStarRender = false;

static Vtx sOmmPowerMeterVertexArray[0x1000];
static Gfx sOmmPowerMeterDisplayList[0x2000];
static Vtx *sOmmPowerMeterVertexArrayHead = NULL;
static Gfx *sOmmPowerMeterDisplayListHead = NULL;

static Vtx sOmmDarkModeVertexArray[OMM_RENDER_DARK_MODE_NUM_POINTS * 6];
static Gfx sOmmDarkModeDisplayList[OMM_RENDER_DARK_MODE_NUM_POINTS * 3 + 16];
static Vtx *sOmmDarkModeVertexArrayHead = NULL;
static Gfx *sOmmDarkModeDisplayListHead = NULL;

//
// Scrolls
//

typedef struct { s32 idx; s32 inc; s32 tmr; s32 val; } Scroll;
static void omm_render_update_scroll(Scroll *scroll, s32 count, s16 stickValue) {
    s32 val = 2 * (stickValue > +60) + 1 * (stickValue < -60);
    s32 inc = (((val ^ scroll->val) & val) == 2) - (((val ^ scroll->val) & val) == 1);
    if (inc != 0) {
        scroll->idx += inc;
        play_sound(SOUND_MENU_CHANGE_SELECT, gGlobalSoundArgs);
    }
    if (scroll->tmr == 10) {
        scroll->tmr = 8;
        scroll->val = 0;
    } else {
        scroll->tmr = (scroll->tmr + 1) * (val != 0);
        scroll->val = val;
    }
    scroll->idx = ((scroll->idx + count) % count);
    scroll->inc = inc;
}

static Scroll sOmmPauseCastleScrollV = { 0 };
static Scroll sOmmPauseCastleScrollH = { 0 };
static Scroll sOmmPauseCourseScroll = { 0 };
static Scroll sOmmStarSelectScrollV = { 0 };
static Scroll sOmmStarSelectScrollH = { 0 };

//
// Vanishing HUD
//

typedef struct {
    s32 alwaysActive;
    s32 displayOnPause;
    s32 fadeIn;
    s32 fadeOut;
    s32 timerMin;
    s32 timerMax;
    s32 timerAlphaMin;
    s32 timerAlphaMax;
    s32 alphaMin;
    s32 alphaMax;
    s32 timer;
    s32 prev;
} OmmHudTimer;

static OmmHudTimer sOmmHudStarsTimer  = { 0, 1, +1, +2,   0, 30,  0, 20, 0x00, 0xFF, 0, 0 };
static OmmHudTimer sOmmHudCoinsTimer  = { 0, 1, +1, +2, -60, 30,  0, 20, 0x00, 0xFF, 0, 0 };
static OmmHudTimer sOmmHudVibeTimer   = { 0, 1, +1, +2, -45, 30,  0, 20, 0x00, 0xFF, 0, 0 };
static OmmHudTimer sOmmHudHealthTimer = { 0, 1, +1, +2, -60, 30,  0, 20, 0x40, 0xFF, 0, 0 };
static OmmHudTimer sOmmHudOxygenTimer = { 1, 0, -1, +1,   0, 30, 24, 30, 0x00, 0xFF, 0, 0 };
static OmmHudTimer sOmmHudCameraTimer = { 0, 0, +1, +2, -45, 30,  0, 20, 0x00, 0xFF, 0, 0 };

static bool should_instant_fade_in(struct MarioState *m, OmmHudTimer *timer) {
    bool res = false; 
    if (timer == &sOmmHudStarsTimer) {
        res = omm_mario_is_star_dancing(m);
    } else if (timer == &sOmmHudCoinsTimer) {
        res = (timer->prev < gHudDisplay.coins);
        timer->prev = gHudDisplay.coins;
    } else if (timer == &sOmmHudVibeTimer) {
        res = (gOmmData->mario->peach.vibeGauge != 0);
    } else if (timer == &sOmmHudHealthTimer) {
        s32 health = (OMM_MOVESET_ODYSSEY ? gOmmData->mario->state.hp : m->health);
        res = (timer->prev != health);
        timer->prev = health;
    } else if (timer == &sOmmHudOxygenTimer) {
        res = (gOmmData->mario->state.o2 != 0);
    } else if (timer == &sOmmHudCameraTimer) {
        s32 status = (omm_camera_is_available(m) ? ((s32) (gOmmCameraMode << 4) + omm_camera_get_relative_dist_mode()) : update_camera_hud_status(gCamera));
        res = (timer->prev != status);
        timer->prev = status;
    }
    return res;
}

static u8 omm_render_hud_update_timer_and_get_alpha(struct MarioState *m, OmmHudTimer *timer) {
    if (OMM_EXTRAS_VANISHING_HUD || timer->alwaysActive) {
        
        // Pause
        if (gMenuMode == 0 || gMenuMode == 1) {
            if (timer->displayOnPause) {
                return 0xFF;
            }
            if (!timer->alwaysActive) {
                return 0x00;
            }
        }

        // Instant fade-in, fade cooldown, fade-in or fade-out
        if (should_instant_fade_in(m, timer)) {
            timer->timer = timer->timerMin;
        } else if (timer->timer < 0) {
            timer->timer = omm_min_s(timer->timer + timer->fadeIn, 0);
        } else {
            struct Controller *cont = m->controller;
            bool contLeftStickIdle = (cont == NULL) || ((cont->stickX == 0) && (cont->stickY == 0) && (cont->stickMag == 0));
            bool contButtonDownIdle = (cont == NULL) || ((cont->buttonDown & ~(U_CBUTTONS | D_CBUTTONS | L_CBUTTONS | R_CBUTTONS | R_TRIG | L_TRIG)) == 0);
            bool contButtonPressedIdle = (cont == NULL) || ((cont->buttonPressed & ~(U_CBUTTONS | D_CBUTTONS | L_CBUTTONS | R_CBUTTONS | R_TRIG | L_TRIG)) == 0);
            if (omm_mario_is_reading(m) || (omm_mario_is_idling(m) && contLeftStickIdle && contButtonDownIdle && contButtonPressedIdle)) {
                timer->timer = omm_max_s(timer->timer - timer->fadeIn, 0);
            } else {
                timer->timer = omm_min_s(timer->timer + timer->fadeOut, timer->timerMax);
            }
        }

        // Return alpha value
        return omm_clamp_s(omm_relerp_0_1_s(timer->timer, timer->timerAlphaMin, timer->timerAlphaMax, timer->alphaMax, timer->alphaMin), 0x00, 0xFF);
    }
    return 0xFF;
}

OMM_ROUTINE_LEVEL_ENTRY(omm_render_at_level_entry) {
    sOmmHudStarsTimer.timer = 0;
    sOmmHudCoinsTimer.timer = 0;
    sOmmHudHealthTimer.timer = 0;
    sOmmHudOxygenTimer.timer = sOmmHudOxygenTimer.timerMax;
    sOmmHudCameraTimer.timer = 0;
}

//
// Freezing water
//

static void omm_render_freeze() {
    u8 alpha = (u8) (160.9f * omm_clamp_f((f32) gOmmData->mario->state.freeze.gfx / (f32) OMM_FREEZING_WATER_TIMER_CRITICAL, 0.f, 1.f));
    if (alpha > 0) {
        if (alpha >= 160) {
            alpha += 40 * (gGlobalTimer & 1);
        }

        f32 x0 = GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(0);
        f32 x1 = GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(0);
        f32 y0 = 0.f;
        f32 y1 = SCREEN_HEIGHT;
        static Vtx sFreezeGfxVtx[4];
        sFreezeGfxVtx[0] = (Vtx) { { { x0, y0, 0 }, 0, { 0x0000, 0x0000 }, { 0xFF, 0xFF, 0xFF, 0xFF } } };
        sFreezeGfxVtx[1] = (Vtx) { { { x1, y0, 0 }, 0, { 0x4000, 0x0000 }, { 0xFF, 0xFF, 0xFF, 0xFF } } };
        sFreezeGfxVtx[2] = (Vtx) { { { x1, y1, 0 }, 0, { 0x4000, 0x2000 }, { 0xFF, 0xFF, 0xFF, 0xFF } } };
        sFreezeGfxVtx[3] = (Vtx) { { { x0, y1, 0 }, 0, { 0x0000, 0x2000 }, { 0xFF, 0xFF, 0xFF, 0xFF } } };

        omm_render_create_dl_ortho_matrix();
        OMM_RENDER_ENABLE_ALPHA;
        gDPPipeSync(gDisplayListHead++);
        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, alpha);
        gDPSetCombineLERP(gDisplayListHead++, 0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, TEXEL0, TEXEL0, 0, ENVIRONMENT, 0);
        gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
        gDPLoadSync(gDisplayListHead++);
        gDPLoadTextureBlock(gDisplayListHead++, OMM_TEXTURE_EFFECT_FREEZE, G_IM_FMT_RGBA, G_IM_SIZ_32b, 256, 128, 0, G_TX_MIRROR, G_TX_MIRROR, 0, 0, G_TX_NOLOD, G_TX_NOLOD);
        gSPVertex(gDisplayListHead++, sFreezeGfxVtx, 4, 0);
        gSP2Triangles(gDisplayListHead++, 0, 1, 2, 0x0, 0, 2, 3, 0x0);
        gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
        gDPSetCombineLERP(gDisplayListHead++, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE);
    }
}

//
// Dark mode
//

static void omm_render_dark_mode() {
    if (omm_sparkly_context_get_data_flags(OMM_SPARKLY_DATA_FLAG_DARK_MODE) && omm_sparkly_context_is_state_ok()) {
        OMM_RENDER_ENABLE_ALPHA;
        omm_render_create_dl_ortho_matrix();
        gSPDisplayList(gDisplayListHead++, sOmmDarkModeDisplayList);
        sOmmDarkModeDisplayListHead = sOmmDarkModeDisplayList;
        sOmmDarkModeVertexArrayHead = sOmmDarkModeVertexArray;

        // Display list
        gDPPipeSync(sOmmDarkModeDisplayListHead++);
        gSPClearGeometryMode(sOmmDarkModeDisplayListHead++, G_LIGHTING | G_CULL_BOTH);
        gDPSetCombineLERP(sOmmDarkModeDisplayListHead++, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE);
        for (s32 i = 0; i != OMM_RENDER_DARK_MODE_NUM_POINTS; ++i) {
            s32 a0 = ((i + 0) * 0x10000) / OMM_RENDER_DARK_MODE_NUM_POINTS;
            s32 a1 = ((i + 1) * 0x10000) / OMM_RENDER_DARK_MODE_NUM_POINTS;
            f32 ca0 = coss(a0);
            f32 ca1 = coss(a1);
            f32 sa0 = sins(a0);
            f32 sa1 = sins(a1);
            sOmmDarkModeVertexArray[6 * i + 0] = (Vtx) { { { OMM_RENDER_DARK_MODE_CENTER_X + sa0 * OMM_RENDER_DARK_MODE_RADIUS_MIN * OMM_RENDER_DARK_MODE_HALF_WIDTH, OMM_RENDER_DARK_MODE_CENTER_Y + ca0 * OMM_RENDER_DARK_MODE_RADIUS_MIN * OMM_RENDER_DARK_MODE_HALF_HEIGHT, 0 }, 0, { 0, 0 }, { 0x00, 0x00, 0x00, 0x00 } } };
            sOmmDarkModeVertexArray[6 * i + 1] = (Vtx) { { { OMM_RENDER_DARK_MODE_CENTER_X + sa1 * OMM_RENDER_DARK_MODE_RADIUS_MIN * OMM_RENDER_DARK_MODE_HALF_WIDTH, OMM_RENDER_DARK_MODE_CENTER_Y + ca1 * OMM_RENDER_DARK_MODE_RADIUS_MIN * OMM_RENDER_DARK_MODE_HALF_HEIGHT, 0 }, 0, { 0, 0 }, { 0x00, 0x00, 0x00, 0x00 } } };
            sOmmDarkModeVertexArray[6 * i + 2] = (Vtx) { { { OMM_RENDER_DARK_MODE_CENTER_X + sa0 * OMM_RENDER_DARK_MODE_RADIUS_MAX * OMM_RENDER_DARK_MODE_HALF_WIDTH, OMM_RENDER_DARK_MODE_CENTER_Y + ca0 * OMM_RENDER_DARK_MODE_RADIUS_MAX * OMM_RENDER_DARK_MODE_HALF_HEIGHT, 0 }, 0, { 0, 0 }, { 0x00, 0x00, 0x00, 0xFF } } };
            sOmmDarkModeVertexArray[6 * i + 3] = (Vtx) { { { OMM_RENDER_DARK_MODE_CENTER_X + sa1 * OMM_RENDER_DARK_MODE_RADIUS_MAX * OMM_RENDER_DARK_MODE_HALF_WIDTH, OMM_RENDER_DARK_MODE_CENTER_Y + ca1 * OMM_RENDER_DARK_MODE_RADIUS_MAX * OMM_RENDER_DARK_MODE_HALF_HEIGHT, 0 }, 0, { 0, 0 }, { 0x00, 0x00, 0x00, 0xFF } } };
            sOmmDarkModeVertexArray[6 * i + 4] = (Vtx) { { { OMM_RENDER_DARK_MODE_CENTER_X + sa0 * OMM_RENDER_DARK_MODE_RADIUS_OUT * OMM_RENDER_DARK_MODE_HALF_WIDTH, OMM_RENDER_DARK_MODE_CENTER_Y + ca0 * OMM_RENDER_DARK_MODE_RADIUS_OUT * OMM_RENDER_DARK_MODE_HALF_HEIGHT, 0 }, 0, { 0, 0 }, { 0x00, 0x00, 0x00, 0xFF } } };
            sOmmDarkModeVertexArray[6 * i + 5] = (Vtx) { { { OMM_RENDER_DARK_MODE_CENTER_X + sa1 * OMM_RENDER_DARK_MODE_RADIUS_OUT * OMM_RENDER_DARK_MODE_HALF_WIDTH, OMM_RENDER_DARK_MODE_CENTER_Y + ca1 * OMM_RENDER_DARK_MODE_RADIUS_OUT * OMM_RENDER_DARK_MODE_HALF_HEIGHT, 0 }, 0, { 0, 0 }, { 0x00, 0x00, 0x00, 0xFF } } };
            gSPVertex(sOmmDarkModeDisplayListHead++, sOmmDarkModeVertexArray + 6 * i, 6, 0);
            gSP2Triangles(sOmmDarkModeDisplayListHead++, 0, 1, 2, 0, 1, 2, 3, 0);
            gSP2Triangles(sOmmDarkModeDisplayListHead++, 2, 3, 4, 0, 3, 4, 5, 0);
        }
        gDPSetCombineLERP(sOmmDarkModeDisplayListHead++, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE);
        gSPSetGeometryMode(sOmmDarkModeDisplayListHead++, G_LIGHTING | G_CULL_BACK);
        gSPEndDisplayList(sOmmDarkModeDisplayListHead);
    }
}

//
// Vibes (Peach only)
//

static Vtx omm_render_vibe_get_vertex(f32 x, f32 y, u8 alpha) {
    Vtx v;
    v.v.ob[0] = x;
    v.v.ob[1] = y;
    v.v.ob[2] = 0.f;
    v.v.tc[0] = (omm_invlerp_f(x, GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(0), GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(0)) * 256 - ((gGlobalTimer & 0x1FF) / 2.f)) * 32;
    v.v.tc[1] = (omm_invlerp_f(y, 0.f, SCREEN_HEIGHT) * 256) * 32;
    v.v.cn[0] = 0xFF;
    v.v.cn[1] = 0xFF;
    v.v.cn[2] = 0xFF;
    v.v.cn[3] = alpha;
    v.v.flag  = 0;
    return v;
}

static void omm_render_vibe() {
    if (OMM_PLAYER_IS_PEACH) {
        static s32 sVibeType = OMM_PEACH_VIBE_TYPE_NONE;
        static f32 sVibeAlpha = 0.f;
        static f32 sVibeBgAlpha = 0.f;
        static Vtx sVibeBgVertices[16];
        static Gfx sVibeBgGfx[32];

        // Textures
        static const char *sVibeTextures[5][2] = {
            { OMM_TEXTURE_HUD_VIBE_NORMAL, NULL },
            { OMM_TEXTURE_HUD_VIBE_JOY, OMM_TEXTURE_EFFECT_VIBE_JOY },
            { OMM_TEXTURE_HUD_VIBE_RAGE, OMM_TEXTURE_EFFECT_VIBE_RAGE },
            { OMM_TEXTURE_HUD_VIBE_GLOOM, OMM_TEXTURE_EFFECT_VIBE_GLOOM },
            { OMM_TEXTURE_HUD_VIBE_CALM, OMM_TEXTURE_EFFECT_VIBE_CALM },
        };

        // Update vibe values
        sVibeAlpha = 1.f - omm_clamp_0_1_f(gOmmData->mario->peach.vibeTimer / 15.f);
        if (gOmmData->mario->peach.vibeType != OMM_PEACH_VIBE_TYPE_NONE) {
            sVibeType = gOmmData->mario->peach.vibeType;
            sVibeBgAlpha = 1.f - sVibeAlpha;
        } else {
            sVibeBgAlpha = sVibeAlpha;
        }

        // Vibe background
        if (sVibeBgAlpha > 0.f) {
            s32 sw = GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(0) - GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(0);
            s32 sh = SCREEN_HEIGHT;
            s32 dw = (s32) omm_relerp_f(sins(gGlobalTimer * 0x300), -1.f, 1.f, sw / 8.f, sw / 6.f);
            s32 dh = (s32) omm_relerp_f(sins(gGlobalTimer * 0x300), -1.f, 1.f, sh / 6.f, sh / 4.f);
            s32 x0 = GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(0);
            s32 x1 = GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(dw);
            s32 x2 = GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(dw);
            s32 x3 = GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(0);
            s32 y0 = 0;
            s32 y1 = dh;
            s32 y2 = SCREEN_HEIGHT - dh;
            s32 y3 = SCREEN_HEIGHT;

            // Vertices
            sVibeBgVertices[0]  = omm_render_vibe_get_vertex(x0, y0, 0xA0 * sVibeBgAlpha);
            sVibeBgVertices[1]  = omm_render_vibe_get_vertex(x1, y0, 0xA0 * sVibeBgAlpha);
            sVibeBgVertices[2]  = omm_render_vibe_get_vertex(x2, y0, 0xA0 * sVibeBgAlpha);
            sVibeBgVertices[3]  = omm_render_vibe_get_vertex(x3, y0, 0xA0 * sVibeBgAlpha);
            sVibeBgVertices[4]  = omm_render_vibe_get_vertex(x0, y1, 0xA0 * sVibeBgAlpha);
            sVibeBgVertices[5]  = omm_render_vibe_get_vertex(x1, y1, 0x00 * sVibeBgAlpha);
            sVibeBgVertices[6]  = omm_render_vibe_get_vertex(x2, y1, 0x00 * sVibeBgAlpha);
            sVibeBgVertices[7]  = omm_render_vibe_get_vertex(x3, y1, 0xA0 * sVibeBgAlpha);
            sVibeBgVertices[8]  = omm_render_vibe_get_vertex(x0, y2, 0xA0 * sVibeBgAlpha);
            sVibeBgVertices[9]  = omm_render_vibe_get_vertex(x1, y2, 0x00 * sVibeBgAlpha);
            sVibeBgVertices[10] = omm_render_vibe_get_vertex(x2, y2, 0x00 * sVibeBgAlpha);
            sVibeBgVertices[11] = omm_render_vibe_get_vertex(x3, y2, 0xA0 * sVibeBgAlpha);
            sVibeBgVertices[12] = omm_render_vibe_get_vertex(x0, y3, 0xA0 * sVibeBgAlpha);
            sVibeBgVertices[13] = omm_render_vibe_get_vertex(x1, y3, 0xA0 * sVibeBgAlpha);
            sVibeBgVertices[14] = omm_render_vibe_get_vertex(x2, y3, 0xA0 * sVibeBgAlpha);
            sVibeBgVertices[15] = omm_render_vibe_get_vertex(x3, y3, 0xA0 * sVibeBgAlpha);

            // Display list
            Gfx *gfx = sVibeBgGfx;
            gDPPipeSync(gfx++);
            gSPClearGeometryMode(gfx++, G_LIGHTING | G_CULL_BOTH);
            gDPSetCombineLERP(gfx++, TEXEL0, 0, SHADE, 0, TEXEL0, 0, SHADE, 0, TEXEL0, 0, SHADE, 0, TEXEL0, 0, SHADE, 0);
            gSPTexture(gfx++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
            gDPLoadSync(gfx++);
            gDPLoadTextureBlock(gfx++, sVibeTextures[sVibeType][1], G_IM_FMT_RGBA, G_IM_SIZ_32b, 256, 256, 0, G_TX_WRAP, G_TX_WRAP, 0, 0, G_TX_NOLOD, G_TX_NOLOD);
            gSPVertex(gfx++, sVibeBgVertices, 16, 0);
            gSP2Triangles(gfx++, 0, 1, 5, 0, 0, 4, 5, 0);
            gSP2Triangles(gfx++, 2, 3, 6, 0, 3, 6, 7, 0);
            gSP2Triangles(gfx++, 8, 9, 12, 0, 9, 12, 13, 0);
            gSP2Triangles(gfx++, 10, 11, 15, 0, 10, 14, 15, 0);
            gSP2Triangles(gfx++, 4, 5, 9, 0, 4, 8, 9, 0);
            gSP2Triangles(gfx++, 6, 7, 11, 0, 6, 10, 11, 0);
            gSP2Triangles(gfx++, 1, 2, 5, 0, 2, 5, 6, 0);
            gSP2Triangles(gfx++, 9, 10, 13, 0, 10, 13, 14, 0);
            gSPTexture(gfx++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
            gDPSetCombineLERP(gfx++, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE);
            gSPSetGeometryMode(gfx++, G_LIGHTING | G_CULL_BACK);
            gSPEndDisplayList(gfx);

            // Master display list
            omm_render_create_dl_ortho_matrix();
            OMM_RENDER_ENABLE_ALPHA;
            gSPDisplayList(gDisplayListHead++, sVibeBgGfx);
        }

        // Vibe heart
        if (sVibeAlpha > 0.f) {
            s16 glyphSize = omm_lerp_s(1.f - sVibeAlpha, 1, SCREEN_HEIGHT);
            omm_render_glyph((SCREEN_WIDTH - glyphSize) / 2.f, (SCREEN_HEIGHT - glyphSize) / 2.f, glyphSize, glyphSize, 0xFF, 0xFF, 0xFF, 0xFF * sVibeAlpha, sVibeTextures[gOmmData->mario->peach.vibeType][0], false);
        }
    }
}

//
// You got a star!
//

void omm_render_start_you_got_a_star(const char *title, const u8 *courseName, const u8 *starName) {
    const u8 *converted = omm_text_convert(title, false);
    OMM_MEMCPY(sOmmYouGotAStarText[0], converted, omm_text_length(converted) + 1);
    OMM_MEMCPY(sOmmYouGotAStarText[1], courseName, omm_text_length(courseName) + 1);
    OMM_MEMCPY(sOmmYouGotAStarText[2], starName, omm_text_length(starName) + 1);
    sOmmYouGotAStarTimer = 4;
    sOmmYouGotAStarRender = true;
}

void omm_render_stop_you_got_a_star() {
    sOmmYouGotAStarTimer = 60;
    sOmmYouGotAStarRender = false;
}

static void omm_render_you_got_a_star() {
    if (sOmmYouGotAStarTimer != 0 || sOmmYouGotAStarRender) {
        f32 t, dy;
        if (sOmmYouGotAStarRender) { // Fade-in
            t = (f32) (4 - sOmmYouGotAStarTimer) / 4.f;
            dy = 2.f * (2 - omm_abs_s(sOmmYouGotAStarTimer - 2));
        } else { // Fade-out
            t = omm_min_f(1.f, sOmmYouGotAStarTimer / 15.f);
            dy = 0;
        }

        // Background
        static const Vtx sOmmYouGotAStarBackgroundVtx[4] = {
            { { { (SCREEN_WIDTH - OMM_RENDER_YOU_GOT_A_STAR_W) / 2, OMM_RENDER_YOU_GOT_A_STAR_Y + OMM_RENDER_YOU_GOT_A_STAR_H, 0 }, 0, { 0, 0 }, { 0x00, 0x00, 0x00, 0x00 } } },
            { { { (SCREEN_WIDTH + OMM_RENDER_YOU_GOT_A_STAR_W) / 2, OMM_RENDER_YOU_GOT_A_STAR_Y + OMM_RENDER_YOU_GOT_A_STAR_H, 0 }, 0, { 0, 0 }, { 0x00, 0x00, 0x00, 0x00 } } },
            { { { (SCREEN_WIDTH - OMM_RENDER_YOU_GOT_A_STAR_W) / 2, OMM_RENDER_YOU_GOT_A_STAR_Y,                               0 }, 0, { 0, 0 }, { 0x00, 0x00, 0x00, 0x80 } } },
            { { { (SCREEN_WIDTH + OMM_RENDER_YOU_GOT_A_STAR_W) / 2, OMM_RENDER_YOU_GOT_A_STAR_Y,                               0 }, 0, { 0, 0 }, { 0x00, 0x00, 0x00, 0x80 } } },
        };
        omm_render_create_dl_ortho_matrix();
        gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 255.f * t);
        gDPPipeSync(gDisplayListHead++);
        gSPClearGeometryMode(gDisplayListHead++, G_LIGHTING);
        gDPSetCombineLERP(gDisplayListHead++, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0);
        gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gSPVertex(gDisplayListHead++, sOmmYouGotAStarBackgroundVtx, 4, 0);
        gSP2Triangles(gDisplayListHead++, 0, 2, 1, 0x0, 2, 3, 1, 0x0);

        // You got a star
        omm_render_string_hud_centered(sOmmYouGotAStarText[0], OMM_RENDER_YOU_GOT_A_STAR_LINE_1_Y + dy, 0xFF, 0xFF, 0xFF, 0xFF * t, false);
        omm_render_string_centered(sOmmYouGotAStarText[1], OMM_RENDER_YOU_GOT_A_STAR_LINE_2_Y + dy, 0xFF, 0xFF, 0xFF, 0xFF * t, true);
        omm_render_string_centered(sOmmYouGotAStarText[2], OMM_RENDER_YOU_GOT_A_STAR_LINE_3_Y + dy, 0xFF, 0xFF, 0xFF, 0xFF * t, true);
    }
    if (sOmmYouGotAStarTimer != 0) {
        sOmmYouGotAStarTimer--;
    }
}

//
// Stars
//

static const void *omm_render_get_star_glyph(s32 index, bool colored) {
    const void *sOmmStarGlyphs[][2] = {
        { (const void *) ((const u8 **) main_hud_lut)[GLYPH_STAR], (const void *) OMM_TEXTURE_STAR_FULL_0 },
        { (const void *) ((const u8 **) main_hud_lut)[GLYPH_STAR], (const void *) OMM_TEXTURE_STAR_FULL_1 },
        { (const void *) ((const u8 **) main_hud_lut)[GLYPH_STAR], (const void *) OMM_TEXTURE_STAR_FULL_2 },
        { (const void *) ((const u8 **) main_hud_lut)[GLYPH_STAR], (const void *) OMM_TEXTURE_STAR_FULL_3 },
        { (const void *) ((const u8 **) main_hud_lut)[GLYPH_STAR], (const void *) OMM_TEXTURE_STAR_FULL_4 },
        { (const void *) ((const u8 **) main_hud_lut)[GLYPH_STAR], (const void *) OMM_TEXTURE_STAR_FULL_5 },
        { (const void *) ((const u8 **) main_hud_lut)[GLYPH_STAR], (const void *) OMM_TEXTURE_STAR_FULL_6 },
        { (const void *) ((const u8 **) main_hud_lut)[GLYPH_STAR], (const void *) OMM_TEXTURE_STAR_FULL_7 },
        { (const void *) ((const u8 **) main_hud_lut)[GLYPH_STAR], (const void *) OMM_TEXTURE_STAR_FULL_8 },
        { (const void *) ((const u8 **) main_hud_lut)[GLYPH_STAR], (const void *) OMM_TEXTURE_STAR_FULL_9 },
        { (const void *) ((const u8 **) main_hud_lut)[GLYPH_STAR], (const void *) OMM_TEXTURE_STAR_FULL_10 },
        { (const void *) ((const u8 **) main_hud_lut)[GLYPH_STAR], (const void *) OMM_TEXTURE_STAR_FULL_11 },
        { (const void *) ((const u8 **) main_hud_lut)[GLYPH_STAR], (const void *) OMM_TEXTURE_STAR_FULL_12 },
        { (const void *) ((const u8 **) main_hud_lut)[GLYPH_STAR], (const void *) OMM_TEXTURE_STAR_FULL_13 },
        { (const void *) ((const u8 **) main_hud_lut)[GLYPH_STAR], (const void *) OMM_TEXTURE_STAR_FULL_14 },
        { (const void *) ((const u8 **) main_hud_lut)[GLYPH_STAR], (const void *) OMM_TEXTURE_STAR_FULL_15 },
        { (const void *) ((const u8 **) main_hud_lut)[GLYPH_STAR], (const void *) OMM_TEXTURE_STAR_FULL_16 },
    };
    return sOmmStarGlyphs[index][colored];
}

static bool omm_render_hud_stars(s16 x, s16 y, u8 alpha, s32 level, bool cond, bool shadow) {
    static const u8 sOmmStarShading[2][2] = {
        { 0xA0, 0xFF },
        { 0x00, 0x00 }
    };
    static const u8 sOmmStarOpacity[2][2] = {
        { 0xFF, 0xFF },
        { 0x00, 0x80 }
    };

    // Star counter
    u8 t = omm_stars_get_bits_total(level);
    if (t == 0 || !cond ||
#if OMM_GAME_IS_SMSR
        level == LEVEL_ENDING ||
#endif
        level == LEVEL_BOWSER_1 ||
        level == LEVEL_BOWSER_2 ||
        level == LEVEL_BOWSER_3) {
        omm_render_glyph_hud(x, y, 0xFF, 0xFF, 0xFF, alpha, omm_render_get_star_glyph(0, OMM_EXTRAS_COLORED_STARS), shadow);
        return false;
    }

    // Course stars
    s32 course = omm_level_get_course(level);
    u8 f = save_file_get_star_flags(gCurrSaveFileNum - 1, course - 1);
    u8 s = omm_stars_get_bits();
    for (u8 i = 0; i != 8; ++i) {
        u8 bit = (1 << i);
        if (bit & t) {
            u8 shading = sOmmStarShading[!(bit & f)][!(bit & s)];
            u8 opacity = sOmmStarOpacity[!(bit & f)][!(bit & s)];
            omm_render_glyph_hud(x, y, shading, shading, shading, (alpha * opacity) / 0xFF, omm_render_get_star_glyph(omm_clamp_s(course, 0, 16), OMM_EXTRAS_COLORED_STARS), shadow);
            x += OMM_RENDER_STAR_OFFSET_X;
        }
    }

    // Sparkly Star
    if (omm_sparkly_is_enabled()) {
        s32 mode = omm_sparkly_get_current_mode();
        for (s32 i = 0; i != 8; ++i) {
            s32 index = omm_sparkly_get_star_index(mode, level, i);
            if (index != -1) {
                bool collected = omm_sparkly_is_star_collected(mode, index);
                bool state = omm_sparkly_context_is_state_ok();
                u8 shading = sOmmStarShading[!collected][state];
                u8 opacity = sOmmStarOpacity[!collected][state];
                omm_render_glyph_hud(x, y, shading, shading, shading, (alpha * opacity) / 0xFF, OMM_SPARKLY_STAR_HUD_GLYPH[mode], shadow);
                return true;
            }
        }
    }

    return true;
}

//
// Coins
//

static void omm_render_hud_coins(s16 x, s16 y, u8 alpha, s32 coins, bool shadow) {
    omm_render_glyph_hud(x, y, 0xFF, 0xFF, 0xFF, alpha, OMM_TEXTURE_HUD_COIN, shadow);
    omm_render_number_hud(coins, 3, x + (OMM_RENDER_VALUE_NUMBER_X - OMM_RENDER_VALUE_GLYPH_X), y, alpha, true, shadow);
}

//
// Power-up
//

static bool omm_render_hud_power_up(struct MarioState *m, s16 y) {
    static const char *sOmmPowerUpGlyphs[4][2][2] = {
        { { OMM_TEXTURE_HUD_CAP_MARIO,   OMM_TEXTURE_HUD_CAP_MARIO_METAL   },
          { OMM_TEXTURE_HUD_CAPPY_MARIO, OMM_TEXTURE_HUD_CAPPY_MARIO_METAL }, },
        { { OMM_TEXTURE_HUD_CAP_PEACH,   OMM_TEXTURE_HUD_CAP_PEACH_METAL   },
          { OMM_TEXTURE_HUD_CAPPY_PEACH, OMM_TEXTURE_HUD_CAPPY_PEACH_METAL }, },
        { { OMM_TEXTURE_HUD_CAP_LUIGI,   OMM_TEXTURE_HUD_CAP_LUIGI_METAL   },
          { OMM_TEXTURE_HUD_CAPPY_LUIGI, OMM_TEXTURE_HUD_CAPPY_LUIGI_METAL }, },
        { { OMM_TEXTURE_HUD_CAP_WARIO,   OMM_TEXTURE_HUD_CAP_WARIO_METAL   },
          { OMM_TEXTURE_HUD_CAPPY_WARIO, OMM_TEXTURE_HUD_CAPPY_WARIO_METAL }, },
    };

    s32 playerIndex = omm_player_get_selected_index();
    bool isCappy = !OMM_CAP_CLASSIC;
    bool isWing = (m->flags & MARIO_WING_CAP) != 0;
    bool isMetal = (m->flags & MARIO_METAL_CAP) != 0;
    bool isVanish = (m->flags & MARIO_VANISH_CAP) != 0;

    if ((m->capTimer > 0) && (isWing || isMetal || isVanish)) {
        u8 alpha = (isVanish ? 0x80 : 0xFF);
        if (isWing) omm_render_glyph_hud(OMM_RENDER_VALUE_GLYPH_X, y, 0xFF, 0xFF, 0xFF, alpha, OMM_TEXTURE_HUD_CAP_WINGS, false);
        omm_render_glyph_hud(OMM_RENDER_VALUE_GLYPH_X, y, 0xFF, 0xFF, 0xFF, alpha, sOmmPowerUpGlyphs[playerIndex][isCappy][isMetal], false);
        omm_render_number_hud((m->capTimer + 29) / 30, 3, OMM_RENDER_VALUE_NUMBER_X, y, 0xFF, true, false);
        return true;
    }
    return false;
}

//
// Camera
//

static void omm_render_hud_camera(struct MarioState *m) {
    if (gHudDisplay.flags & HUD_DISPLAY_FLAG_CAMERA_AND_POWER) {
        u8 alpha = omm_render_hud_update_timer_and_get_alpha(m, &sOmmHudCameraTimer);
        if (alpha) {

            // OMM cam
            if (!OMM_CAMERA_CLASSIC) {
                omm_render_create_dl_ortho_matrix();
                omm_render_glyph_hud(OMM_RENDER_CAMERA_X, OMM_RENDER_CAMERA_Y, 0xFF, 0xFF, 0xFF, alpha, OMM_TEXTURE_HUD_CAMERA, false);
                if (OMM_CAMERA_8_DIRECTIONS) {
                    omm_render_glyph_hud(OMM_RENDER_CAMERA_X + OMM_RENDER_CAMERA_OFFSET_X, OMM_RENDER_CAMERA_Y, 0xFF, 0xFF, 0xFF, alpha, OMM_TEXTURE_HUD_CAMERA_8, false);
                } else if (OMM_CAMERA_16_DIRECTIONS) {
                    omm_render_glyph_hud(OMM_RENDER_CAMERA_X + OMM_RENDER_CAMERA_OFFSET_X, OMM_RENDER_CAMERA_Y, 0xFF, 0xFF, 0xFF, alpha, OMM_TEXTURE_HUD_CAMERA_16, false);
                }
                s32 mode = omm_camera_get_relative_dist_mode();
                s32 zoom = omm_abs_s(mode);
                s32 sign = (mode > 0 ? -1 : +1);
                for (s32 i = zoom; i > 0; --i) {
                    omm_render_glyph_hud(OMM_RENDER_CAMERA_X, OMM_RENDER_CAMERA_Y + sign * (OMM_RENDER_CAMERA_OFFSET_Y + OMM_RENDER_CAMERA_STEP_Y * i), 0xFF, 0xFF, 0xFF, alpha, (mode > 0 ? OMM_TEXTURE_HUD_CAMERA_FAR : OMM_TEXTURE_HUD_CAMERA_NEAR), false);
                }
            }
            
            // Puppy/better cam
            else if (BETTER_CAM_IS_ENABLED) {
                omm_render_create_dl_ortho_matrix();
                omm_render_glyph_hud(OMM_RENDER_CAMERA_X, OMM_RENDER_CAMERA_Y, 0xFF, 0xFF, 0xFF, alpha, ((const u8 **) main_hud_camera_lut)[GLYPH_CAM_CAMERA], false);
#if BETTER_CAM_IS_PUPPY_CAM
                omm_render_glyph_hud(OMM_RENDER_CAMERA_X + OMM_RENDER_CAMERA_OFFSET_X, OMM_RENDER_CAMERA_Y, 0xFF, 0xFF, 0xFF, alpha, texture_hud_char_puppycam, false);
#else
                omm_render_glyph_hud(OMM_RENDER_CAMERA_X + OMM_RENDER_CAMERA_OFFSET_X, OMM_RENDER_CAMERA_Y, 0xFF, 0xFF, 0xFF, alpha, ((const u8 **) main_hud_camera_lut)[GLYPH_CAM_LAKITU_HEAD], false);
#endif
            }

            // Lakitu cam
            else {
                s32 status = update_camera_hud_status(gCamera);
                if (status) {
                    omm_render_create_dl_ortho_matrix();
                    omm_render_glyph_hud(OMM_RENDER_CAMERA_X, OMM_RENDER_CAMERA_Y, 0xFF, 0xFF, 0xFF, alpha, ((const u8 **) main_hud_camera_lut)[GLYPH_CAM_CAMERA], false);
                    switch (status & CAM_STATUS_MODE_GROUP) {
                        case CAM_STATUS_MARIO:  omm_render_glyph_hud(OMM_RENDER_CAMERA_X + OMM_RENDER_CAMERA_OFFSET_X, OMM_RENDER_CAMERA_Y, 0xFF, 0xFF, 0xFF, alpha, ((const u8 **) main_hud_camera_lut)[GLYPH_CAM_MARIO_HEAD], false); break;
                        case CAM_STATUS_LAKITU: omm_render_glyph_hud(OMM_RENDER_CAMERA_X + OMM_RENDER_CAMERA_OFFSET_X, OMM_RENDER_CAMERA_Y, 0xFF, 0xFF, 0xFF, alpha, ((const u8 **) main_hud_camera_lut)[GLYPH_CAM_LAKITU_HEAD], false); break;
                        case CAM_STATUS_FIXED:  omm_render_glyph_hud(OMM_RENDER_CAMERA_X + OMM_RENDER_CAMERA_OFFSET_X, OMM_RENDER_CAMERA_Y, 0xFF, 0xFF, 0xFF, alpha, ((const u8 **) main_hud_camera_lut)[GLYPH_CAM_FIXED], false); break;
                    }
                    switch (status & CAM_STATUS_C_MODE_GROUP) {
                        case CAM_STATUS_C_DOWN: omm_render_small_glyph(OMM_RENDER_CAMERA_X + OMM_RENDER_GLYPH_SIZE / 4, OMM_RENDER_CAMERA_Y + OMM_RENDER_GLYPH_SIZE / 4 - OMM_RENDER_CAMERA_OFFSET_Y, OMM_RENDER_GLYPH_SIZE / 2, OMM_RENDER_GLYPH_SIZE / 2, 0xFF, 0xFF, 0xFF, alpha, ((const u8 **) main_hud_camera_lut)[GLYPH_CAM_ARROW_DOWN], false); break;
                        case CAM_STATUS_C_UP:   omm_render_small_glyph(OMM_RENDER_CAMERA_X + OMM_RENDER_GLYPH_SIZE / 4, OMM_RENDER_CAMERA_Y + OMM_RENDER_GLYPH_SIZE / 4 + OMM_RENDER_CAMERA_OFFSET_Y, OMM_RENDER_GLYPH_SIZE / 2, OMM_RENDER_GLYPH_SIZE / 2, 0xFF, 0xFF, 0xFF, alpha, ((const u8 **) main_hud_camera_lut)[GLYPH_CAM_ARROW_UP], false); break;
                    }
                }
            }
        }
    }
}

//
// Health meter
//

static void omm_render_hud_power_meter_background(f32 x, f32 y, u8 alpha) {
    for (s32 i = 0; i != OMM_RENDER_POWER_BACKGROUND_NUM_TRIS; ++i) {
        f32 a0 = ((i + 0) * 65536.f) / OMM_RENDER_POWER_BACKGROUND_NUM_TRIS;
        f32 a1 = ((i + 1) * 65536.f) / OMM_RENDER_POWER_BACKGROUND_NUM_TRIS;
        f32 d  = OMM_RENDER_POWER_SEGMENT_RADIUS_3 + 0.5f;
        sOmmPowerMeterVertexArrayHead[0] = (Vtx) { { { x,                y,                0 }, 0, { 0, 0 }, { OMM_RENDER_POWER_BACKGROUND_R, OMM_RENDER_POWER_BACKGROUND_G, OMM_RENDER_POWER_BACKGROUND_B, alpha } } };
        sOmmPowerMeterVertexArrayHead[1] = (Vtx) { { { x + d * sins(a0), y + d * coss(a0), 0 }, 0, { 0, 0 }, { OMM_RENDER_POWER_BACKGROUND_R, OMM_RENDER_POWER_BACKGROUND_G, OMM_RENDER_POWER_BACKGROUND_B, alpha } } };
        sOmmPowerMeterVertexArrayHead[2] = (Vtx) { { { x + d * sins(a1), y + d * coss(a1), 0 }, 0, { 0, 0 }, { OMM_RENDER_POWER_BACKGROUND_R, OMM_RENDER_POWER_BACKGROUND_G, OMM_RENDER_POWER_BACKGROUND_B, alpha } } };
        gSPVertex(sOmmPowerMeterDisplayListHead++, sOmmPowerMeterVertexArrayHead, 3, 0);
        gSP1Triangle(sOmmPowerMeterDisplayListHead++, 0, 2, 1, 0x0);
        sOmmPowerMeterVertexArrayHead += 3;
    }
}

static void omm_render_hud_power_meter_segments(f32 x0, f32 y0, u8 alpha, s32 hp, bool isLifeUp) {
    f32 ma = (65536.f * hp) / (OMM_RENDER_POWER_HP_FULL_SEGMENTS * OMM_RENDER_POWER_HP_PER_SEGMENT);
    f32 da = (65536.f)      / (OMM_RENDER_POWER_HP_FULL_SEGMENTS * OMM_RENDER_POWER_SEGMENT_NUM_QUADS);
    for (s32 segment = 0; segment != OMM_RENDER_POWER_HP_FULL_SEGMENTS; ++segment) {
        f32 sa = (segment * 65536 + 32768) / OMM_RENDER_POWER_HP_FULL_SEGMENTS;
        f32 x = x0 + OMM_RENDER_POWER_SEGMENT_DELTA * sins(sa);
        f32 y = y0 + OMM_RENDER_POWER_SEGMENT_DELTA * coss(sa);
        for (s32 i = 0; i != OMM_RENDER_POWER_SEGMENT_NUM_QUADS; ++i) {
            f32 a0 = da * (segment * OMM_RENDER_POWER_SEGMENT_NUM_QUADS + i + 0);
            f32 a1 = da * (segment * OMM_RENDER_POWER_SEGMENT_NUM_QUADS + i + 1);
            s32 index = 0;
            if (((a0 + a1) / 2) <= ma) {
                if (isLifeUp) {
                    index = 4;
                } else if (hp < OMM_RENDER_POWER_HP_CRITICAL) {
                    index = 1;
                } else if (hp < OMM_RENDER_POWER_HP_LOW) {
                    index = 2;
                } else if (hp < OMM_RENDER_POWER_HP_NORMAL) {
                    index = 3;
                } else {
                    index = 4;
                }
            }
            sOmmPowerMeterVertexArrayHead[0] = (Vtx) { { { x + sins(a0) * OMM_RENDER_POWER_SEGMENT_RADIUS_0, y + coss(a0) * OMM_RENDER_POWER_SEGMENT_RADIUS_0, 0 }, 0, { 0, 0 }, { OMM_RENDER_POWER_SEGMENT_BORDER_R, OMM_RENDER_POWER_SEGMENT_BORDER_G, OMM_RENDER_POWER_SEGMENT_BORDER_B, alpha } } };
            sOmmPowerMeterVertexArrayHead[1] = (Vtx) { { { x + sins(a0) * OMM_RENDER_POWER_SEGMENT_RADIUS_1, y + coss(a0) * OMM_RENDER_POWER_SEGMENT_RADIUS_1, 0 }, 0, { 0, 0 }, { OMM_RENDER_POWER_SEGMENT_CENTER_R, OMM_RENDER_POWER_SEGMENT_CENTER_G, OMM_RENDER_POWER_SEGMENT_CENTER_B, alpha } } };
            sOmmPowerMeterVertexArrayHead[2] = (Vtx) { { { x + sins(a0) * OMM_RENDER_POWER_SEGMENT_RADIUS_2, y + coss(a0) * OMM_RENDER_POWER_SEGMENT_RADIUS_2, 0 }, 0, { 0, 0 }, { OMM_RENDER_POWER_SEGMENT_CENTER_R, OMM_RENDER_POWER_SEGMENT_CENTER_G, OMM_RENDER_POWER_SEGMENT_CENTER_B, alpha } } };
            sOmmPowerMeterVertexArrayHead[3] = (Vtx) { { { x + sins(a0) * OMM_RENDER_POWER_SEGMENT_RADIUS_3, y + coss(a0) * OMM_RENDER_POWER_SEGMENT_RADIUS_3, 0 }, 0, { 0, 0 }, { OMM_RENDER_POWER_SEGMENT_BORDER_R, OMM_RENDER_POWER_SEGMENT_BORDER_G, OMM_RENDER_POWER_SEGMENT_BORDER_B, alpha } } };
            sOmmPowerMeterVertexArrayHead[4] = (Vtx) { { { x + sins(a1) * OMM_RENDER_POWER_SEGMENT_RADIUS_0, y + coss(a1) * OMM_RENDER_POWER_SEGMENT_RADIUS_0, 0 }, 0, { 0, 0 }, { OMM_RENDER_POWER_SEGMENT_BORDER_R, OMM_RENDER_POWER_SEGMENT_BORDER_G, OMM_RENDER_POWER_SEGMENT_BORDER_B, alpha } } };
            sOmmPowerMeterVertexArrayHead[5] = (Vtx) { { { x + sins(a1) * OMM_RENDER_POWER_SEGMENT_RADIUS_1, y + coss(a1) * OMM_RENDER_POWER_SEGMENT_RADIUS_1, 0 }, 0, { 0, 0 }, { OMM_RENDER_POWER_SEGMENT_CENTER_R, OMM_RENDER_POWER_SEGMENT_CENTER_G, OMM_RENDER_POWER_SEGMENT_CENTER_B, alpha } } };
            sOmmPowerMeterVertexArrayHead[6] = (Vtx) { { { x + sins(a1) * OMM_RENDER_POWER_SEGMENT_RADIUS_2, y + coss(a1) * OMM_RENDER_POWER_SEGMENT_RADIUS_2, 0 }, 0, { 0, 0 }, { OMM_RENDER_POWER_SEGMENT_CENTER_R, OMM_RENDER_POWER_SEGMENT_CENTER_G, OMM_RENDER_POWER_SEGMENT_CENTER_B, alpha } } };
            sOmmPowerMeterVertexArrayHead[7] = (Vtx) { { { x + sins(a1) * OMM_RENDER_POWER_SEGMENT_RADIUS_3, y + coss(a1) * OMM_RENDER_POWER_SEGMENT_RADIUS_3, 0 }, 0, { 0, 0 }, { OMM_RENDER_POWER_SEGMENT_BORDER_R, OMM_RENDER_POWER_SEGMENT_BORDER_G, OMM_RENDER_POWER_SEGMENT_BORDER_B, alpha } } };
            gSPVertex(sOmmPowerMeterDisplayListHead++, sOmmPowerMeterVertexArrayHead, 8, 0);
            gSP2Triangles(sOmmPowerMeterDisplayListHead++, 2, 6, 3, 0x0, 3, 6, 7, 0x0);
            gSP2Triangles(sOmmPowerMeterDisplayListHead++, 1, 5, 2, 0x0, 2, 5, 6, 0x0);
            gSP2Triangles(sOmmPowerMeterDisplayListHead++, 0, 4, 1, 0x0, 1, 4, 5, 0x0);
            sOmmPowerMeterVertexArrayHead += 8;
        }
    }
}

static void omm_render_hud_power_meter_heart(f32 x, f32 y, u8 alpha, s32 hp) {
    for (s32 i = 0; i != OMM_RENDER_POWER_HEART_NUM_PIECES; ++i) {
        f32 t0 = (i + 0 - 32) / (OMM_RENDER_POWER_HEART_NUM_PIECES / 2.f);
        f32 t1 = (i + 1 - 32) / (OMM_RENDER_POWER_HEART_NUM_PIECES / 2.f);
        s32 index = OMM_RENDER_POWER_HEART_PULSE;

        // Heart shape <3
        f32 a0 = t0 * 65536.f / (2.f * M_PI);
        f32 a1 = t1 * 65536.f / (2.f * M_PI);
        f32 r0 = (1.f - omm_abs_f(t0)) * (1.f + 3.f * omm_abs_f(t0));
        f32 r1 = (1.f - omm_abs_f(t1)) * (1.f + 3.f * omm_abs_f(t1));
        f32 dx0 = r0 * sins(a0);
        f32 dy0 = r0 * coss(a0) - 0.65f;
        f32 dx1 = r1 * sins(a1);
        f32 dy1 = r1 * coss(a1) - 0.65f;
        sOmmPowerMeterVertexArrayHead[0] = (Vtx) { { { x,                                           y,                                           0 }, 0, { 0, 0 }, { OMM_RENDER_POWER_HEART_CENTER_R, OMM_RENDER_POWER_HEART_CENTER_G, OMM_RENDER_POWER_HEART_CENTER_B, alpha } } };
        sOmmPowerMeterVertexArrayHead[1] = (Vtx) { { { x + dx0 * OMM_RENDER_POWER_HEART_RADIUS_1_X, y + dy0 * OMM_RENDER_POWER_HEART_RADIUS_1_Y, 0 }, 0, { 0, 0 }, { OMM_RENDER_POWER_HEART_CENTER_R, OMM_RENDER_POWER_HEART_CENTER_G, OMM_RENDER_POWER_HEART_CENTER_B, alpha } } };
        sOmmPowerMeterVertexArrayHead[2] = (Vtx) { { { x + dx0 * OMM_RENDER_POWER_HEART_RADIUS_2_X, y + dy0 * OMM_RENDER_POWER_HEART_RADIUS_2_Y, 0 }, 0, { 0, 0 }, { OMM_RENDER_POWER_HEART_BORDER_R, OMM_RENDER_POWER_HEART_BORDER_G, OMM_RENDER_POWER_HEART_BORDER_B, alpha } } };
        sOmmPowerMeterVertexArrayHead[3] = (Vtx) { { { x + dx1 * OMM_RENDER_POWER_HEART_RADIUS_1_X, y + dy1 * OMM_RENDER_POWER_HEART_RADIUS_1_Y, 0 }, 0, { 0, 0 }, { OMM_RENDER_POWER_HEART_CENTER_R, OMM_RENDER_POWER_HEART_CENTER_G, OMM_RENDER_POWER_HEART_CENTER_B, alpha } } };
        sOmmPowerMeterVertexArrayHead[4] = (Vtx) { { { x + dx1 * OMM_RENDER_POWER_HEART_RADIUS_2_X, y + dy1 * OMM_RENDER_POWER_HEART_RADIUS_2_Y, 0 }, 0, { 0, 0 }, { OMM_RENDER_POWER_HEART_BORDER_R, OMM_RENDER_POWER_HEART_BORDER_G, OMM_RENDER_POWER_HEART_BORDER_B, alpha } } };
        gSPVertex(sOmmPowerMeterDisplayListHead++, sOmmPowerMeterVertexArrayHead, 5, 0);
        gSP2Triangles(sOmmPowerMeterDisplayListHead++, 2, 1, 4, 0x0, 1, 3, 4, 0x0);
        gSP1Triangle(sOmmPowerMeterDisplayListHead++, 1, 0, 3, 0x0);
        sOmmPowerMeterVertexArrayHead += 5;
    }
}

static void omm_render_hud_power_meter_health_gauge(f32 x, f32 y, u8 alpha, s32 hp, f32 luPos, bool isLifeUp) {
    if (isLifeUp) {
        if (hp > omm_health_get_max_hp(0)) {
            x = x + luPos * ((SCREEN_WIDTH * 0.5f) - x);
            y = y + luPos * ((SCREEN_HEIGHT * 0.65f) - y);
            omm_render_hud_power_meter_background(x, y, alpha);
            omm_render_hud_power_meter_segments(x, y, alpha, hp - omm_health_get_max_hp(0), true);
            omm_render_hud_power_meter_heart(x, y - OMM_RENDER_POWER_HEART_OFFSET_Y, alpha, hp);
            omm_render_number_hud(OMM_RENDER_POWER_HP_VALUE, 1, x - OMM_RENDER_POWER_NUMBER_OFFSET_X, y - OMM_RENDER_POWER_NUMBER_OFFSET_Y, alpha, false, false);
        }
    } else {
        hp = omm_min_s(hp, omm_health_get_max_hp(0));
        omm_render_hud_power_meter_segments(x, y, alpha, hp, false);
        if (luPos > 0.1f) {
            omm_render_hud_power_meter_heart(x, y - OMM_RENDER_POWER_HEART_OFFSET_Y, alpha, hp);
            omm_render_number_hud(OMM_RENDER_POWER_HP_VALUE, 1, x - OMM_RENDER_POWER_NUMBER_OFFSET_X, y - OMM_RENDER_POWER_NUMBER_OFFSET_Y, alpha, false, false);
        }
    }
}

static void omm_render_hud_power_meter(struct MarioState *m) {
    if ((gHudDisplay.flags & HUD_DISPLAY_FLAG_CAMERA_AND_POWER) && !g1HPMode) {
        u8 alpha = omm_render_hud_update_timer_and_get_alpha(m, &sOmmHudHealthTimer);
        if (alpha) {
            OMM_RENDER_ENABLE_ALPHA;
            omm_render_create_dl_ortho_matrix();
            gSPDisplayList(gDisplayListHead++, sOmmPowerMeterDisplayList);
            sOmmPowerMeterDisplayListHead = sOmmPowerMeterDisplayList;
            sOmmPowerMeterVertexArrayHead = sOmmPowerMeterVertexArray;

            // Display list
            gDPPipeSync(sOmmPowerMeterDisplayListHead++);
            gSPClearGeometryMode(sOmmPowerMeterDisplayListHead++, G_LIGHTING);
            gDPSetCombineLERP(sOmmPowerMeterDisplayListHead++, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE);

            // Odyssey Health (3/6 HP)
            if (OMM_MOVESET_ODYSSEY) {
                s32 hp = gOmmData->mario->state.hp;
                f32 rumble = omm_get_life_gauge_rumble_strength(m);
                f32 luPos = omm_get_life_up_gauge_position(hp);
                f32 x = OMM_RENDER_POWER_X + (random_float() * rumble) - (rumble / 2.f);
                f32 y = OMM_RENDER_POWER_Y + (random_float() * rumble) - (rumble / 2.f);
                omm_render_hud_power_meter_health_gauge(x, y, alpha, hp, luPos, false);
                omm_render_hud_power_meter_health_gauge(x - OMM_RENDER_POWER_LIFE_UP_OFFSET_X, y, alpha, hp, luPos, true);
            }

            // Vanilla Health (8 segments)
            else {
                s32 hp = (m->health >> 8);
                omm_render_hud_power_meter_health_gauge(OMM_RENDER_POWER_X, OMM_RENDER_POWER_Y, alpha, hp, 1.f, false);
            }

            gDPPipeSync(sOmmPowerMeterDisplayListHead++);
            gSPSetGeometryMode(sOmmPowerMeterDisplayListHead++, G_LIGHTING);
            gSPEndDisplayList(sOmmPowerMeterDisplayListHead);
        }
    }
}

//
// Oxygen meter
//

static void omm_render_hud_o2_meter(struct MarioState *m) {
    if (OMM_MOVESET_ODYSSEY && (gHudDisplay.flags & OMM_HUD_DISPLAY_FLAG_O2_METER)) {
        u8 alpha = omm_render_hud_update_timer_and_get_alpha(m, &sOmmHudOxygenTimer);
        if (alpha) {

            // Camera normal
            Vec3f camN;
            vec3f_dif(camN, gLakituState.focus, gLakituState.pos);
            vec3f_norm(camN);

            // Camera horizontal axis
            s16 camYaw = atan2s(camN[2], camN[0]) - 0x4000;
            Vec3f camH = { sins(camYaw), 0.f, coss(camYaw) };

            // Camera vertical axis
            Vec3f camV;
            vec3f_cross(camV, camN, camH);
            vec3f_norm(camV);

            // Mario's position to camera plane
            Vec2f pos2d;
            f32 dist;
            vec3f_to_2d_plane(pos2d, &dist, m->pos, gLakituState.pos, camN, camH, camV);
            if (dist > 0.f) {
                f32 x = (SCREEN_WIDTH  / 2) * (1.f + (pos2d[0] / dist)) + ((SCREEN_WIDTH  * 1.6f) / sqrtf(dist));
                f32 y = (SCREEN_HEIGHT / 2) * (1.f - (pos2d[1] / dist)) + ((SCREEN_HEIGHT * 3.2f) / sqrtf(dist));
                f32 t = omm_clamp_0_1_f((f32) (gOmmData->mario->state.o2 - (OMM_O2_MAX_DURATION / 100.f)) / (f32) (OMM_O2_MAX_DURATION * 0.99f));
                f32 j = t * OMM_RENDER_O2_NUM_QUADS;
                bool blink = (t == 1.f) && (gGlobalTimer & 1);

                // Vertices
                static Vtx sOmmO2MeterVertices[OMM_RENDER_O2_NUM_QUADS * 4];
                Vtx *vertices = sOmmO2MeterVertices;
                for (s32 i = 0; i != OMM_RENDER_O2_NUM_QUADS; ++i) {
                    s32 a0 = ((i + 0) * -65536) / OMM_RENDER_O2_NUM_QUADS;
                    s32 a1 = ((i + 1) * -65536) / OMM_RENDER_O2_NUM_QUADS;
                    vertices[0] = (Vtx) { { { x + OMM_RENDER_O2_RADIUS_IN  * sins(a0), y + OMM_RENDER_O2_RADIUS_IN  * coss(a0), 0 }, 0, { 0, 0 }, { OMM_RENDER_O2_COLOR_R, OMM_RENDER_O2_COLOR_G, OMM_RENDER_O2_COLOR_B, alpha } } };
                    vertices[1] = (Vtx) { { { x + OMM_RENDER_O2_RADIUS_OUT * sins(a0), y + OMM_RENDER_O2_RADIUS_OUT * coss(a0), 0 }, 0, { 0, 0 }, { OMM_RENDER_O2_COLOR_R, OMM_RENDER_O2_COLOR_G, OMM_RENDER_O2_COLOR_B, alpha } } };
                    vertices[2] = (Vtx) { { { x + OMM_RENDER_O2_RADIUS_IN  * sins(a1), y + OMM_RENDER_O2_RADIUS_IN  * coss(a1), 0 }, 0, { 0, 0 }, { OMM_RENDER_O2_COLOR_R, OMM_RENDER_O2_COLOR_G, OMM_RENDER_O2_COLOR_B, alpha } } };
                    vertices[3] = (Vtx) { { { x + OMM_RENDER_O2_RADIUS_OUT * sins(a1), y + OMM_RENDER_O2_RADIUS_OUT * coss(a1), 0 }, 0, { 0, 0 }, { OMM_RENDER_O2_COLOR_R, OMM_RENDER_O2_COLOR_G, OMM_RENDER_O2_COLOR_B, alpha } } };
                    vertices += 4;
                }

                // Triangles
                static Gfx sOmmO2MeterTriangles[OMM_RENDER_O2_NUM_QUADS * 2 + 1];
                Gfx *triangles = sOmmO2MeterTriangles;
                for (s32 i = 0; i != OMM_RENDER_O2_NUM_QUADS; ++i) {
                    gSPVertex(triangles++, sOmmO2MeterVertices + 4 * i, 4, 0);
                    gSP2Triangles(triangles++, 0, 1, 2, 0, 3, 2, 1, 0);
                }
                gSPEndDisplayList(triangles);

                // Display list
                OMM_RENDER_ENABLE_ALPHA;
                gDPPipeSync(gDisplayListHead++);
                gSPClearGeometryMode(gDisplayListHead++, G_LIGHTING);
                gDPPipeSync(gDisplayListHead++);
                gDPSetCombineLERP(gDisplayListHead++, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE);
                gSPDisplayList(gDisplayListHead++, sOmmO2MeterTriangles);
                gDPPipeSync(gDisplayListHead++);
                gSPSetGeometryMode(gDisplayListHead++, G_LIGHTING);
                gDPPipeSync(gDisplayListHead++);
            }
        }
    }
}

//
// Timer
//

static void omm_render_hud_timer() {
    if (gHudDisplay.flags & HUD_DISPLAY_FLAG_TIMER) {
        static const char *sOmmTimerGlyphs[] = {
            OMM_TEXTURE_HUD_0,
            OMM_TEXTURE_HUD_1,
            OMM_TEXTURE_HUD_2,
            OMM_TEXTURE_HUD_3,
            OMM_TEXTURE_HUD_4,
            OMM_TEXTURE_HUD_5,
            OMM_TEXTURE_HUD_6,
            OMM_TEXTURE_HUD_7,
            OMM_TEXTURE_HUD_8,
            OMM_TEXTURE_HUD_9,
            OMM_TEXTURE_HUD_M,
            OMM_TEXTURE_HUD_S,
        };

        s32 frames10 = (gHudDisplay.timer * 10);
        s32 glyphs[] = {
            ((frames10 / 180000) % 10),
            ((frames10 /  18000) % 10),
            10,
            ((frames10 /   3000) %  6),
            ((frames10 /    300) % 10),
            11,
            ((frames10 /     30) % 10),
            ((frames10 /      3) % 10),
        };

        for (s32 i = 0, j = 0; i != 8; ++i) {
            j = omm_max_s(j, glyphs[i] * (glyphs[i] < 10));
            omm_render_glyph_hud(OMM_RENDER_TIMER_X + i * OMM_RENDER_TIMER_OFFSET_X, OMM_RENDER_TIMER_Y, 0xFF, 0xFF, 0xFF, 0xFF - (0xAA * (j == 0)), sOmmTimerGlyphs[glyphs[i]], false);
        }
    }
}

//
// Sparkly Stars timer
//

static void omm_render_sparkly_timer(s16 x, s16 y, s16 w, u8 alpha, s32 mode, s32 timer, bool blink, bool renderStarGlyph) {
    static const char *sOmmSparklyTimerGlyphs[] = {
        OMM_TEXTURE_HUD_0,
        OMM_TEXTURE_HUD_1,
        OMM_TEXTURE_HUD_2,
        OMM_TEXTURE_HUD_3,
        OMM_TEXTURE_HUD_4,
        OMM_TEXTURE_HUD_5,
        OMM_TEXTURE_HUD_6,
        OMM_TEXTURE_HUD_7,
        OMM_TEXTURE_HUD_8,
        OMM_TEXTURE_HUD_9,
        OMM_TEXTURE_HUD_DOTS,
        OMM_TEXTURE_STAR_FULL_16,
        OMM_TEXTURE_STAR_FULL_17,
        OMM_TEXTURE_STAR_FULL_18,
        OMM_TEXTURE_STAR_FULL_19,
    };

    s32 glyphs[] = {
        11 + mode,
        -1,
        ((timer / 1080000) % 10),
        ((timer /  108000) % 10),
        10,
        ((timer /   18000) %  6),
        ((timer /    1800) % 10),
        10,
        ((timer /     300) %  6),
        ((timer /      30) % 10),
    };
    for (s32 i = (renderStarGlyph ? 0 : 2); i != 10; ++i) {
        if (glyphs[i] != -1) {
            u8 a = alpha / ((glyphs[i] == 10 && blink && (timer % 30) >= 15) ? 3 : 1);
            omm_render_glyph(
                x + (i - 2) * OMM_RENDER_SPARKLY_TIMER_OFFSET_X(w), y, w, w,
                0xFF, 0xFF, 0xFF, a, sOmmSparklyTimerGlyphs[glyphs[i]], false
            );
        }
    }
}

//
// Cannon
//

static void omm_render_hud_cannon_reticle() {
    if (gCurrentArea && gCurrentArea->camera && gCurrentArea->camera->mode == CAMERA_MODE_INSIDE_CANNON) {
        render_hud_cannon_reticle();
    }
}

//
// Values
//

static void omm_render_hud_values(struct MarioState *m) {
    s16 y = OMM_RENDER_VALUE_Y;

    // Star count
    if (gHudDisplay.flags & HUD_DISPLAY_FLAG_STAR_COUNT) {
        u8 alpha = omm_render_hud_update_timer_and_get_alpha(m, &sOmmHudStarsTimer);
        if (alpha) {
            if (omm_render_hud_stars(OMM_RENDER_VALUE_GLYPH_X, y, alpha, gCurrLevelNum, !omm_is_game_paused(), false)) {
                y -= OMM_RENDER_OFFSET_Y;
            } else {

                // Power star counter
                omm_render_number_hud(gHudDisplay.stars, 3, OMM_RENDER_VALUE_NUMBER_X, y, alpha, true, false);
                y -= OMM_RENDER_OFFSET_Y;

                // Sparkly Star counter
                if (omm_sparkly_is_enabled()) {
                    s32 mode = omm_sparkly_get_current_mode();
                    omm_render_glyph_hud(OMM_RENDER_VALUE_GLYPH_X, y, 0xFF, 0xFF, 0xFF, alpha, OMM_SPARKLY_STAR_HUD_GLYPH[mode], false);
                    omm_render_number_hud(omm_sparkly_get_star_count(mode), 3, OMM_RENDER_VALUE_NUMBER_X, y, alpha, true, false);
                    y -= OMM_RENDER_OFFSET_Y;
                }
            }
        }
    }

    // Coint count
    if (gHudDisplay.flags & HUD_DISPLAY_FLAG_COIN_COUNT) {
        u8 alpha = omm_render_hud_update_timer_and_get_alpha(m, &sOmmHudCoinsTimer);
        if (alpha) {

            // Coins
            omm_render_hud_coins(OMM_RENDER_VALUE_GLYPH_X, y, alpha, gHudDisplay.coins, false);

            // Sparkles
            if (OMM_EXTRAS_VANISHING_HUD || sOmmHudCoinsTimer.alwaysActive) {
                static s32 sOmmHudCoinSparkles[OMM_RENDER_HUD_COIN_SPARKLE_COUNT] = { 0 };

                // New sparkle
                if (sOmmHudCoinsTimer.timer == sOmmHudCoinsTimer.timerMin) {
                    for (s32 i = 0; i != OMM_RENDER_HUD_COIN_SPARKLE_COUNT; ++i) {
                        if (sOmmHudCoinSparkles[i] <= 0) {
                            sOmmHudCoinSparkles[i] = OMM_RENDER_HUD_COIN_SPARKLE_FRAMES * OMM_RENDER_HUD_COIN_SPARKLE_DELAY;
                            break;
                        }
                    }
                }

                // Update
                for (s32 i = 0; i != OMM_RENDER_HUD_COIN_SPARKLE_COUNT; ++i) {
                    if (sOmmHudCoinSparkles[i]-- > 0) {
                        omm_render_texrect(
                            OMM_RENDER_HUD_COIN_SPARKLE_X, OMM_RENDER_HUD_COIN_SPARKLE_Y,
                            OMM_RENDER_HUD_COIN_SPARKLE_SIZE, OMM_RENDER_HUD_COIN_SPARKLE_SIZE,
                            G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 32, 
                            0xFF, 0xFF, 0xFF, 0xFF,
                            (const void *) (OMM_ARRAY_OF(const char *) {
                                OMM_ASSET_SPARKLE_5,
                                OMM_ASSET_SPARKLE_4,
                                OMM_ASSET_SPARKLE_3,
                                OMM_ASSET_SPARKLE_2,
                                OMM_ASSET_SPARKLE_1,
                                OMM_ASSET_SPARKLE_0,
                            })[sOmmHudCoinSparkles[i] / OMM_RENDER_HUD_COIN_SPARKLE_DELAY]
                        );
                    }
                }
            }
            y -= OMM_RENDER_OFFSET_Y;
        }
    }

    // Vibe gauge
    if (gHudDisplay.flags & OMM_HUD_DISPLAY_FLAG_VIBE_GAUGE) {
        if (OMM_PLAYER_IS_PEACH) {
            u8 alpha = omm_render_hud_update_timer_and_get_alpha(m, &sOmmHudVibeTimer);
            if (alpha) {
                switch (gOmmData->mario->peach.vibeType) {
                    case OMM_PEACH_VIBE_TYPE_NONE:  omm_render_glyph_hud(OMM_RENDER_VALUE_GLYPH_X, y, 0xFF, 0xFF, 0xFF, alpha, OMM_TEXTURE_HUD_VIBE_NORMAL, false); break;
                    case OMM_PEACH_VIBE_TYPE_JOY:   omm_render_glyph_hud(OMM_RENDER_VALUE_GLYPH_X, y, 0xFF, 0xFF, 0xFF, alpha, OMM_TEXTURE_HUD_VIBE_JOY, false); break;
                    case OMM_PEACH_VIBE_TYPE_RAGE:  omm_render_glyph_hud(OMM_RENDER_VALUE_GLYPH_X, y, 0xFF, 0xFF, 0xFF, alpha, OMM_TEXTURE_HUD_VIBE_RAGE, false); break;
                    case OMM_PEACH_VIBE_TYPE_GLOOM: omm_render_glyph_hud(OMM_RENDER_VALUE_GLYPH_X, y, 0xFF, 0xFF, 0xFF, alpha, OMM_TEXTURE_HUD_VIBE_GLOOM, false); break;
                    case OMM_PEACH_VIBE_TYPE_CALM:  omm_render_glyph_hud(OMM_RENDER_VALUE_GLYPH_X, y, 0xFF, 0xFF, 0xFF, alpha, OMM_TEXTURE_HUD_VIBE_CALM, false); break;
                }
                s32 value = omm_relerp_0_1_s(gOmmData->mario->peach.vibeGauge, OMM_PEACH_VIBE_GAUGE_MAX, OMM_PEACH_VIBE_GAUGE_HEART_INC, 0, 100);
                omm_render_number_hud(value, 3, OMM_RENDER_VALUE_NUMBER_X, y, alpha, true, false);
                y -= OMM_RENDER_OFFSET_Y;
            }
        }
    }

    // Power-up duration
    if (gHudDisplay.flags & OMM_HUD_DISPLAY_FLAG_POWER_UP) {
        if (omm_render_hud_power_up(m, y)) {
            y -= OMM_RENDER_OFFSET_Y;
        }
    }

    if (gHudDisplay.flags & OMM_HUD_DISPLAY_FLAG_SPARKLY_STARS_CONDITIONS) {

        // Remaining flames
        if (omm_sparkly_context_get_data_flags(OMM_SPARKLY_DATA_FLAG_FLAMES) && omm_sparkly_context_is_state_ok()) {
            s32 flames = omm_sparkly_context_get_remaining_flames();
            if (flames > 0) {
                s32 flameW = 20;
                s32 flameX = OMM_RENDER_VALUE_GLYPH_X - (flameW - OMM_RENDER_GLYPH_SIZE) / 2;
                s32 flameY = y - (flameW - OMM_RENDER_GLYPH_SIZE) / 2;
                s32 flameT = (gGlobalTimer / 2) % 8;

                // Render the animated flame
                omm_render_texrect(
                    flameX, flameY, flameW, flameW,
                    G_IM_FMT_IA, G_IM_SIZ_16b, 32, 32, 
                    0xFF, 0x00, 0x00, 0xFF,
                    (const void *) (OMM_ARRAY_OF(const char *) {
                        OMM_ASSET_FLAME_0,
                        OMM_ASSET_FLAME_1,
                        OMM_ASSET_FLAME_2,
                        OMM_ASSET_FLAME_3,
                        OMM_ASSET_FLAME_4,
                        OMM_ASSET_FLAME_5,
                        OMM_ASSET_FLAME_6,
                        OMM_ASSET_FLAME_7,
                    })[flameT]
                );

                // Render the number
                omm_render_number_hud(flames, 3, OMM_RENDER_VALUE_NUMBER_X, y, 0xFF, true, false);
                y -= OMM_RENDER_OFFSET_Y;
            }
        }

        // Remaining boxes
        if (omm_sparkly_context_get_data_flags(OMM_SPARKLY_DATA_FLAG_BOXES) && omm_sparkly_context_is_state_ok()) {
            s32 boxes = omm_sparkly_context_get_remaining_boxes();
            if (boxes > 0) {
                s32 boxW = OMM_RENDER_GLYPH_SIZE;
                s32 boxX = OMM_RENDER_VALUE_GLYPH_X - (boxW - OMM_RENDER_GLYPH_SIZE) / 2;
                s32 boxY = y - (boxW - OMM_RENDER_GLYPH_SIZE) / 2;

                // Render the box icon
                omm_render_texrect(
                    boxX, boxY, boxW, boxW,
                    G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 32, 
                    0xFF, 0xFF, 0xFF, 0xFF,
                    (const void *) OMM_ASSET_EXCLAMATION_BOX
                );

                // Render the number
                omm_render_number_hud(boxes, 3, OMM_RENDER_VALUE_NUMBER_X, y, 0xFF, true, false);
                y -= OMM_RENDER_OFFSET_Y;
            }
        }

        // Remaining 1-up mushrooms
        s32 numMushrooms = omm_sparkly_context_get_data_flags(OMM_SPARKLY_DATA_BITS_MUSHROOMS);
        if (numMushrooms > 0 && omm_sparkly_context_is_state_ok()) {
            s32 mushrooms = numMushrooms - omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_1UP_MUSHROOMS);
            if (mushrooms > 0) {
                s32 mushroomW = OMM_RENDER_GLYPH_SIZE;
                s32 mushroomX = OMM_RENDER_VALUE_GLYPH_X - (mushroomW - OMM_RENDER_GLYPH_SIZE) / 2;
                s32 mushroomY = y - (mushroomW - OMM_RENDER_GLYPH_SIZE) / 2;

                // Render the mushroom icon
                omm_render_texrect(
                    mushroomX, mushroomY, mushroomW, mushroomW,
                    G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 64, 
                    0xFF, 0xFF, 0xFF, 0xFF,
                    (const void *) OMM_ASSET_MUSHROOM_1UP
                );

                // Render the number
                omm_render_number_hud(mushrooms, 3, OMM_RENDER_VALUE_NUMBER_X, y, 0xFF, true, false);
                y -= OMM_RENDER_OFFSET_Y;
            }
        }
    }
}

//
// Red coins radar
//

static struct Object *obj_get_red_coin_star() {

    // Hidden red coin star
    struct Object *hiddenRedCoinStar = obj_get_first_with_behavior(bhvHiddenRedCoinStar);
    if (hiddenRedCoinStar) {
        return hiddenRedCoinStar;
    }

    // Bowser red coin star
    struct Object *bowserRedCoinStar = obj_get_first_with_behavior(bhvBowserCourseRedCoinStar);
    if (bowserRedCoinStar) {
        return bowserRedCoinStar;
    }

    // Actual red coin star
    for_each_until_null(const BehaviorScript *, bhv, OMM_ARRAY_OF(const BehaviorScript *) { bhvStar, bhvStarSpawnCoordinates, NULL }) {
        for_each_object_with_behavior(star, *bhv) {
            if (star->activeFlags & ACTIVE_FLAG_RED_COIN_STAR) {
                return star;
            }
        }
    }

    // No red coin star
    return NULL;
}

static void omm_render_hud_red_coins_radar(struct MarioState *m) {
    if (OMM_EXTRAS_RED_COINS_RADAR && (gHudDisplay.flags & OMM_HUD_DISPLAY_FLAG_RED_COINS_RADAR)) {
        struct Object *radarTarget = obj_get_red_coin_star();
        if (radarTarget) {

            // Target and display the nearest red coin
            struct Object *redCoin = obj_get_nearest_with_behavior(m->marioObj, bhvRedCoin);
            if (redCoin) {
                radarTarget = redCoin;
                omm_render_create_dl_ortho_matrix();
                omm_render_texrect(
                    OMM_RENDER_RADAR_X - OMM_RENDER_RADAR_COIN_SIZE / 2,
                    OMM_RENDER_RADAR_Y - OMM_RENDER_RADAR_COIN_SIZE / 2,
                    OMM_RENDER_RADAR_COIN_SIZE,
                    OMM_RENDER_RADAR_COIN_SIZE,
                    G_IM_FMT_IA, G_IM_SIZ_16b, 32, 32, 
                    0xFF, 0x00, 0x00, 0xFF,
                    (const void *) (OMM_ARRAY_OF(const char *) {
                    OMM_ASSET_COIN_0,
                    OMM_ASSET_COIN_1,
                    OMM_ASSET_COIN_2,
                    OMM_ASSET_COIN_3,
                    })[(gGlobalTimer >> 1) & 3]
                );
            }
            
            // Target and display the red coin star
            else {
                omm_render_create_dl_ortho_matrix();
                omm_render_texrect(
                    OMM_RENDER_RADAR_X - OMM_RENDER_RADAR_STAR_SIZE / 2,
                    OMM_RENDER_RADAR_Y - OMM_RENDER_RADAR_STAR_SIZE / 2,
                    OMM_RENDER_RADAR_STAR_SIZE,
                    OMM_RENDER_RADAR_STAR_SIZE,
                    G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, 32, 
                    0xFF, 0x80, 0x80, 0xFF,
                    (const void *) (OMM_ARRAY_OF(const char *) {
                    OMM_ASSET_RED_STAR_0,
                    OMM_ASSET_RED_STAR_1,
                    OMM_ASSET_RED_STAR_2,
                    OMM_ASSET_RED_STAR_3,
                    OMM_ASSET_RED_STAR_4,
                    OMM_ASSET_RED_STAR_5,
                    OMM_ASSET_RED_STAR_6,
                    OMM_ASSET_RED_STAR_7,
                    })[(gGlobalTimer >> 1) & 7]
                );
            }
            
            // Radar arrow vertex buffer
            static Vtx sRadarArrowVtx[4] = {
                { { { 0, 0, 0 }, 0, { 0, 4096 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
                { { { 0, 0, 0 }, 0, { 4096, 4096 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
                { { { 0, 0, 0 }, 0, { 4096, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
                { { { 0, 0, 0 }, 0, { 0, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } }
            };
            s16 angle = atan2s(
                radarTarget->oPosZ - m->pos[2],
                radarTarget->oPosX - m->pos[0]
            ) - atan2s(
                m->pos[2] - gCamera->pos[2],
                m->pos[0] - gCamera->pos[0]
            );
            for (s32 i = 0; i != 4; ++i) {
                s16 a = angle + ((i * 0x4000) - 0x6000);
                sRadarArrowVtx[i].v.ob[0] = OMM_RENDER_RADAR_X + OMM_RENDER_RADAR_RADIUS * coss(angle + 0x4000) + OMM_RENDER_RADAR_ARROW_SIZE * OMM_INVSQRT2 * coss(a);
                sRadarArrowVtx[i].v.ob[1] = OMM_RENDER_RADAR_Y + OMM_RENDER_RADAR_RADIUS * sins(angle + 0x4000) + OMM_RENDER_RADAR_ARROW_SIZE * OMM_INVSQRT2 * sins(a);
            }
            
            // Radar arrow
            gDPSetTexturePersp(gDisplayListHead++, G_TP_NONE);
            gDPSetRenderMode(gDisplayListHead++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
            gDPSetCombineLERP(gDisplayListHead++, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0);
            gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, 0xFF);
            gDPSetTextureFilter(gDisplayListHead++, G_TF_POINT);
            gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
            gDPPipeSync(gDisplayListHead++);
            gDPLoadTextureBlock(gDisplayListHead++, OMM_TEXTURE_HUD_ARROW, G_IM_FMT_RGBA, G_IM_SIZ_16b, 128, 128, 0, G_TX_CLAMP, G_TX_CLAMP, 0, 0, 0, 0);
            gSPVertex(gDisplayListHead++, sRadarArrowVtx, 4, 0);
            gSP2Triangles(gDisplayListHead++, 0, 1, 2, 0, 0, 2, 3, 0);
            gDPSetTexturePersp(gDisplayListHead++, G_TP_PERSP);
            gDPSetRenderMode(gDisplayListHead++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
            gDPSetCombineLERP(gDisplayListHead++, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE);
            gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, 0xFF);
            gDPSetTextureFilter(gDisplayListHead++, G_TF_BILERP);
            gSPTexture(gDisplayListHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
        }
    }
}

//
// HUD
//

void omm_render_hud() {
    struct MarioState *m = gMarioState;

    // Cake end screen
    // Render the custom cake end screen on top of
    // the regular cake end screen, if necessary
    if (omm_is_ending_cake_screen()) {
        omm_sparkly_ending_render_screen();
        return;
    }

    // Version number
    // Only on title screen/main menu
    if (omm_is_main_menu()) {
        const u8 *textVersion = omm_text_convert("OMM V" STRINGIFY(OMM_VERSION), false);
        s32 textVersionWidth = omm_render_get_string_width(OMM_RENDER_FONT_MENU, textVersion);
        omm_render_string_left_align(textVersion, GFX_DIMENSIONS_FROM_RIGHT_EDGE(textVersionWidth + 2), 3, 0xFF, 0xFF, 0xFF, 0xFF, true);
        return;
    }

    // HUD
    if (gHudDisplay.flags && configHUD) {
        omm_render_create_dl_ortho_matrix();
        omm_render_hud_cannon_reticle();
        omm_render_dark_mode();
        omm_render_vibe();
        omm_render_freeze();
        omm_render_you_got_a_star();
        omm_render_hud_power_meter(m);
        omm_render_hud_o2_meter(m);
        omm_render_hud_values(m);
        omm_render_hud_camera(m);
        omm_render_hud_timer();
        omm_render_hud_red_coins_radar(m);
    }
}

//
// Pause
//

static void omm_render_pause_init() {
    sOmmDialogTextAlpha = 0;
    level_set_transition(-1, 0);
    play_sound(SOUND_MENU_PAUSE | 0xFF00, gGlobalSoundArgs);
    if (gCurrCourseNum != COURSE_NONE) {
        sOmmDialogBoxState = 1;
        sOmmPauseCourseScroll.idx = 0;
    } else {
        sOmmDialogBoxState = 2;
        if (omm_level_get_course(gLastCompletedCourseNum) != COURSE_NONE) {
            sOmmPauseCastleScrollV.idx = -1;
        }
    }
}

//
// Collectibles
//

inline static void omm_render_collectible(s16 x, s16 y, const void *texture, bool collected) {
    omm_render_glyph_hud(x, y, 0xFF * collected, 0xFF * collected, 0xFF * collected, sOmmDialogTextAlpha / (2 - collected), texture, false);
}

static void omm_render_collectibles() {
#if OMM_GAME_IS_R96A

    // Luigi keys
    if ((gHudDisplay.flags & HUD_DISPLAY_FLAG_KEYS) && gHudDisplay.keys > 0 && gHudDisplay.keys < NUM_KEYS) {
        for (s32 i = 0; i != NUM_KEYS; ++i) {
            omm_render_collectible(
                OMM_RENDER_COLLECTIBLE_X + OMM_RENDER_NUMBER_OFFSET_X * i,
                OMM_RENDER_COLLECTIBLE_Y,
                "textures/segment2/boo_key.rgba16",
                save_file_taken_key(gCurrSaveFileNum - 1, i)
            );
        }
    }

    // Wario coins
    if ((gHudDisplay.flags & HUD_DISPLAY_FLAG_KEYS) && gHudDisplay.wario_coins > 0 && gHudDisplay.wario_coins < NUM_WARIO_COINS) {
        for (s32 i = 0; i != NUM_WARIO_COINS; ++i) {
            omm_render_collectible(
                OMM_RENDER_COLLECTIBLE_X + OMM_RENDER_STAR_OFFSET_X * i,
                OMM_RENDER_COLLECTIBLE_Y + OMM_RENDER_OFFSET_Y,
                "textures/segment2/wario_coin.rgba16",
                save_file_taken_wario_coin(gCurrSaveFileNum - 1, i)
            );
        }
    }
#endif
}

//
// Pause (Course)
//

static void omm_render_pause_course() {
    s32 actNum = gCurrActNum;
    s32 levelNum = OMM_BOWSER_IN_THE_LEVEL(gCurrLevelNum);
    s32 courseNum = omm_level_get_course(levelNum);
    s32 displayAct = OMM_STARS_CLASSIC && COURSE_IS_MAIN_COURSE(courseNum);
    s16 y = OMM_RENDER_PAUSE_COURSE_INFO_Y - OMM_RENDER_PAUSE_COURSE_OFFSET_Y * !displayAct;

    // Course number and name
    u8 *textCourse = omm_text_convert(OMM_TEXT_COURSE, false);
    if (COURSE_IS_MAIN_COURSE(courseNum)) {
        s32 length = omm_text_length(textCourse);
        textCourse[length + 0] = DIALOG_CHAR_SPACE;
        if (courseNum >= 10) {
            textCourse[length + 1] = courseNum / 10;
            textCourse[length + 2] = courseNum % 10;
            textCourse[length + 3] = 0xFF;
        } else {
            textCourse[length + 1] = courseNum;
            textCourse[length + 2] = 0xFF;
        }
    }
    omm_render_string_right_align(textCourse, OMM_RENDER_PAUSE_COURSE_RIGHT_ALIGN_X, y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, true);
    omm_render_string_left_align(omm_level_get_name(levelNum, false, false), OMM_RENDER_PAUSE_COURSE_LEFT_ALIGN_X, y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, false);
    y -= OMM_RENDER_PAUSE_COURSE_OFFSET_Y;

    // Act number and name
    if (displayAct) {
        u8 *textAct = omm_text_convert(OMM_TEXT_ACT, false);
        s32 length = omm_text_length(textAct);
        textAct[length + 0] = DIALOG_CHAR_SPACE;
        textAct[length + 1] = actNum;
        textAct[length + 2] = 0xFF;
        omm_render_string_right_align(textAct, OMM_RENDER_PAUSE_COURSE_RIGHT_ALIGN_X, y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, true);
        omm_render_string_left_align(omm_level_get_act_name(levelNum, actNum, false, false), OMM_RENDER_PAUSE_COURSE_LEFT_ALIGN_X, y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, false);
        y -= OMM_RENDER_PAUSE_COURSE_OFFSET_Y;
    }

    // Course stars and coins
    if (omm_stars_get_bits_total(levelNum) != 0) {
            
        // Stars
        u8 *textStars = omm_text_convert(OMM_TEXT_MY_STARS, false);
        omm_render_string_right_align(textStars, OMM_RENDER_PAUSE_COURSE_RIGHT_ALIGN_X, y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, true);
        omm_render_hud_stars(OMM_RENDER_PAUSE_COURSE_LEFT_ALIGN_X, y - ((OMM_RENDER_GLYPH_SIZE - 8) / 2), sOmmDialogTextAlpha, levelNum, true, false);
        y -= OMM_RENDER_PAUSE_COURSE_OFFSET_Y;

        // Coin score
        u8 *textScore = omm_text_convert(OMM_TEXT_MY_SCORE, false);
        omm_render_string_right_align(textScore, OMM_RENDER_PAUSE_COURSE_RIGHT_ALIGN_X, y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, true);
        omm_render_hud_coins(OMM_RENDER_PAUSE_COURSE_LEFT_ALIGN_X, y - ((OMM_RENDER_GLYPH_SIZE - 8) / 2), sOmmDialogTextAlpha, save_file_get_course_coin_score(gCurrSaveFileNum - 1, courseNum - 1), false);
    }

    // Red coins
    s32 numRedCoins = omm_level_get_num_red_coins(gCurrLevelNum, gCurrAreaIndex);
    if (numRedCoins > 0 && obj_get_red_coin_star()) {
        s32 redCoinsCollected = numRedCoins - obj_get_count_with_behavior(bhvRedCoin);
        s32 x = OMM_RENDER_RED_COINS_X;
        omm_render_texrect(
            x, OMM_RENDER_RED_COINS_Y, OMM_RENDER_GLYPH_SIZE, OMM_RENDER_GLYPH_SIZE,
            G_IM_FMT_IA, G_IM_SIZ_16b, 32, 32, 
            0xFF, 0x00, 0x00, 0xFF,
            (const void *) (OMM_ARRAY_OF(const char *) {
                OMM_ASSET_COIN_0,
                OMM_ASSET_COIN_1,
                OMM_ASSET_COIN_2,
                OMM_ASSET_COIN_3,
            })[(gGlobalTimer >> 1) & 3]
        );
        x += 2 * OMM_RENDER_NUMBER_OFFSET_X;
        omm_render_number_hud(redCoinsCollected, 2, x, OMM_RENDER_RED_COINS_Y, sOmmDialogTextAlpha, true, false);
        x += OMM_RENDER_NUMBER_OFFSET_X + OMM_RENDER_GLYPH_SIZE;
        omm_render_glyph_hud(x, OMM_RENDER_RED_COINS_Y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, OMM_TEXTURE_HUD_SLASH, false);
        x += OMM_RENDER_GLYPH_SIZE;
        omm_render_number_hud(numRedCoins, 2, x, OMM_RENDER_RED_COINS_Y, sOmmDialogTextAlpha, true, false);
    }
    
    // Collectibles
    omm_render_collectibles();

    // Sparkly Stars timer
    if (omm_sparkly_is_enabled()) {
        s32 mode = omm_sparkly_get_current_mode();
        omm_render_sparkly_timer(
            OMM_RENDER_SPARKLY_TIMER_X(OMM_RENDER_SPARKLY_TIMER_GLYPH_SIZE),
            OMM_RENDER_SPARKLY_TIMER_Y,
            OMM_RENDER_SPARKLY_TIMER_GLYPH_SIZE,
            sOmmDialogTextAlpha,
            mode,
            omm_sparkly_get_timer(mode),
           !omm_sparkly_is_grand_star_collected(mode),
            true
        );
    }

    // Course options
    omm_render_string_left_align(omm_text_convert(OMM_TEXT_CONTINUE, false),         OMM_RENDER_PAUSE_COURSE_LEFT_ALIGN_X, OMM_RENDER_PAUSE_COURSE_OPTIONS_Y - 0 * OMM_RENDER_PAUSE_COURSE_OFFSET_Y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, false);
    omm_render_string_left_align(omm_text_convert(OMM_TEXT_RESTART_LEVEL, false),    OMM_RENDER_PAUSE_COURSE_LEFT_ALIGN_X, OMM_RENDER_PAUSE_COURSE_OPTIONS_Y - 1 * OMM_RENDER_PAUSE_COURSE_OFFSET_Y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, false);
    omm_render_string_left_align(omm_text_convert(OMM_TEXT_EXIT_LEVEL, false),       OMM_RENDER_PAUSE_COURSE_LEFT_ALIGN_X, OMM_RENDER_PAUSE_COURSE_OPTIONS_Y - 2 * OMM_RENDER_PAUSE_COURSE_OFFSET_Y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, false);
    omm_render_string_left_align(omm_text_convert(OMM_TEXT_RETURN_TO_CASTLE, false), OMM_RENDER_PAUSE_COURSE_LEFT_ALIGN_X, OMM_RENDER_PAUSE_COURSE_OPTIONS_Y - 3 * OMM_RENDER_PAUSE_COURSE_OFFSET_Y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, false);
    omm_render_update_scroll(&sOmmPauseCourseScroll, 4, -gPlayer1Controller->stickY);

    // White triangle
    static const Vtx sOmmHudWhiteTriangleVertices[] = {
        { { {  0,  0, 0 }, 0, { 0, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
        { { { -6, -6, 0 }, 0, { 0, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
        { { { -6, +6, 0 }, 0, { 0, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
    };
    create_dl_translation_matrix(MENU_MTX_PUSH, OMM_RENDER_PAUSE_COURSE_LEFT_ALIGN_X - 6, OMM_RENDER_PAUSE_COURSE_OPTIONS_Y - sOmmPauseCourseScroll.idx * OMM_RENDER_PAUSE_COURSE_OFFSET_Y + 4, 0);
    gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha);
    gSPClearGeometryMode(gDisplayListHead++, G_LIGHTING);
    gDPSetCombineLERP(gDisplayListHead++, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0);
    gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gSPVertex(gDisplayListHead++, sOmmHudWhiteTriangleVertices, 3, 0);
    gSP1Triangle(gDisplayListHead++, 2, 1, 0, 0);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    // Selected option
    if (gPlayer1Controller->buttonPressed & (A_BUTTON | START_BUTTON)) {
        level_set_transition(0, 0);
        play_sound(SOUND_MENU_PAUSE_2, gGlobalSoundArgs);
        sOmmDialogBoxState = 0;
        gMenuMode = -1;
        switch (sOmmPauseCourseScroll.idx) {
            case 0: { // Continue
                raise_background_noise(1);
                gCameraMovementFlags &= ~CAM_MOVE_PAUSE_SCREEN;
                set_play_mode(0);
            } break;

            case 1: { // Restart Level
                omm_restart_level();
            } break;
            
            case 2: { // Exit Level
                omm_exit_level();
            } break;

            case 3: { // Return to Castle (same as Exit Course)
                omm_return_to_castle(true, true);
            } break;
        }
    }
}

//
// Pause (Castle)
//

static void omm_render_pause_castle() {
    static const s32 sLevelList[] = {
        LEVEL_BOB,
        LEVEL_WF,
        LEVEL_JRB,
        LEVEL_CCM,
        LEVEL_BBH,
        LEVEL_HMC,
        LEVEL_LLL,
        LEVEL_SSL,
        LEVEL_DDD,
        LEVEL_SL,
        LEVEL_WDW,
        LEVEL_TTM,
        LEVEL_THI,
        LEVEL_TTC,
        LEVEL_RR,
        LEVEL_BITDW,
        LEVEL_BITFS,
        LEVEL_BITS,
        LEVEL_TOTWC,
        LEVEL_COTMC,
        LEVEL_VCUTM,
        LEVEL_PSS,
        LEVEL_SA,
        LEVEL_WMOTR,
        LEVEL_CASTLE
    };
    static const s32 sLevelCount = OMM_ARRAY_SIZE(sLevelList);

#if OMM_CODE_TIME_TRIALS
    gDialogTextAlpha = sOmmDialogTextAlpha;
    omm_render_create_dl_ortho_matrix();
    if (!time_trials_render_time_table(&gDialogLineIndex)) {
#endif

    // Init scrollV with the index of the last level visited
    if (sOmmPauseCastleScrollV.idx == -1) {
        for (s32 i = 0; i != sLevelCount; ++i) {
            if (gLastCompletedCourseNum == sLevelList[i]) {
                sOmmPauseCastleScrollV.idx = i;
                gLastCompletedCourseNum = LEVEL_CASTLE;
                break;
            }
        }
    }

    // Colorful centered PAUSE text
    u8 *textPause = omm_text_convert(OMM_TEXT_PAUSE, false);
    omm_render_string_hud_centered(textPause, SCREEN_HEIGHT / 2, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, false);

    // Black box
    static const Vtx sOmmHudBlackBoxVertices[] = {
        { { { -OMM_RENDER_PAUSE_CASTLE_BOX_W / 2, -OMM_RENDER_PAUSE_CASTLE_BOX_H / 2, 0 }, 0, { 0, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
        { { { -OMM_RENDER_PAUSE_CASTLE_BOX_W / 2, +OMM_RENDER_PAUSE_CASTLE_BOX_H / 2, 0 }, 0, { 0, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
        { { { +OMM_RENDER_PAUSE_CASTLE_BOX_W / 2, -OMM_RENDER_PAUSE_CASTLE_BOX_H / 2, 0 }, 0, { 0, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
        { { { +OMM_RENDER_PAUSE_CASTLE_BOX_W / 2, +OMM_RENDER_PAUSE_CASTLE_BOX_H / 2, 0 }, 0, { 0, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
    };
    create_dl_translation_matrix(MENU_MTX_PUSH, SCREEN_WIDTH / 2, OMM_RENDER_PAUSE_CASTLE_BOX_Y, 0);
    gDPSetEnvColor(gDisplayListHead++, 0x00, 0x00, 0x00, sOmmDialogTextAlpha / 2);
    gSPClearGeometryMode(gDisplayListHead++, G_LIGHTING);
    gDPSetCombineLERP(gDisplayListHead++, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0);
    gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gSPVertex(gDisplayListHead++, sOmmHudBlackBoxVertices, 4, 0);
    gSP2Triangles(gDisplayListHead++, 2, 1, 0, 0, 1, 2, 3, 0);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    // White triangles
    static const Vtx sOmmHudWhiteTriangleVertices[] = {
        { { {  0, -8, 0 }, 0, { 0, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
        { { { -8,  0, 0 }, 0, { 0, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
        { { { +8,  0, 0 }, 0, { 0, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
        { { {  0, +8, 0 }, 0, { 0, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
    };
    create_dl_translation_matrix(MENU_MTX_PUSH, SCREEN_WIDTH / 2, OMM_RENDER_PAUSE_CASTLE_BOX_TRIANGLE_DOWN_Y, 0);
    gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha);
    gSPClearGeometryMode(gDisplayListHead++, G_LIGHTING);
    gDPSetCombineLERP(gDisplayListHead++, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0);
    gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gSPVertex(gDisplayListHead++, sOmmHudWhiteTriangleVertices + 0, 3, 0);
    gSP1Triangle(gDisplayListHead++, 2, 1, 0, 0);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
    create_dl_translation_matrix(MENU_MTX_PUSH, SCREEN_WIDTH / 2, OMM_RENDER_PAUSE_CASTLE_BOX_TRIANGLE_UP_Y, 0);
    gDPSetEnvColor(gDisplayListHead++, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha);
    gSPClearGeometryMode(gDisplayListHead++, G_LIGHTING);
    gDPSetCombineLERP(gDisplayListHead++, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0);
    gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gSPVertex(gDisplayListHead++, sOmmHudWhiteTriangleVertices + 1, 3, 0);
    gSP1Triangle(gDisplayListHead++, 0, 1, 2, 0);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    // Update scroll index
    omm_render_update_scroll(&sOmmPauseCastleScrollV, sLevelCount + omm_sparkly_is_enabled(), -gPlayer1Controller->stickY);

    // Display course
    if (sOmmPauseCastleScrollV.idx != sLevelCount) {
        s32 levelNum = sLevelList[sOmmPauseCastleScrollV.idx];
        s32 courseIndex = omm_level_get_course(levelNum) - 1;
        
        // Course name
        const u8 *textCourseName = ((courseIndex != -1) ? omm_level_get_name(levelNum, false, true) : omm_text_convert(OMM_TEXT_LEVEL_SECRET_STARS, false));
        omm_render_string_centered(textCourseName, OMM_RENDER_PAUSE_CASTLE_BOX_LINE_1_Y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, false);

        // Course stars
        if (courseIndex == -1) {
            omm_render_counter_hud(0xFA, save_file_get_course_star_count(gCurrSaveFileNum - 1, -1), OMM_RENDER_PAUSE_CASTLE_BOX_LINE_2_Y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha);
        } else {
            s16 x = (SCREEN_WIDTH - 92) / 2;
            u8 starBits = omm_stars_get_bits_total(levelNum);
            u8 starFlags = save_file_get_star_flags(gCurrSaveFileNum - 1, courseIndex);
            u8 starGlyph[] = { DIALOG_CHAR_STAR_FILLED, 0xFF };
            for (s32 i = 0; i != 7; ++i) {
                if (starBits & (1 << i)) {
                    if (starFlags & (1 << i)) {
                        omm_render_string_left_align(starGlyph, x, OMM_RENDER_PAUSE_CASTLE_BOX_LINE_2_Y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, false);
                    } else {
                        omm_render_string_left_align(starGlyph, x, OMM_RENDER_PAUSE_CASTLE_BOX_LINE_2_Y, 0x80, 0x80, 0x80, sOmmDialogTextAlpha, false);
                    }
                    x += 14;
                }
            }
        }

        // Course score
        if (courseIndex != -1) {
            omm_render_counter_hud(0xF9, save_file_get_course_coin_score(gCurrSaveFileNum - 1, courseIndex), OMM_RENDER_PAUSE_CASTLE_BOX_LINE_3_Y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha);
        }
    }

    // Display Sparkly Stars
    else {
        omm_render_update_scroll(&sOmmPauseCastleScrollH, 30, gPlayer1Controller->stickX);
        s32 mode = omm_sparkly_get_current_mode();
        u8 textR = OMM_SPARKLY_STAR_HUD_COLOR[mode][0];
        u8 textG = OMM_SPARKLY_STAR_HUD_COLOR[mode][1];
        u8 textB = OMM_SPARKLY_STAR_HUD_COLOR[mode][2];

        // Title
        u8 *textSparklyStars = omm_text_convert(OMM_SPARKLY_STARS_TEXT[mode], false);
        omm_render_string_centered(textSparklyStars, OMM_RENDER_PAUSE_CASTLE_BOX_LINE_1_Y, textR, textG, textB, sOmmDialogTextAlpha, false);

        // Star count
        omm_render_counter_hud(0xFA, omm_sparkly_get_star_count(mode), OMM_RENDER_PAUSE_CASTLE_BOX_LINE_2_Y, textR, textG, textB, sOmmDialogTextAlpha);

        // Level display
        u8 *textLevelName = omm_sparkly_get_star_level_name(mode, sOmmPauseCastleScrollH.idx);
        if ((sOmmPauseCastleScrollH.idx == 29 && omm_sparkly_is_grand_star_collected(mode)) || omm_sparkly_is_star_collected(mode, sOmmPauseCastleScrollH.idx)) {
            omm_render_string_centered(textLevelName, OMM_RENDER_PAUSE_CASTLE_BOX_LINE_3_Y, textR, textG, textB, sOmmDialogTextAlpha, false);
        } else {
            omm_render_string_centered(textLevelName, OMM_RENDER_PAUSE_CASTLE_BOX_LINE_3_Y, textR / 2, textG / 2, textB / 2, sOmmDialogTextAlpha, false);
        }

        // Sparkly Stars timer
        omm_render_sparkly_timer(
            OMM_RENDER_SPARKLY_TIMER_X(OMM_RENDER_SPARKLY_TIMER_GLYPH_SIZE),
            OMM_RENDER_SPARKLY_TIMER_Y,
            OMM_RENDER_SPARKLY_TIMER_GLYPH_SIZE,
            sOmmDialogTextAlpha,
            mode,
            omm_sparkly_get_timer(mode),
           !omm_sparkly_is_grand_star_collected(mode),
            true
        );
    }

#if OMM_CODE_TIME_TRIALS
    }
#endif

    // Collectibles
    omm_render_collectibles();

#if OMM_GAME_IS_SM74
    // Display "Super Mario 74" and current mode
    const u8 *textSm74 = omm_text_convert(OMM_TEXT_SM74_SUPER_MARIO_74, false);
    omm_render_string_centered(textSm74, SCREEN_HEIGHT - 40, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, true);
    switch (sm74_mode__omm_render_pause_castle) {
        case 1: {
            const u8 *textMode = omm_text_convert(OMM_TEXT_SM74_NORMAL_EDITION, false);
            omm_render_string_centered(textMode, SCREEN_HEIGHT - 54, 0x40, 0xFF, 0x40, sOmmDialogTextAlpha, true);
        } break;
        case 2: {
            const u8 *textMode = omm_text_convert(OMM_TEXT_SM74_EXTREME_EDITION, false);
            omm_render_string_centered(textMode, SCREEN_HEIGHT - 54, 0xFF, 0x40, 0x20, sOmmDialogTextAlpha, true);
        } break;
    }
#endif

    // Close the pause menu if A or Start is pressed
    if (gPlayer1Controller->buttonPressed & (A_BUTTON | START_BUTTON)) {
        level_set_transition(0, 0);
        play_sound(SOUND_MENU_PAUSE_2, gGlobalSoundArgs);
        sOmmDialogBoxState = 0;
        gMenuMode = -1;

        // Unpause the game
        raise_background_noise(1);
        gCameraMovementFlags &= ~CAM_MOVE_PAUSE_SCREEN;
        set_play_mode(0);
    }
}

//
// Pause (render)
//

s32 omm_render_pause() {
    static u8 sOptMenuOpen = 0;
    omm_render_create_dl_ortho_matrix();
    shade_screen();
    if (optmenu_open) {
        optmenu_draw();
        sOptMenuOpen = true;
    } else {
        if (sOptMenuOpen) {
            save_file_do_save(gCurrSaveFileNum - 1);
            sOptMenuOpen = false;
        }
        switch (sOmmDialogBoxState) {
            case 0: omm_render_pause_init(); break;
            case 1: omm_render_pause_course(); break;
            case 2: omm_render_pause_castle(); break;
        }
    }
    optmenu_check_buttons();
    optmenu_draw_prompt();
    sOmmDialogTextAlpha = omm_min_s(sOmmDialogTextAlpha + 0x20, 0xFF);
    return 0;
}

//
// Course complete
//

s32 omm_render_course_complete() {
    s32 actNum = gLastCompletedStarNum;
    s32 levelNum = OMM_BOWSER_IN_THE_LEVEL(gLastCompletedCourseNum);
    s32 courseNum = omm_level_get_course(levelNum);
    s32 displayAct = COURSE_IS_MAIN_COURSE(courseNum);
    s16 y = ((SCREEN_HEIGHT - ((16 * (3 + (displayAct ? 1 : 0))) - 8)) / 2);
    f32 t = ((1.f + sins(gGlobalTimer * 0x1000)) / 2.f);

    // Init
    if (sOmmDialogBoxState == 0) {
        sOmmCourseCompleteCoins = -1;
        sOmmCourseCompleteTimer = -1;
        sOmmDialogTextAlpha = 0;
        sOmmDialogBoxState = 1;
    }

    // Update coins counter
    if (sOmmCourseCompleteCoins < gHudDisplay.coins) {
        sOmmCourseCompleteCoins = omm_min_s(sOmmCourseCompleteCoins + 1, gHudDisplay.coins);
        play_sound(SOUND_MENU_YOSHI_GAIN_LIVES, gGlobalSoundArgs);
        if (sOmmCourseCompleteCoins == gHudDisplay.coins) {
            sOmmCourseCompleteTimer = 0;
            audio_play_star_fanfare();
            if (gGotFileCoinHiScore) {
                play_sound(SOUND_MENU_MARIO_CASTLE_WARP2, gGlobalSoundArgs);
            }
        }
    }

    // Shade screen
    omm_render_create_dl_ortho_matrix();
    create_dl_translation_matrix(MENU_MTX_PUSH, GFX_DIMENSIONS_FROM_LEFT_EDGE(0), SCREEN_HEIGHT, 0);
    create_dl_scale_matrix(MENU_MTX_NOPUSH, (GFX_DIMENSIONS_ASPECT_RATIO * SCREEN_HEIGHT) / 130.f, SCREEN_HEIGHT / 80.f, 1.f);
    gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, sOmmDialogTextAlpha / 2);
    gSPDisplayList(gDisplayListHead++, dl_draw_text_bg_box);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    // Coin score
    u8 *textScore = omm_text_convert(OMM_TEXT_MY_SCORE, false);
    omm_render_string_right_align(textScore, OMM_RENDER_COURSE_COMPLETE_RIGHT_ALIGN_X, y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, true);
    omm_render_hud_coins(OMM_RENDER_COURSE_COMPLETE_LEFT_ALIGN_X, y - ((OMM_RENDER_GLYPH_SIZE - 8) / 2), sOmmDialogTextAlpha, sOmmCourseCompleteCoins, false);
    if (sOmmCourseCompleteCoins == gHudDisplay.coins && gGotFileCoinHiScore) {
        u8 intensity = (0xFF * t);
        u8 *textHiScore = omm_text_convert(OMM_TEXT_COURSE_COMPLETE_HI_SCORE, false);
        omm_render_string_left_align(textHiScore, OMM_RENDER_COURSE_COMPLETE_HI_SCORE_X, y, intensity, intensity, intensity, sOmmDialogTextAlpha, true);
    }
    y += OMM_RENDER_COURSE_COMPLETE_OFFSET_Y;

    // Course stars
    u8 *textStars = omm_text_convert(OMM_TEXT_MY_STARS, false);
    omm_render_string_right_align(textStars, OMM_RENDER_COURSE_COMPLETE_RIGHT_ALIGN_X, y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, true);
    omm_render_hud_stars(OMM_RENDER_COURSE_COMPLETE_LEFT_ALIGN_X, y - ((OMM_RENDER_GLYPH_SIZE - 8) / 2), sOmmDialogTextAlpha, levelNum, true, false);
    y += OMM_RENDER_COURSE_COMPLETE_OFFSET_Y;

    // Act number and name
    if (displayAct) {
        u8 *textAct = omm_text_convert(OMM_TEXT_ACT, false);
        s32 length = omm_text_length(textAct);
        textAct[length + 0] = DIALOG_CHAR_SPACE;
        textAct[length + 1] = actNum;
        textAct[length + 2] = 0xFF;
        omm_render_string_right_align(textAct, OMM_RENDER_COURSE_COMPLETE_RIGHT_ALIGN_X, y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, true);
        omm_render_string_left_align(omm_level_get_act_name(levelNum, actNum, false, false), OMM_RENDER_COURSE_COMPLETE_LEFT_ALIGN_X, y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, false);
        y += OMM_RENDER_COURSE_COMPLETE_OFFSET_Y;
    }

    // Course number and name
    u8 *textCourse = omm_text_convert(OMM_TEXT_COURSE, false);
    if (COURSE_IS_MAIN_COURSE(courseNum)) {
        s32 length = omm_text_length(textCourse);
        textCourse[length + 0] = DIALOG_CHAR_SPACE;
        if (courseNum >= 10) {
            textCourse[length + 1] = courseNum / 10;
            textCourse[length + 2] = courseNum % 10;
            textCourse[length + 3] = 0xFF;
        } else {
            textCourse[length + 1] = courseNum;
            textCourse[length + 2] = 0xFF;
        }
    }
    omm_render_string_right_align(textCourse, OMM_RENDER_COURSE_COMPLETE_RIGHT_ALIGN_X, y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, true);
    omm_render_string_left_align(omm_level_get_name(levelNum, false, false), OMM_RENDER_COURSE_COMPLETE_LEFT_ALIGN_X, y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, false);
    y += OMM_RENDER_COURSE_COMPLETE_OFFSET_Y;

    // Message
    y = (((SCREEN_HEIGHT + (y - 8)) / 2) - 8);
    u8 intensity = 0x80 + (0x7F * t);
    u8 *textMessage = omm_text_convert(((gLastCompletedCourseNum == LEVEL_BITDW || gLastCompletedCourseNum == LEVEL_BITFS) ? OMM_TEXT_COURSE_COMPLETE_CONGRATULATIONS : OMM_TEXT_COURSE_COMPLETE_GOT_A_STAR), false);
    omm_render_string_hud_centered(textMessage, y, intensity, intensity, intensity, sOmmDialogTextAlpha, false);

    // Wait for the coins to count up, then wait for 90 frames and Mario's
    // animation end to fade out and close the course complete screen
    if (sOmmCourseCompleteTimer != -1 && sOmmCourseCompleteTimer++ >= 90 && obj_anim_is_at_end(gMarioObject)) {
        sOmmDialogTextAlpha = omm_max_s(sOmmDialogTextAlpha - 0x10, 0x00);
        if (sOmmDialogTextAlpha == 0) {
            level_set_transition(0, 0);
            gSaveOptSelectIndex = 1; // Save and continue
            sOmmDialogBoxState = 0;
            gMenuMode = -1;
        }
    } else {
        sOmmDialogTextAlpha = omm_min_s(sOmmDialogTextAlpha + 0x40, 0xFF);
    }
    return 0;
}

//
// Menus and dialogs
//

enum OmmDialogBoxState {
    OMM_DIALOG_STATE_OPENING,
    OMM_DIALOG_STATE_VERTICAL,
    OMM_DIALOG_STATE_HORIZONTAL,
    OMM_DIALOG_STATE_CLOSING
};

enum OmmDialogBoxPageState {
    OMM_DIALOG_PAGE_STATE_NONE,
    OMM_DIALOG_PAGE_STATE_SCROLL,
    OMM_DIALOG_PAGE_STATE_END
};

static void omm_handle_dialog_text_and_pages(s8 colorMode, struct DialogEntry *dialog, s8 lowerBound) {
    u8 *str = omm_text_replace_names((u8 *) dialog->str, false);
    s8 linesPerBox = dialog->linesPerBox;
    s8 totalLines = 1 + linesPerBox * (gDialogState == OMM_DIALOG_STATE_HORIZONTAL ? 2 : 1);
    s8 lineNum = 1;
    s16 linePos = 0;
    s16 strIdx = gDialogPageStart;
    s8 xMatrix = 1;

    // Init text drawing
    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    if (gDialogState == OMM_DIALOG_STATE_HORIZONTAL) {
        create_dl_translation_matrix(MENU_MTX_NOPUSH, 0, (f32) gDialogScrollOffsetY, 0);
    }
    create_dl_translation_matrix(MENU_MTX_PUSH, 0, -14, 0);

    // Draw the current page
    s8 pageState = OMM_DIALOG_PAGE_STATE_NONE;
    for (; pageState == OMM_DIALOG_PAGE_STATE_NONE; ++strIdx) {
        change_and_flash_dialog_text_color_lines(colorMode, lineNum);
        switch (str[strIdx]) {

            // 0xFF terminator, end of text
            case DIALOG_CHAR_TERMINATOR: {
                pageState = OMM_DIALOG_PAGE_STATE_END;
                gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
            } break;

            // New-line
            case DIALOG_CHAR_NEWLINE: {
                lineNum++;
                handle_dialog_scroll_page_state(lineNum, totalLines, &pageState, &xMatrix, &linePos);
            } break;

            // Blank space
            case DIALOG_CHAR_SPACE: {
                xMatrix++;
                linePos++;
            } break;

            // Tabulation
            case DIALOG_CHAR_SLASH: {
                xMatrix += 2;
                linePos += 2;
            } break;

            // "The"
            case DIALOG_CHAR_MULTI_THE: {
                render_multi_text_string_lines(0, lineNum, &linePos, linesPerBox, xMatrix, lowerBound);
                xMatrix = 1;
            } break;

            // "You"
            case DIALOG_CHAR_MULTI_YOU: {
                render_multi_text_string_lines(1, lineNum, &linePos, linesPerBox, xMatrix, lowerBound);
                xMatrix = 1;
            } break;

            // Star count
            case DIALOG_CHAR_STAR_COUNT: {
                render_star_count_dialog_text(&xMatrix, &linePos);
            } break;

            // Common characters
            default: {
                if (lineNum >= lowerBound && lineNum <= lowerBound + linesPerBox) {
                    if (linePos != 0 || xMatrix != 1) {
                        create_dl_translation_matrix(MENU_MTX_NOPUSH, (f32) gDialogCharWidths[DIALOG_CHAR_SPACE] * (xMatrix - 1), 0, 0);
                    }
                    render_generic_char(str[strIdx]);
                    create_dl_translation_matrix(MENU_MTX_NOPUSH, (f32) gDialogCharWidths[str[strIdx]], 0, 0);
                    xMatrix = 1;
                    linePos++;
                }
            } break;
        }
    }
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);

    // Update line num and cursor pos
    gLastDialogLineNum = lineNum;
    if (gDialogState == OMM_DIALOG_STATE_VERTICAL) {
        if (pageState == OMM_DIALOG_PAGE_STATE_END) {
            gDialogPageStartNext = -1;
        } else {
            gDialogPageStartNext = strIdx;
        }
    }
}

static void omm_render_dialog_entry() {
#if OMM_GAME_IS_SM74
    struct DialogEntry *dialog = omm_get_dialog_entry((sm74_mode__omm_render_dialog_entry == 2 ?
                                                      (void **) seg2_dialog_table_EE :
                                                      (void **) seg2_dialog_table), gDialogID);
#elif !OMM_GAME_IS_R96A
    struct DialogEntry *dialog = omm_get_dialog_entry((void **) seg2_dialog_table, gDialogID);
#else
    struct DialogEntry *dialog = omm_get_dialog_entry(NULL, gDialogID);
#endif

    if (!dialog) {
        gDialogID = -1;
        return;
    }

    // Updating dialog box state
    s8 lowerBound;
    switch (gDialogState) {

        // Opening the dialog box
        case OMM_DIALOG_STATE_OPENING: {
            if (gDialogAngle == 90.f) {
                play_dialog_sound(gDialogID);
                play_sound(SOUND_MENU_MESSAGE_APPEAR, gGlobalSoundArgs);
            }
            if (gDialogType == 0) {
                gDialogAngle -= 7.5f;
                gDialogScale -= 1.5f;
            } else {
                gDialogAngle -= 10.f;
                gDialogScale -= 2.f;
            }
            if (gDialogAngle == 0.f) {
                gDialogState = OMM_DIALOG_STATE_VERTICAL;
                gDialogLineIndex = 1;
            }
            lowerBound = 1;
        } break;

        // Waiting for next page
        case OMM_DIALOG_STATE_VERTICAL: {
            gDialogAngle = 0.f;
            if (gPlayer1Controller->buttonPressed & (A_BUTTON | B_BUTTON)) {
                if (gDialogPageStartNext == -1) {
                    handle_special_dialog_text(gDialogID);
                    gDialogState = OMM_DIALOG_STATE_CLOSING;
                } else {
                    play_sound(SOUND_MENU_MESSAGE_NEXT_PAGE, gGlobalSoundArgs);
                    gDialogState = OMM_DIALOG_STATE_HORIZONTAL;
                }
            }
            lowerBound = 1;
        } break;

        // Scrolling the dialog box
        case OMM_DIALOG_STATE_HORIZONTAL: {
            gDialogScrollOffsetY += dialog->linesPerBox * 2;
            if (gDialogScrollOffsetY >= dialog->linesPerBox * 16) {
                gDialogPageStart = gDialogPageStartNext;
                gDialogState = OMM_DIALOG_STATE_VERTICAL;
                gDialogScrollOffsetY = 0;
            }
            lowerBound = (gDialogScrollOffsetY / 16) + 1;
        } break;

        // Closing the dialog box
        case OMM_DIALOG_STATE_CLOSING: {
            if (gDialogAngle == 20.f) {
                level_set_transition(0, 0);
                play_sound(SOUND_MENU_MESSAGE_DISAPPEAR, gGlobalSoundArgs);
                if (gDialogType == 1) {
                    trigger_cutscene_dialog(2);
                }
                gDialogResponse = gDialogLineIndex;
            }
            gDialogAngle = gDialogAngle + 10.f;
            gDialogScale = gDialogScale + 2.f;
            if (gDialogAngle == 90.f) {
                gDialogState = OMM_DIALOG_STATE_OPENING;
                gDialogID = -1;
                gDialogPageStart = 0;
                gDialogChoice = 0;
                gDialogPageStartNext = 0;
                gDialogResponse = 0;
            }
            lowerBound = 1;
        } break;
    }

    // Rendering
    render_dialog_box_type(dialog, dialog->linesPerBox);
    gDPSetScissor(gDisplayListHead++, G_SC_NON_INTERLACE, 0, omm_max_s(0, 240 - dialog->width), SCREEN_WIDTH, omm_max_s(0, 240 + ((dialog->linesPerBox * 80) / 5) - dialog->width));
    omm_handle_dialog_text_and_pages(0, dialog, lowerBound);
    if (gDialogPageStartNext == -1 && gDialogChoice == 1) {
        render_dialog_triangle_choice();
    }
    gDPSetScissor(gDisplayListHead++, G_SC_NON_INTERLACE, 2, 2, SCREEN_WIDTH - BORDER_HEIGHT / 2, SCREEN_HEIGHT - BORDER_HEIGHT / 2);
    if (gDialogPageStartNext != -1 && gDialogState == OMM_DIALOG_STATE_VERTICAL) {
        render_dialog_triangle_page(dialog->linesPerBox);
    }
}

void omm_render_dialog_entries() {
    sOmmDialogBoxState = 0;
    if (gDialogID != -1) {
        omm_render_create_dl_ortho_matrix();
        if (gDialogID == DIALOG_020) {
            print_peach_letter_message();
        } else {
            omm_render_dialog_entry();
        }
    }
}

OMM_ROUTINE_UPDATE(omm_menus_and_dialogs_update) {
    if (!omm_is_game_paused()) {
        sOmmDialogBoxState = 0;
    }
}

//
// Star select
//

#if OMM_GAME_IS_R96A
s32 lvl_star_select() {
    r96_play_menu_jingle(R96_EVENT_STAR_SELECT);
}
#endif

static void omm_render_star_select_background_rect(s32 x0, s32 y0, u8 r0, u8 g0, u8 b0, u8 a0, s32 x1, s32 y1, u8 r1, u8 g1, u8 b1, u8 a1, bool vrt) {
    Vtx *vtx = alloc_display_list(sizeof(Vtx) * 4);
    vtx[0] = (Vtx) { { { x0, y0, 0 }, 0, { 0, 0 }, { r0, g0, b0, (a0              * sOmmDialogTextAlpha) / 0xFF } } };
    vtx[1] = (Vtx) { { { x1, y0, 0 }, 0, { 0, 0 }, { r0, g0, b0, ((vrt ? a0 : a1) * sOmmDialogTextAlpha) / 0xFF } } };
    vtx[2] = (Vtx) { { { x1, y1, 0 }, 0, { 0, 0 }, { r1, g1, b1, (a1              * sOmmDialogTextAlpha) / 0xFF } } };
    vtx[3] = (Vtx) { { { x0, y1, 0 }, 0, { 0, 0 }, { r1, g1, b1, ((vrt ? a1 : a0) * sOmmDialogTextAlpha) / 0xFF } } };
    omm_render_create_dl_ortho_matrix();
    gDPPipeSync(gDisplayListHead++);
    gSPClearGeometryMode(gDisplayListHead++, G_LIGHTING);
    gDPSetCombineLERP(gDisplayListHead++, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE);;
    gSPVertex(gDisplayListHead++, vtx, 4, 0);
    gSP2Triangles(gDisplayListHead++, 0, 1, 2, 0x0, 0, 2, 3, 0x0);
    gSPSetGeometryMode(gDisplayListHead++, G_LIGHTING);
}

static s32 omm_render_star_select_get_string_width(const u8 *str64, s32 glyphSize, s32 spacing) {
    s32 width = -spacing;
    for (; *str64 != 0xFF; ++str64) {
        width += spacing;
        switch (*str64) {
            case OMM_RENDER_STAR_SELECT_SPACE:      width +=  8; break;
            case OMM_RENDER_STAR_SELECT_COMMA:      width +=  8; break;
            case OMM_RENDER_STAR_SELECT_HYPHEN:     width += 14; break;
            case OMM_RENDER_STAR_SELECT_APOSTROPHE: width +=  8; break;
            default:                                width += 12; break;
        }
    }
    return ((width * glyphSize) / 16);
}

static void omm_render_star_select_string(const u8 *str64, s32 x, s32 y, s32 glyphSize, s32 spacing, bool outline, bool shadow) {
    for (; *str64 != 0xFF; ++str64) {
        s32 dw = 0;
        s32 dx = 0;
        s32 dy = 0;
        const void *texture = NULL;
        switch (*str64) {
            case OMM_RENDER_STAR_SELECT_SPACE:      { dw =  8; dx =  0; dy =  0; texture = NULL; } break;
            case OMM_RENDER_STAR_SELECT_COMMA:      { dw =  8; dx = -2; dy = -6; texture = ((const u8 **) main_hud_lut)[GLYPH_APOSTROPHE]; } break;
            case OMM_RENDER_STAR_SELECT_HYPHEN:     { dw = 14; dx =  0; dy =  0; texture = OMM_TEXTURE_HUD_HYPHEN; } break;
            case OMM_RENDER_STAR_SELECT_APOSTROPHE: { dw =  8; dx = -2; dy = +6; texture = ((const u8 **) main_hud_lut)[GLYPH_APOSTROPHE]; } break;
            case OMM_RENDER_STAR_SELECT_J:          { dw = 12; dx =  0; dy =  0; texture = OMM_TEXTURE_HUD_J; } break;
            case OMM_RENDER_STAR_SELECT_Q:          { dw = 12; dx =  0; dy =  0; texture = OMM_TEXTURE_HUD_Q; } break;
            case OMM_RENDER_STAR_SELECT_V:          { dw = 12; dx =  0; dy =  0; texture = OMM_TEXTURE_HUD_V; } break;
            case OMM_RENDER_STAR_SELECT_X:          { dw = 12; dx =  0; dy =  0; texture = OMM_TEXTURE_HUD_X; } break;
            case OMM_RENDER_STAR_SELECT_Z:          { dw = 12; dx =  0; dy =  0; texture = OMM_TEXTURE_HUD_Z; } break;
            default:                                { dw = 12; dx =  0; dy =  0; texture = ((const u8 **) main_hud_lut)[*str64]; } break;
        }
        dw += spacing;
        dw = ((dw * glyphSize) / 16);
        dx = ((dx * glyphSize) / 16);
        dy = ((dy * glyphSize) / 16);
        if (texture) {
            if (shadow) {
            omm_render_glyph(x + dx + 3, y + dy - 3, glyphSize, glyphSize, 0x20, 0x20, 0x20, sOmmDialogTextAlpha / 2, texture, false);
            }
            if (outline) {
            omm_render_glyph(x + dx + 1, y + dy,     glyphSize, glyphSize, 0x00, 0x00, 0x00, sOmmDialogTextAlpha, texture, false);
            omm_render_glyph(x + dx - 1, y + dy,     glyphSize, glyphSize, 0x00, 0x00, 0x00, sOmmDialogTextAlpha, texture, false);
            omm_render_glyph(x + dx,     y + dy + 1, glyphSize, glyphSize, 0x00, 0x00, 0x00, sOmmDialogTextAlpha, texture, false);
            omm_render_glyph(x + dx,     y + dy - 1, glyphSize, glyphSize, 0x00, 0x00, 0x00, sOmmDialogTextAlpha, texture, false);
            }
            omm_render_glyph(x + dx,     y + dy,     glyphSize, glyphSize, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, texture, false);
        }
        x += dw;
    }
}

static s32 omm_render_star_select_update(s32 action) {
    static struct Object *sStarSelectStars[6];
    static f32 sStarSelectCurrentIndex;
    static s32 sStarSelectTargetIndex;
    static s32 sStarSelectCourseIndex;
    static s32 sStarSelectTimer;

    sOmmDialogTextAlpha = (omm_clamp_s(sStarSelectTimer, 0, 15) * 17);
    u8 starFlags = save_file_get_star_flags(gCurrSaveFileNum - 1, gCurrCourseNum - 1);

    switch (action) {

        // Init values
        case 0: {
            sStarSelectTimer = 0;
            sStarSelectCourseIndex = gCurrCourseNum - COURSE_MIN;
            sStarSelectCurrentIndex = 0;
            while ((sStarSelectCurrentIndex < 6) && (starFlags & (1 << ((s32) (sStarSelectCurrentIndex))))) sStarSelectCurrentIndex += 1;
            sStarSelectCurrentIndex = (((s32) sStarSelectCurrentIndex) % 6);
            sStarSelectTargetIndex = sStarSelectCurrentIndex;
        } break;

        // Init objects
        case 1: {
            sStarSelectStars[0] = spawn_object(gCurrentObject, MODEL_STAR, omm_bhv_act_select_star);
            sStarSelectStars[1] = spawn_object(gCurrentObject, MODEL_STAR, omm_bhv_act_select_star);
            sStarSelectStars[2] = spawn_object(gCurrentObject, MODEL_STAR, omm_bhv_act_select_star);
            sStarSelectStars[3] = spawn_object(gCurrentObject, MODEL_STAR, omm_bhv_act_select_star);
            sStarSelectStars[4] = spawn_object(gCurrentObject, MODEL_STAR, omm_bhv_act_select_star);
            sStarSelectStars[5] = spawn_object(gCurrentObject, MODEL_STAR, omm_bhv_act_select_star);
        } break;

        // Update objects
        case 2: {
            s32 selectedIndex = ((((s32) (sStarSelectCurrentIndex < 0.f ? (sStarSelectCurrentIndex - 0.5f) : (sStarSelectCurrentIndex + 0.5f))) + 6) % 6);
            for (s32 i = 0; i != 6; ++i) {
                f32 t = (f32) i - sStarSelectCurrentIndex;
                sStarSelectStars[i]->oPosX = OMM_RENDER_STAR_SELECT_ACT_STAR_X + OMM_RENDER_STAR_SELECT_ACT_STAR_DX * sins((t * 0x10000) / 6);
                sStarSelectStars[i]->oPosY = OMM_RENDER_STAR_SELECT_ACT_STAR_Y;
                sStarSelectStars[i]->oPosZ = OMM_RENDER_STAR_SELECT_ACT_STAR_Z + OMM_RENDER_STAR_SELECT_ACT_STAR_DZ * coss((t * 0x10000) / 6);
                sStarSelectStars[i]->oGraphNode = gLoadedGraphNodes[((starFlags & (1 << i)) ? MODEL_STAR : MODEL_TRANSPARENT_STAR)];
                sStarSelectStars[i]->oFaceAngleYaw = (sStarSelectStars[i]->oFaceAngleYaw + 0x800) * (i == selectedIndex);
            }
        } break;

        // Update inputs
        case 3: {

            // Up-down scroll to change the course
            // A course is available only if at least 1 star has been collected
            // The course that showed up first is always available
            sOmmStarSelectScrollV.idx = gCurrCourseNum - COURSE_MIN;
            omm_render_update_scroll(&sOmmStarSelectScrollV, COURSE_STAGES_COUNT, -gPlayer1Controller->stickY);
            while (sOmmStarSelectScrollV.inc != 0) {
                s32 nextCourseIndex = (((gCurrCourseNum - COURSE_MIN) + sOmmStarSelectScrollV.inc + COURSE_STAGES_COUNT) % COURSE_STAGES_COUNT);
                if (nextCourseIndex == sStarSelectCourseIndex || (save_file_get_course_star_count(gCurrSaveFileNum - 1, nextCourseIndex) != 0)) {
                    gCurrLevelNum = omm_level_get_list()[nextCourseIndex];
                    gCurrCourseNum = omm_level_get_course(gCurrLevelNum);
                    gSavedCourseNum = gCurrCourseNum;
                    sWarpDest.levelNum = gCurrLevelNum;
                    starFlags = save_file_get_star_flags(gCurrSaveFileNum - 1, gCurrCourseNum - 1);
                    break;
                }
                gCurrCourseNum = nextCourseIndex + COURSE_MIN;
            }

            // Left-right scroll to change the act
            // An act is available only if its star or the star of the previous act has been collected
            // The first act is always available
            s32 targetIndex = ((sStarSelectTargetIndex + 6) % 6);
            if (targetIndex != 0 && !(starFlags & (1 << targetIndex)) && !(starFlags & (1 << (targetIndex - 1)))) {
                sOmmStarSelectScrollH.inc = -1;
            } else {
                sOmmStarSelectScrollH.idx = ((sStarSelectTargetIndex + 6) % 6);
                omm_render_update_scroll(&sOmmStarSelectScrollH, 6, +gPlayer1Controller->stickX);
            }
            while (sOmmStarSelectScrollH.inc != 0) {
                sStarSelectTargetIndex += sOmmStarSelectScrollH.inc;
                s32 nextTargetIndex = ((sStarSelectTargetIndex + 6) % 6);
                if (nextTargetIndex == 0 || (starFlags & (1 << nextTargetIndex)) || (starFlags & (1 << (nextTargetIndex - 1)))) {
                    break;
                }
            }

            // Move smoothly the star selector
            f32 delta = omm_clamp_f(sStarSelectTargetIndex - sStarSelectCurrentIndex, -1.f, +1.f) / 4.f;
            sStarSelectCurrentIndex += delta;
            if (omm_abs_f(sStarSelectTargetIndex - sStarSelectCurrentIndex) < 0.01f) { sStarSelectCurrentIndex = sStarSelectTargetIndex; }
            while (sStarSelectTargetIndex <= -6) { sStarSelectTargetIndex += 6; sStarSelectCurrentIndex += 6; }
            while (sStarSelectTargetIndex >= +6) { sStarSelectTargetIndex -= 6; sStarSelectCurrentIndex -= 6; }
            while (omm_abs_f(sStarSelectTargetIndex - sStarSelectCurrentIndex) >= 6) { sStarSelectCurrentIndex += 6 * omm_sign_f(sStarSelectTargetIndex - sStarSelectCurrentIndex); }

            // Press A, B or Start to select the act
            if ((sStarSelectTimer++ >= 15) && (gPlayer1Controller->buttonPressed & (A_BUTTON | B_BUTTON | START_BUTTON))) {
                play_sound(SOUND_MENU_STAR_SOUND_LETS_A_GO, gGlobalSoundArgs);
                s32 selectedIndex = ((sStarSelectTargetIndex + 6) % 6);
                gDialogCourseActNum = selectedIndex + 1;
                return selectedIndex + 1;
            }

            // Update objects
            area_update_objects();
        } break;

        // Render
        case 4: {
            s32 selectedIndex = ((sStarSelectTargetIndex + 6) % 6);
            u32 color = (OMM_GAME_IS_SMMS ? 0x00F000FF : (OMM_EXTRAS_COLORED_STARS ? omm_stars_get_color(gCurrLevelNum) : 0xFFFFFFFF));
            u8 r = (color >> 24) & 0xFF;
            u8 g = (color >> 16) & 0xFF;
            u8 b = (color >>  8) & 0xFF;

            // Background
            omm_render_star_select_background_rect(GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(0), 0, r, g, b, 0xFF, SCREEN_WIDTH / 2, SCREEN_HEIGHT, r, g, b, 0, false);
            omm_render_star_select_background_rect(SCREEN_WIDTH / 2, 0, r, g, b, 0, GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(0), SCREEN_HEIGHT, r, g, b, 0xFF, false);
            omm_render_star_select_background_rect(GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(0), OMM_RENDER_STAR_SELECT_BACKGROUND_BOTTOM_Y0, r, g, b, 0xFF, GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(0), OMM_RENDER_STAR_SELECT_BACKGROUND_BOTTOM_Y1, r / 2, g / 2, b / 2, 0xFF, true);
            omm_render_star_select_background_rect(GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(0), OMM_RENDER_STAR_SELECT_BACKGROUND_TOP_Y0, r / 2, g / 2, b / 2, 0xFF, GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(0), OMM_RENDER_STAR_SELECT_BACKGROUND_TOP_Y1, r, g, b, 0xFF, true);

            // Course name
            const u8 *courseName = omm_level_get_name(gCurrLevelNum, false, false);
            s32 length = omm_text_length(courseName);
            s32 courseNum = omm_level_get_course(gCurrLevelNum);
            u8 buffer[64];
            if (courseNum < 10) {
                buffer[0] = courseNum;
                buffer[1] = DIALOG_CHAR_SPACE;
                buffer[2] = OMM_RENDER_STAR_SELECT_HYPHEN;
                buffer[3] = DIALOG_CHAR_SPACE;
                OMM_MEMCPY(&buffer[4], courseName, length + 1);
            } else {
                buffer[0] = courseNum / 10;
                buffer[1] = courseNum % 10;
                buffer[2] = DIALOG_CHAR_SPACE;
                buffer[3] = OMM_RENDER_STAR_SELECT_HYPHEN;
                buffer[4] = DIALOG_CHAR_SPACE;
                OMM_MEMCPY(&buffer[5], courseName, length + 1);
            }
            omm_text_capitalize(buffer);
            omm_render_star_select_string(buffer, (SCREEN_WIDTH - omm_render_star_select_get_string_width(buffer, 16, 1)) / 2, OMM_RENDER_STAR_SELECT_COURSE_NAME_Y, 16, 1, true, true);
            
            // Act text
            u8 *actText = omm_text_convert(OMM_TEXT_ACT, false);
            s32 actTextW = omm_render_star_select_get_string_width(actText, 12, 2);
            s32 actTextX = (SCREEN_WIDTH - (actTextW + 18)) / 2;
            s32 actTextY = OMM_RENDER_STAR_SELECT_BACKGROUND_BOTTOM_Y1 - 6;
            omm_render_star_select_string(actText, actTextX, actTextY, 12, 2, true, true);

            // Act number
            u8 actNumber[] = { selectedIndex + 1, 0xFF };
            s32 actNumberX = actTextX + actTextW + 6;
            s32 actNumberY = OMM_RENDER_STAR_SELECT_BACKGROUND_BOTTOM_Y1 - 8;
            omm_render_star_select_string(actNumber, actNumberX, actNumberY, 16, 0, true, true);

            // Act name
            u8 *actName = omm_level_get_act_name(gCurrLevelNum, selectedIndex + 1, false, false);
            s32 actNameX = (SCREEN_WIDTH - omm_render_get_string_width(OMM_RENDER_FONT_MENU, actName)) / 2;
            omm_render_string(OMM_RENDER_FONT_MENU, actName, actNameX - 1, OMM_RENDER_STAR_SELECT_ACT_NAME_Y, 0x00, 0x00, 0x00, sOmmDialogTextAlpha, false);
            omm_render_string(OMM_RENDER_FONT_MENU, actName, actNameX + 1, OMM_RENDER_STAR_SELECT_ACT_NAME_Y, 0x00, 0x00, 0x00, sOmmDialogTextAlpha, false);
            omm_render_string(OMM_RENDER_FONT_MENU, actName, actNameX, OMM_RENDER_STAR_SELECT_ACT_NAME_Y - 1, 0x00, 0x00, 0x00, sOmmDialogTextAlpha, false);
            omm_render_string(OMM_RENDER_FONT_MENU, actName, actNameX, OMM_RENDER_STAR_SELECT_ACT_NAME_Y + 1, 0x00, 0x00, 0x00, sOmmDialogTextAlpha, false);
            omm_render_string(OMM_RENDER_FONT_MENU, actName, actNameX, OMM_RENDER_STAR_SELECT_ACT_NAME_Y, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, false);

            // Score text
            u8 *scoreText = omm_text_convert(OMM_TEXT_MY_SCORE, false);
            s32 scoreTextX = (SCREEN_WIDTH / 2) - (omm_render_get_string_width(OMM_RENDER_FONT_MENU, scoreText) + 11);
            omm_render_string(OMM_RENDER_FONT_MENU, scoreText, scoreTextX - 1, OMM_RENDER_STAR_SELECT_SCORE_Y, 0x40, 0x40, 0x00, sOmmDialogTextAlpha, false);
            omm_render_string(OMM_RENDER_FONT_MENU, scoreText, scoreTextX + 1, OMM_RENDER_STAR_SELECT_SCORE_Y, 0x40, 0x40, 0x00, sOmmDialogTextAlpha, false);
            omm_render_string(OMM_RENDER_FONT_MENU, scoreText, scoreTextX, OMM_RENDER_STAR_SELECT_SCORE_Y - 1, 0x40, 0x40, 0x00, sOmmDialogTextAlpha, false);
            omm_render_string(OMM_RENDER_FONT_MENU, scoreText, scoreTextX, OMM_RENDER_STAR_SELECT_SCORE_Y + 1, 0x40, 0x40, 0x00, sOmmDialogTextAlpha, false);
            omm_render_string(OMM_RENDER_FONT_MENU, scoreText, scoreTextX, OMM_RENDER_STAR_SELECT_SCORE_Y, 0xFF, 0xFF, 0x00, sOmmDialogTextAlpha, false);

            // Coin score
            s32 scoreCoins = save_file_get_course_coin_score(gCurrSaveFileNum - 1, gCurrCourseNum - 1);
            s32 scoreCoinsX = (SCREEN_WIDTH / 2) - 7;
            omm_render_hud_coins(scoreCoinsX, OMM_RENDER_STAR_SELECT_SCORE_Y - ((OMM_RENDER_GLYPH_SIZE - 8) / 2), sOmmDialogTextAlpha, scoreCoins, false);

            // 100 coins star
            if (starFlags & 0x40) {
                s32 coinsStarX = OMM_RENDER_STAR_SELECT_100_COINS_STAR_X;
                s32 coinsStarY = OMM_RENDER_STAR_SELECT_SCORE_Y - ((OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE - 8) / 2) + 1;
                const void *coinsStarTex = omm_render_get_star_glyph(omm_clamp_s(gCurrCourseNum, 0, 16), OMM_EXTRAS_COLORED_STARS);
                omm_render_glyph(coinsStarX + 1, coinsStarY, OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, 0x00, 0x00, 0x00, sOmmDialogTextAlpha, coinsStarTex, false);
                omm_render_glyph(coinsStarX - 1, coinsStarY, OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, 0x00, 0x00, 0x00, sOmmDialogTextAlpha, coinsStarTex, false);
                omm_render_glyph(coinsStarX, coinsStarY + 1, OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, 0x00, 0x00, 0x00, sOmmDialogTextAlpha, coinsStarTex, false);
                omm_render_glyph(coinsStarX, coinsStarY - 1, OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, 0x00, 0x00, 0x00, sOmmDialogTextAlpha, coinsStarTex, false);
                omm_render_glyph(coinsStarX, coinsStarY, OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, OMM_RENDER_STAR_SELECT_100_COINS_STAR_SIZE, 0xFF, 0xFF, 0xFF, sOmmDialogTextAlpha, coinsStarTex, false);
            }

#if OMM_CODE_TIME_TRIALS
            time_trials_render_star_select_time(selectedIndex);
#endif
        } break;
    }
    return 0;
}

void bhv_act_selector_star_type_loop(void) {
}

void bhv_act_selector_init(void) {
    omm_render_star_select_update(1);
}

void bhv_act_selector_loop(void) {
    omm_render_star_select_update(2);
}

s32 lvl_init_act_selector_values_and_stars(UNUSED s32 arg, UNUSED s32 unused) {
    return omm_render_star_select_update(0);
}

s32 lvl_update_obj_and_load_act_button_actions(UNUSED s32 arg, UNUSED s32 unused) {
    return omm_render_star_select_update(3);
}

#ifdef AVOID_UB
Gfx *geo_act_selector_strings(s16 callContext, UNUSED struct GraphNode *node, UNUSED void *context) {
#else
Gfx *geo_act_selector_strings(s16 callContext, UNUSED struct GraphNode *node) {
#endif
    if (callContext == GEO_CONTEXT_RENDER) {
        omm_render_star_select_update(4);
    }
    return NULL;
}

//
// File select
//

static void omm_menu_update_inputs(struct Object *characterSelectButton, struct Object *sparklyStarsScore, f32 *cursorPos) {
    
    // Sparkly Stars score, press A to return or hold B to delete the Sparkly Stars data
    if (sparklyStarsScore) {
        static s32 sHoldTimer = 0;
        sparklyStarsScore->oOpacity = 0xFF * omm_invlerp_0_1_f(sparklyStarsScore->oTimer, 0, 10) * sqrtf(omm_invlerp_0_1_f(sHoldTimer, 90, 30));
    
        bool closeScoreScreen = false;
        if (gPlayer1Controller->buttonPressed & A_BUTTON) {
            closeScoreScreen = true;
        } else if (gPlayer1Controller->buttonDown & B_BUTTON) {
            if (++sHoldTimer >= 90) {
                omm_sparkly_clear_mode(sparklyStarsScore->oAction);
                play_sound(SOUND_MARIO_WAAAOOOW, gGlobalSoundArgs);
                closeScoreScreen = true;
            }
        } else if (sHoldTimer > 0) {
            closeScoreScreen = true;
        } else {
            sHoldTimer = 0;
        }

        if (closeScoreScreen) {
            obj_mark_for_deletion(sparklyStarsScore);
            gPlayer1Controller->buttonPressed = 0;
            gPlayer2Controller->buttonPressed = 0;
            gPlayer3Controller->buttonPressed = 0;
            sHoldTimer = 0;
        }
        return;
    }
    
    // Character select button
    if (characterSelectButton) {

        // Animation
        if (characterSelectButton->oMenuButtonState == MENU_BUTTON_STATE_ZOOM_IN_OUT) {
            characterSelectButton->oMenuButtonScale = omm_lerp_f(sins(omm_invlerp_0_1_f(characterSelectButton->oMenuButtonTimer, 0, 7) * 0x8000), 1.f, 0.8f);
        }
        
        // Select a button by pressing A, B or Start
        else if (gPlayer1Controller->buttonPressed & (A_BUTTON | B_BUTTON | START_BUTTON)) {

            // Change the current character by pressing the character select button
            if (omm_abs_f(cursorPos[0] - (characterSelectButton->oPosX / 67.5f)) < 20.f &&
                omm_abs_f(cursorPos[1] - (characterSelectButton->oPosY / 67.5f)) < 16.f) {

                // Advance the character index by 1 until an unlocked character is found and select it
                do { gOmmCharacter = (gOmmCharacter + 1) % OMM_NUM_PLAYABLE_CHARACTERS;
                } while (!omm_player_is_unlocked(gOmmCharacter));
                omm_player_select(gOmmCharacter);

                // Play the character's 'Here-we-go' sound and change the button appearance
                play_sound(SOUND_MARIO_HERE_WE_GO, gGlobalSoundArgs);
                play_sound(SOUND_MENU_CLICK_FILE_SELECT | 0xFF00, gGlobalSoundArgs);
                characterSelectButton->oMenuButtonState = MENU_BUTTON_STATE_ZOOM_IN_OUT;
                switch (omm_player_get_selected_index()) {
                    case OMM_PLAYER_MARIO: characterSelectButton->oGraphNode = omm_geo_get_graph_node(omm_geo_menu_button_mario, true); break;
                    case OMM_PLAYER_PEACH: characterSelectButton->oGraphNode = omm_geo_get_graph_node(omm_geo_menu_button_peach, true); break;
                    case OMM_PLAYER_LUIGI: characterSelectButton->oGraphNode = omm_geo_get_graph_node(omm_geo_menu_button_luigi, true); break;
                    case OMM_PLAYER_WARIO: characterSelectButton->oGraphNode = omm_geo_get_graph_node(omm_geo_menu_button_wario, true); break;
                }
            }

            // Select a star icon by pressing A to display the Sparkly Stars score screen
            else if (gPlayer1Controller->buttonPressed & A_BUTTON) {
                for (s32 mode = OMM_SPARKLY_MODE_NORMAL; mode != OMM_SPARKLY_MODE_COUNT; ++mode) {
                    if (omm_sparkly_is_mode_selectible(mode)) {
                        if (cursorPos[0] >= 56 + mode * 18 && cursorPos[1] >= 72 &&
                            cursorPos[0] <= 73 + mode * 18 && cursorPos[1] <= 90) {
                            spawn_object(characterSelectButton, MODEL_NONE, omm_bhv_act_select_star)->oAction = mode;
                            play_sound(SOUND_MENU_STAR_SOUND, gGlobalSoundArgs);
                            break;
                        }
                    }
                }
            }
        }

#if !OMM_GAME_IS_SMSR
        // Hard mode toggle
        if (gPlayer1Controller->buttonPressed & L_TRIG) {
            play_sound(SOUND_MENU_MARIO_CASTLE_WARP, gGlobalSoundArgs);
            g1HPMode = !g1HPMode;
        }
#endif
    }
}

static void omm_menu_render(struct Object *characterSelectButton, struct Object *sparklyStarsScore, u8 alpha) {
    omm_render_create_dl_ortho_matrix();

    // Print current character name
    if (characterSelectButton) {
        const u8 *str64 = omm_text_convert(omm_player_get_name(omm_player_get_selected_index()), false);
        s16 dx = omm_render_get_string_width(OMM_RENDER_FONT_GENERIC, str64) / 2;
        omm_render_string(OMM_RENDER_FONT_GENERIC, str64, 253 - dx, 39, 0xFF, 0xFF, 0xFF, alpha, false);
    }

#if !OMM_GAME_IS_SMSR
    // Hard mode, press L to toggle
#if defined(WIDESCREEN)
    s16 hardModeX = GFX_DIMENSIONS_RECT_FROM_LEFT_EDGE(3);
#else
    s16 hardModeX = -6;
#endif
    if (g1HPMode) {
        omm_render_string(OMM_RENDER_FONT_MENU, omm_text_convert(OMM_TEXT_MENU_HARD_MODE_ON, false), hardModeX, 3, 0xFF, 0xFF, 0xFF, alpha, false);
    } else {
        omm_render_string(OMM_RENDER_FONT_MENU, omm_text_convert(OMM_TEXT_MENU_HARD_MODE_OFF, false), hardModeX, 3, 0x40, 0x40, 0x40, alpha / 2, false);
    }
#endif

    // Sparkly Stars icons
    for (s32 mode = OMM_SPARKLY_MODE_NORMAL; mode != OMM_SPARKLY_MODE_COUNT; ++mode) {
        if (omm_sparkly_is_mode_selectible(mode)) {
            if (omm_sparkly_is_mode_completed(mode)) {
                omm_render_glyph(216 + 18 * mode, SCREEN_HEIGHT - 51, 16, 16, 0xFF, 0xFF, 0xFF, alpha, OMM_SPARKLY_STAR_HUD_GLYPH[mode], false);
            } else {
                omm_render_glyph(216 + 18 * mode, SCREEN_HEIGHT - 51, 16, 16, 0x00, 0x00, 0x00, alpha / 2, OMM_SPARKLY_STAR_HUD_GLYPH[mode], false);
            }
        }
    }

    // Sparkly Stars score screen
    if (sparklyStarsScore) {
        s32 mode = sparklyStarsScore->oAction;

        // Black box
        static Vtx sOmmSparklyStarsScoreBlackBoxVertices[4];
        sOmmSparklyStarsScoreBlackBoxVertices[0] = (Vtx) { { { GFX_DIMENSIONS_FROM_LEFT_EDGE(0), 0, 0 }, 0, { 0, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } };
        sOmmSparklyStarsScoreBlackBoxVertices[1] = (Vtx) { { { GFX_DIMENSIONS_FROM_LEFT_EDGE(0), SCREEN_HEIGHT, 0 }, 0, { 0, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } };
        sOmmSparklyStarsScoreBlackBoxVertices[2] = (Vtx) { { { GFX_DIMENSIONS_FROM_RIGHT_EDGE(0), 0, 0 }, 0, { 0, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } };
        sOmmSparklyStarsScoreBlackBoxVertices[3] = (Vtx) { { { GFX_DIMENSIONS_FROM_RIGHT_EDGE(0), SCREEN_HEIGHT, 0 }, 0, { 0, 0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } };
        gDPSetEnvColor(gDisplayListHead++, 0x00, 0x00, 0x00, 0.85f * sparklyStarsScore->oOpacity);
        gSPClearGeometryMode(gDisplayListHead++, G_LIGHTING);
        gDPSetCombineLERP(gDisplayListHead++, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0, SHADE, 0, ENVIRONMENT, 0);
        gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gSPVertex(gDisplayListHead++, sOmmSparklyStarsScoreBlackBoxVertices, 4, 0);
        gSP2Triangles(gDisplayListHead++, 2, 1, 0, 0, 1, 2, 3, 0);

        // Star count
        omm_render_glyph(10, SCREEN_HEIGHT - 36, 16, 16, 0xFF, 0xFF, 0xFF, sparklyStarsScore->oOpacity, OMM_SPARKLY_STAR_HUD_GLYPH[mode], false);
        omm_render_number(omm_sparkly_get_star_count(mode), 2, 30, SCREEN_HEIGHT - 36, 16, 16, 12, sparklyStarsScore->oOpacity, true, false);

        // Timer
        omm_render_sparkly_timer(220, SCREEN_HEIGHT - 36, 14, sparklyStarsScore->oOpacity, mode, omm_sparkly_get_timer(mode), false, false);

        // Courses
        u8 r = OMM_SPARKLY_STAR_HUD_COLOR[mode][0];
        u8 g = OMM_SPARKLY_STAR_HUD_COLOR[mode][1];
        u8 b = OMM_SPARKLY_STAR_HUD_COLOR[mode][2];
        for (s32 i = 0; i != 30; ++i) {
            s16 x = 12 + 140 * (i / 15);
            s16 y = SCREEN_HEIGHT - 8 - (44 + 12 * (i % 15));
            u8 *name = omm_sparkly_get_star_level_name(mode, i);
            if ((i == 29 && omm_sparkly_is_grand_star_collected(mode)) || omm_sparkly_is_star_collected(mode, i)) {
                omm_render_string(OMM_RENDER_FONT_MENU, name, x, y, r, g, b, sparklyStarsScore->oOpacity, false);
            } else {
                omm_render_string(OMM_RENDER_FONT_MENU, name, x, y, r / 3, g / 3, b / 3, sparklyStarsScore->oOpacity, false);
            }
        }
    }
}

void omm_menu_update(f32 *cursorPos, s8 selectedButtonId, u8 alpha) {
    switch (selectedButtonId) {
        case MENU_BUTTON_PLAY_FILE_A: {
            omm_select_save_file(1);
            omm_player_select(gOmmCharacter);
        } break;

        case MENU_BUTTON_PLAY_FILE_B: {
            omm_select_save_file(2);
            omm_player_select(gOmmCharacter);
        } break;

        case MENU_BUTTON_PLAY_FILE_C: {
            omm_select_save_file(3);
            omm_player_select(gOmmCharacter);
        } break;

        case MENU_BUTTON_PLAY_FILE_D: {
            omm_select_save_file(4);
            omm_player_select(gOmmCharacter);
        } break;

        case MENU_BUTTON_NONE: {
            omm_menu_update_inputs(obj_get_first_with_behavior(omm_bhv_menu_character_select_button), obj_get_first_with_behavior(omm_bhv_act_select_star), cursorPos);
            omm_menu_render(obj_get_first_with_behavior(omm_bhv_menu_character_select_button), obj_get_first_with_behavior(omm_bhv_act_select_star), alpha);
        } break;
    }
}
