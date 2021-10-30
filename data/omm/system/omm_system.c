#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS
s16 gOmmHardMode = FALSE;

//
// Data
//

typedef struct OmmRoutine {
    s32 type;
    void (*func)(void);
    struct OmmRoutine *next;
} OmmRoutine;

static OmmRoutine sOmmRoutines = { 0, NULL, NULL };
static bool sOmmIsMainMenu = true;
static bool sOmmIsLevelEntry = false;
static bool sOmmIsEndingCutscene = false;
static bool sOmmIsEndingCakeScreen = false;
static s32  sOmmTimerReturnToMainMenu = 0;

//
// Complete save cheat
// Press C-Up, C-Up, C-Down, C-Down, C-Left, C-Right, C-Left, C-Right, Z, R and select a file with A
// Collects all stars, sets all flags, opens all cannons and unlocks Peach
//

static const u16 sOmmCompleteSaveButtons[] = { U_CBUTTONS, U_CBUTTONS, D_CBUTTONS, D_CBUTTONS, L_CBUTTONS, R_CBUTTONS, L_CBUTTONS, R_CBUTTONS, Z_TRIG, R_TRIG, A_BUTTON, 0xFFFF };
static s32 sOmmCompleteSaveSequenceIndex = 0;
extern void omm_set_complete_save_file(s32 fileIndex);
extern void omm_stars_init_bits();

//
// Routines
//

void omm_add_routine(s32 type, void (*func)(void)) {
    for (OmmRoutine *routine = &sOmmRoutines;; routine = routine->next) {

        // Existing routine
        if (routine->type == type &&
            routine->func == func) {
            return;
        }

        // New routine
        if (routine->next == NULL) {
            routine->next = OMM_MEMNEW(OmmRoutine, 1);
            routine->next->type = type;
            routine->next->func = func;
            routine->next->next = NULL;
            return;
        }
    }
}

static void omm_execute_routines(s32 type) {
    for (OmmRoutine *routine = &sOmmRoutines; routine != NULL; routine = routine->next) {
        if (routine->type == type &&
            routine->func != NULL) {
            routine->func();
        }
    }
}

//
// Main Menu
//

void omm_select_save_file(s32 saveFileNum) {
    gMarioState->health = omm_health_get_max(0);
    gCurrSaveFileNum = saveFileNum;
    sOmmIsMainMenu = false;
    sOmmIsLevelEntry = false;
    sOmmIsEndingCutscene = false;
    if (sOmmCompleteSaveSequenceIndex == OMM_ARRAY_SIZE(sOmmCompleteSaveButtons) - 1) {
        omm_set_complete_save_file(saveFileNum - 1);
    }
    sOmmCompleteSaveSequenceIndex = 0;
}

void omm_return_to_main_menu() {
    if (optmenu_open) optmenu_toggle();
    fade_into_special_warp(-2, 0);
    gDialogBoxState = 0;
    gMenuMode = -1;
}

//
// Update
//

void omm_update() {
    omm_debug_start_counter();

    // Level entry
    if (sOmmIsLevelEntry) {
        omm_execute_routines(OMM_ROUTINE_TYPE_LEVEL_ENTRY);
        sOmmIsLevelEntry = false;
    }

    // Update routines
    omm_execute_routines(OMM_ROUTINE_TYPE_UPDATE);

    // Complete save cheat inputs
    if (sOmmIsMainMenu) {
        if (gPlayer1Controller->buttonPressed != 0) {
            u16 buttonPressed = gPlayer1Controller->buttonPressed;
            u16 buttonRequired = sOmmCompleteSaveButtons[sOmmCompleteSaveSequenceIndex];
            if ((buttonPressed & buttonRequired) == buttonRequired) {
                sOmmCompleteSaveSequenceIndex++;
                if (sOmmCompleteSaveButtons[sOmmCompleteSaveSequenceIndex] == A_BUTTON) {
                    play_sound(SOUND_GENERAL2_RIGHT_ANSWER | 0xFF00, gGlobalSoundArgs);
                }
            } else {
                sOmmCompleteSaveSequenceIndex = 0;
            }
        }
    } else {
        sOmmCompleteSaveSequenceIndex = 0;
    }

    // Inhibit inputs (except camera controls) during a transition
    if (omm_is_transition_active() || omm_is_warping()) {
        for (s32 i = 0; i != OMM_ARRAY_SIZE(gControllers); ++i) {
            gControllers[i].rawStickX      = 0;
            gControllers[i].rawStickY      = 0;
            gControllers[i].stickX         = 0;
            gControllers[i].stickY         = 0;
            gControllers[i].stickMag       = 0;
            gControllers[i].buttonDown    &= (U_CBUTTONS | D_CBUTTONS | L_CBUTTONS | R_CBUTTONS | L_TRIG | R_TRIG);
            gControllers[i].buttonPressed &= (U_CBUTTONS | D_CBUTTONS | L_CBUTTONS | R_CBUTTONS | L_TRIG | R_TRIG);
        }
    }

    // Misc stuff
    sOmmTimerReturnToMainMenu--;
    gPrevFrameObjectCount = 0;
    omm_debug_end_counter();
    omm_debug_start_counter();
}

