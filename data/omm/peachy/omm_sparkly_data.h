#ifndef OMM_SPARKLY_DATA_H
#define OMM_SPARKLY_DATA_H

#include "data/omm/omm_includes.h"

//
// Shortcuts
//

#define omm_sparkly_ending                              gOmmData->mario->sparkly.ending
#define omm_sparkly_cheats                              gOmmData->mario->sparkly.cheats
#define omm_sparkly_flags_get(flag)                     OMM_GET_FLAG(gOmmData->mario->sparkly.flags, flag)
#define omm_sparkly_flags_set(flag, value)              OMM_SET_FLAG(gOmmData->mario->sparkly.flags, flag, value)

//
// Lists
//

#define OMM_SPARKLY_STAR_OPAQUE_GEO                     (OMM_ARRAY_OF(const GeoLayout *) { NULL, omm_geo_sparkly_star_1_opaque, omm_geo_sparkly_star_2_opaque, omm_geo_sparkly_star_3_opaque })
#define OMM_SPARKLY_STAR_TRANSPARENT_GEO                (OMM_ARRAY_OF(const GeoLayout *) { NULL, omm_geo_sparkly_star_1_transparent, omm_geo_sparkly_star_2_transparent, omm_geo_sparkly_star_3_transparent })
#define OMM_SPARKLY_STAR_SPARKLE_GEO                    (OMM_ARRAY_OF(const GeoLayout *) { NULL, omm_geo_sparkly_star_1_sparkle, omm_geo_sparkly_star_2_sparkle, omm_geo_sparkly_star_3_sparkle })
#define OMM_SPARKLY_STAR_HINT_GEO                       (OMM_ARRAY_OF(const GeoLayout *) { NULL, omm_geo_sparkly_star_1_hint, omm_geo_sparkly_star_2_hint, omm_geo_sparkly_star_3_hint })
#define OMM_SPARKLY_STAR_BLOCK_GEO                      (OMM_ARRAY_OF(const GeoLayout *) { NULL, omm_geo_sparkly_star_1_block, omm_geo_sparkly_star_2_block, omm_geo_sparkly_star_3_block })
#define OMM_SPARKLY_STAR_BLOCK_BHV                      (OMM_ARRAY_OF(const BehaviorScript *) { NULL, omm_bhv_sparkly_star_1_block, omm_bhv_sparkly_star_2_block, omm_bhv_sparkly_star_3_block })
#define OMM_SPARKLY_STAR_BLOCK_POS                      (OMM_ARRAY_OF(Vec3f) { { 0, 0, 0 }, { -360, 1120, 480 }, { 0, 1120, 480 }, { +360, 1120, 480 } })
#define OMM_SPARKLY_STAR_TEXT                           (OMM_ARRAY_OF(const char *) { NULL, OMM_TEXT_SPARKLY_STAR_1, OMM_TEXT_SPARKLY_STAR_2, OMM_TEXT_SPARKLY_STAR_3 })
#define OMM_SPARKLY_STARS_TEXT                          (OMM_ARRAY_OF(const char *) { NULL, OMM_TEXT_SPARKLY_STARS_1, OMM_TEXT_SPARKLY_STARS_2, OMM_TEXT_SPARKLY_STARS_3 })
#define OMM_SPARKLY_STAR_HUD_GLYPH                      (OMM_ARRAY_OF(const char *) { NULL, OMM_TEXTURE_STAR_FULL_17, OMM_TEXTURE_STAR_FULL_18, OMM_TEXTURE_STAR_FULL_19 })
#define OMM_SPARKLY_STAR_HUD_COLOR                      (OMM_ARRAY_OF(Vec3s) { { 0xFF, 0xFF, 0xFF }, { 0xFF, 0xE0, 0x80 }, { 0xC0, 0xE0, 0xFF }, { 0xFF, 0x40, 0x80 } })
#define OMM_SPARKLY_BOWSER_4_SHOULD_UNLOCK              (OMM_ARRAY_OF(bool) { false, true, true, false })
#define OMM_SPARKLY_BOWSER_4_DIALOG_UNLOCKED            (OMM_ARRAY_OF(s16) { 0, OMM_DIALOG_SPARKLY_STAR_1_BOWSER_4_UNLOCKED, OMM_DIALOG_SPARKLY_STAR_2_BOWSER_4_UNLOCKED, 0 })
#define OMM_SPARKLY_BOWSER_4_DIALOG_INTRO               (OMM_ARRAY_OF(s16) { 0, OMM_DIALOG_SPARKLY_STAR_1_BOWSER_4_INTRO, OMM_DIALOG_SPARKLY_STAR_2_BOWSER_4_INTRO, 0 })
#define OMM_SPARKLY_BOWSER_4_DIALOG_DEFEAT              (OMM_ARRAY_OF(s16) { 0, OMM_DIALOG_SPARKLY_STAR_1_BOWSER_4_DEFEAT, OMM_DIALOG_SPARKLY_STAR_2_BOWSER_4_DEFEAT, 0 })

