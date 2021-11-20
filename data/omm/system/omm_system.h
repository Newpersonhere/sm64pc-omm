#ifndef OMM_SYSTEM_H
#define OMM_SYSTEM_H

#include "data/omm/omm_includes.h"

//
// System
//

void omm_add_routine(s32 type, void (*func)(void));
void omm_select_save_file(s32 saveFileNum);
void omm_return_to_main_menu();
void omm_update();
void omm_update_gfx();
void *omm_update_cmd(void *cmd, s32 reg);
bool omm_is_main_menu();
bool omm_is_game_paused();
bool omm_is_transition_active();
bool omm_is_ending_cutscene();
bool omm_is_ending_cake_screen();
bool omm_sanity_check_graph_node(struct GraphNode *node);
void omm_debug_start_counter();
void omm_debug_end_counter();

#if defined(SM74)
void omm_opt_sm74_change_mode(UNUSED void *opt, s32 arg);
#define sm74_mode__omm_save                     ((omm_is_main_menu() ? (s32) sWarpDest.areaIdx : (s32) gCurrAreaIndex) - 1)
#define sm74_mode__omm_opt_init_warp_to_level   sWarpDest.areaIdx
#define sm74_mode__omm_stars_get_bits_total     sWarpDest.areaIdx
#define sm74_mode__omm_level_get_name           sWarpDest.areaIdx
#define sm74_mode__omm_level_get_act_name       sWarpDest.areaIdx
#define sm74_mode__omm_render_pause_castle      gCurrAreaIndex
#define sm74_mode__omm_render_dialog_entry      gCurrAreaIndex
#endif

//
// Player
//

bool omm_player_is_unlocked(s32 index);
bool omm_player_is_selected(s32 index);
void omm_player_select(s32 index);
s32 omm_player_get_selected_index();

const char *omm_player_get_name(s32 index);
u32 omm_player_get_color(s32 index);
s32 omm_player_get_model(s32 index);
s32 omm_player_get_normal_cap(s32 index);
s32 omm_player_get_wing_cap(s32 index);
s32 omm_player_get_metal_cap(s32 index);
s32 omm_player_get_winged_metal_cap(s32 index);
s32 omm_player_get_cap(s32 index, bool wing, bool metal);

f32 omm_player_get_ground_speed_multiplier(s32 index);
f32 omm_player_get_air_speed_multiplier(s32 index);
f32 omm_player_get_jump_multiplier(s32 index);

#define OMM_PLAYER_SEL_GFX_INDEX                            omm_player_get_selected_index()
#define OMM_PLAYER_SEL_PHY_INDEX                            omm_player_get_selected_index() + (omm_peach_vibe_is_active() ? (gOmmData->mario->peach.vibeType + 2) : (0))

#define omm_player_get_selected_selected_name()             omm_player_get_name(OMM_PLAYER_SEL_GFX_INDEX)
#define omm_player_get_selected_color()                     omm_player_get_color(OMM_PLAYER_SEL_GFX_INDEX)
#define omm_player_get_selected_model()                     omm_player_get_model(OMM_PLAYER_SEL_GFX_INDEX)
#define omm_player_get_selected_normal_cap()                omm_player_get_normal_cap(OMM_PLAYER_SEL_GFX_INDEX)
#define omm_player_get_selected_wing_cap()                  omm_player_get_wing_cap(OMM_PLAYER_SEL_GFX_INDEX)
#define omm_player_get_selected_metal_cap()                 omm_player_get_metal_cap(OMM_PLAYER_SEL_GFX_INDEX)
#define omm_player_get_selected_winged_metal_cap()          omm_player_get_winged_metal_cap(OMM_PLAYER_SEL_GFX_INDEX)
#define omm_player_get_selected_cap(wing, metal)            omm_player_get_cap(OMM_PLAYER_SEL_GFX_INDEX, wing, metal)

