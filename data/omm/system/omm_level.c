#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// Data
//

#define STUB_LEVEL(_0, _1, courseenum, _3, _4, _5, _6, _7, _8) courseenum,
#define DEFINE_LEVEL(_0, _1, courseenum, _3, _4, _5, _6, _7, _8, _9, _10) courseenum,
s8 gLevelToCourseNumTable[] = {
#include "levels/level_defines.h"
};
#undef STUB_LEVEL
#undef DEFINE_LEVEL

typedef struct {
    s16 area; // 0
    s16 id; // 1
    s16 type; // 2
    s16 x; // 3
    s16 y; // 4
    s16 z; // 5
    s16 yaw; // 6
    s16 destLevel; // 7
    s16 destArea; // 8
    s16 destId; // 9
} OmmWarpData;

typedef struct {
    const LevelScript *script;
    OmmWarpData warps[0x100];
    s32 warpCount;
    s32 areas;
    s32 redCoins[8];
} OmmLevelData;

static OmmLevelData sOmmLevelData[LEVEL_COUNT] = { 0 };
static s32 sOmmLevelList[LEVEL_COUNT] = { 0 }; // Ordered by Course Id, COURSE_NONE excluded
static s32 sOmmLevelCount = 0;

//
// Init
//

static OmmWarpData *omm_level_get_warp_data(s32 level, u8 area, u8 id) {

    // Existing warp
    for (s32 i = 0; i != sOmmLevelData[level].warpCount; ++i) {
        if (sOmmLevelData[level].warps[i].area == area &&
            sOmmLevelData[level].warps[i].id == id) {
            return &sOmmLevelData[level].warps[i];
        }
    }

    // New warp
    OmmWarpData *warp = &sOmmLevelData[level].warps[sOmmLevelData[level].warpCount++];
    if (warp->area == 0) {
        warp->area = area;
        warp->id = id;
        warp->type = -1;
        warp->x = 0;
        warp->y = 0;
        warp->z = 0;
        warp->yaw = 0;
        warp->destLevel = 0;
        warp->destArea = 0;
        warp->destId = 0;
    }
    return warp;
};

static s32 omm_level_preprocess_master_script(u8 type, void *cmd) {
    static bool sScriptExecLevelTable = false;
    static s32 sLevelNum = -1;
    
    if (!sScriptExecLevelTable) {
        switch (type) {
            case 0x06: { // JUMP_LINK
                sScriptExecLevelTable = true;
                return OMM_LEVEL_SCRIPT_CONTINUE;
            } break;
        }
    } else {
        switch (type) {
            case 0x00: { // EXECUTE
                const LevelScript *script = (const LevelScript *) omm_level_cmd_get(cmd, 0x0C);
                if (sLevelNum >= 0 && sLevelNum < LEVEL_COUNT && !sOmmLevelData[sLevelNum].script) {
                    sOmmLevelData[sLevelNum].script = script;
                }
                sLevelNum = -1;
                return OMM_LEVEL_SCRIPT_RETURN;
            } break;

            case 0x02: { // EXIT
                return OMM_LEVEL_SCRIPT_STOP;
            } break;

            case 0x03: { // SLEEP
                return OMM_LEVEL_SCRIPT_STOP;
            } break;

            case 0x0C: { // JUMP_IF
                sLevelNum = (s32) omm_level_cmd_get(cmd, 0x04);
                return OMM_LEVEL_SCRIPT_CONTINUE;
            } break;
        }
    }
    return OMM_LEVEL_SCRIPT_CONTINUE;
}