//
// Update GFX
//

static void omm_gfx_update_stars_models() {
#if !defined(SMMS)
    static struct GraphNode *sOmmStarGraphNodes[34] = { NULL };
    if (OMM_UNLIKELY(!sOmmStarGraphNodes[0])) {
        sOmmStarGraphNodes[0]  = omm_geo_get_graph_node(omm_geo_star_0_opaque, true);
        sOmmStarGraphNodes[1]  = omm_geo_get_graph_node(omm_geo_star_1_opaque, true);
        sOmmStarGraphNodes[2]  = omm_geo_get_graph_node(omm_geo_star_2_opaque, true);
        sOmmStarGraphNodes[3]  = omm_geo_get_graph_node(omm_geo_star_3_opaque, true);
        sOmmStarGraphNodes[4]  = omm_geo_get_graph_node(omm_geo_star_4_opaque, true);
        sOmmStarGraphNodes[5]  = omm_geo_get_graph_node(omm_geo_star_5_opaque, true);
        sOmmStarGraphNodes[6]  = omm_geo_get_graph_node(omm_geo_star_6_opaque, true);
        sOmmStarGraphNodes[7]  = omm_geo_get_graph_node(omm_geo_star_7_opaque, true);
        sOmmStarGraphNodes[8]  = omm_geo_get_graph_node(omm_geo_star_8_opaque, true);
        sOmmStarGraphNodes[9]  = omm_geo_get_graph_node(omm_geo_star_9_opaque, true);
        sOmmStarGraphNodes[10] = omm_geo_get_graph_node(omm_geo_star_10_opaque, true);
        sOmmStarGraphNodes[11] = omm_geo_get_graph_node(omm_geo_star_11_opaque, true);
        sOmmStarGraphNodes[12] = omm_geo_get_graph_node(omm_geo_star_12_opaque, true);
        sOmmStarGraphNodes[13] = omm_geo_get_graph_node(omm_geo_star_13_opaque, true);
        sOmmStarGraphNodes[14] = omm_geo_get_graph_node(omm_geo_star_14_opaque, true);
        sOmmStarGraphNodes[15] = omm_geo_get_graph_node(omm_geo_star_15_opaque, true);
        sOmmStarGraphNodes[16] = omm_geo_get_graph_node(omm_geo_star_16_opaque, true);
        sOmmStarGraphNodes[17] = omm_geo_get_graph_node(omm_geo_star_0_transparent, true);
        sOmmStarGraphNodes[18] = omm_geo_get_graph_node(omm_geo_star_1_transparent, true);
        sOmmStarGraphNodes[19] = omm_geo_get_graph_node(omm_geo_star_2_transparent, true);
        sOmmStarGraphNodes[20] = omm_geo_get_graph_node(omm_geo_star_3_transparent, true);
        sOmmStarGraphNodes[21] = omm_geo_get_graph_node(omm_geo_star_4_transparent, true);
        sOmmStarGraphNodes[22] = omm_geo_get_graph_node(omm_geo_star_5_transparent, true);
        sOmmStarGraphNodes[23] = omm_geo_get_graph_node(omm_geo_star_6_transparent, true);
        sOmmStarGraphNodes[24] = omm_geo_get_graph_node(omm_geo_star_7_transparent, true);
        sOmmStarGraphNodes[25] = omm_geo_get_graph_node(omm_geo_star_8_transparent, true);
        sOmmStarGraphNodes[26] = omm_geo_get_graph_node(omm_geo_star_9_transparent, true);
        sOmmStarGraphNodes[27] = omm_geo_get_graph_node(omm_geo_star_10_transparent, true);
        sOmmStarGraphNodes[28] = omm_geo_get_graph_node(omm_geo_star_11_transparent, true);
        sOmmStarGraphNodes[29] = omm_geo_get_graph_node(omm_geo_star_12_transparent, true);
        sOmmStarGraphNodes[30] = omm_geo_get_graph_node(omm_geo_star_13_transparent, true);
        sOmmStarGraphNodes[31] = omm_geo_get_graph_node(omm_geo_star_14_transparent, true);
        sOmmStarGraphNodes[32] = omm_geo_get_graph_node(omm_geo_star_15_transparent, true);
        sOmmStarGraphNodes[33] = omm_geo_get_graph_node(omm_geo_star_16_transparent, true);
    }

    s32 starColor = omm_clamp_s(gCurrCourseNum, 0, 16);
    if (gCurrLevelNum != LEVEL_BOWSER_1 && gCurrLevelNum != LEVEL_BOWSER_2) {
        for_each_until_null(const BehaviorScript *, bhv, omm_obj_get_star_behaviors()) {
            for_each_object_with_behavior(obj, *bhv) {
                if (gOmmExtrasColoredStars) {
                    if (obj_check_model(obj, MODEL_STAR) || (obj->behavior == bhvCelebrationStar)) {
                        obj->oGraphNode = sOmmStarGraphNodes[starColor];
                    } else if (obj_check_model(obj, MODEL_TRANSPARENT_STAR)) {
                        obj->oGraphNode = sOmmStarGraphNodes[starColor + 17 * !omm_is_ending_cutscene()];
                    }
                } else {
                    for (s32 i = 0; i != 34; ++i) {
                        if (obj_has_graph_node(obj, sOmmStarGraphNodes[i])) {
                            obj->oGraphNode = gLoadedGraphNodes[i < 17 ? MODEL_STAR : MODEL_TRANSPARENT_STAR];
                            break;
                        }
                    }
                }
            }
        }
    }
#endif
}