#define omm_player_get_selected_ground_speed_multiplier()   omm_player_get_ground_speed_multiplier(OMM_PLAYER_SEL_PHY_INDEX)
#define omm_player_get_selected_air_speed_multiplier()      omm_player_get_air_speed_multiplier(OMM_PLAYER_SEL_PHY_INDEX)
#define omm_player_get_selected_jump_multiplier()           omm_player_get_jump_multiplier(OMM_PLAYER_SEL_PHY_INDEX)
#define get_additive_y_vel_for_jumps()                      ((initialVelY + (m->forwardVel * multiplier)) * (omm_player_get_selected_jump_multiplier() - 1.f))

//
// Health
//

s32  omm_health_get_max(s32 health);
s32  omm_health_get_max_hp(s32 hp);
bool omm_health_is_mario_losing_hp(struct MarioState *m);
void omm_health_update(struct MarioState *m);
void omm_health_heal_mario(struct MarioState *m);
void omm_health_fully_heal_mario(struct MarioState *m);
void omm_health_life_up(struct MarioState *m);
void omm_health_set(struct MarioState *m, s16 health);
bool omm_health_is_at_max(struct MarioState *m);

//
// Text
//

u8 *omm_text_convert(const char *str, bool heapAlloc);
u8 *omm_text_copy(const u8 *str64, bool heapAlloc);
u8 *omm_text_capitalize(u8 *str64);
u8 *omm_text_decapitalize(u8 *str64);
u8 *omm_text_replace_char(u8 *str64, u8 from, u8 to);
u8 *omm_text_replace_names(u8 *str64, bool inplace);
s32 omm_text_length(const u8 *str64);
struct DialogEntry *omm_get_dialog_entry(void **dialogTable, s16 dialogId);

//
// Render
//

#define OMM_RENDER_FONT_GENERIC 0
#define OMM_RENDER_FONT_MENU 1
#define OMM_RENDER_FONT_HUD 2

s32  omm_render_get_string_width(s32 font, const u8 *str64);
void omm_render_string(s32 font, const u8 *str64, s16 x, s16 y, u8 red, u8 green, u8 blue, u8 alpha, bool shadow);
void omm_render_texrect(s16 x, s16 y, s16 w, s16 h, s32 fmt, s32 siz, s16 texw, s16 texh, u8 r, u8 g, u8 b, u8 a, const void *texp);
void omm_render_glyph(s16 x, s16 y, s16 w, s16 h, u8 red, u8 green, u8 blue, u8 alpha, const void *texture, bool shadow);
void omm_render_number(s32 number, s32 digits, s16 x, s16 y, s16 w, s16 h, s16 xStep, u8 alpha, bool makeZerosTransparent, bool shadow);
void omm_render_create_dl_ortho_matrix();
void omm_render_start_you_got_a_star(const char *title, const u8 *courseName, const u8 *starName);
void omm_render_stop_you_got_a_star();
void omm_render_file_select_screen(s8 selectedButtonId, u8 alpha);
void omm_render_hud();
s32  omm_render_pause();
s32  omm_render_course_complete();
void omm_render_dialog_entries();

//
// Camera
//

void omm_camera_init();
bool omm_camera_update(struct Camera *c, struct MarioState *m);
void omm_camera_set_yaw(s16 yaw);
s16 omm_camera_get_intended_yaw(struct MarioState *m);
s32 omm_camera_get_relative_dist_mode();
bool omm_camera_is_available(struct MarioState *m);

//
// Audio
//

void omm_sound_play(s32 id, f32 *pos);
void omm_sound_stop(s32 id);
bool omm_sound_is_playing(s32 id);
bool omm_sound_is_bank_playing(u8 bank);
bool omm_sound_play_character_sound_n64(s32 id, f32 *pos);
bool omm_sound_play_character_sound_omm(s32 id, f32 *pos);
bool omm_sound_play_character_sound_r96(const char *id, f32 *pos);
bool omm_sound_stop_character_sound_n64(s32 id, f32 *pos);
bool omm_sound_stop_character_sound_omm(s32 id, f32 *pos);
bool omm_sound_stop_character_sound_r96(const char *id, f32 *pos);