static s32 sCurrentLevelNum;
static s32 omm_level_fill_warp_data(u8 type, void *cmd) {
    static u8 sCurrentAreaIndex = 0;
    switch (type) {
        case 0x03:   // SLEEP
        case 0x04: { // SLEEP_BEFORE_EXIT
            return OMM_LEVEL_SCRIPT_STOP;
        } break;

        case 0x1F: { // AREA
            sCurrentAreaIndex = (u8) omm_level_cmd_get(cmd, 2);
            sOmmLevelData[sCurrentLevelNum].areas |= (1 << sCurrentAreaIndex);
        } break;

        case 0x24: { // OBJECT
            const BehaviorScript *bhv = (const BehaviorScript *) omm_level_cmd_get(cmd, 20);
            sOmmLevelData[sCurrentLevelNum].redCoins[sCurrentAreaIndex] += (bhv == bhvRedCoin);
            for (s32 i = 0; i != 20; ++i) {
                if (sWarpBhvSpawnTable[i] == bhv) {
                    OmmWarpData *warp = omm_level_get_warp_data(sCurrentLevelNum, sCurrentAreaIndex, ((((u32) omm_level_cmd_get(cmd, 16)) >> 16) & 0xFF));
                    if (warp->type == -1) {
                        warp->type = i;
                        warp->x = (s16) omm_level_cmd_get(cmd, 4);
                        warp->y = (s16) omm_level_cmd_get(cmd, 6);
                        warp->z = (s16) omm_level_cmd_get(cmd, 8);
                        warp->yaw = (s16) ((((s32) ((s16) omm_level_cmd_get(cmd, 12))) * 0x8000) / 180);
                    }
                    break;
                }
            }
        } break;

        case 0x26:   // WARP_NODE
        case 0x27: { // PAINTING_WARP_NODE
            OmmWarpData *warp = omm_level_get_warp_data(sCurrentLevelNum, sCurrentAreaIndex, (u8) omm_level_cmd_get(cmd, 2));
            if (warp->destLevel == 0) {
                warp->destLevel = (u8) omm_level_cmd_get(cmd, 3);
                warp->destArea = (u8) omm_level_cmd_get(cmd, 4);
                warp->destId = (u8) omm_level_cmd_get(cmd, 5);
            }
        } break;

        case 0x39: { // MACRO_OBJECTS
            MacroObject *data = (MacroObject *) omm_level_cmd_get(cmd, 4);
            for (; *data != MACRO_OBJECT_END(); data += 5) {
                s32 presetId = (s32) ((data[0] & 0x1FF) - 0x1F);
                sOmmLevelData[sCurrentLevelNum].redCoins[sCurrentAreaIndex] += (presetId == macro_red_coin);
            }
        } break;
    }
    return OMM_LEVEL_SCRIPT_CONTINUE;
}

static void omm_level_init() {
    static bool sInited = false;
    if (OMM_UNLIKELY(!sInited)) {

        // Level scripts
        omm_level_parse_script(level_main_scripts_entry, omm_level_preprocess_master_script);

        // Level warps
        for (sCurrentLevelNum = 0; sCurrentLevelNum != LEVEL_COUNT; ++sCurrentLevelNum) {
            if (sOmmLevelData[sCurrentLevelNum].script) {
                OMM_MEMSET(sOmmLevelData[sCurrentLevelNum].redCoins, 0, sizeof(sOmmLevelData[sCurrentLevelNum].redCoins));
                omm_level_parse_script(sOmmLevelData[sCurrentLevelNum].script, omm_level_fill_warp_data);
            }
        }

        // Level list ordered by course id
        for (s32 i = COURSE_MIN; i <= COURSE_MAX; ++i) {
            if (i == COURSE_CAKE_END) continue;
            for (s32 j = 1; j != LEVEL_COUNT; ++j) {
                if (gLevelToCourseNumTable[j - 1] == i) {
                    sOmmLevelList[sOmmLevelCount++] = j;
                }
            }
        }

        // Done
        sInited = true;
    }
}

//
// Common data
//

inline static void convert_text_and_set_buffer(u8 *buffer, const char *str) {
    u8 *str64 = omm_text_convert(str, false);
    OMM_MEMCPY(buffer, str64, omm_text_length(str64) + 1);
}

s32 omm_level_get_count() {
    omm_level_init();
    return sOmmLevelCount;
}

s32 *omm_level_get_list() {
    omm_level_init();
    return sOmmLevelList;
}

s32 omm_level_get_course(s32 level) {
    omm_level_init();
    return (s32) gLevelToCourseNumTable[level - 1];
}

const LevelScript *omm_level_get_script(s32 level) {
    omm_level_init();
    return sOmmLevelData[level].script;
}

s32 omm_level_get_areas(s32 level) {
    omm_level_init();
    return sOmmLevelData[level].areas;
}

s32 omm_level_get_num_red_coins(s32 level, s32 area) {
    omm_level_init();
    return sOmmLevelData[level].redCoins[area];
}

