#ifndef OMM_MACROS_H
#define OMM_MACROS_H

//
// Versions (auto-detected by omm.mk)
// SMEX : sm64ex-nightly
// SMMS : sm64ex-nightly + Moonshine
// R96A : Render96ex-alpha
// XALO : sm64ex-alo
// SM74 : Super Mario 74
// SMSR : Super Mario Star Road
//

#if   defined(SMEX)
#define DEF(smex, smms, r96a, xalo, sm74, smsr) smex
#elif defined(SMMS)
#define DEF(smex, smms, r96a, xalo, sm74, smsr) smms
#elif defined(R96A)
#define DEF(smex, smms, r96a, xalo, sm74, smsr) r96a
#elif defined(XALO)
#define DEF(smex, smms, r96a, xalo, sm74, smsr) xalo
#elif defined(SM74)
#define DEF(smex, smms, r96a, xalo, sm74, smsr) sm74
#elif defined(SMSR)
#define DEF(smex, smms, r96a, xalo, sm74, smsr) smsr
#else
#error "No version defined!"
#endif

//
// OMM macros
// Depending on which repo/conversion/hack we are building the game,
// we need to enable/disable some code for OMM to work properly.
// The four following macros can be passed to the make command with values 0 or 1.
//

// OMM_VANILLA | Enables code to enhance Vanilla SM64 (world objects, OMM camera presets, some QoL, and speedrun support)
#if defined(OMM_VANILLA)
#define OMM_CODE_VANILLA        DEF(OMM_VANILLA, 0, OMM_VANILLA, OMM_VANILLA, 0, 0)
#else
#define OMM_CODE_VANILLA        DEF(1, 0, 1, 1, 0, 0)
#endif

// OMM_BOWSER | Replaces Vanilla Bowser with OMM Bowser
#if defined(OMM_BOWSER)
#define OMM_CODE_BOWSER         DEF(OMM_BOWSER, OMM_BOWSER, OMM_BOWSER, OMM_BOWSER, OMM_BOWSER, OMM_BOWSER)
#else
#define OMM_CODE_BOWSER         DEF(1, 0, 1, 1, 0, 0)
#endif

// OMM_SPARKLY | Enables the post-game content (Sparkly Stars and alt endings)
#if defined(OMM_SPARKLY)
#define OMM_CODE_SPARKLY        DEF(OMM_SPARKLY, 0, OMM_SPARKLY, OMM_SPARKLY, 0, 0)
#else
#define OMM_CODE_SPARKLY        DEF(1, 0, 1, 1, 0, 0)
#endif

// OMM_DEBUG | Enables some debug stuff
#if defined(OMM_DEBUG)
#define OMM_CODE_DEBUG          DEF(OMM_DEBUG, OMM_DEBUG, OMM_DEBUG, OMM_DEBUG, OMM_DEBUG, OMM_DEBUG)
#else
#define OMM_CODE_DEBUG          DEF(0, 0, 0, 0, 0, 0)
#endif

// EXTERNAL_DATA | If not set, enables STBI implementation in file omm_gfx.c
#if defined(EXTERNAL_DATA)
#define OMM_STBI_IMPL           DEF(0, 0, 0, 0, 0, 0)
#else
#define OMM_STBI_IMPL           DEF(1, 1, 0, 1, 1, 1)
#endif

// EXT_OPTIONS_MENU | If set, enables options types implementation in file omm_options.c
#if defined(EXT_OPTIONS_MENU)
#define OMM_OPT_TYPES_IMPL      DEF(1, 1, 1, 0, 0, 0)
#else
#define OMM_OPT_TYPES_IMPL      DEF(0, 0, 0, 0, 0, 0)
#endif

//
// Macros
//