static void omm_gfx_update_caps_models() {
    s32 playerCaps[] = {
        omm_player_get_selected_normal_cap(),
        omm_player_get_selected_wing_cap(),
        omm_player_get_selected_metal_cap(),
        omm_player_get_selected_winged_metal_cap()
    };
    for (s32 i = 0; i != OMM_NUM_PLAYABLE_CHARACTERS; ++i) {
        s32 characterCaps[] = {
            omm_player_get_normal_cap(i),
            omm_player_get_wing_cap(i),
            omm_player_get_metal_cap(i),
            omm_player_get_winged_metal_cap(i)
        };
        for_each_until_null(const BehaviorScript *, bhv, omm_obj_get_cap_behaviors()) {
            for_each_object_with_behavior(obj, *bhv) {
                for (s32 i = 0; i != 4; ++i) {
                    if (obj_check_model(obj, characterCaps[i])) {
                        obj->oGraphNode = gLoadedGraphNodes[playerCaps[i]];
                        break;
                    }
                }
            }
        }
    }
}

void omm_update_gfx() {
    omm_debug_end_counter();
    omm_debug_start_counter();
    omm_execute_routines(OMM_ROUTINE_TYPE_GFX);

    // Mario
    if (gMarioState->marioBodyState && gOmmExtrasInvisibleMode) {
        gMarioState->marioBodyState->modelState &= ~0xFF;
        gMarioState->marioBodyState->modelState |= 0x100;
    }

    // Crystal sparkles
    if ((gOmmExtrasCrystalStarsReward &= omm_sparkly_is_mode_completed(OMM_SPARKLY_MODE_HARD)) && gMarioObject && !omm_is_game_paused() &&
        ((gGlobalTimer & 1) || vec3f_dist(gMarioState->pos, gOmmData->mario->state.previous.pos) > 20.f)) {
        omm_spawn_sparkly_star_sparkle_mario(gMarioObject, OMM_SPARKLY_MODE_HARD, 60.f, 10.f, 0.5f, 30.f);
    }

    // Object models
    omm_gfx_update_stars_models();
    omm_gfx_update_caps_models();

    // Invisible mode
    for_each_until_null(const BehaviorScript *, bhv, omm_obj_get_player_behaviors()) {
        for_each_object_with_behavior(obj, *bhv) {
            if (gOmmExtrasInvisibleMode) {
                obj->header.gfx.node.flags |= GRAPH_RENDER_INVISIBLE;
            } else {
                obj->header.gfx.node.flags &= ~GRAPH_RENDER_INVISIBLE;
            }
        }
    }
    omm_debug_end_counter();
    omm_debug_start_counter();
}

