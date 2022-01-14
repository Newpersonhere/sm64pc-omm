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
// Versions
//

#define OMM_SPARKLY_REQUIRED_STARS                      DEF(120, 50, 120, 120, 157, 130)
#define OMM_SPARKLY_STAR_BLOCKS_LEVEL                   DEF(LEVEL_GROUNDS, LEVEL_CASTLE, LEVEL_GROUNDS, LEVEL_GROUNDS, LEVEL_COURT, LEVEL_COURT)
#define OMM_SPARKLY_STAR_BLOCKS_AREA                    DEF(1, 1, 1, 1, 2, 1)
#define OMM_SPARKLY_STAR_BLOCKS_COUNT                   DEF(3, 1, 3, 3, 1, 1)
#define OMM_SPARKLY_STAR_BLOCKS_AVAILABLE               DEF(0b1111, 0b1001, 0b1111, 0b1111, 0b1001, 0b1001)
#define OMM_SPARKLY_STAR_BLOCKS_X                       DEF(     0,      0,      0,      0,   6464,   -990)
#define OMM_SPARKLY_STAR_BLOCKS_Y                       DEF(   820,  -2780,    820,    820,   -920,    720)
#define OMM_SPARKLY_STAR_BLOCKS_Z                       DEF(   480,   1700,    480,    480,   6300,  -2340)
#define OMM_SPARKLY_STAR_BLOCKS_ANGLE                   DEF(0x4000, 0x0000, 0x4000, 0x4000, 0x0000, 0x0000)

//
// Lists
//

#define OMM_SPARKLY_STAR_OPAQUE_GEO                     (OMM_ARRAY_OF(const GeoLayout *) { NULL, omm_geo_sparkly_star_1_opaque, omm_geo_sparkly_star_2_opaque, omm_geo_sparkly_star_3_opaque })
#define OMM_SPARKLY_STAR_TRANSPARENT_GEO                (OMM_ARRAY_OF(const GeoLayout *) { NULL, omm_geo_sparkly_star_1_transparent, omm_geo_sparkly_star_2_transparent, omm_geo_sparkly_star_3_transparent })
#define OMM_SPARKLY_STAR_SPARKLE_GEO                    (OMM_ARRAY_OF(const GeoLayout *) { NULL, omm_geo_sparkly_star_1_sparkle, omm_geo_sparkly_star_2_sparkle, omm_geo_sparkly_star_3_sparkle })
#define OMM_SPARKLY_STAR_HINT_GEO                       (OMM_ARRAY_OF(const GeoLayout *) { NULL, omm_geo_sparkly_star_1_hint, omm_geo_sparkly_star_2_hint, omm_geo_sparkly_star_3_hint })
#define OMM_SPARKLY_STAR_BLOCK_GEO                      (OMM_ARRAY_OF(const GeoLayout *) { NULL, omm_geo_sparkly_star_1_block, omm_geo_sparkly_star_2_block, omm_geo_sparkly_star_3_block })
#define OMM_SPARKLY_STAR_BLOCK_BHV                      (OMM_ARRAY_OF(const BehaviorScript *) { NULL, omm_bhv_sparkly_star_1_block, omm_bhv_sparkly_star_2_block, omm_bhv_sparkly_star_3_block })
#define OMM_SPARKLY_STAR_TEXT                           (OMM_ARRAY_OF(const char *) { NULL, OMM_TEXT_SPARKLY_STAR_1, OMM_TEXT_SPARKLY_STAR_2, OMM_TEXT_SPARKLY_STAR_3 })
#define OMM_SPARKLY_STARS_TEXT                          (OMM_ARRAY_OF(const char *) { NULL, OMM_TEXT_SPARKLY_STARS_1, OMM_TEXT_SPARKLY_STARS_2, OMM_TEXT_SPARKLY_STARS_3 })
#define OMM_SPARKLY_STAR_HUD_GLYPH                      (OMM_ARRAY_OF(const char *) { NULL, OMM_TEXTURE_STAR_FULL_17, OMM_TEXTURE_STAR_FULL_18, OMM_TEXTURE_STAR_FULL_19 })
#define OMM_SPARKLY_STAR_HUD_COLOR                      (OMM_ARRAY_OF(Vec3s) { { 0xFF, 0xFF, 0xFF }, { 0xFF, 0xE0, 0x80 }, { 0xC0, 0xE0, 0xFF }, { 0xFF, 0x40, 0x80 } })

//
// Modes
//