// Better camera
#define BETTER_CAM_IS_PUPPY_CAM         DEF(0, 0, 0, 1, 0, 1)
#define BETTER_CAM_IS_ENABLED           DEF(newcam_active, newcam_active, newcam_active, gPuppyCam.enabled, gPuppyCam.enabled, gPuppyCam.enabled)
#define BETTER_CAM_MODE                 DEF(CAMERA_MODE_NEWCAM, CAMERA_MODE_NEWCAM, CAMERA_MODE_NEWCAM, 0, 0, 0)
#define BETTER_CAM_YAW                  DEF(newcam_yaw, newcam_yaw, newcam_yaw, 0, 0, 0)
#define BETTER_CAM_RAYCAST_ARGS         DEF(, , , __EXPAND(, UNUSED s32 flags), __EXPAND(, UNUSED s32 flags), __EXPAND(, UNUSED s32 flags))

// Animation
#define AnimInfoStruct                  DEF(GraphNodeObject_sub, GraphNodeObject_sub, GraphNodeObject_sub, AnimInfo, AnimInfo, AnimInfo)
#define mAreaIndex                      DEF(unk18, unk18, unk18, areaIndex, areaIndex, areaIndex)
#define mActiveAreaIndex                DEF(unk19, unk19, unk19, activeAreaIndex, activeAreaIndex, activeAreaIndex)
#define mAnimInfo                       DEF(unk38, unk38, curAnim, animInfo, animInfo, animInfo)
#define mAnimYTransDivisor              DEF(unk02, unk02, unk02, animYTransDivisor, animYTransDivisor, animYTransDivisor)
#define mStartFrame                     DEF(unk04, unk04, unk04, startFrame, startFrame, startFrame)
#define mLoopStart                      DEF(unk06, unk06, unk06, loopStart, loopStart, loopStart)
#define mLoopEnd                        DEF(unk08, unk08, unk08, loopEnd, loopEnd, loopEnd)
#define gCurrentAnimType                DEF(gCurAnimType, gCurAnimType, gCurAnimType, gCurrAnimType, gCurrAnimType, gCurrAnimType)
#define gCurrentAnimData                DEF(gCurAnimData, gCurAnimData, gCurAnimData, gCurrAnimData, gCurrAnimData, gCurrAnimData)

// Audio
#define gGlobalSoundArgs                DEF(gDefaultSoundArgs, gDefaultSoundArgs, gDefaultSoundArgs, gGlobalSoundSource, gGlobalSoundSource, gGlobalSoundSource)
#define sAcousticReachPerLevel          DEF(D_80332028, D_80332028, D_80332028, sLevelAcousticReaches, sLevelAcousticReaches, sLevelAcousticReaches)
#define music_fade_out                  DEF(sequence_player_fade_out, sequence_player_fade_out, sequence_player_fade_out, seq_player_fade_out, seq_player_fade_out, seq_player_fade_out)
#define music_lower_volume              DEF(func_8031FFB4, func_8031FFB4, func_8031FFB4, seq_player_lower_volume, seq_player_lower_volume, seq_player_lower_volume)
#define music_unlower_volume            DEF(sequence_player_unlower, sequence_player_unlower, sequence_player_unlower, seq_player_unlower_volume, seq_player_unlower_volume, seq_player_unlower_volume)
#define music_play_sequence_define      DEF(0, 0, 0, 1, 1, 1)
#define audio_mute                      DEF(set_sound_disabled, set_sound_disabled, set_sound_disabled, set_audio_muted, set_audio_muted, set_audio_muted)
#define sound_stop                      DEF(func_803205E8, func_803205E8, func_803205E8, stop_sound, stop_sound, stop_sound)
#define sound_stop_from_source          DEF(func_803206F8, func_803206F8, func_803206F8, stop_sounds_from_source, stop_sounds_from_source, stop_sounds_from_source)
#define sound_stop_in_continuous_banks  DEF(func_80320890, func_80320890, func_80320890, stop_sounds_in_continuous_banks, stop_sounds_in_continuous_banks, stop_sounds_in_continuous_banks)
#define sound_set_moving_speed          DEF(func_80320A4C, func_80320A4C, func_80320A4C, set_sound_moving_speed, set_sound_moving_speed, set_sound_moving_speed)
#define SOUND_OBJ_WHOMP_SLAM            DEF(SOUND_OBJ_WHOMP_LOWPRIO, SOUND_OBJ_WHOMP_LOWPRIO, SOUND_OBJ_WHOMP_LOWPRIO, SOUND_OBJ_WHOMP, SOUND_OBJ_WHOMP, SOUND_OBJ_WHOMP)

