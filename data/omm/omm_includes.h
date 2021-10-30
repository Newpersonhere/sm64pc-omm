#ifndef OMM_INCLUDES_H
#define OMM_INCLUDES_H

// Required headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>
#include <dirent.h>
#include <SDL2/SDL.h>
#include "types.h"
#include "libc/math.h"
#include "game/camera.h"
#include "game/ingame_menu.h"
#include "game/mario_step.h"
#include "game/object_list_processor.h"
#include "engine/graph_node.h"
#include "engine/surface_collision.h"
#include "pc/configfile.h"
#undef STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// OMM headers
#include "data/omm/omm_macros.h"
#include "data/omm/omm_defines.h"
#include "data/omm/omm_patches.h"
#include "data/omm/system/omm_memory.h"
#include "data/omm/system/omm_system.h"
#include "data/omm/object/omm_object.h"
#include "data/omm/mario/omm_mario.h"
#include "data/omm/mario/omm_mario_actions.h"
#include "data/omm/cappy/omm_cappy.h"
#include "data/omm/capture/omm_capture_all.h"
#include "data/omm/peachy/omm_sparkly.h"
#include "data/omm/peachy/omm_peach.h"
#include "data/omm/peachy/omm_peach_actions.h"

// #define OMM_ALL_HEADERS before including this file
// to include all headers, not only the OMM ones
#ifdef OMM_ALL_HEADERS
#include "sm64.h"
#include "behavior_data.h"
#include "config.h"
#include "course_table.h"
#include "dialog_ids.h"
#include "geo_commands.h"
#include "gfx_dimensions.h"
#include "level_commands.h"
#include "level_misc_macros.h"
#include "macro_preset_names.h"
#include "model_ids.h"
#include "seq_ids.h"
#include "special_preset_names.h"
#include "actors/common0.h"
#include "actors/common1.h"
#include "actors/group0.h"
#include "actors/group1.h"
#include "actors/group2.h"
#include "actors/group3.h"
#include "actors/group4.h"
#include "actors/group5.h"
#include "actors/group6.h"
#include "actors/group7.h"
#include "actors/group8.h"
#include "actors/group9.h"
#include "actors/group10.h"
#include "actors/group11.h"
#include "actors/group12.h"
#include "actors/group13.h"
#include "actors/group14.h"
#include "actors/group15.h"
#include "actors/group16.h"
#include "actors/group17.h"
#include "game/behavior_actions.h"
#include "game/game_init.h"
#include "game/hud.h"
#include "game/interaction.h"
#include "game/level_geo.h"
#include "game/level_update.h"
#include "game/mario.h"
#include "game/mario_actions_airborne.h"
#include "game/mario_actions_automatic.h"
#include "game/mario_actions_cutscene.h"
#include "game/mario_actions_moving.h"
#include "game/mario_actions_object.h"
#include "game/mario_actions_stationary.h"
#include "game/mario_actions_submerged.h"
#include "game/mario_misc.h"
#include "game/mario_step.h"
#include "game/moving_texture.h"
#include "game/obj_behaviors.h"
#include "game/object_helpers.h"
#include "game/print.h"
#include "game/rendering_graph_node.h"
#include "game/save_file.h"
#include "game/segment2.h"
#include "game/sound_init.h"
#include "game/spawn_object.h"
#include "game/spawn_sound.h"
#include "engine/behavior_script.h"
#include "engine/level_script.h"
#include "engine/math_util.h"
#include "engine/surface_load.h"
#include "audio/external.h"
#include "audio/load.h"
#include "menu/file_select.h"
#include "levels/intro/header.h"
#include "levels/bowser_2/header.h"
#include "levels/ending/header.h"
#include "levels/menu/header.h"
#include "levels/scripts.h"
#include "pc/cliopts.h"
#include "pc/platform.h"
#include "pc/controller/controller_api.h"
#include "pc/gfx/gfx_rendering_api.h"
#include "pc/fs/fs.h"
#include FILE_OPTIONS_H
#include FILE_SOUNDS_H
#include FILE_CHEATS_H
#include FILE_TITLE_H
#include FILE_BETTERCAM_H
#include FILE_TXT_CONV_H
#include FILE_TEXT_LOADER_H
#include FILE_R96_SYSTEM_H
#endif

// SM64 globals
extern s8 gDialogBoxState;
extern s8 gDialogBoxType;
extern s8 gDialogLineNum;
extern s8 gLastDialogResponse;
extern s8 sPunchingForwardVelocities[8];
extern s8 sYoshiDead;
extern u8 gCurrentAnimType;
extern u8 sFramesSinceCutsceneEnded;
extern u8 sSpawnTypeFromWarpBhv[];
extern u8 gDialogCharWidths[256];
extern u8 texture_hud_char_puppycam[];
extern u8 *gEndCutsceneStringsEn[];
extern s16 *gCurrentAnimData;
extern s16 gCurrAnimFrame;
extern s16 gCutsceneMsgIndex;
extern s16 gDialogID;
extern s16 gDialogScrollOffsetY;
extern s16 gDialogTextPos;
extern s16 gLastDialogPageStrPos;
extern s16 gMenuMode;
extern s16 gIsHardMode;
extern s16 sInvulnerable;
extern s16 sStatusFlags;
extern s16 sYawSpeed;
extern u16 sAcousticReachPerLevel[];
extern u16 *gCurrAnimAttribute;
extern s32 gFindFloorForCutsceneStar;
extern s32 gWdwWaterLevelSet;
extern s32 sNumActiveFirePiranhaPlants;
extern s32 sNumKilledFirePiranhaPlants;
extern f32 gDialogBoxOpenTimer;
extern f32 gDialogBoxScale;
extern const Gfx NULL_dl[];
extern const BehaviorScript *sWarpBhvSpawnTable[];
extern struct PlayerGeometry sMarioGeometry;
extern struct PlayerCameraState *sMarioCamState;

