#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

#define OMM_SPARKLY_SAVE_FLAG_MODE_UNLOCKED       0
#define OMM_SPARKLY_SAVE_FLAG_TIMER_STARTED       1
#define OMM_SPARKLY_SAVE_FLAG_STARS               2
#define OMM_SPARKLY_SAVE_FLAG_BOWSER_4_UNLOCKED   31
#define OMM_SPARKLY_SAVE_FLAG_GRAND_STAR          32
#define OMM_SPARKLY_SAVE_FLAG_MODE_COMPLETED      33
#define OMM_SPARKLY_SAVE_FLAG_COUNT               34
#define OMM_SPARKLY_SAVE_TIMER_MAX                10799970 // 99h 59m 59s

static bool sOmmSparklySaveDataFlags[OMM_SPARKLY_MODE_COUNT][OMM_SPARKLY_SAVE_FLAG_COUNT];
static s32  sOmmSparklySaveDataTimer[OMM_SPARKLY_MODE_COUNT];

bool omm_sparkly_is_mode_unlocked(s32 mode) {
    if (OMM_LIKELY(mode >= 1 && mode < OMM_SPARKLY_MODE_COUNT)) {
        return sOmmSparklySaveDataFlags[mode][OMM_SPARKLY_SAVE_FLAG_MODE_UNLOCKED];
    }
    return false;
}

bool omm_sparkly_is_timer_started(s32 mode) {
    if (OMM_LIKELY(mode >= 1 && mode < OMM_SPARKLY_MODE_COUNT)) {
        return sOmmSparklySaveDataFlags[mode][OMM_SPARKLY_SAVE_FLAG_TIMER_STARTED];
    }
    return false;
}

bool omm_sparkly_is_star_collected(s32 mode, s32 index) {
    if (OMM_LIKELY(mode >= 1 && mode < OMM_SPARKLY_MODE_COUNT && index >= 0 && index < 29)) {
        return sOmmSparklySaveDataFlags[mode][OMM_SPARKLY_SAVE_FLAG_STARS + index];
    }
    return false;
}

bool omm_sparkly_is_bowser_4_unlocked(s32 mode) {
    if (OMM_LIKELY(mode >= 1 && mode < OMM_SPARKLY_MODE_COUNT)) {
        return sOmmSparklySaveDataFlags[mode][OMM_SPARKLY_SAVE_FLAG_BOWSER_4_UNLOCKED];
    }
    return false;
}

bool omm_sparkly_is_grand_star_collected(s32 mode) {
    if (OMM_LIKELY(mode >= 1 && mode < OMM_SPARKLY_MODE_COUNT)) {
        return sOmmSparklySaveDataFlags[mode][OMM_SPARKLY_SAVE_FLAG_GRAND_STAR];
    }
    return false;
}

bool omm_sparkly_is_mode_completed(s32 mode) {
    if (OMM_LIKELY(mode >= 1 && mode < OMM_SPARKLY_MODE_COUNT)) {
        return sOmmSparklySaveDataFlags[mode][OMM_SPARKLY_SAVE_FLAG_MODE_COMPLETED];
    }
    return false;
}

s32 omm_sparkly_get_timer(s32 mode) {
    if (OMM_LIKELY(mode >= 1 && mode < OMM_SPARKLY_MODE_COUNT)) {
        return sOmmSparklySaveDataTimer[mode];
    }
    return 0;
}

#if OMM_CODE_SPARKLY
void omm_sparkly_unlock_mode(s32 mode) {
    if (OMM_LIKELY(mode >= 1 && mode < OMM_SPARKLY_MODE_COUNT)) {
        sOmmSparklySaveDataFlags[mode][OMM_SPARKLY_SAVE_FLAG_MODE_UNLOCKED] = true;
        gSaveFileModified = true;
    }
}

void omm_sparkly_start_timer(s32 mode) {
    if (OMM_LIKELY(mode >= 1 && mode < OMM_SPARKLY_MODE_COUNT)) {
        sOmmSparklySaveDataFlags[mode][OMM_SPARKLY_SAVE_FLAG_TIMER_STARTED] = true;
        sOmmSparklySaveDataTimer[mode] = 0;
        gSaveFileModified = true;
    }
}

void omm_sparkly_collect_star(s32 mode, s32 index) {
    if (OMM_LIKELY(mode >= 1 && mode < OMM_SPARKLY_MODE_COUNT && index >= 0 && index < 29)) {
        sOmmSparklySaveDataFlags[mode][OMM_SPARKLY_SAVE_FLAG_STARS + index] = true;
        gSaveFileModified = true;
    }
}