//
// Level commands
//

static const uintptr_t cmd_sm64_splash_screen[] = { CALL(0, lvl_intro_update) };
static const uintptr_t cmd_goddard_head_hello[] = { CALL_LOOP(1, lvl_intro_update) };
static const uintptr_t cmd_goddard_head_dizzy[] = { CALL_LOOP(2, lvl_intro_update) };
static const uintptr_t cmd_debug_level_select[] = { CALL_LOOP(3, lvl_intro_update) };
static const uintptr_t cmd_level_entry_start[]  = { CALL(0, lvl_init_or_update) };
static const uintptr_t cmd_star_select_start[]  = { EXECUTE(0x14, _menuSegmentRomStart, _menuSegmentRomEnd, level_main_menu_entry_2) };
static const uintptr_t cmd_skip_star_select[]   = { EXIT() };
static const uintptr_t cmd_ending_cake_start[]  = { AREA(1, ending_geo_000050) };
static const uintptr_t cmd_ending_cake_sound[]  = { CALL(0, lvl_play_the_end_screen_sound) };

void *omm_update_cmd(void *cmd, s32 reg) {

    // Main Menu
    if (OMM_MEMCMP(cmd, cmd_sm64_splash_screen, sizeof(cmd_sm64_splash_screen)) ||
        OMM_MEMCMP(cmd, cmd_goddard_head_hello, sizeof(cmd_goddard_head_hello)) ||
        OMM_MEMCMP(cmd, cmd_goddard_head_dizzy, sizeof(cmd_goddard_head_dizzy)) ||
        OMM_MEMCMP(cmd, cmd_debug_level_select, sizeof(cmd_debug_level_select))) {
        gMarioState->action = 0;
        sOmmIsMainMenu = true;
        sOmmIsLevelEntry = false;
        sOmmIsEndingCutscene = false;
        sOmmIsEndingCakeScreen = false;
    }

    // Level Entry
    if (OMM_MEMCMP(cmd, cmd_level_entry_start, sizeof(cmd_level_entry_start))) {
        sOmmIsMainMenu = false;
        sOmmIsLevelEntry = true;
        sOmmIsEndingCakeScreen = false;
        omm_stars_init_bits();
    }

    // Star Select
    if (OMM_MEMCMP(cmd, cmd_star_select_start, sizeof(cmd_star_select_start)) && OMM_STARS_NON_STOP_NOT_ENDING_CUTSCENE) {
        gCurrLevelNum = reg;
        gCurrCourseNum = omm_level_get_course(reg);
        gCurrActNum = 1;
#if !defined(SM74)
        gCurrAreaIndex = 1;
#endif
        gDialogCourseActNum = gCurrActNum;

        // Reset the coin counter and star flags unless Mario enters a Bowser fight or returns to the Castle
        if (gCurrLevelNum != LEVEL_BOWSER_1 &&
            gCurrLevelNum != LEVEL_BOWSER_2 &&
            gCurrLevelNum != LEVEL_BOWSER_3 &&
            gCurrLevelNum != LEVEL_CASTLE   &&
            gCurrLevelNum != LEVEL_GROUNDS  &&
            gCurrLevelNum != LEVEL_COURT) {
            gMarioState->numCoins = 0;
            gHudDisplay.coins = 0;
            gCurrCourseStarFlags = save_file_get_star_flags(gCurrSaveFileNum - 1, gCurrCourseNum - 1);
        }

        // Return an EXIT command to skip the star select screen
        return (void *) cmd_skip_star_select;
    }

    // Ending Cutscene
    if (gMarioState->action == ACT_JUMBO_STAR_CUTSCENE ||
        gMarioState->action == ACT_END_WAVING_CUTSCENE ||
        gMarioState->action == ACT_END_PEACH_CUTSCENE ||
        gMarioState->action == ACT_CREDITS_CUTSCENE) {
        sOmmIsMainMenu = false;
        sOmmIsEndingCutscene = true;
    }

    // Ending return to Main Menu
    switch (sOmmTimerReturnToMainMenu) {

        // Sets sOmmIsEndingCakeScreen to true if loading the cake screen
        // Starts the timer when hearing the "Thank you so much"
        default: {
            if (OMM_MEMCMP(cmd, cmd_ending_cake_start, sizeof(cmd_ending_cake_start))) {
                sOmmIsEndingCakeScreen = true;
            }
            if (OMM_MEMCMP(cmd, cmd_ending_cake_sound, sizeof(cmd_ending_cake_sound))) {
                sOmmTimerReturnToMainMenu = 300;
            }
        } break;

        // After 5 seconds, stops the timer, and wait for the player to press A or Start
        case 150: {
            if (gPlayer1Controller->buttonPressed & (A_BUTTON | START_BUTTON)) {
                play_transition(WARP_TRANSITION_FADE_INTO_COLOR, 90, 0, 0, 0);
            } else {
                sOmmTimerReturnToMainMenu++;
            }
        } break;

        // After 5 other seconds and a transition, return to the main menu
        case 0: {
            gHudDisplay.flags = 0;
            sOmmTimerReturnToMainMenu = -1;
            sOmmIsEndingCakeScreen = false;
            return (void *) level_intro_goddard;
        } break;
    }

    // Warp update
    return omm_update_warp(cmd, sOmmIsLevelEntry);
}