// SM64 functions
extern s32 analog_stick_held_back(struct MarioState *m);
extern s32 apply_slope_decel(struct MarioState *m, f32);
extern s32 begin_braking_action(struct MarioState *m);
extern s32 check_clicked_button(s16 x, s16 y, f32 depth);
extern s32 check_ground_dive_or_punch(struct MarioState *m);
extern s32 check_horizontal_wind(struct MarioState *m);
extern s32 lava_boost_on_wall(struct MarioState *);
extern s32 perform_water_step(struct MarioState *m);
extern s32 should_begin_sliding(struct MarioState *m);
extern u32 common_air_action_step(struct MarioState *, u32, s32, u32);
extern u32 determine_interaction(struct MarioState *m, struct Object *o);
extern u32 interact_cap(struct MarioState *m, UNUSED u32 interactType, struct Object *o);
extern u32 interact_coin(struct MarioState *m, UNUSED u32 interactType, struct Object *o);
extern u32 interact_warp(struct MarioState *m, UNUSED u32 interactType, struct Object *o);
extern f32 get_buoyancy(struct MarioState *m);
extern f32 water_ring_calc_mario_dist(void);
extern bool cur_obj_update_behavior_func(void (*func)(void));
extern void add_tree_leaf_particles(struct MarioState *m);
extern void apply_slope_accel(struct MarioState *m);
extern void bhv_coin_fix_step(struct Object *o, s32 update);
extern void bhv_chain_chomp_update_chain_parts(struct Object *o, bool isFreed);
extern void change_and_flash_dialog_text_color_lines(s8 colorMode, s8 lineNum);
extern void check_lava_boost(struct MarioState *m);
extern void check_ledge_climb_down(struct MarioState *m);
extern void create_dl_identity_matrix(void);
extern void create_dl_translation_matrix(s8 pushOp, f32 x, f32 y, f32 z);
extern void create_dl_rotation_matrix(s8 pushOp, f32 a, f32 x, f32 y, f32 z);
extern void create_dl_scale_matrix(s8 pushOp, f32 x, f32 y, f32 z);
extern void cutscene_ending_look_at_sky(struct Camera *);
extern void cutscene_ending_mario_fall(struct Camera *);
extern void cutscene_ending_mario_land(struct Camera *);
extern void cutscene_ending_mario_land_closeup(struct Camera *);
extern void cutscene_ending_reset_spline(struct Camera *);
extern void cutscene_ending_stars_free_peach(struct Camera *);
extern void cutscene_ending_stop(struct Camera *);
extern void cutscene_ending_zoom_fov(struct Camera *);
extern void disable_background_sound();
extern void enable_background_sound();
extern void func_8031D838(s32 player, s32 fadeInTime, u8 targetVolume);
extern void handle_dialog_scroll_page_state(s8 lineNum, s8 totalLines, s8 *pageState, s8 *xMatrix, s16 *linePos);
extern void handle_power_meter_actions(s16);
extern void handle_special_dialog_text(s16);
extern void initiate_warp(s16, s16, s16, s32);
extern void load_object_collision_model(void);
extern void mario_reset_bodystate(struct MarioState *m);
extern void mario_update_hitbox_and_cap_model(struct MarioState *m);
extern void play_far_fall_sound(struct MarioState *m);
extern void play_flip_sounds(struct MarioState *m, s16 frame1, s16 frame2, s16 frame3);
extern void play_sequence(u8, u8, u16);
extern void print_main_menu_strings();
extern void print_score_menu_strings();
extern void print_copy_menu_strings();
extern void print_erase_menu_strings();
extern void print_save_file_scores(s8);
extern void print_sound_mode_menu_strings();
extern void print_peach_letter_message(void);
extern void push_mario_out_of_object(struct MarioState *, struct Object *, f32);
extern void push_or_sidle_wall(struct MarioState *m, Vec3f startPos);
extern void render_dialog_box_type(struct DialogEntry *, s8);
extern void render_dialog_entries(void);
extern void render_dialog_triangle_choice(void);
extern void render_dialog_triangle_page(s8);
extern void render_dialog_triangle_next(s8);
extern void render_generic_char(u8 c);
extern void render_hud_camera_status();
extern void render_multi_text_string_lines(s8 multiTextId, s8 lineNum, s16 *linePos, s8 linesPerBox, s8 xMatrix, s8 lowerBound);
extern void render_pause_my_score_coins();
extern void render_pause_red_coins();
extern void render_star_count_dialog_text(s8 *xMatrix, s16 *linePos);
extern void set_mario_initial_action(struct MarioState *, u32, u32);
extern void set_play_mode(s16);
extern void set_submerged_cam_preset_and_spawn_bubbles(struct MarioState *m);
extern void shade_screen();
extern void sink_mario_in_quicksand(struct MarioState *m);
extern void spawn_particle(u32 activeParticleFlag, s16 model, const BehaviorScript *behavior);
extern void squish_mario_model(struct MarioState *m);
extern void tilt_body_walking(struct MarioState *m, s16 startYaw);
extern void update_lakitu(struct Camera *c);
extern void update_mario_health(struct MarioState *);
extern void update_mario_info_for_cam(struct MarioState *m);
extern void update_mario_inputs(struct MarioState *m);
extern void update_walking_speed(struct MarioState *m);
extern void warp_special(s32 arg);
extern struct Surface *get_pseudo_floor_at_pos(f32 x, f32 y, f32 z);

#endif // OMM_INCLUDES_H