u8 *omm_level_get_name(s32 level, bool decaps, bool num) {
    omm_level_init();
    static u8 sBuffer[256];
    OMM_MEMSET(sBuffer, 0xFF, 256);
    s32 course = omm_level_get_course(level);

    // Level name
    if (level == LEVEL_BOWSER_1) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_BOWSER_1);
    } else if (level == LEVEL_BOWSER_2) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_BOWSER_2);
    } else if (level == LEVEL_BOWSER_3) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_BOWSER_3);
    } else if (course < COURSE_BOB) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_CASTLE);
    } else if (course >= COURSE_CAKE_END) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_CASTLE);
    } else {
#if defined(SM74)
        const u8 *courseName = (sm74_mode__omm_level_get_name == 2 ?
                                (const u8 **) seg2_course_name_table_EE :
                                (const u8 **) seg2_course_name_table)[course - COURSE_BOB] + 3;
#else
        const u8 *courseName = ((const u8 **) seg2_course_name_table)[course - COURSE_BOB] + 3;
#endif
        OMM_MEMCPY(sBuffer, courseName, omm_text_length(courseName));
    }

    // Decaps
    if (decaps) {
        omm_text_decapitalize(sBuffer);
    }

    // Course number
    if (num && (course >= COURSE_BOB) && (course <= COURSE_STAGES_MAX)) {
        OMM_MEMMOV(sBuffer + 5, sBuffer, omm_text_length(sBuffer));
        sBuffer[0] = ((course / 10) == 0 ? 158 : (course / 10));
        sBuffer[1] = (course % 10);
        sBuffer[2] = 158;
        sBuffer[3] = 159;
        sBuffer[4] = 158;
    }

    return sBuffer;
}

u8 *omm_level_get_act_name(s32 level, s32 act, bool decaps, bool num) {
    omm_level_init();
    static u8 sBuffer[256];
    OMM_MEMSET(sBuffer, 0xFF, 256);
    s32 course = omm_level_get_course(level);

    // Star name
    if (course < COURSE_BOB) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_ONE_SECRET_STAR);
#if OMM_CODE_VANILLA
    } else if (level == LEVEL_BITDW) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_RED_COINS_STAR);
    } else if (level == LEVEL_BITFS) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_RED_COINS_STAR);
    } else if (level == LEVEL_BITS) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_RED_COINS_STAR);
    } else if (course > COURSE_STAGES_MAX) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_EMPTY);
#else
#if defined(SMSR)
    } else if ((const BehaviorScript *) gMarioState->riddenObj == bhvCustomSMSRStarReplica) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_STAR_REPLICA);
#endif
    } else if (course > COURSE_STAGES_MAX) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_STAR__);
        sBuffer[5] = act;
#endif
    } else if (act >= 7) {
        convert_text_and_set_buffer(sBuffer, OMM_TEXT_LEVEL_100_COINS_STAR);
    } else {
#if defined(SM74)
        const u8 *actName = (sm74_mode__omm_level_get_act_name == 2 ?
                             (const u8 **) seg2_act_name_table_EE :
                             (const u8 **) seg2_act_name_table)[(course - COURSE_BOB) * 6 + (act - 1)];
#else
        const u8 *actName = ((const u8 **) seg2_act_name_table)[(course - COURSE_BOB) * 6 + (act - 1)];
#endif
        OMM_MEMCPY(sBuffer, actName, omm_text_length(actName));
    }

    // Decaps
    if (decaps) {
        omm_text_decapitalize(sBuffer);
    }

    // Star number
    if (num && (course >= COURSE_BOB) && (course <= COURSE_STAGES_MAX)) {
        OMM_MEMMOV(sBuffer + 5, sBuffer, omm_text_length(sBuffer));
        sBuffer[0] = ((act / 10) == 0 ? 158 : (act / 10));
        sBuffer[1] = (act % 10);
        sBuffer[2] = 158;
        sBuffer[3] = 159;
        sBuffer[4] = 158;
    }

    return sBuffer;
}

//
// Level Script Preprocessing
// By default,
// - Ifs are always true
// - Skips are always false
// - Loops break after the first loop
//

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

typedef struct {
    u8 type;
    u8 size;
} LvlCmd;

typedef struct {
    u64 data[32];
    s32 baseIndex;
    s32 topIndex;
} Stack;

static void stack_push(Stack *stack, u64 value) {
    if (stack->topIndex >= 0) {
        stack->data[stack->topIndex] = value;
        stack->topIndex++;
    }
}