// Object fields
#define oBitsPlatformBowserObject       DEF(oPlatformUnkF8, oPlatformUnkF8, oPlatformUnkF8, oBitsPlatformBowser, oBitsPlatformBowser, oBitsPlatformBowser)
#define oBowserRainbowLightEffect       DEF(oBowserUnk1B2, oBowserUnk1B2, oBowserUnk1B2, oBowserRainbowLight, oBowserRainbowLight, oBowserRainbowLight)
#define oSnowmansBodyScale              DEF(oSnowmansBottomUnkF4, oSnowmansBottomUnkF4, oSnowmansBottomUnkF4, oSnowmansBottomScale, oSnowmansBottomScale, oSnowmansBottomScale)

// Misc
#define GodModeCheat                    DEF(Cheats.GodMode, Cheats.GodMode, Cheats.GodMode, Cheats.InfiniteHealth, Cheats.InfiniteHealth, Cheats.InfiniteHealth)
#define sDemoCounter                    DEF(gDemoCountdown, gDemoCountdown, gDemoCountdown, sDemoCountdown, sDemoCountdown, sDemoCountdown)
#define level_intro_goddard             DEF(level_intro_entry_2, level_intro_entry_2, level_intro_entry_2, level_intro_mario_head_regular, level_intro_mario_head_regular, level_intro_mario_head_regular)
#define find_static_floor               DEF(find_floor, find_floor, find_floor, find_floor, find_floor, find_floor)
#define find_dynamic_floor              DEF(find_floor, find_floor, find_floor, find_floor, find_floor, find_floor)
#define render_dialog_triangle_page     DEF(render_dialog_string_color, render_dialog_string_color, render_dialog_string_color, render_dialog_triangle_next, render_dialog_triangle_next, render_dialog_triangle_next)
#define load_gfx_memory_pool            DEF(config_gfx_pool, config_gfx_pool, config_gfx_pool, select_gfx_pool, select_gfx_pool, select_gfx_pool)
#define init_scene_rendering            DEF(init_render_image, init_render_image, init_render_image, init_rcp, init_rcp, init_rcp)
#define spawnInfoModel                  DEF(unk18, unk18, unk18, model, model, model)
#define gIsHardMode                     DEF(gOmmHardMode, gOmmHardMode, gOmmHardMode, gOmmHardMode, gOmmHardMode, gStarRoadHardMode)

