#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS
extern void omm_sparkly_level_update(struct MarioState *m, const OmmSparklyStarsData *data);

//
// Struct
//

static struct {
    const OmmSparklyStarsData *data;
    struct Object *star;
    s32 state;
    s32 flags;
    u64 counters[2];
} sOmmSparklyContext[1];
struct Object **sOmmSparklyStar = &sOmmSparklyContext->star;

//
// Getters/Setters
//

bool omm_sparkly_context_get_flag(s32 flag) {
    return (sOmmSparklyContext->flags & flag) != 0;
}

void omm_sparkly_context_set_flag(s32 flag, bool cond, bool unsetIfFalse) {
    if (cond) {
        sOmmSparklyContext->flags |= (flag);
    } else if (unsetIfFalse) {
        sOmmSparklyContext->flags &= ~(flag);
    }
}

void omm_sparkly_context_unset_flag(s32 flag, bool cond) {
    if (cond) {
        sOmmSparklyContext->flags &= ~(flag);
    }
}

s32 omm_sparkly_context_get_counter(s32 counter) {
    return (s32) ((sOmmSparklyContext->counters[0] >> (counter * 8llu)) & 0xFFllu);
}

void omm_sparkly_context_set_counter(s32 counter, s32 value) {
    u64 bits = 0xFFllu << (counter * 8llu);
    sOmmSparklyContext->counters[0] &= ~bits;
    sOmmSparklyContext->counters[0] |= ((u64) (value & 0xFFllu) << (counter * 8llu));
}

void omm_sparkly_context_inc_counter(s32 counter, s32 inc) {
    u64 bits = 0xFFllu << (counter * 8llu);
    s32 value = omm_sparkly_context_get_counter(counter);
    sOmmSparklyContext->counters[0] &= ~bits;
    sOmmSparklyContext->counters[0] |= ((u64) ((value + inc) & 0xFFllu) << (counter * 8llu));
}

s32 omm_sparkly_context_dif_counter(s32 counter) {
    s32 curr = (s32) ((sOmmSparklyContext->counters[0] >> (counter * 8llu)) & 0xFFllu);
    s32 prev = (s32) ((sOmmSparklyContext->counters[1] >> (counter * 8llu)) & 0xFFllu);
    return (curr - prev);
}

bool omm_sparkly_context_check_star(struct Object *star) {
    if (sOmmSparklyContext->data == NULL ||
        sOmmSparklyContext->star == NULL ||
        sOmmSparklyContext->star != star) {
        sOmmSparklyContext->star = NULL;
        obj_mark_for_deletion(star);
        return false;
    }
    return true;
}

bool omm_sparkly_context_is_state_ok() {
    return sOmmSparklyContext->state == OMM_SPARKLY_CONTEXT_STATE_OK;
}

void omm_sparkly_context_set_state(s32 state, bool sound) {
    if (sound && state != OMM_SPARKLY_CONTEXT_STATE_OK && omm_sparkly_context_is_state_ok()) {
        play_sound(SOUND_MENU_CAMERA_BUZZ | 0xFF00, gGlobalSoundArgs);
    }
    sOmmSparklyContext->state = omm_max_s(sOmmSparklyContext->state, state);
}

void omm_sparkly_context_reset_data() {
    if (sOmmSparklyContext->star) {
        obj_mark_for_deletion(sOmmSparklyContext->star);
    }
    sOmmSparklyContext->data = NULL;
    sOmmSparklyContext->star = NULL;
}

void omm_sparkly_context_restart() {
    gPlayer1Controller->buttonPressed *= (gCurrCourseNum == COURSE_NONE);
    gPlayer1Controller->buttonDown *= (gCurrCourseNum == COURSE_NONE);
    gOmmData->mario->sparkly.mode = gOmmSparklyStarsMode;
    OMM_MEMSET(sOmmSparklyContext, 0, sizeof(sOmmSparklyContext));
}