//
// Modes
//

#define OMM_SPARKLY_MODE_DISABLED                       0
#define OMM_SPARKLY_MODE_NORMAL                         1
#define OMM_SPARKLY_MODE_HARD                           2
#define OMM_SPARKLY_MODE_LUNATIC                        3
#define OMM_SPARKLY_MODE_COUNT                          3

//
// Endings
//

#define OMM_SPARKLY_ENDING_REGULAR                      0
#define OMM_SPARKLY_ENDING_BAD                          1
#define OMM_SPARKLY_ENDING_PEACH                        2

//
// Flags
//

#define OMM_SPARKLY_FLAG_STAR_BLOCK                     (1 << 0)
#define OMM_SPARKLY_FLAG_GRAND_STAR                     (1 << 1)
#define OMM_SPARKLY_FLAG_GAME_PAUSED                    (1 << 2)
#define OMM_SPARKLY_FLAG_TIME_STOP                      (1 << 3)
#define OMM_SPARKLY_FLAG_TRANSITION                     (1 << 4)
#define OMM_SPARKLY_FLAG_MARIO_UPDATED                  (1 << 5)
#define OMM_SPARKLY_FLAG_CHEAT_DETECTED                 (1 << 6)
#define OMM_SPARKLY_FLAG_NON_STOP_MODE                  (1 << 7)

//
// Some constants
//

#define OMM_SPARKLY_BOWSER_4_WARP_CENTER_X              0
#define OMM_SPARKLY_BOWSER_4_WARP_CENTER_Y              2500
#define OMM_SPARKLY_BOWSER_4_WARP_CENTER_Z              -2000
#define OMM_SPARKLY_BOWSER_4_WARP_HITBOX_X              250
#define OMM_SPARKLY_BOWSER_4_WARP_HITBOX_Y              300
#define OMM_SPARKLY_BOWSER_4_WARP_HITBOX_Z              50

//
// Data
//

#if OMM_CODE_SPARKLY
typedef struct OmmSparklyStarsData { s32 level; s32 area; f32 starX; f32 starY; f32 starZ; f32 signX; f32 signY; f32 signZ; s16 signA; s32 signD; s32 flags; } OmmSparklyStarsData;
extern const OmmSparklyStarsData gOmmSparklyStarsData[4][29];
#endif

//
// Data flags
//

#define OMM_SPARKLY_DATA_BITS_COINS                     0x000000FF
#define OMM_SPARKLY_DATA_BITS_BUTTONS                   0x00FFFF00
#define OMM_SPARKLY_DATA_FLAG_GET_COINS(flags)          (((flags) & OMM_SPARKLY_DATA_BITS_COINS)   >> 0)
#define OMM_SPARKLY_DATA_FLAG_GET_BUTTONS(flags)        (((flags) & OMM_SPARKLY_DATA_BITS_BUTTONS) >> 8)
#define OMM_SPARKLY_DATA_FLAG_SET_COINS(value)          (((value) << 0) & OMM_SPARKLY_DATA_BITS_COINS  )
#define OMM_SPARKLY_DATA_FLAG_SET_BUTTONS(value)        (((value) << 8) & OMM_SPARKLY_DATA_BITS_BUTTONS)
#define OMM_SPARKLY_DATA_FLAG_CONDITION                 0x01000000
#define OMM_SPARKLY_DATA_FLAG_ALL_RED_COINS             0x02000000
#define OMM_SPARKLY_DATA_FLAG_NO_YELLOW_COIN            0x04000000
#define OMM_SPARKLY_DATA_FLAG_NO_BLUE_COIN              0x08000000
#define OMM_SPARKLY_DATA_FLAG_NO_RED_COIN               0x10000000
#define OMM_SPARKLY_DATA_FLAG_NO_1UP_MUSHROOM           0x20000000
#define OMM_SPARKLY_DATA_FLAG_NO_CAPTURE                0x40000000

//
// Data per level
//

#define OMM_SPARKLY_MODE_VARS(type, ...)                (OMM_ARRAY_OF(type) { __VA_ARGS__ })[omm_sparkly_get_current_mode()]

// Bob-omb Battlefield
#define OMM_SPARKLY_BOB_SECRET_COUNT                    5

// Whomp Fortress
#define OMM_SPARKLY_WF_TIMER                            3300

