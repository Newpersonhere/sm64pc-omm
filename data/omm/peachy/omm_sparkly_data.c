#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS
#if OMM_CODE_SPARKLY

// Helper macros to make Sparkly Stars data easier to read and write
#define COINS(value)        OMM_SPARKLY_DATA_FLAG_SET_COINS(value)
#define NO_BUTTON(value)    OMM_SPARKLY_DATA_FLAG_SET_BUTTONS(value)
#define CONDITION           OMM_SPARKLY_DATA_FLAG_CONDITION
#define ALL_R_COINS         OMM_SPARKLY_DATA_FLAG_ALL_RED_COINS
#define NO_Y_COIN           OMM_SPARKLY_DATA_FLAG_NO_YELLOW_COIN
#define NO_B_COIN           OMM_SPARKLY_DATA_FLAG_NO_BLUE_COIN
#define NO_R_COIN           OMM_SPARKLY_DATA_FLAG_NO_RED_COIN
#define NO_1UP              OMM_SPARKLY_DATA_FLAG_NO_1UP_MUSHROOM
#define NO_CAPTURE          OMM_SPARKLY_DATA_FLAG_NO_CAPTURE

const OmmSparklyStarsData gOmmSparklyStarsData[4][29] = {
// Disabled
{
},
// Normal mode: Pink Gold Stars
{
    { LEVEL_BOB,     1,  4360,  4500,  1230, -5200,     0,  6750, 0xC000, OMM_DIALOG_SPARKLY_STAR_1_BOB,      CONDITION },
    { LEVEL_WF,      1,     0,  6250,     0,  3075,   256,  5245, 0xC000, OMM_DIALOG_SPARKLY_STAR_1_WF,       CONDITION | COINS(90) | NO_B_COIN | NO_R_COIN },
    { LEVEL_JRB,     1,  5000,  2500,  2500, -6060,  1126,   940, 0xA000, OMM_DIALOG_SPARKLY_STAR_1_JRB,      CONDITION | COINS(80) },
    { LEVEL_CCM,     2, -4880,  4800,  5000,     0,     0,     0, 0x0000, 0,                                  0 },
    { LEVEL_BBH,     1,   666,  4000,   260,   916,  -204,  3064, 0x0000, OMM_DIALOG_SPARKLY_STAR_1_BBH,      CONDITION | COINS(100) | NO_Y_COIN | NO_R_COIN },
    { LEVEL_HMC,     1,  6000,  4500,  6200,     0,     0,     0, 0x0000, 0,                                  0 },
    { LEVEL_LLL,     2,  2530,  5500,  -900,  -750,   103, -1225, 0xE000, OMM_DIALOG_SPARKLY_STAR_1_LLL,      CONDITION | ALL_R_COINS },
    { LEVEL_SSL,     1, -2050,  5400, -1020,     0,     0,     0, 0x0000, 0,                                  0 },
    { LEVEL_DDD,     2,  3900,  2400, -4000,     0,     0,     0, 0x0000, 0,                                  0 },
    { LEVEL_SL,      1,     0,  6000,     0,  2035,  1023,   475, 0x6000, OMM_DIALOG_SPARKLY_STAR_1_SL,       CONDITION },
    { LEVEL_WDW,     1, -1075,  6000,  1280,     0,     0,     0, 0x0000, 0,                                  0 },
    { LEVEL_TTM,     3,  1775,   800, -4430,     0,     0,     0, 0x0000, 0,                                  0 },
    { LEVEL_THI,     2, -1900,  -400,  1800, -1700,  -890,  2250, 0xC000, OMM_DIALOG_SPARKLY_STAR_1_THI,      CONDITION },
    { LEVEL_TTC,     1,     0,  4800,     0,   800, -4822,   140, 0x6000, OMM_DIALOG_SPARKLY_STAR_1_TTC,      CONDITION },
    { LEVEL_RR,      1, -4200,  7200, -5850,  3100, -1833,  2070, 0xC000, OMM_DIALOG_SPARKLY_STAR_1_RR,       NO_BUTTON(A_BUTTON) },
    { LEVEL_BITDW,   1, -7440, -2600,  3900, -7440, -3153,  3530, 0x0000, OMM_DIALOG_SPARKLY_STAR_1_BITDW,    CONDITION | ALL_R_COINS },
    { LEVEL_BITFS,   1,  6740,  3333,    90, -7577, -2764,  -360, 0x0000, OMM_DIALOG_SPARKLY_STAR_1_BITFS,    CONDITION },
    { LEVEL_BITS,    1,   350,  6800, -5300, -7360, -4812,     0, 0x4000, OMM_DIALOG_SPARKLY_STAR_1_BITS,     CONDITION | ALL_R_COINS | NO_BUTTON(A_BUTTON) },
    { LEVEL_TOTWC,   1,     0, 19950,     0, -1000, -2047,     0, 0x4000, OMM_DIALOG_SPARKLY_STAR_1_TOTWC,    CONDITION | COINS(63) },
    { LEVEL_VCUTM,   1, -2300,  6300, -6400, -6400,  5734, -6140, 0x4000, OMM_DIALOG_SPARKLY_STAR_1_VCUTM,    CONDITION },
    { LEVEL_COTMC,   1,     0,  1500, -7300, -4000,    20,  -333, 0xE000, OMM_DIALOG_SPARKLY_STAR_1_COTMC,    0 },
    { LEVEL_PSS,     1, -6360, -4100,  3150,  3566,  6144, -6100, 0x4000, OMM_DIALOG_SPARKLY_STAR_1_PSS,      CONDITION | COINS(50) | NO_BUTTON(A_BUTTON | B_BUTTON | START_BUTTON | R_TRIG | L_TRIG | Z_TRIG | X_BUTTON | Y_BUTTON) | NO_B_COIN | NO_R_COIN },
    { LEVEL_WMOTR,   1, -3200,  5000, -4500,  -410,  1669,   326, 0x6000, OMM_DIALOG_SPARKLY_STAR_1_WMOTR,    CONDITION | ALL_R_COINS },
    { LEVEL_SA,      1,     0, -2200,     0, -9000,     0,     0, 0x0000, OMM_DIALOG_SPARKLY_STAR_1_SA,       CONDITION | ALL_R_COINS },
    { LEVEL_GROUNDS, 1,     0,  9400, -4020,     0,   800,  1400, 0x0000, OMM_DIALOG_SPARKLY_STAR_1_GROUNDS,  0 },
    { LEVEL_CASTLE,  1, -1024,   250,   717, -1350,     0,     0, 0x0000, OMM_DIALOG_SPARKLY_STAR_1_INSIDE,   CONDITION | ALL_R_COINS },
    { LEVEL_CASTLE,  3,  7050, -1000,  -512,  -800, -1279,  2592, 0x8000, OMM_DIALOG_SPARKLY_STAR_1_BASEMENT, CONDITION },
    { LEVEL_CASTLE,  2,  -205,  3900,  6750,     0,     0,     0, 0x0000, 0,                                  CONDITION | ALL_R_COINS },
    { LEVEL_COURT,   1,     0,   300,   960,     0,     0, -3590, 0x0000, OMM_DIALOG_SPARKLY_STAR_1_COURT,    CONDITION },
},
// Hard mode: Crystal Stars
{
    { LEVEL_BOB,     1, -1506,  7000,  1250, -5200,     0,  6750, 0xC000, OMM_DIALOG_SPARKLY_STAR_2_BOB,      CONDITION },
    { LEVEL_WF,      1,     0,  1000,     0,  3075,   256,  5245, 0xC000, OMM_DIALOG_SPARKLY_STAR_2_WF,       CONDITION | COINS(100) | NO_B_COIN | NO_R_COIN },
    { LEVEL_JRB,     1, -1080, 19500,  7070, -6060,  1126,   940, 0xA000, OMM_DIALOG_SPARKLY_STAR_2_JRB,      CONDITION | COINS(101) },
    { LEVEL_CCM,     1,   -55,  9100,  -150,     0,     0,     0, 0x0000, 0,                                  0 },
    { LEVEL_BBH,     1,   666,  4000,   260,   916,  -204,  3064, 0x0000, OMM_DIALOG_SPARKLY_STAR_2_BBH,      CONDITION | COINS(100) | NO_Y_COIN | NO_R_COIN },
    { LEVEL_HMC,     1, -7500,  2500,  7500, -7100,  2048,  5900, 0x2000, OMM_DIALOG_SPARKLY_STAR_2_HMC,      CONDITION },
    { LEVEL_LLL,     2,     0,  7000,     0,  -750,   103, -1225, 0xE000, OMM_DIALOG_SPARKLY_STAR_2_LLL,      CONDITION | ALL_R_COINS },
    { LEVEL_SSL,     2,     0,  5650,   256,   600,     0,  4500, 0xC000, OMM_DIALOG_SPARKLY_STAR_2_SSL,      CONDITION | COINS(62) },
    { LEVEL_DDD,     2,  3900,  4000,  6000,     0,     0,     0, 0x0000, 0,                                  0 },
    { LEVEL_SL,      1,     0,  8400,     0,  2035,  1023,   475, 0x6000, OMM_DIALOG_SPARKLY_STAR_2_SL,       CONDITION },
    { LEVEL_WDW,     0,     0,     0,     0,  1400,     0,  -700, 0x4000, OMM_DIALOG_SPARKLY_STAR_2_WDW,      CONDITION },
    { LEVEL_TTM,     3, -6000,  7000, -6000,     0,     0,     0, 0x0000, 0,                                  0 },
    { LEVEL_THI,     1,     0,  8900, -1540, -7800, -2969,  7800, 0xE000, OMM_DIALOG_SPARKLY_STAR_2_THI,      CONDITION },
    { LEVEL_TTC,     1,     0,  8000,     0,   800, -4822,   140, 0x6000, OMM_DIALOG_SPARKLY_STAR_2_TTC,      0 },
    { LEVEL_RR,      1,  3554,  5000, -2327,  3100, -1833,  2070, 0xC000, OMM_DIALOG_SPARKLY_STAR_2_RR,       CONDITION | NO_BUTTON(A_BUTTON | B_BUTTON | Z_TRIG) },
    { LEVEL_BITDW,   1, -7440, -2048,  3900, -7440, -3153,  3530, 0x0000, OMM_DIALOG_SPARKLY_STAR_2_BITDW,    CONDITION },
    { LEVEL_BITFS,   1,  1240,  8000,    90, -7577, -2764,  -360, 0x0000, OMM_DIALOG_SPARKLY_STAR_2_BITFS,    CONDITION },
    { LEVEL_BITS,    1,   350,  7700, -6030, -7360, -4812,     0, 0x4000, OMM_DIALOG_SPARKLY_STAR_2_BITS,     CONDITION | ALL_R_COINS | NO_BUTTON(A_BUTTON | B_BUTTON | Z_TRIG) | NO_Y_COIN | NO_B_COIN | NO_1UP | NO_CAPTURE },
    { LEVEL_TOTWC,   1,     0, 19950,     0, -1000, -2047,     0, 0x4000, OMM_DIALOG_SPARKLY_STAR_2_TOTWC,    CONDITION | COINS(63) },
    { LEVEL_VCUTM,   1, -2300,  6400, -6400, -6400,  5734, -6140, 0x4000, OMM_DIALOG_SPARKLY_STAR_2_VCUTM,    CONDITION },
    { LEVEL_COTMC,   1,     0,  4000, -7450, -4000,    20,  -333, 0xE000, OMM_DIALOG_SPARKLY_STAR_2_COTMC,    0 },
    { LEVEL_PSS,     1, -6360, -4400,  3450,  3566,  6144, -6100, 0x4000, OMM_DIALOG_SPARKLY_STAR_2_PSS,      CONDITION | COINS(50) | NO_BUTTON(A_BUTTON | B_BUTTON | START_BUTTON | R_TRIG | L_TRIG | Z_TRIG | X_BUTTON | Y_BUTTON) | NO_B_COIN | NO_R_COIN },
    { LEVEL_WMOTR,   1,   200,  7100, -2340,  -410,  1669,   326, 0x6000, OMM_DIALOG_SPARKLY_STAR_2_WMOTR,    CONDITION | COINS(56) },
    { LEVEL_SA,      1,     0, -2200,     0, -9000,     0,     0, 0x0000, OMM_DIALOG_SPARKLY_STAR_2_SA,       CONDITION | COINS(56) | NO_BUTTON(X_BUTTON) },
    { LEVEL_GROUNDS, 1,     0, 10000, -4020,     0,   800,  1400, 0x0000, OMM_DIALOG_SPARKLY_STAR_2_GROUNDS,  0 },
    { LEVEL_CASTLE,  1, -6960,  1080, -5770, -1350,     0,     0, 0x0000, OMM_DIALOG_SPARKLY_STAR_2_INSIDE,   0 },
    { LEVEL_CASTLE,  3, -1509,  -900, -1564,     0,     0,     0, 0x0000, 0,                                  CONDITION },
    { LEVEL_CASTLE,  2,  7350,  1750,  1830,  3975,  1408,  1830, 0xC000, OMM_DIALOG_SPARKLY_STAR_2_UPSTAIRS, 0 },
    { LEVEL_COURT,   1,     9,  3800, -1735,     0,     0, -3590, 0x0000, OMM_DIALOG_SPARKLY_STAR_2_COURT,    CONDITION },
},
// Lunatic mode: Nebula Stars
{
    { LEVEL_BOB,     1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_WF,      1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_JRB,     1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_CCM,     2,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_BBH,     1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_HMC,     1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_LLL,     2,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_SSL,     2,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_DDD,     2,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_SL,      1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_WDW,     2,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_TTM,     3,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_THI,     1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_TTC,     1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_RR,      1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_BITDW,   1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_BITFS,   1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_BITS,    1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_TOTWC,   1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_VCUTM,   1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_COTMC,   1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_PSS,     1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_WMOTR,   1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_SA,      1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_GROUNDS, 1,     0,     0,     0,     0,   800,  1400, 0x0000, OMM_DIALOG_SPARKLY_STAR_3_GROUNDS,  0 },
    { LEVEL_CASTLE,  1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_CASTLE,  3,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_CASTLE,  2,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
    { LEVEL_COURT,   1,     0,     0,     0,     0,     0,     0, 0x0000, 0,                            CONDITION },
},
};
#endif

//
// Functions
//

bool omm_sparkly_is_enabled() {
    return omm_sparkly_get_current_mode() != OMM_SPARKLY_MODE_DISABLED;
}

bool omm_sparkly_is_bowser_4() {
#if OMM_CODE_SPARKLY
    return gCurrLevelNum == LEVEL_GROUNDS && gCurrAreaIndex == 2;
#else
    return false;
#endif
}

bool omm_sparkly_is_mode_selected(s32 mode) {
    return omm_sparkly_get_current_mode() == mode;
}

s32 omm_sparkly_get_current_mode() {
#if OMM_CODE_SPARKLY
    return gOmmData->mario->sparkly.mode;
#else
    return OMM_SPARKLY_MODE_DISABLED;
#endif
}

s32 omm_sparkly_get_star_index(s32 mode, s32 level, s32 area) {
#if OMM_CODE_SPARKLY
    if (OMM_LIKELY(mode >= 1 && mode < OMM_SPARKLY_MODE_COUNT)) {
        for (s32 i = 0; i != 29; ++i) {
            if (gOmmSparklyStarsData[mode][i].level == level && (
                gOmmSparklyStarsData[mode][i].area == 0 ||
                gOmmSparklyStarsData[mode][i].area == area)) {
                return i;
            }
        }
    }
#else
    OMM_UNUSED(mode);
    OMM_UNUSED(level);
    OMM_UNUSED(area);
#endif
    return -1;
}

s32 omm_sparkly_get_star_level(s32 mode, s32 index) {
#if OMM_CODE_SPARKLY
    if (OMM_LIKELY(mode >= 1 && mode < OMM_SPARKLY_MODE_COUNT && index >= 0 && index < 29)) {
        return gOmmSparklyStarsData[mode][index].level;
    }
#else
    OMM_UNUSED(mode);
    OMM_UNUSED(index);
#endif
    return LEVEL_NONE;
}

s32 omm_sparkly_get_star_area(s32 mode, s32 index) {
#if OMM_CODE_SPARKLY
    if (OMM_LIKELY(mode >= 1 && mode < OMM_SPARKLY_MODE_COUNT && index >= 0 && index < 29)) {
        return gOmmSparklyStarsData[mode][index].area;
    }
#else
    OMM_UNUSED(mode);
    OMM_UNUSED(index);
#endif
    return LEVEL_NONE;
}

u8 *omm_sparkly_get_star_name(s32 mode, s32 index) {
#if OMM_CODE_SPARKLY
    if (OMM_LIKELY(mode >= 1 && mode < OMM_SPARKLY_MODE_COUNT && index >= 0 && index < 29)) {
        const u8 *starNames = omm_get_dialog_entry(NULL, OMM_SPARKLY_STAR_NAMES_DIALOG[mode])->str;

        // Name start
        const u8 *nameStart = starNames;
        for (s32 i = 0; i != index; ++i) { while (*(nameStart++) != DIALOG_CHAR_NEWLINE); }

        // Name end
        const u8 *nameEnd = nameStart;
        while (*(++nameEnd) != DIALOG_CHAR_NEWLINE);

        // Copy
        s32 length = (s32) (nameEnd - nameStart);
        static u8 sStarName[0x100];
        OMM_MEMSET(sStarName, 0xFF, sizeof(sStarName));
        OMM_MEMCPY(sStarName, nameStart, omm_min_s(sizeof(sStarName) - 1, length));

        // Done
        return sStarName;
    }
#else
    OMM_UNUSED(mode);
    OMM_UNUSED(index);
#endif
    return NULL;
}

s32 omm_sparkly_get_star_count(s32 mode) {
    s32 count = 0;
    for (s32 i = 0; i != 29; ++i) {
        count += omm_sparkly_is_star_collected(mode, i);
    }
    return count + omm_sparkly_is_grand_star_collected(mode);
}