void omm_sparkly_context_update(s32 flag) {
    if (flag & OMM_SPARKLY_CONTEXT_UPDATE_YELLOW_COIN)  omm_sparkly_context_inc_counter(OMM_SPARKLY_CONTEXT_COUNTER_YELLOW_COINS, 1);
    if (flag & OMM_SPARKLY_CONTEXT_UPDATE_LOST_COIN)    omm_sparkly_context_inc_counter(OMM_SPARKLY_CONTEXT_COUNTER_LOST_COINS, 1);
    if (flag & OMM_SPARKLY_CONTEXT_UPDATE_BLUE_COIN)    omm_sparkly_context_inc_counter(OMM_SPARKLY_CONTEXT_COUNTER_BLUE_COINS, 1);
    if (flag & OMM_SPARKLY_CONTEXT_UPDATE_RED_COIN)     omm_sparkly_context_inc_counter(OMM_SPARKLY_CONTEXT_COUNTER_RED_COINS, 1);
    if (flag & OMM_SPARKLY_CONTEXT_UPDATE_1UP_MUSHROOM) omm_sparkly_context_inc_counter(OMM_SPARKLY_CONTEXT_COUNTER_1UP_MUSHROOMS, 1);
}

OMM_ROUTINE_LEVEL_ENTRY(omm_sparkly_context_init) {
    gOmmData->mario->sparkly.mode = gOmmSparklyStarsMode;
    if (omm_sparkly_is_enabled()) {
        omm_sparkly_context_restart();
        omm_sparkly_turn_off_cheats();
    }
}

//
// State
//