//
// Game states
//

bool omm_is_main_menu() {
    return sOmmIsMainMenu;
}

bool omm_is_game_paused() {
    return gMenuMode != -1;
}

bool omm_is_transition_active() {
    static s16 sOmmTransitionTimer = 0;
    if (sTransitionTimer > 0) sOmmTransitionTimer = sTransitionTimer;
    bool isTransitionActive = (gWarpTransition.isActive || sOmmTransitionTimer > 0);
    sOmmTransitionTimer = sTransitionTimer;
    return isTransitionActive;
}

bool omm_is_ending_cutscene() {
    return sOmmIsEndingCutscene;
}

bool omm_is_ending_cake_screen() {
    return sOmmIsEndingCakeScreen;
}

//
// Sanity checks
//

bool omm_sanity_check_graph_node(struct GraphNode *node) {
    return (node       != NULL                                ) && (
           (node->type == GRAPH_NODE_TYPE_ROOT                ) ||
           (node->type == GRAPH_NODE_TYPE_ORTHO_PROJECTION    ) ||
           (node->type == GRAPH_NODE_TYPE_PERSPECTIVE         ) ||
           (node->type == GRAPH_NODE_TYPE_MASTER_LIST         ) ||
           (node->type == GRAPH_NODE_TYPE_START               ) ||
           (node->type == GRAPH_NODE_TYPE_LEVEL_OF_DETAIL     ) ||
           (node->type == GRAPH_NODE_TYPE_SWITCH_CASE         ) ||
           (node->type == GRAPH_NODE_TYPE_CAMERA              ) ||
           (node->type == GRAPH_NODE_TYPE_TRANSLATION_ROTATION) ||
           (node->type == GRAPH_NODE_TYPE_TRANSLATION         ) ||
           (node->type == GRAPH_NODE_TYPE_ROTATION            ) ||
           (node->type == GRAPH_NODE_TYPE_OBJECT              ) ||
           (node->type == GRAPH_NODE_TYPE_ANIMATED_PART       ) ||
           (node->type == GRAPH_NODE_TYPE_BILLBOARD           ) ||
           (node->type == GRAPH_NODE_TYPE_DISPLAY_LIST        ) ||
           (node->type == GRAPH_NODE_TYPE_SCALE               ) ||
           (node->type == GRAPH_NODE_TYPE_SHADOW              ) ||
           (node->type == GRAPH_NODE_TYPE_OBJECT_PARENT       ) ||
           (node->type == GRAPH_NODE_TYPE_GENERATED_LIST      ) ||
           (node->type == GRAPH_NODE_TYPE_BACKGROUND          ) ||
           (node->type == GRAPH_NODE_TYPE_HELD_OBJ            ) ||
           (node->type == GRAPH_NODE_TYPE_CULLING_RADIUS      ));
}