// Jolly Roger Bay
#define OMM_SPARKLY_JRB_START_DIALOG                    OMM_SPARKLY_MODE_VARS(s16, 0, OMM_DIALOG_SPARKLY_STAR_1_JRB, OMM_DIALOG_SPARKLY_STAR_2_JRB, 0)

// Big Boo's Haunt
#define OMM_SPARKLY_BBH_TIMER                           4050

// Hazy Maze Cave
#define OMM_SPARKLY_HMC_FLAME_MULT                      3.f
#define OMM_SPARKLY_HMC_TIMER                           5400

// Lethal Lava Land
#define OMM_SPARKLY_LLL_LAVA_PARAMS                     0, 0, 0, 0, 3570, OMM_SPARKLY_MODE_VARS(f32, 0.f, 2.f, 3.f, 0.f), 4096, 16, 10
#define OMM_SPARKLY_LLL_STAR_SHARDS_POS                 OMM_SPARKLY_MODE_VARS(const f32 *, NULL, \
                                                        (OMM_ARRAY_OF(f32) {\
                                                        -1700,   60,  -440, \
                                                         2550,  630,  1120, \
                                                          250, 1210,  2640, \
                                                        -2080, 1915,  -300, \
                                                        -1600, 2440, -2150, \
                                                         1300, 3705, -2500, \
                                                          420, 2365,   575, \
                                                          662, 3300,   708 }), \
                                                        (OMM_ARRAY_OF(f32) {\
                                                        -1700,   60,  -440, \
                                                          180, 1050, -1360, \
                                                         2550,  630,  1120, \
                                                         -480, 1210,  2630, \
                                                          705, 2160,   330, \
                                                            0, 4690, -2800, \
                                                          662, 3900,   708, \
                                                         2800, 3000,   250 }), \
                                                        NULL)

// Shifting Sand Land
#define OMM_SPARKLY_SSL_TIMER                           450

// Snowman's Land
#define OMM_SPARKLY_SL_REACH_X                          0
#define OMM_SPARKLY_SL_REACH_Y                          4860
#define OMM_SPARKLY_SL_REACH_Z                          0
#define OMM_SPARKLY_SL_REACH_R                          512
#define OMM_SPARKLY_SL_NUM_SNOWMEN                      OMM_SPARKLY_MODE_VARS(s32, 0, 1, 4, 0)

// Tick Tock Clock
#define OMM_SPARKLY_TTC_BOX_COUNT                       13
#define OMM_SPARKLY_TTC_BOX_TIMER                       1800

// Rainbow Ride
#define OMM_SPARKLY_RR_1UP_MUSHROOM_COUNT               9

// Bowser in the Dark World
#define OMM_SPARKLY_BITDW_SAFE_ZONE_X_MIN               -7925
#define OMM_SPARKLY_BITDW_SAFE_ZONE_X_MAX               -6975
#define OMM_SPARKLY_BITDW_SAFE_ZONE_Z_MIN               3350
#define OMM_SPARKLY_BITDW_SAFE_ZONE_Z_MAX               4425
#define OMM_SPARKLY_BITDW_TIMER                         3150

// Bowser in the Fire Sea
#define OMM_SPARKLY_BITFS_1UP_MUSHROOM_COUNT            7
#define OMM_SPARKLY_BITFS_SECRET_COUNT                  6
#define OMM_SPARKLY_BITFS_SECRETS_POS                   (OMM_ARRAY_OF(f32) { \
                                                        -8000, -2800, -8000, \
                                                        +8000, -2800, -8000, \
                                                        +8000, -2800, +8000, \
                                                        -8000, -2800, +8000, \
                                                         4300,  3000,     0, \
                                                        -5528,  -300,     0 })

// Tower of the Wing Cap
#define OMM_SPARKLY_TOTWC_CAP_TIMER                     OMM_SPARKLY_MODE_VARS(s32, 0, 1800, 1050, 0)

// Vanish Cap under the Moat
#define OMM_SPARKLY_VCUTM_CAP_TIMER                     OMM_SPARKLY_MODE_VARS(s32, 0, 1500, 1200, 0)
#define OMM_SPARKLY_VCUTM_ZONE_COUNT                    5
#define OMM_SPARKLY_VCUTM_ZONE_RADIUS                   300
#define OMM_SPARKLY_VCUTM_ZONES                         { { -6500, -6500 }, { -6500, +6500 }, { +4865, +6500 }, { +4865, -6500 }, { -2400, -6500 } }

// Cavern of the Metal Cap
#define OMM_SPARKLY_COTMC_CAP_TIMER                     OMM_SPARKLY_MODE_VARS(s32, 0, 1200, 900, 0)
#define OMM_SPARKLY_COTMC_START_DIALOG                  OMM_SPARKLY_MODE_VARS(s16, 0, OMM_DIALOG_SPARKLY_STAR_1_COTMC, OMM_DIALOG_SPARKLY_STAR_2_COTMC, 0)
#define OMM_SPARKLY_COTMC_START_ACTION                  ACT_LONG_JUMP