//
// Level
//

s32 omm_level_get_count();
s32 *omm_level_get_list();
s32 omm_level_get_course(s32 level);
const LevelScript *omm_level_get_script(s32 level);
s32 omm_level_get_areas(s32 level);
s32 omm_level_get_num_red_coins(s32 level, s32 area);
u8 *omm_level_get_name(s32 level, bool decaps, bool num);
u8 *omm_level_get_act_name(s32 level, s32 act, bool decaps, bool num);
u64 omm_level_cmd_get(void *cmd, u64 offset);
void omm_level_parse_script(const LevelScript *script, s32 (*func)(u8, void *));
s16 *omm_level_get_warp(s32 level, s32 area, u8 id);
s16 *omm_level_get_entry_warp(s32 level, s32 area);
s16 *omm_level_get_exit_warp(s32 level, s32 area);
s16 *omm_level_get_death_warp(s32 level, s32 area);
bool omm_level_can_warp(s32 level);

//
// Warps
//

bool omm_warp_to_level(s32 level, s32 area, s32 act);
bool omm_restart_level();
bool omm_restart_area();
bool omm_exit_level();
bool omm_return_to_castle(bool fadeOut, bool force);
bool omm_is_warping();
void *omm_update_warp(void *cmd, bool inited);

//
// Options
//

#define DECLARE_KBINDS(option)                  extern u32 option[MAX_BINDS]
#define DECLARE_TOGGLE(option)                  extern bool option
#define DECLARE_TOGGLE_SC(option)               extern bool option; DECLARE_KBINDS(option##Shortcuts)
#define DECLARE_CHOICE(option)                  extern u32 option
#define DECLARE_CHOICE_SC(option)               extern u32 option; DECLARE_KBINDS(option##Shortcuts)

#define DEFINE_KBINDS(option, v0, v1, v2)       u32 option[MAX_BINDS] = { v0, v1, v2 }
#define DEFINE_TOGGLE(option, v)                bool option = v
#define DEFINE_TOGGLE_SC(option, v)             bool option = v; DEFINE_KBINDS(option##Shortcuts, VK_INVALID, VK_INVALID, VK_INVALID)
#define DEFINE_CHOICE(option, v)                u32 option = v
#define DEFINE_CHOICE_SC(option, v)             u32 option = v; DEFINE_KBINDS(option##Shortcuts, VK_INVALID, VK_INVALID, VK_INVALID)

#define READ_KBINDS(option, buf)                if (strcmp(buf[0], #option) == 0) { sscanf(buf[1], "%X", &option[0]); sscanf(buf[2], "%X", &option[1]); sscanf(buf[3], "%X", &option[2]); continue; }
#define READ_TOGGLE(option, buf)                if (strcmp(buf[0], #option) == 0) { option = (buf[1][0] == '1'); continue; }
#define READ_TOGGLE_SC(option, buf)             if (strcmp(buf[0], #option) == 0) { option = (buf[1][0] == '1'); continue; } READ_KBINDS(option##Shortcuts, buf)
#define READ_CHOICE(option, buf)                if (strcmp(buf[0], #option) == 0) { sscanf(buf[1], "%u", &option); continue; }
#define READ_CHOICE_SC(option, buf)             if (strcmp(buf[0], #option) == 0) { sscanf(buf[1], "%u", &option); continue; } READ_KBINDS(option##Shortcuts, buf)

#define WRITE_KBINDS(option)                    write("%s = %04hX %04hX %04hX\n", #option, (u16) option[0], (u16) option[1], (u16) option[2])
#define WRITE_TOGGLE(option)                    write("%s = %u\n", #option, (option ? 1 : 0))
#define WRITE_TOGGLE_SC(option)                 write("%s = %u\n", #option, (option ? 1 : 0)); WRITE_KBINDS(option##Shortcuts)
#define WRITE_CHOICE(option)                    write("%s = %u\n", #option, option)
#define WRITE_CHOICE_SC(option)                 write("%s = %u\n", #option, option); WRITE_KBINDS(option##Shortcuts)