void omm_sparkly_context_update_state(struct MarioState *m) {
    s32 index = omm_sparkly_get_star_index(omm_sparkly_get_current_mode(), gCurrLevelNum, gCurrAreaIndex);
    const OmmSparklyStarsData *data = (index != -1 ? &gOmmSparklyStarsData[omm_sparkly_get_current_mode()][index] : NULL);

    // Area
    // Reset some values when entering a new area, unless the data->area is 0
    // Invalidate the context if a timer was started
    // Restart the entire context inside the Castle
    static s32 sPrevAreaIndex = -1;
    if (sPrevAreaIndex != gCurrAreaIndex) {
        sPrevAreaIndex = gCurrAreaIndex;
        if (!data || data->area != 0) {
            if (omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED)) {
                omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
                gHudDisplay.flags &= ~HUD_DISPLAY_FLAG_TIMER;
            }
            if (gCurrCourseNum == COURSE_NONE) {
                omm_sparkly_context_restart();
            } else {
                omm_sparkly_context_unset_flag(OMM_SPARKLY_CONTEXT_FLAG_INITED, 1);
                omm_sparkly_context_unset_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, 1);
                omm_sparkly_context_unset_flag(OMM_SPARKLY_CONTEXT_FLAG_STAR_SPAWNED, 1);
                omm_sparkly_context_unset_flag(OMM_SPARKLY_CONTEXT_FLAG_OBJECTS_SPAWNED, 1);
                omm_sparkly_context_unset_flag(OMM_SPARKLY_CONTEXT_FLAG_ENTRY_DIALOG, 1);
                omm_sparkly_context_unset_flag(OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED, 1);
                omm_sparkly_context_unset_flag(OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED, 1);
                omm_sparkly_context_set_counter(OMM_SPARKLY_CONTEXT_COUNTER_YELLOW_COINS, 0);
                omm_sparkly_context_set_counter(OMM_SPARKLY_CONTEXT_COUNTER_LOST_COINS, 0);
                omm_sparkly_context_set_counter(OMM_SPARKLY_CONTEXT_COUNTER_BLUE_COINS, 0);
                omm_sparkly_context_set_counter(OMM_SPARKLY_CONTEXT_COUNTER_RED_COINS, 0);
                omm_sparkly_context_set_counter(OMM_SPARKLY_CONTEXT_COUNTER_1UP_MUSHROOMS, 0);
            }
        }
    }

    // Data
    if (sOmmSparklyContext->data != data || !data) {
        sOmmSparklyContext->data = data;
        sOmmSparklyContext->star = NULL;
    }

    // Per level
    u64 dataFlags = 0llu;
    if (data) {
        omm_sparkly_context_set_counter(OMM_SPARKLY_CONTEXT_COUNTER_SECRETS, obj_get_count_with_behavior(bhvHiddenStarTrigger));
        omm_sparkly_level_update(m, data);
        dataFlags = data->flags;

        // Spawn coin arrow
        if (data->spawnCoinArrow && !omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_COIN_ARROW_SPAWNED)) {
            f32 x = data->coinX;
            f32 y = data->coinY;
            f32 z = data->coinZ;
            f32 dx = coss(data->coinA);
            f32 dz = sins(data->coinA);
            spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin, x,            y +  80, z,            0, 0, 0);
            spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin, x,            y + 160, z,            0, 0, 0);
            spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin, x,            y + 240, z,            0, 0, 0);
            spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin, x,            y + 320, z,            0, 0, 0);
            spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin, x,            y + 400, z,            0, 0, 0);
            spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin, x - dx *  50, y + 350, z - dz *  50, 0, 0, 0);
            spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin, x - dx * 100, y + 300, z - dz * 100, 0, 0, 0);
            spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin, x + dx *  50, y + 350, z + dz *  50, 0, 0, 0);
            spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin, x + dx * 100, y + 300, z + dz * 100, 0, 0, 0);
            omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_COIN_ARROW_SPAWNED, 1, 0);
        }

        // Coin requirement
        if (dataFlags & OMM_SPARKLY_DATA_BITS_COINS) {
            s32 requiredCoins = OMM_SPARKLY_DATA_FLAG_GET_COINS(dataFlags);
            s32 collectedCoins = (
                omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_YELLOW_COINS) * 1 +
                omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_BLUE_COINS) * 5 +
                omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_RED_COINS) * 2
            );
            omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, collectedCoins >= requiredCoins, 0);
        }

        // All red coins
        s32 redCoins = omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_RED_COINS);
        if (dataFlags & OMM_SPARKLY_DATA_FLAG_ALL_RED_COINS) {
            omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, redCoins >= 8, 0);
        }
    }

    // Show the Sparkly Star hint on level entry
    static s32 sLastEntryDialogStarIndex = -1;
    if (omm_sparkly_context_is_state_ok() &&                                    // State OK
        !omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_ENTRY_DIALOG) && // Not already triggered
        !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_GAME_PAUSED) &&                 // Not paused
        !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_TRANSITION) && (                // Not a transition
        ((m->action & ACT_GROUP_MASK) != ACT_GROUP_CUTSCENE) ||                 // Not in a cutscene action
        !(m->action & ACT_FLAG_AIR)))                                           // Not airborne
    {
        if (data && data->showOnEntry && omm_get_dialog_entry(NULL, OMM_DIALOG_SPARKLY_STAR + data->index)) {
            if (data->showOnEntry == 2) {
                omm_mario_set_action(m, ACT_READING_AUTOMATIC_DIALOG, OMM_DIALOG_SPARKLY_STAR + data->index, 0xFFFF);
                sLastEntryDialogStarIndex = -1;
            } else if (OMM_EXTRAS_SPARKLY_STARS_HINT) {
                s32 entryDialogStarIndex = data->index + 30 * omm_sparkly_get_current_mode();
                if (entryDialogStarIndex != sLastEntryDialogStarIndex) {
                    level_set_transition(-1, NULL);
                    create_dialog_box(OMM_DIALOG_SPARKLY_STAR + data->index);
                }
                sLastEntryDialogStarIndex = entryDialogStarIndex;
            } else {
                sLastEntryDialogStarIndex = -1;
            }
        } else {
            sLastEntryDialogStarIndex = -1;
        }
        omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_ENTRY_DIALOG, 1, 0);
    }

    // Anti-cheats
    // Kills Mario if a cheat is detected
    // Update only when the game is not paused
    // If detected, doesn't check for cheats until the Bowser dialog is done
    if (!omm_sparkly_flags_get(OMM_SPARKLY_FLAG_GAME_PAUSED) &&
        !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_CHEAT_DETECTED)) {
        omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_CHEAT_DETECTED, omm_sparkly_check_cheats(m), false);
        if (!omm_sparkly_flags_get(OMM_SPARKLY_FLAG_TRANSITION) &&
            omm_sparkly_flags_get(OMM_SPARKLY_FLAG_MARIO_UPDATED) &&
            omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_CHEAT_DETECTED)) {
            omm_sparkly_flags_set(OMM_SPARKLY_FLAG_CHEAT_DETECTED, 1);
            omm_sparkly_context_unset_flag(OMM_SPARKLY_CONTEXT_FLAG_CHEAT_DETECTED, 1);
            omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 0);
            omm_sparkly_turn_off_cheats();

            // Nice effects: Bob-omb explosion, knockback animation that leads to Mario's death,
            // and trigger the classic death sequence to warp Mario back to the Castle
            omm_mario_unpossess_object(m, OMM_MARIO_UNPOSSESS_ACT_NONE, false, 0);
            omm_spawn_problem(m->marioObj);
            if (m->action & ACT_FLAG_AIR) {
                omm_mario_set_action(m, (random_u16() & 1) ? ACT_HARD_FORWARD_AIR_KB : ACT_HARD_BACKWARD_AIR_KB, 1, 0);
            } else if (m->action & (ACT_FLAG_SWIMMING | ACT_FLAG_METAL_WATER)) {
                omm_mario_set_action(m, (random_u16() & 1) ? ACT_FORWARD_WATER_KB : ACT_BACKWARD_WATER_KB, 1, 0);
            } else {
                omm_mario_set_action(m, (random_u16() & 1) ? ACT_HARD_FORWARD_GROUND_KB : ACT_HARD_BACKWARD_GROUND_KB, 1, 0);
            }
        }
    }

    // Rules
    // Collecting all 30 Sparkly Stars of the current difficulty or spawning
    // a star in Normal mode allows the player to bypass the rules
    if (!omm_sparkly_is_mode_completed(omm_sparkly_get_current_mode()) && (
        !omm_sparkly_is_mode_selected(OMM_SPARKLY_MODE_NORMAL) ||
        !omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_STAR_SPAWNED))) {

        // No caps
        if (((m->flags & MARIO_WING_CAP) && !(dataFlags & OMM_SPARKLY_DATA_FLAG_WING_CAP)) ||
            ((m->flags & MARIO_METAL_CAP) && !(dataFlags & OMM_SPARKLY_DATA_FLAG_METAL_CAP)) ||
            ((m->flags & MARIO_VANISH_CAP) && !(dataFlags & OMM_SPARKLY_DATA_FLAG_VANISH_CAP))) {
            omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
        }

        // No cannon
        if (m->action == ACT_IN_CANNON && !omm_sparkly_is_mode_selected(OMM_SPARKLY_MODE_NORMAL)) {
            omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
        }

        // No Vibe outside Extreme mode
        if (!omm_sparkly_is_mode_selected(OMM_SPARKLY_MODE_EXTREME) && omm_peach_vibe_is_active()) {
            omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
        }

        // Peach, Non-Stop mode and 1 HP mode are required in Extreme mode
        if (omm_sparkly_is_mode_selected(OMM_SPARKLY_MODE_EXTREME) && (!OMM_PLAYER_IS_PEACH || !OMM_STARS_NON_STOP || !g1HPMode)) {
            omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
        }
    }

    // Requirements
    if (omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_INITED) && (
        !omm_sparkly_is_mode_selected(OMM_SPARKLY_MODE_NORMAL) ||
        !omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_STAR_SPAWNED))) {

        // No yellow coin
        if ((dataFlags & OMM_SPARKLY_DATA_FLAG_NO_YELLOW_COIN) && (
            omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_YELLOW_COINS) != 0 ||
            omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_LOST_COINS) != 0)) {
            omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
        }

        // No blue coin
        if ((dataFlags & OMM_SPARKLY_DATA_FLAG_NO_BLUE_COIN) &&
            omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_BLUE_COINS) != 0) {
            omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
        }

        // No red coin
        if ((dataFlags & OMM_SPARKLY_DATA_FLAG_NO_RED_COIN) &&
            omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_RED_COINS) != 0) {
            omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
        }

        // No mushroom 1-up
        if ((dataFlags & OMM_SPARKLY_DATA_FLAG_NO_1UP_MUSHROOM) &&
            omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_1UP_MUSHROOMS) != 0) {
            omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
        }

        // No capture
        if ((dataFlags & OMM_SPARKLY_DATA_FLAG_NO_CAPTURE) &&
            (m->action == ACT_OMM_POSSESSION || gOmmData->mario->capture.obj != NULL)) {
            omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
        }

        // No button press
        if (omm_sparkly_flags_get(OMM_SPARKLY_FLAG_MARIO_UPDATED)) {
            u16 buttons = (m->controller->buttonPressed | m->controller->buttonDown);
            u16 noButtons = OMM_SPARKLY_DATA_FLAG_GET_BUTTONS(dataFlags);
            if (!omm_mario_is_reading(m) && (buttons & noButtons) != 0) {
                omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
            }
        }
    }

    // Star
    // Unload if data is NULL or state == 2
    // Make it intangible if already spawned and state == 1
    if (!data || sOmmSparklyContext->state == OMM_SPARKLY_CONTEXT_STATE_INVALID) {
        sOmmSparklyContext->star = NULL;
    } else if (sOmmSparklyContext->star && sOmmSparklyContext->state == OMM_SPARKLY_CONTEXT_STATE_FAIL) {
        sOmmSparklyContext->star->oAction = 3;
    }

    // Counters
    sOmmSparklyContext->counters[1] = sOmmSparklyContext->counters[0];
}