static u64 stack_pop(Stack *stack) {
    if (stack->topIndex <= 0) {
        return 0;
    }
    stack->topIndex--;
    return stack->data[stack->topIndex];
}

u64 omm_level_cmd_get(void *cmd, u64 offset) {
    offset = (((offset) & 3llu) | (((offset) & ~3llu) << (sizeof(void *) >> 3llu)));
    return *((u64 *) (((u64) cmd) + offset));
}

static LvlCmd *omm_level_cmd_next(void *cmd, u64 size) {
    u64 offset = (((size) & 3llu) | (((size) & ~3llu) << (sizeof(void *) >> 3llu)));
    return (void *) (((u64) cmd) + offset);
}

static LvlCmd *omm_level_cmd_execute(Stack *stack, LvlCmd *cmd) {
    stack_push(stack, (u64) omm_level_cmd_next(cmd, cmd->size));
    stack_push(stack, stack->baseIndex);
    stack->baseIndex = stack->topIndex;
    return (LvlCmd *) omm_level_cmd_get(cmd, 12);
}

static LvlCmd *omm_level_cmd_exit_and_execute(Stack *stack, LvlCmd *cmd) {
    stack->topIndex = stack->baseIndex;
    return (LvlCmd *) omm_level_cmd_get(cmd, 12);
}

static LvlCmd *omm_level_cmd_exit(Stack *stack, LvlCmd *cmd) {
    stack->topIndex = stack->baseIndex;
    stack->baseIndex = (s32) stack_pop(stack);
    return (LvlCmd *) stack_pop(stack);
}