extern u8 gOmmOptMenu[];
extern u8 gOmmOptControls[];
#if !OMM_CODE_DYNOS
extern u8 gOmmOptWarpToLevel[];
extern u8 gOmmOptReturnToMainMenu[];
#endif

DECLARE_KBINDS(gOmmControlsButtonA);
DECLARE_KBINDS(gOmmControlsButtonB);
DECLARE_KBINDS(gOmmControlsButtonX);
DECLARE_KBINDS(gOmmControlsButtonY);
DECLARE_KBINDS(gOmmControlsButtonStart);
DECLARE_KBINDS(gOmmControlsTriggerL);
DECLARE_KBINDS(gOmmControlsTriggerR);
DECLARE_KBINDS(gOmmControlsTriggerZ);
DECLARE_KBINDS(gOmmControlsCUp);
DECLARE_KBINDS(gOmmControlsCDown);
DECLARE_KBINDS(gOmmControlsCLeft);
DECLARE_KBINDS(gOmmControlsCRight);
DECLARE_KBINDS(gOmmControlsDUp);
DECLARE_KBINDS(gOmmControlsDDown);
DECLARE_KBINDS(gOmmControlsDLeft);
DECLARE_KBINDS(gOmmControlsDRight);
DECLARE_KBINDS(gOmmControlsStickUp);
DECLARE_KBINDS(gOmmControlsStickDown);
DECLARE_KBINDS(gOmmControlsStickLeft);
DECLARE_KBINDS(gOmmControlsStickRight);
DECLARE_CHOICE_SC(gOmmCharacter);
DECLARE_CHOICE_SC(gOmmMovesetType);
DECLARE_CHOICE_SC(gOmmCapType);
DECLARE_CHOICE_SC(gOmmStarsMode);
DECLARE_CHOICE_SC(gOmmPowerUpsType);
DECLARE_CHOICE_SC(gOmmCameraMode);
DECLARE_CHOICE_SC(gOmmSparklyStarsMode);
DECLARE_TOGGLE(gOmmCheatUnlimitedCappyBounces);
DECLARE_TOGGLE(gOmmCheatCappyStaysForever);
DECLARE_TOGGLE(gOmmCheatHomingAttackGlobalRange);
DECLARE_TOGGLE(gOmmCheatMarioTeleportsToCappy);
DECLARE_TOGGLE(gOmmCheatPeachEndlessVibeGauge);
DECLARE_TOGGLE_SC(gOmmExtrasCappyEyesOnMariosCap);
DECLARE_TOGGLE_SC(gOmmExtrasColoredStars);
DECLARE_TOGGLE_SC(gOmmExtrasVanishingHUD);
DECLARE_TOGGLE_SC(gOmmExtrasRevealSecrets);
DECLARE_TOGGLE_SC(gOmmExtrasRedCoinsRadar);
DECLARE_TOGGLE_SC(gOmmExtrasShowStarNumber);
DECLARE_TOGGLE_SC(gOmmExtrasInvisibleMode);
DECLARE_TOGGLE_SC(gOmmExtrasCrystalStarsReward);
#if OMM_CODE_DEBUG
DECLARE_TOGGLE_SC(gOmmDebugHitbox);
DECLARE_TOGGLE_SC(gOmmDebugHurtbox);
DECLARE_TOGGLE_SC(gOmmDebugWallbox);
DECLARE_TOGGLE_SC(gOmmDebugMario);
DECLARE_TOGGLE_SC(gOmmDebugCappy);
DECLARE_TOGGLE_SC(gOmmDebugProfiler);
#if OMM_CODE_DEV_DEBUG
DECLARE_TOGGLE_SC(gOmmDebugGameSpeedEnabler);
DECLARE_CHOICE_SC(gOmmDebugGameSpeedModifier);
#endif
#endif

void omm_opt_return_to_main_menu(UNUSED void *opt, s32 arg);
void omm_opt_reset_binds(u32 *binds);

#endif // OMM_SYSTEM_H