#define OMM_SPARKLY_MODE_DISABLED                       0
#define OMM_SPARKLY_MODE_NORMAL                         1
#define OMM_SPARKLY_MODE_HARD                           2
#define OMM_SPARKLY_MODE_EXTREME                        3
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
// Data
//

typedef struct OmmSparklyStarsData {
    s32 index, game, level, area;
    s32 starX, starY, starZ;
    s32 signX, signY, signZ, signA, showOnEntry;
    s32 coinX, coinY, coinZ, coinA, spawnCoinArrow;
    s32 timer;
    u64 flags;
} OmmSparklyStarsData;
extern const OmmSparklyStarsData gOmmSparklyStarsData[4][30];

//
// Data flags
//

#define OMM_SPARKLY_DATA_BITS_COINS                     0x00000000000000FFllu
#define OMM_SPARKLY_DATA_BITS_BUTTONS                   0x0000000000FFFF00llu
#define OMM_SPARKLY_DATA_BITS_MUSHROOMS                 0x000000000F000000llu
#define OMM_SPARKLY_DATA_FLAG_CONDITION                 0x0000000010000000llu
#define OMM_SPARKLY_DATA_FLAG_ALL_RED_COINS             0x0000000020000000llu
#define OMM_SPARKLY_DATA_FLAG_NO_YELLOW_COIN            0x0000000040000000llu
#define OMM_SPARKLY_DATA_FLAG_NO_BLUE_COIN              0x0000000080000000llu
#define OMM_SPARKLY_DATA_FLAG_NO_RED_COIN               0x0000000100000000llu
#define OMM_SPARKLY_DATA_FLAG_NO_1UP_MUSHROOM           0x0000000200000000llu
#define OMM_SPARKLY_DATA_FLAG_NO_CAPTURE                0x0000000400000000llu
#define OMM_SPARKLY_DATA_FLAG_WING_CAP                  0x0000000800000000llu
#define OMM_SPARKLY_DATA_FLAG_VANISH_CAP                0x0000001000000000llu
#define OMM_SPARKLY_DATA_FLAG_METAL_CAP                 0x0000002000000000llu
#define OMM_SPARKLY_DATA_FLAG_DARK_MODE                 0x0000004000000000llu
#define OMM_SPARKLY_DATA_FLAG_FLAMES                    0x0000008000000000llu
#define OMM_SPARKLY_DATA_FLAG_BOXES                     0x0000010000000000llu

#define OMM_SPARKLY_DATA_FLAG_GET_COINS(flags)          (((flags) & OMM_SPARKLY_DATA_BITS_COINS)     >>  0llu)
#define OMM_SPARKLY_DATA_FLAG_GET_BUTTONS(flags)        (((flags) & OMM_SPARKLY_DATA_BITS_BUTTONS)   >>  8llu)
#define OMM_SPARKLY_DATA_FLAG_GET_MUSHROOMS(flags)      (((flags) & OMM_SPARKLY_DATA_BITS_MUSHROOMS) >> 24llu)
#define OMM_SPARKLY_DATA_FLAG_SET_COINS(value)          (((value) <<  0llu) & OMM_SPARKLY_DATA_BITS_COINS    )
#define OMM_SPARKLY_DATA_FLAG_SET_BUTTONS(value)        (((value) <<  8llu) & OMM_SPARKLY_DATA_BITS_BUTTONS  )
#define OMM_SPARKLY_DATA_FLAG_SET_MUSHROOMS(value)      (((value) << 24llu) & OMM_SPARKLY_DATA_BITS_MUSHROOMS)
#define OMM_SPARKLY_DATA_FLAG_CAPS                      (OMM_SPARKLY_DATA_FLAG_WING_CAP | OMM_SPARKLY_DATA_FLAG_VANISH_CAP | OMM_SPARKLY_DATA_FLAG_METAL_CAP)

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
#define OMM_SPARKLY_CONTEXT_FLAG_COIN_ARROW_SPAWNED     (1 << 3)
#define OMM_SPARKLY_CONTEXT_FLAG_OBJECTS_SPAWNED        (1 << 4)
#define OMM_SPARKLY_CONTEXT_FLAG_CHEAT_DETECTED         (1 << 5)
#define OMM_SPARKLY_CONTEXT_FLAG_ENTRY_DIALOG           (1 << 6)
#define OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED          (1 << 7)
#define OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED          (1 << 8)
#define OMM_SPARKLY_CONTEXT_FLAG_ZONES                  (1 << 9)

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