// Princess' Secret Slide
#define OMM_SPARKLY_COTMC_PSS_TIMER                     OMM_SPARKLY_MODE_VARS(s32, 0, 720, 660, 630)

// Secret Aquarium
#define OMM_SPARKLY_SA_START_DIALOG                     OMM_SPARKLY_MODE_VARS(s16, 0, OMM_DIALOG_SPARKLY_STAR_1_SA, OMM_DIALOG_SPARKLY_STAR_2_SA, 0)

// Castle Inside
#define OMM_SPARKLY_INSIDE_STAR_SHARDS_POS              (OMM_ARRAY_OF(f32) {\
                                                        -1024,  400,  -250, \
                                                        -1024,  800,  1400, \
                                                        -4500, 1200,  -450, \
                                                        -2280, -400, -3040, \
                                                         -128, -300, -2640, \
                                                         2000,  900, -1700, \
                                                         1910,  850, -2120, \
                                                        -5500,  600, -4300 })

// Castle Basement
#define OMM_SPARKLY_BASEMENT_MOAT_PILLARS_RADIUS        200

// Castle Upstairs
#define OMM_SPARKLY_UPSTAIRS_STAR_SHARDS_POS            (OMM_ARRAY_OF(f32) {\
                                                        -1000, 1210,  1480, \
                                                        -4780, 3200, -3240, \
                                                         3538, 1680,  3880, \
                                                         -610, 1900,  2500, \
                                                         -200, 2800,  5000, \
                                                        -3400, 3600,  5886, \
                                                         3000, 2900,  5886, \
                                                         -200, 5400, -3700 })

// Castle Courtyard
#define OMM_SPARKLY_COURT_SECRET_COUNT                  8
#define OMM_SPARKLY_COURT_SECRETS_POS                   (OMM_ARRAY_OF(f32) {\
                                                         1500,    0, -1740, \
                                                         -820,   20, -3630, \
                                                          820,   20, -3630, \
                                                         -820,   20,   200, \
                                                          820,   20,   200, \
                                                        -1500,    0, -1740, \
                                                         2040, -200,  -480, \
                                                        -2240, -200, -2815 })
#define OMM_SPARKLY_COURT_CAPPY_STAR_POS                (Vec3f) { 9, 433, -1735 }
#define OMM_SPARKLY_COURT_CAPPY_STAR_RADIUS             20.f

//
// Context state
//

#define OMM_SPARKLY_CONTEXT_STATE_OK                    0
#define OMM_SPARKLY_CONTEXT_STATE_FAIL                  1
#define OMM_SPARKLY_CONTEXT_STATE_INVALID               2

//
// Context flags
//

#define OMM_SPARKLY_CONTEXT_FLAG_INITED                 (1 << 0)
#define OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL             (1 << 1)
#define OMM_SPARKLY_CONTEXT_FLAG_STAR_SPAWNED           (1 << 2)
#define OMM_SPARKLY_CONTEXT_FLAG_COINS_SPAWNED          (1 << 3)
#define OMM_SPARKLY_CONTEXT_FLAG_CHEAT_DETECTED         (1 << 4)
#define OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED          (1 << 5)
#define OMM_SPARKLY_CONTEXT_FLAG_DIALOG_STARTED         (1 << 6)
#define OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED          (1 << 7)
#define OMM_SPARKLY_CONTEXT_FLAG_ZONES                  (1 << 8)

//
// Context counters
//

#define OMM_SPARKLY_CONTEXT_COUNTER_YELLOW_COINS        0
#define OMM_SPARKLY_CONTEXT_COUNTER_LOST_COINS          1
#define OMM_SPARKLY_CONTEXT_COUNTER_BLUE_COINS          2
#define OMM_SPARKLY_CONTEXT_COUNTER_RED_COINS           3
#define OMM_SPARKLY_CONTEXT_COUNTER_1UP_MUSHROOMS       4
#define OMM_SPARKLY_CONTEXT_COUNTER_SECRETS             5

//
// Context update
//

#define OMM_SPARKLY_CONTEXT_UPDATE_YELLOW_COIN          (1 << 0)
#define OMM_SPARKLY_CONTEXT_UPDATE_LOST_COIN            (1 << 1)
#define OMM_SPARKLY_CONTEXT_UPDATE_BLUE_COIN            (1 << 2)
#define OMM_SPARKLY_CONTEXT_UPDATE_RED_COIN             (1 << 3)
#define OMM_SPARKLY_CONTEXT_UPDATE_1UP_MUSHROOM         (1 << 4)

#endif