// OMM
#define OMM_LEVEL_ENTRY_WARP(level)     DEF(0x0A, 0x0A, 0x0A, 0x0A, 0x0A, (level == LEVEL_JRB ? 0x01 : 0x0A))
#define OMM_LEVEL_EXIT_DISTANCE         DEF(500.f, 500.f, 500.f, 500.f, 150.f, 500.f)
#define OMM_LEVEL_SLIDE                 DEF(LEVEL_PSS, LEVEL_PSS, LEVEL_PSS, LEVEL_PSS, LEVEL_PSS, LEVEL_SA)
#define OMM_CAMERA_LOOK_UP_WARP_STARS   DEF(10, 10, 10, 10, 10, 120)
#define OMM_CAMERA_IS_BOWSER_FIGHT      DEF(omm_camera_is_bowser_fight(), omm_camera_is_bowser_fight(), omm_camera_is_bowser_fight(), omm_camera_is_bowser_fight(), omm_camera_is_bowser_fight(), gCurrLevelNum == LEVEL_BOWSER_3)
#define OMM_TEXT_(id, str)              static const char OMM_TEXT_##id[] = \
                                        DEF(str, str, "OMM_TEXT_" #id, str, str, str);
#define OMM_NUM_PLAYABLE_CHARACTERS     DEF(2, 2, 4, 2, 2, 2)
#define STAR                            DEF("STAR", "MOON", "STAR", "STAR", "STAR", "STAR")
#define Star                            DEF("Star", "Moon", "Star", "Star", "Star", "Star")

//
// Surface collision redefines
// OMM has a more robust surface load and collision support,
// but it needs to redefine some macros
//

#include "surface_terrains.h"
#undef COL_INIT
#undef COL_VERTEX_INIT
#undef COL_VERTEX
#undef COL_TRI_INIT
#undef COL_TRI
#undef COL_TRI_SPECIAL
#undef COL_TRI_STOP
#undef COL_END
#undef COL_SPECIAL_INIT
#undef COL_WATER_BOX_INIT
#undef COL_WATER_BOX
#define CMD_COL_INIT                            0x0000
#define CMD_COL_VERTEX_INIT                     0x0001
#define CMD_COL_VERTEX                          0x0002
#define CMD_COL_TRI_INIT                        0x0003
#define CMD_COL_TRI                             0x0004
#define CMD_COL_TRI_SPECIAL                     0x0005
#define CMD_COL_TRI_STOP                        0x0006
#define CMD_COL_END                             0x0007
#define CMD_COL_SPECIAL_INIT                    0x0008
#define CMD_COL_WATER_BOX_INIT                  0x0009
#define CMD_COL_END_BYTE_SEQUENCE               0x8708, 0x9749, 0x6E13, 0xA3B6, 0x83F8, 0x7245, 0x575D, 0xB1AA
#define COL_INIT()                              CMD_COL_INIT
#define COL_VERTEX_INIT(vtxNum)                 CMD_COL_VERTEX_INIT, vtxNum
#define COL_VERTEX(x, y, z)                     CMD_COL_VERTEX, x, y, z
#define COL_TRI_INIT(surfType, triNum)          CMD_COL_TRI_INIT, surfType, triNum
#define COL_TRI(v1, v2, v3)                     CMD_COL_TRI, v1, v2, v3
#define COL_TRI_SPECIAL(v1, v2, v3, param)      CMD_COL_TRI_SPECIAL, v1, v2, v3, param
#define COL_TRI_STOP()                          CMD_COL_TRI_STOP
#define COL_END()                               CMD_COL_END, CMD_COL_END_BYTE_SEQUENCE
#define COL_SPECIAL_INIT(num)                   CMD_COL_SPECIAL_INIT, num
#define COL_WATER_BOX_INIT(num)                 CMD_COL_WATER_BOX_INIT, num
#define COL_WATER_BOX(id, x1, z1, x2, z2, y)    id, x1, z1, x2, z2, y

//
// QoL redefines
// Some of them can bug OMM
// Some of them are already fixed
// Some of them are just annoying
// Some of them must be disabled
//

#include "config.h"
#if defined(QOL_FEATURES)
#include "qol_defines.h"
#endif
#undef USE_SYSTEM_MALLOC
#undef Z_TRIG_EXTRA_ACT
#undef QOL_FEATURE_Z_BUTTON_EXTRA_OPTION
#undef QOL_FEATURE_PROPER_SHOW_COLLECTABLE
#undef QOL_FEATURE_RESPAWN_BLUE_COIN_SWITCH
#undef QOL_FEATURE_BETTER_WF_BREAKEABLE_WALL
#undef QOL_FEATURE_BOUNCE_BOOST
#undef QOL_FEATURE_TINY_GOOMBA_DROP_COIN
#undef QOL_FIX_WATER_RING
#undef QOL_FIX_GROUND_POUND_WALL
#undef QOL_FIX_LAVA_INTERACTION
#define Z_TRIG_EXTRA_ACT 0
#define QOL_FEATURE_Z_BUTTON_EXTRA_OPTION 0
#define QOL_FEATURE_PROPER_SHOW_COLLECTABLE 0
#define QOL_FEATURE_RESPAWN_BLUE_COIN_SWITCH 0
#define QOL_FEATURE_BETTER_WF_BREAKEABLE_WALL 0
#define QOL_FEATURE_BOUNCE_BOOST 0
#define QOL_FEATURE_TINY_GOOMBA_DROP_COIN 0
#define QOL_FIX_WATER_RING 0
#define QOL_FIX_GROUND_POUND_WALL 0
#define QOL_FIX_LAVA_INTERACTION 0

//
// Extended bounds redefines
// OMM has built-in extended bounds
// Most of these "features" are unecessary and can break OMM
//

#if defined(EXT_BOUNDS)
#include "engine/extended_bounds.h"
#undef EXTENDED_BOUNDS_MODE
#undef MAX_REFEREMCED_WALLS
#undef CELL_HEIGHT_LIMIT
#undef FLOOR_LOWER_LIMIT
#undef FLOOR_LOWER_LIMIT_MISC
#undef FLOOR_LOWER_LIMIT_SHADOW
#undef SURFACE_POOLS_FULL_MESSAGES
#undef NEW_WATER_SURFACES
#undef BETTER_WALL_COLLISION
#undef CEILING_MARGINS
#undef FIND_FLOOR_EXCLUDE_DYNAMIC_CHECK
#undef FIX_LEDGE_GRABS
#undef CENTERED_COLLISION
#undef UNDERWATER_STEEP_FLOORS_AS_WALLS
#undef MIN_UNDERWATER_FLOOR_NORMAL_Y
#undef MIN_FLOOR_NORMAL_Y
#undef MAX_CEIL_NORMAL_Y
#undef NULL_FLOOR_STEPS
#undef WATER_NUM_STEPS
#undef GROUND_NUM_STEPS
#undef AIR_NUM_STEPS
#undef LEVEL_BOUNDARY_MAX
#undef WORLD_SCALE
#undef CELL_SIZE
#undef NUM_CELLS
#undef NUM_CELLS_INDEX
#define EXTENDED_BOUNDS_H
#define MAX_REFEREMCED_WALLS        4
#define CELL_HEIGHT_LIMIT           20000
#define FLOOR_LOWER_LIMIT           -11000
#define FLOOR_LOWER_LIMIT_MISC      (FLOOR_LOWER_LIMIT + 1000)
#define FLOOR_LOWER_LIMIT_SHADOW    FLOOR_LOWER_LIMIT_MISC
#define LEVEL_BOUNDARY_MAX          OMM_COLLISION_LEVEL_BOUNDARY
#define CELL_SIZE                   OMM_COLLISION_CELL_SIZE
#define NUM_CELLS                   OMM_COLLISION_CELL_COUNT
#define NUM_CELLS_INDEX             OMM_COLLISION_CELL_BITS
#endif

//
// DynOS defines
// OMM has new models that must be added to DynOS actors list
//

#if defined(DYNOS) && defined(DYNOS_CPP_H)
#include "data/omm/object/omm_object_data.h"
#define OMM_DYNOS_ACTORS \
define_actor(omm_geo_star_0_opaque), \
define_actor(omm_geo_star_1_opaque), \
define_actor(omm_geo_star_2_opaque), \
define_actor(omm_geo_star_3_opaque), \
define_actor(omm_geo_star_4_opaque), \
define_actor(omm_geo_star_5_opaque), \
define_actor(omm_geo_star_6_opaque), \
define_actor(omm_geo_star_7_opaque), \
define_actor(omm_geo_star_8_opaque), \
define_actor(omm_geo_star_9_opaque), \
define_actor(omm_geo_star_10_opaque), \
define_actor(omm_geo_star_11_opaque), \
define_actor(omm_geo_star_12_opaque), \
define_actor(omm_geo_star_13_opaque), \
define_actor(omm_geo_star_14_opaque), \
define_actor(omm_geo_star_15_opaque), \
define_actor(omm_geo_star_16_opaque), \
define_actor(omm_geo_star_17_opaque), \
define_actor(omm_geo_star_18_opaque), \
define_actor(omm_geo_star_19_opaque), \
define_actor(omm_geo_star_0_transparent), \
define_actor(omm_geo_star_1_transparent), \
define_actor(omm_geo_star_2_transparent), \
define_actor(omm_geo_star_3_transparent), \
define_actor(omm_geo_star_4_transparent), \
define_actor(omm_geo_star_5_transparent), \
define_actor(omm_geo_star_6_transparent), \
define_actor(omm_geo_star_7_transparent), \
define_actor(omm_geo_star_8_transparent), \
define_actor(omm_geo_star_9_transparent), \
define_actor(omm_geo_star_10_transparent), \
define_actor(omm_geo_star_11_transparent), \
define_actor(omm_geo_star_12_transparent), \
define_actor(omm_geo_star_13_transparent), \
define_actor(omm_geo_star_14_transparent), \
define_actor(omm_geo_star_15_transparent), \
define_actor(omm_geo_star_16_transparent), \
define_actor(omm_geo_star_17_transparent), \
define_actor(omm_geo_star_18_transparent), \
define_actor(omm_geo_star_19_transparent), \
define_actor(omm_geo_fire_smoke_red), \
define_actor(omm_geo_fire_smoke_blue), \
define_actor(omm_geo_metal_sparkle), \
define_actor(omm_geo_vanish_mist), \
define_actor(omm_geo_break_particle), \
define_actor(omm_geo_snowball), \
define_actor(omm_geo_mr_i_beam), \
define_actor(omm_geo_explosion), \
define_actor(omm_geo_shockwave_whomp), \
define_actor(omm_geo_shockwave_spindrift), \
define_actor(omm_geo_shockwave_fire), \
define_actor(omm_geo_rising_lava), \
define_actor(omm_geo_bowser_mine), \
define_actor(omm_geo_bowser_flame), \
define_actor(omm_geo_bowser_fireball), \
define_actor(omm_geo_bowser_fireball_flame), \
define_actor(omm_geo_flaming_bobomb), \
define_actor(omm_geo_flaming_bobomb_aura), \
define_actor(omm_geo_flaming_bobomb_explosion), \
define_actor(omm_geo_sparkly_star_1_opaque), \
define_actor(omm_geo_sparkly_star_1_transparent), \
define_actor(omm_geo_sparkly_star_1_sparkle), \
define_actor(omm_geo_sparkly_star_1_block), \
define_actor(omm_geo_sparkly_star_1_hint), \
define_actor(omm_geo_sparkly_star_2_opaque), \
define_actor(omm_geo_sparkly_star_2_transparent), \
define_actor(omm_geo_sparkly_star_2_sparkle), \
define_actor(omm_geo_sparkly_star_2_block), \
define_actor(omm_geo_sparkly_star_2_hint), \
define_actor(omm_geo_sparkly_star_3_opaque), \
define_actor(omm_geo_sparkly_star_3_transparent), \
define_actor(omm_geo_sparkly_star_3_sparkle), \
define_actor(omm_geo_sparkly_star_3_block), \
define_actor(omm_geo_sparkly_star_3_hint), \
define_actor(omm_geo_peach), \
define_actor(omm_geo_peach_crown), \
define_actor(omm_geo_peach_crown_metal), \
define_actor(omm_geo_perry), \
define_actor(omm_geo_perry_trail), \
define_actor(omm_geo_perry_shockwave), \
define_actor(omm_geo_peach_vibe_aura), \
define_actor(omm_geo_peach_vibe_sparkle), \
define_actor(omm_geo_peach_vibe_joy_tornado), \
define_actor(omm_geo_peach_vibe_joy_gust), \
define_actor(omm_geo_peach_vibe_rage_aura), \
define_actor(omm_geo_peach_vibe_rage_shockwave), \
define_actor(omm_geo_peach_vibe_gloom_aura), \
define_actor(omm_geo_peach_vibe_gloom_tear), \
define_actor(omm_geo_peach_vibe_gloom_tear_small), \
define_actor(omm_geo_peach_vibe_calm_aura), \
define_actor(omm_geo_peach_vibe_calm_sparkle), \
define_actor(omm_geo_menu_button_mario), \
define_actor(omm_geo_menu_button_peach), \
define_actor(omm_geo_menu_button_luigi), \
define_actor(omm_geo_menu_button_wario), \
define_actor(omm_geo_problem)
#endif

//
// Some useful macros
// (don't use the macros starting with underscores)
//

// Count the number of args inside a variadic macro (up to 8)
#ifdef _MSC_VER
#define N_ARGS(...) __EXPAND_ARGS(__ARGS_AUG(__VA_ARGS__))
#define __ARGS_AUG(...) unused, __VA_ARGS__
#define __EXPAND(...) __VA_ARGS__
#define __EXPAND_ARGS(...) __EXPAND(__COUNT_ARGS(__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1, 0))
#define __COUNT_ARGS(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, count, ...) count
#else
#define N_ARGS(...) __COUNT_ARGS(0, ##__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define __EXPAND(...) __VA_ARGS__
#define __COUNT_ARGS(_0_, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, count, ...) count
#endif

// Turn things into a string
#undef __STRINGIFY
#undef STRINGIFY
#define __STRINGIFY(...) #__VA_ARGS__
#define STRINGIFY(...) __STRINGIFY(__VA_ARGS__)

// Concatenate two macros
#define __CAT(A, B) A##B
#define CAT(A, B) __CAT(A, B)

//
// Button masks and binds
// (Note to future self: this code needs to
//  be here, don't move it to omm_patches.h)
//

#define X_BUTTON 0x0040
#define Y_BUTTON 0x0080

#define OMM_ADD_BIND(type, cont, var) extern u32 var[]; type##_add_binds(cont, var)
#define OMM_ADD_BINDS(type)                                         \
        OMM_ADD_BIND(type, A_BUTTON, gOmmControlsButtonA);          \
        OMM_ADD_BIND(type, B_BUTTON, gOmmControlsButtonB);          \
        OMM_ADD_BIND(type, X_BUTTON, gOmmControlsButtonX);          \
        OMM_ADD_BIND(type, Y_BUTTON, gOmmControlsButtonY);          \
        OMM_ADD_BIND(type, START_BUTTON, gOmmControlsButtonStart);  \
        OMM_ADD_BIND(type, L_TRIG, gOmmControlsTriggerL);           \
        OMM_ADD_BIND(type, R_TRIG, gOmmControlsTriggerR);           \
        OMM_ADD_BIND(type, Z_TRIG, gOmmControlsTriggerZ);           \
        OMM_ADD_BIND(type, U_CBUTTONS, gOmmControlsCUp);            \
        OMM_ADD_BIND(type, D_CBUTTONS, gOmmControlsCDown);          \
        OMM_ADD_BIND(type, L_CBUTTONS, gOmmControlsCLeft);          \
        OMM_ADD_BIND(type, R_CBUTTONS, gOmmControlsCRight);         \
        OMM_ADD_BIND(type, U_JPAD, gOmmControlsDUp);                \
        OMM_ADD_BIND(type, D_JPAD, gOmmControlsDDown);              \
        OMM_ADD_BIND(type, L_JPAD, gOmmControlsDLeft);              \
        OMM_ADD_BIND(type, R_JPAD, gOmmControlsDRight);             \
        OMM_ADD_BIND(type, STICK_UP, gOmmControlsStickUp);          \
        OMM_ADD_BIND(type, STICK_DOWN, gOmmControlsStickDown);      \
        OMM_ADD_BIND(type, STICK_LEFT, gOmmControlsStickLeft);      \
        OMM_ADD_BIND(type, STICK_RIGHT, gOmmControlsStickRight);
        
#define keyboard_bindkeys_0 keyboard_bindkeys()
#define keyboard_bindkeys_1 keyboard_bindkeys(void) { \
    bzero(keyboard_mapping, sizeof(keyboard_mapping)); \
    num_keybinds = 0; \
    OMM_ADD_BINDS(keyboard); \
 } void keyboard_bindkeys_(void)
#define keyboard_bindkeys(...) CAT(keyboard_bindkeys_, N_ARGS(__VA_ARGS__))

#define controller_sdl_bind_0 controller_sdl_bind()
#define controller_sdl_bind_1 controller_sdl_bind(void) { \
    bzero(joy_binds, sizeof(joy_binds)); \
    bzero(mouse_binds, sizeof(mouse_binds)); \
    num_joy_binds = 0; \
    num_mouse_binds = 0; \
    OMM_ADD_BINDS(controller); \
 } void controller_sdl_bind_(void)
#define controller_sdl_bind(...) CAT(controller_sdl_bind_, N_ARGS(__VA_ARGS__))

#endif