static LvlCmd *omm_level_cmd_sleep(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_sleep_before_exit(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_jump(Stack *stack, LvlCmd *cmd) {
    return (LvlCmd *) omm_level_cmd_get(cmd, 4);
}

static LvlCmd *omm_level_cmd_jump_link(Stack *stack, LvlCmd *cmd) {
    stack_push(stack, (u64) omm_level_cmd_next(cmd, cmd->size));
    return (LvlCmd *) omm_level_cmd_get(cmd, 4);
}

static LvlCmd *omm_level_cmd_return(Stack *stack, UNUSED LvlCmd *cmd) {
    return (LvlCmd *) stack_pop(stack);
}

static LvlCmd *omm_level_cmd_jump_link_push_arg(Stack *stack, LvlCmd *cmd) {
    stack_push(stack, (u64) omm_level_cmd_next(cmd, cmd->size));
    stack_push(stack, omm_level_cmd_get(cmd, 2));
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_jump_repeat(Stack *stack, LvlCmd *cmd) {
    stack->topIndex -= 2;
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_loop_begin(Stack *stack, LvlCmd *cmd) {
    stack_push(stack, (u64) omm_level_cmd_next(cmd, cmd->size));
    stack_push(stack, 0);
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_loop_until(Stack *stack, LvlCmd *cmd) {
    stack->topIndex -= 2;
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_jump_if(Stack *stack, LvlCmd *cmd) {
    stack_push(stack, (u64) omm_level_cmd_next(cmd, cmd->size)); // Not an error, that's intentional
    return (LvlCmd *) omm_level_cmd_get(cmd, 8);
}

static LvlCmd *omm_level_cmd_jump_link_if(Stack *stack, LvlCmd *cmd) {
    stack_push(stack, (u64) omm_level_cmd_next(cmd, cmd->size));
    return (LvlCmd *) omm_level_cmd_get(cmd, 8);
}

static LvlCmd *omm_level_cmd_skip_if(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_skip(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_skip_nop(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_call(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_call_loop(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_set_register(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_push_pool(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_pop_pool(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_load_fixed(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_load_raw(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_load_mio0(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_load_mario_head(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_load_mio0_texture(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_init_level(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_clear_level(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_alloc_level_pool(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_free_level_pool(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_begin_area(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_end_area(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_load_model_from_dl(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_load_model_from_geo(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_23(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_mario(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_object(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_warp_node(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_instant_warp(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_set_terrain_type(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_painting_warp_node(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_3A(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_set_whirlpool(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_set_blackout(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_set_gamma(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_set_terrain(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_set_rooms(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_macro_objects(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_load_area(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_unload_area(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_set_mario_start_pos(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_2C(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_2D(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_set_transition(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_nop(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_show_dialog(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_set_background_music(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_set_menu_music(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_stop_music(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_get_or_set(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_advance_demo(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_clr_demo_pointer(Stack *stack, LvlCmd *cmd) {
    return omm_level_cmd_next(cmd, cmd->size);
}

static LvlCmd *omm_level_cmd_jump_area(Stack *stack, LvlCmd *cmd, s32 (*func)(u8, void *)) {
    omm_level_parse_script((const LevelScript *) omm_level_cmd_get(cmd, 8), func);
    return omm_level_cmd_next(cmd, cmd->size);
}

void omm_level_parse_script(const LevelScript *script, s32 (*func)(u8, void *)) {
    Stack stack;
    stack.baseIndex = -1;
    stack.topIndex = 0;
    for (LvlCmd *cmd = (LvlCmd *) script; cmd != NULL;) {
        u8 type = (cmd->type & 0x3F);
        s32 act = func(type, (void *) cmd);
        switch (act) {
            case OMM_LEVEL_SCRIPT_CONTINUE:
                switch (type) {
                    case 0x00: cmd = omm_level_cmd_execute(&stack, cmd); break;
                    case 0x01: cmd = omm_level_cmd_exit_and_execute(&stack, cmd); break;
                    case 0x02: cmd = omm_level_cmd_exit(&stack, cmd); break;
                    case 0x03: cmd = omm_level_cmd_sleep(&stack, cmd); break;
                    case 0x04: cmd = omm_level_cmd_sleep_before_exit(&stack, cmd); break;
                    case 0x05: cmd = omm_level_cmd_jump(&stack, cmd); break;
                    case 0x06: cmd = omm_level_cmd_jump_link(&stack, cmd); break;
                    case 0x07: cmd = omm_level_cmd_return(&stack, cmd); break;
                    case 0x08: cmd = omm_level_cmd_jump_link_push_arg(&stack, cmd); break;
                    case 0x09: cmd = omm_level_cmd_jump_repeat(&stack, cmd); break;
                    case 0x0A: cmd = omm_level_cmd_loop_begin(&stack, cmd); break;
                    case 0x0B: cmd = omm_level_cmd_loop_until(&stack, cmd); break;
                    case 0x0C: cmd = omm_level_cmd_jump_if(&stack, cmd); break;
                    case 0x0D: cmd = omm_level_cmd_jump_link_if(&stack, cmd); break;
                    case 0x0E: cmd = omm_level_cmd_skip_if(&stack, cmd); break;
                    case 0x0F: cmd = omm_level_cmd_skip(&stack, cmd); break;
                    case 0x10: cmd = omm_level_cmd_skip_nop(&stack, cmd); break;
                    case 0x11: cmd = omm_level_cmd_call(&stack, cmd); break;
                    case 0x12: cmd = omm_level_cmd_call_loop(&stack, cmd); break;
                    case 0x13: cmd = omm_level_cmd_set_register(&stack, cmd); break;
                    case 0x14: cmd = omm_level_cmd_push_pool(&stack, cmd); break;
                    case 0x15: cmd = omm_level_cmd_pop_pool(&stack, cmd); break;
                    case 0x16: cmd = omm_level_cmd_load_fixed(&stack, cmd); break;
                    case 0x17: cmd = omm_level_cmd_load_raw(&stack, cmd); break;
                    case 0x18: cmd = omm_level_cmd_load_mio0(&stack, cmd); break;
                    case 0x19: cmd = omm_level_cmd_load_mario_head(&stack, cmd); break;
                    case 0x1A: cmd = omm_level_cmd_load_mio0_texture(&stack, cmd); break;
                    case 0x1B: cmd = omm_level_cmd_init_level(&stack, cmd); break;
                    case 0x1C: cmd = omm_level_cmd_clear_level(&stack, cmd); break;
                    case 0x1D: cmd = omm_level_cmd_alloc_level_pool(&stack, cmd); break;
                    case 0x1E: cmd = omm_level_cmd_free_level_pool(&stack, cmd); break;
                    case 0x1F: cmd = omm_level_cmd_begin_area(&stack, cmd); break;
                    case 0x20: cmd = omm_level_cmd_end_area(&stack, cmd); break;
                    case 0x21: cmd = omm_level_cmd_load_model_from_dl(&stack, cmd); break;
                    case 0x22: cmd = omm_level_cmd_load_model_from_geo(&stack, cmd); break;
                    case 0x23: cmd = omm_level_cmd_23(&stack, cmd); break;
                    case 0x24: cmd = omm_level_cmd_object(&stack, cmd); break;
                    case 0x25: cmd = omm_level_cmd_mario(&stack, cmd); break;
                    case 0x26: cmd = omm_level_cmd_warp_node(&stack, cmd); break;
                    case 0x27: cmd = omm_level_cmd_painting_warp_node(&stack, cmd); break;
                    case 0x28: cmd = omm_level_cmd_instant_warp(&stack, cmd); break;
                    case 0x29: cmd = omm_level_cmd_load_area(&stack, cmd); break;
                    case 0x2A: cmd = omm_level_cmd_unload_area(&stack, cmd); break;
                    case 0x2B: cmd = omm_level_cmd_set_mario_start_pos(&stack, cmd); break;
                    case 0x2C: cmd = omm_level_cmd_2C(&stack, cmd); break;
                    case 0x2D: cmd = omm_level_cmd_2D(&stack, cmd); break;
                    case 0x2E: cmd = omm_level_cmd_set_terrain(&stack, cmd); break;
                    case 0x2F: cmd = omm_level_cmd_set_rooms(&stack, cmd); break;
                    case 0x30: cmd = omm_level_cmd_show_dialog(&stack, cmd); break;
                    case 0x31: cmd = omm_level_cmd_set_terrain_type(&stack, cmd); break;
                    case 0x32: cmd = omm_level_cmd_nop(&stack, cmd); break;
                    case 0x33: cmd = omm_level_cmd_set_transition(&stack, cmd); break;
                    case 0x34: cmd = omm_level_cmd_set_blackout(&stack, cmd); break;
                    case 0x35: cmd = omm_level_cmd_set_gamma(&stack, cmd); break;
                    case 0x36: cmd = omm_level_cmd_set_background_music(&stack, cmd); break;
                    case 0x37: cmd = omm_level_cmd_set_menu_music(&stack, cmd); break;
                    case 0x38: cmd = omm_level_cmd_stop_music(&stack, cmd); break;
                    case 0x39: cmd = omm_level_cmd_macro_objects(&stack, cmd); break;
                    case 0x3A: cmd = omm_level_cmd_3A(&stack, cmd); break;
                    case 0x3B: cmd = omm_level_cmd_set_whirlpool(&stack, cmd); break;
                    case 0x3C: cmd = omm_level_cmd_get_or_set(&stack, cmd); break;
                    case 0x3D: cmd = omm_level_cmd_advance_demo(&stack, cmd); break;
                    case 0x3E: cmd = omm_level_cmd_clr_demo_pointer(&stack, cmd); break;
                    case 0x3F: cmd = omm_level_cmd_jump_area(&stack, cmd, func); break;
                } break;

            case OMM_LEVEL_SCRIPT_NEXT_CMD:
                cmd = omm_level_cmd_next(cmd, cmd->size);
                break;

            case OMM_LEVEL_SCRIPT_RETURN:
                cmd = omm_level_cmd_return(&stack, cmd);
                break;

            case OMM_LEVEL_SCRIPT_STOP:
                return;
        }
    }
}

#pragma GCC diagnostic pop

//
// Warps
//

s16 *omm_level_get_warp(s32 level, s32 area, u8 id) {
    omm_level_init();
    for (s32 i = 0; i != 0x100; ++i) {
        if (sOmmLevelData[level].warps[i].area == area &&
            sOmmLevelData[level].warps[i].id == id) {
            return (s16 *) &sOmmLevelData[level].warps[i];
        }
    }
    return NULL;
}

s16 *omm_level_get_entry_warp(s32 level, s32 area) {
    omm_level_init();
#if OMM_CODE_VANILLA
    if (level == LEVEL_TTM && area > 2) return NULL;
#endif
    return omm_level_get_warp(level, area, OMM_LEVEL_ENTRY_WARP(level));
}

s16 *omm_level_get_exit_warp(s32 level, s32 area) {
    omm_level_init();
    return omm_level_get_warp(level, area, 0xF0);
}

s16 *omm_level_get_death_warp(s32 level, s32 area) {
    omm_level_init();
    s16 *warp = omm_level_get_warp(level, area, 0xF1);
    if (!warp) warp = omm_level_get_warp(level, area, 0xF3);
    return warp;
}

bool omm_level_can_warp(s32 level) {
    omm_level_init();
    return omm_level_get_course(level) != COURSE_NONE;
}