void omm_sparkly_unlock_bowser_4(s32 mode) {
    if (OMM_LIKELY(mode >= 1 && mode < OMM_SPARKLY_MODE_COUNT)) {
        sOmmSparklySaveDataFlags[mode][OMM_SPARKLY_SAVE_FLAG_BOWSER_4_UNLOCKED] = true;
        gSaveFileModified = true;
    }
}

void omm_sparkly_collect_grand_star(s32 mode) {
    if (OMM_LIKELY(mode >= 1 && mode < OMM_SPARKLY_MODE_COUNT)) {
        sOmmSparklySaveDataFlags[mode][OMM_SPARKLY_SAVE_FLAG_GRAND_STAR] = true;
        gSaveFileModified = true;
    }
}

void omm_sparkly_clear_mode(s32 mode) {
    if (OMM_LIKELY(mode >= 1 && mode < OMM_SPARKLY_MODE_COUNT)) {
        OMM_MEMSET(sOmmSparklySaveDataFlags[mode], 0, OMM_SPARKLY_SAVE_FLAG_MODE_COMPLETED);
        sOmmSparklySaveDataTimer[mode] = 0;
        gSaveFileModified = true;
    }
}
#endif

//
// Save data
//

static s32 i2c(s32 input, bool shuffle, bool c2i) {
    static const char sDefaultTable[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static char sRandomTable[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    if (shuffle) {
        for (s32 i = 0; i != 62; ++i) {
            s32 j = rand() % 62;
            char c = sRandomTable[i];
            sRandomTable[i] = sRandomTable[j];
            sRandomTable[j] = c;
        }
    } else {
        OMM_MEMCPY(sRandomTable, sDefaultTable, 62);
    }
    if (c2i) {
        for (s32 i = 0; i != 62; ++i) {
            if (sRandomTable[i] == (char) input) {
                return i;
            }
        }
        return 0;
    }
    return (s32) sRandomTable[input];
}

static s32 read_chars(const char **data, s32 count, bool shuffle) {
    s32 output = 0;
    for (s32 i = 0, j = 1; i != count; ++i, j *= 62) {
        s32 value = i2c((*data)[i], shuffle, true);
        output += value * j;
    }
    (*data) += count;
    return output;
}

bool omm_sparkly_read_save(const char **tokens) {
    if (strcmp(tokens[0], "sparkly_stars") == 0) {
        const char *data = tokens[1];

        // Seed
        s32 seed = read_chars(&data, 2, false);
        srand(seed);

        // Data 1
        bool flags1[OMM_SPARKLY_SAVE_FLAG_COUNT];
        for (s32 i = 0; i != OMM_SPARKLY_SAVE_FLAG_COUNT; ++i) {
            flags1[i] = (read_chars(&data, 1, true) == 1);
        }
        s32 timer1 = read_chars(&data, 4, true);
        s32 mode1 = read_chars(&data, 1, true);

        // Data 2
        bool flags2[OMM_SPARKLY_SAVE_FLAG_COUNT];
        for (s32 i = 0; i != OMM_SPARKLY_SAVE_FLAG_COUNT; ++i) {
            flags2[i] = (read_chars(&data, 1, true) == 1);
        }
        s32 timer2 = read_chars(&data, 4, true);
        s32 mode2 = read_chars(&data, 1, true);

        // Check modes
        if (mode1 == mode2 && mode1 >= OMM_SPARKLY_MODE_NORMAL && mode1 < OMM_SPARKLY_MODE_COUNT) {
        
            // Check flags
            if (OMM_MEMCMP(flags1, flags2, sizeof(flags1))) {
                OMM_MEMCPY(sOmmSparklySaveDataFlags[mode1], flags1, sizeof(sOmmSparklySaveDataFlags[mode1]));
            } else {
                OMM_MEMSET(sOmmSparklySaveDataFlags[mode1], 0, sizeof(sOmmSparklySaveDataFlags[mode1]));
            }
        
            // Check timers
            if (timer1 == timer2) {
                sOmmSparklySaveDataTimer[mode1] = timer1;
            } else {
                sOmmSparklySaveDataTimer[mode1] = OMM_SPARKLY_SAVE_TIMER_MAX;
            }
        }
        return true;
    }
    return false;
}

#define write(...) { *buffer += sprintf(*buffer, __VA_ARGS__); }
static void write_chars(char **buffer, s32 value, s32 count, bool shuffle) {
    for (s32 i = 0; i != count; ++i) {
        char c = i2c(value % 62, shuffle, false);
        write("%c", c);
        value /= 62;
    }
}

void omm_sparkly_write_save(char **buffer) {
    srand(random_u16());
    write("[sparkly_stars]\n");
    for (s32 mode = OMM_SPARKLY_MODE_NORMAL; mode != OMM_SPARKLY_MODE_COUNT; ++mode) {
        if (omm_sparkly_is_mode_unlocked(mode)) {
            write("sparkly_stars = ");
            s32 seed = ((rand() % 3843) + 1);
            write_chars(buffer, seed, 2, false);
            srand(seed);
            for (s32 j = 0; j != 2; ++j) {
                for (s32 i = 0; i != OMM_SPARKLY_SAVE_FLAG_COUNT; ++i) {
                    write_chars(buffer, sOmmSparklySaveDataFlags[mode][i] ? 1 : 0, 1, true);
                }
                write_chars(buffer, sOmmSparklySaveDataTimer[mode], 4, true);
                write_chars(buffer, mode, 1, true);
            }
            write("\n");
        }
    }
    write("\n");
}

//
// Update
//

inline static void omm_sparkly_update_flags(s32 mode, s32 from, s32 to, bool set) {
    if (OMM_LIKELY(mode >= 1 && mode < OMM_SPARKLY_MODE_COUNT)) {
        OMM_MEMSET(sOmmSparklySaveDataFlags[mode] + from, set, 1 + to - from);
    }
}

OMM_ROUTINE_UPDATE(omm_sparkly_update_save_data) {
    for (s32 mode = OMM_SPARKLY_MODE_NORMAL; mode != OMM_SPARKLY_MODE_COUNT; ++mode) {

        // If flag 0 is not set, unset flags 1~32
        if (!omm_sparkly_is_mode_unlocked(mode)) {
            omm_sparkly_update_flags(mode, OMM_SPARKLY_SAVE_FLAG_TIMER_STARTED, OMM_SPARKLY_SAVE_FLAG_GRAND_STAR, false);
        }

        // If flag 1 is not set, set total time to 0, and unset flags 2~32
        if (!omm_sparkly_is_timer_started(mode)) {
            omm_sparkly_update_flags(mode, OMM_SPARKLY_SAVE_FLAG_STARS, OMM_SPARKLY_SAVE_FLAG_GRAND_STAR, false);
            sOmmSparklySaveDataTimer[mode] = 0;
        }

        // If flags 2~30 are not all set, unset flags 31~32
        for (s32 i = 0; i != 29; ++i) {
            if (!omm_sparkly_is_star_collected(mode, i)) {
                omm_sparkly_update_flags(mode, OMM_SPARKLY_SAVE_FLAG_BOWSER_4_UNLOCKED, OMM_SPARKLY_SAVE_FLAG_GRAND_STAR, false);
                break;
            }
        }

        // If flag 31 is not set, unset flag 32
        if (!omm_sparkly_is_bowser_4_unlocked(mode)) {
            omm_sparkly_update_flags(mode, OMM_SPARKLY_SAVE_FLAG_GRAND_STAR, OMM_SPARKLY_SAVE_FLAG_GRAND_STAR, false);
        }

        // If flag 32 is set, set flag 33
        if (omm_sparkly_is_grand_star_collected(mode)) {
            omm_sparkly_update_flags(mode, OMM_SPARKLY_SAVE_FLAG_MODE_COMPLETED, OMM_SPARKLY_SAVE_FLAG_MODE_COMPLETED, true);
        }

        // Unlock current mode and next mode if flag 33 is set, otherwise unset all flags of next mode (including flag 33)
        if (omm_sparkly_is_mode_completed(mode)) {
            omm_sparkly_update_flags(mode, OMM_SPARKLY_SAVE_FLAG_MODE_UNLOCKED, OMM_SPARKLY_SAVE_FLAG_MODE_UNLOCKED, true);
            omm_sparkly_update_flags(mode + 1, OMM_SPARKLY_SAVE_FLAG_MODE_UNLOCKED, OMM_SPARKLY_SAVE_FLAG_MODE_UNLOCKED, true);
        } else if (mode + 1 < OMM_SPARKLY_MODE_COUNT) {
            omm_sparkly_update_flags(mode + 1, OMM_SPARKLY_SAVE_FLAG_MODE_UNLOCKED, OMM_SPARKLY_SAVE_FLAG_MODE_COMPLETED, false);
        }
    }

    // If Sparkly Stars mode is enabled, advance the timer
    if (omm_sparkly_is_enabled()) {
        s32 mode = omm_sparkly_get_current_mode();
        if (!omm_sparkly_is_grand_star_collected(mode)) {
            sOmmSparklySaveDataTimer[mode] = omm_min_s(sOmmSparklySaveDataTimer[mode] + 1, OMM_SPARKLY_SAVE_TIMER_MAX);
        }
    }
}