//
// Star
//

void omm_sparkly_context_spawn_star(struct MarioState *m) {
    if (omm_sparkly_context_is_state_ok() &&
        sOmmSparklyContext->data != NULL &&
        sOmmSparklyContext->star == NULL) {

        // Spawn the star only if not condition star or condition is fulfilled
        bool isConditionStar = (sOmmSparklyContext->data->flags & OMM_SPARKLY_DATA_FLAG_CONDITION) != 0;
        if (!isConditionStar || omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL)) {
            sOmmSparklyContext->star = omm_spawn_sparkly_star(m->marioObj,
                omm_sparkly_get_current_mode(),
                sOmmSparklyContext->data->starX,
                sOmmSparklyContext->data->starY,
                sOmmSparklyContext->data->starZ,
                isConditionStar
            );
            omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_STAR_SPAWNED, isConditionStar, 0);
        }
    }
}

//
// Sign
//

void omm_sparkly_context_spawn_sign(struct MarioState *m) {
    if (omm_sparkly_context_is_state_ok() && sOmmSparklyContext->data != NULL) {

        // Spawn sign
        if (!obj_get_first_with_behavior(omm_bhv_sparkly_star_hint) && omm_get_dialog_entry(NULL, OMM_DIALOG_SPARKLY_STAR + sOmmSparklyContext->data->index)) {
            omm_spawn_sparkly_star_hint(m->marioObj,
                omm_sparkly_get_current_mode(),
                sOmmSparklyContext->data->signX + 10,
                sOmmSparklyContext->data->signY,
                sOmmSparklyContext->data->signZ,
                sOmmSparklyContext->data->signA,
                OMM_DIALOG_SPARKLY_STAR + sOmmSparklyContext->data->index
            );
        }

        // Make objects awake after Mario finishes reading the hint sign
        if (m->prevAction == ACT_READING_SIGN && m->interactObj && m->interactObj->behavior == omm_bhv_sparkly_star_hint) {
            for_each_object_with_behavior(obj, omm_bhv_rising_lava) obj_set_dormant(obj, false);
            for_each_object_with_behavior(obj, bhvRedCoin) obj_set_dormant(obj, false);
            for_each_object_with_behavior(obj, bhvGoomba) obj_set_dormant(obj, false);
        }
    }
}

