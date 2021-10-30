#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// Actions
//

static s32 omm_act_punching(struct MarioState *m) {
    action_condition(OMM_MOVESET_ODYSSEY && OMM_PLAYER_IS_PEACH, ACT_OMM_PEACH_ATTACK_GROUND, ((m->prevAction == ACT_OMM_SPIN_GROUND) ? 4 : 0), RETURN_CANCEL);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

//
// Object
//

static bool omm_check_common_object_cancels(struct MarioState *m) {

    // Plunge
    if (m->pos[1] < m->waterLevel - 100) {
        set_water_plunge_action(m);
        return true;
    }

    // Squished
    if (m->input & INPUT_SQUISHED) {
        drop_and_set_mario_action(m, ACT_SQUISHED, 0);
        return true;
    }

    // Dead
    if (m->health <= OMM_HEALTH_DEAD) {
        drop_and_set_mario_action(m, ACT_STANDING_DEATH, 0);
        return true;
    }

    return false;
}

s32 omm_mario_execute_object_action(struct MarioState *m) {
    gOmmData->mario->wallSlide.jumped = false;
    gOmmData->mario->peach.floated = false;
    gOmmData->mario->cappy.bounced = false;
    gOmmData->mario->state.airCombo = 0;
    gOmmData->mario->midairSpin.counter = 0;

    // Cancels
    if (omm_check_common_object_cancels(m)) {
        return OMM_MARIO_ACTION_RESULT_CANCEL;
    }

    // Quicksand
    // Needs to be 0 to not apply the sinking twice per frame
    if (mario_update_quicksand(m, 0)) {
        return OMM_MARIO_ACTION_RESULT_CANCEL;
    }

    // Actions
    switch (m->action) {
        case ACT_PUNCHING: return omm_act_punching(m);
    }

    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}