//
// Utils
//

s32 omm_sparkly_context_get_data_flags(u64 flags) {
    if (sOmmSparklyContext->data) {
        switch (flags) {
            case OMM_SPARKLY_DATA_BITS_COINS: return OMM_SPARKLY_DATA_FLAG_GET_COINS(sOmmSparklyContext->data->flags);
            case OMM_SPARKLY_DATA_BITS_BUTTONS: return OMM_SPARKLY_DATA_FLAG_GET_BUTTONS(sOmmSparklyContext->data->flags);
            case OMM_SPARKLY_DATA_BITS_MUSHROOMS: return OMM_SPARKLY_DATA_FLAG_GET_MUSHROOMS(sOmmSparklyContext->data->flags);
            default: return OMM_GET_FLAG(sOmmSparklyContext->data->flags, flags);
        }
    }
    return 0;
}

s32 omm_sparkly_context_get_remaining_flames() {
    return obj_get_count_with_behavior(bhvFlame);
}

s32 omm_sparkly_context_get_remaining_boxes() {
    s32 remainingBoxes = obj_get_count_with_behavior(bhvExclamationBox);
    for_each_object_with_behavior(obj, bhvExclamationBox) {
        remainingBoxes -= (!obj_is_dormant(obj) && obj->oBehParams2ndByte <= 2);
    }
    return remainingBoxes;
}
