#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// Actions
//

static s32 omm_act_in_cannon(struct MarioState *m) {
    if (OMM_MOVESET_ODYSSEY && m->actionState == 2 && (m->controller->buttonPressed & B_BUTTON)) {
        m->input |= INPUT_A_PRESSED;
    }
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_holding_pole(struct MarioState *m) {
    if (OMM_MOVESET_ODYSSEY) {
        if (m->controller->buttonPressed & Z_TRIG) {
            mario_set_forward_vel(m, -10.f);
            omm_mario_set_action(m, ACT_FREEFALL, 0, Z_TRIG);
            gOmmData->mario->state.poleTimer = 30;
            return OMM_MARIO_ACTION_RESULT_CANCEL;
        }
    }
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_climbing_pole(struct MarioState *m) {
    if (OMM_MOVESET_ODYSSEY) {
        if (m->controller->buttonDown & B_BUTTON) {
            m->controller->stickY *= 2.f;
        }
    }
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

//
// OMM Bowser
//

static bool sAboutToReleaseBowser = false;
static s16 sTargetMineYawAngle = 0;
static s16 sTargetMineYawVel = 0;
static struct Object *sTargetMine = NULL;

static s32 omm_act_grab_bowser(struct MarioState *m) {
    action_init(0, 0, 0, SOUND_MARIO_HRMM, omm_mario_set_animation(m, MARIO_ANIM_GRAB_BOWSER, 1.f, 0); sAboutToReleaseBowser = false;);
    if (omm_mario_is_anim_at_end(m)) {
        omm_mario_set_action(m, ACT_OMM_HOLD_BOWSER, 0, 0);
        m->angleVel[1] = 0;
    }
    stationary_ground_step(m);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_hold_bowser(struct MarioState *m) {
    omm_mario_set_animation(m, m->angleVel[1] == 0 ? MARIO_ANIM_HOLDING_BOWSER : MARIO_ANIM_SWINGING_BOWSER, 1.f, -1);

    // Prepare release
    if (!sAboutToReleaseBowser && (m->controller->buttonPressed & B_BUTTON) && (omm_abs_s(m->angleVel[1]) >= 0x0E00)) {
        
        // Target the nearest mine to throw Bowser into
        // Takes the current rotation into account
        struct Object *targetMine = NULL;
        s16 targetAngle = 0;
        u16 minAngle = 0xFFFF;
        for_each_object_with_behavior(mine, omm_bhv_bowser_mine) {
            s16 mineAngle = atan2s(mine->oPosZ, mine->oPosX);
            for (s32 deltaAngle = 0; omm_abs_s(deltaAngle) < 0x10000; deltaAngle += m->angleVel[1]) {
                s16 nextAngle = m->faceAngle[1] + (s16) deltaAngle;
                u16 angleDiff = (u16) omm_abs_s(nextAngle - mineAngle);
                if (angleDiff < (u16) omm_abs_s(m->angleVel[1])) {
                    if ((u16) omm_abs_s(deltaAngle) < minAngle) {
                        minAngle = (u16) omm_abs_s(deltaAngle);
                        targetAngle = mineAngle;
                        targetMine = mine;
                    }
                    break;
                }
            }
        }

        sTargetMine = targetMine;
        sTargetMineYawAngle = targetAngle;
        sTargetMineYawVel = m->angleVel[1];
        sAboutToReleaseBowser = true;
    }

    // About to release
    if (sAboutToReleaseBowser) {
        u16 angleDiff = (u16) omm_abs_s(m->faceAngle[1] - sTargetMineYawAngle);
        if (angleDiff < (u16) omm_abs_s((s16) sTargetMineYawVel)) {
        
            // Set Bowser's starting point and direction
            m->faceAngle[1] = sTargetMineYawAngle;
            struct Object *bowser = obj_get_first_with_behavior(omm_bhv_bowser);
            f32 distBowserFromCenter = sqrtf(omm_sqr_f(bowser->oPosX) + omm_sqr_f(bowser->oPosZ));
            bowser->oPosX = distBowserFromCenter * sins(m->faceAngle[1]);
            bowser->oPosZ = distBowserFromCenter * coss(m->faceAngle[1]);
            bowser->oFaceAngleYaw = m->faceAngle[1];
            bowser->oBowserMine = sTargetMine;

            // "So long-a Bowser"
            omm_mario_set_animation(m, MARIO_ANIM_RELEASE_BOWSER, 1.f, 0);
            play_sound(SOUND_MARIO_SO_LONGA_BOWSER | 0xFF00, m->marioObj->oCameraToObject);
            m->actionState = 1;

            // Release
            sAboutToReleaseBowser = false;
            omm_mario_set_action(m, ACT_OMM_RELEASE_BOWSER, 0, B_BUTTON);
            return OMM_MARIO_ACTION_RESULT_CANCEL;
        }
        m->angleVel[1] = sTargetMineYawVel;
    
    } else {
    
        // Spin accel
        if (m->intendedMag > 20.0f) {
            if (m->actionArg == 0) {
                m->actionArg = 1;
                m->twirlYaw = m->intendedYaw;
            } else {
                s16 angleAccel = omm_clamp_s((m->intendedYaw - m->twirlYaw) / 0x80, -0x80, +0x80);
                m->twirlYaw = m->intendedYaw;
                m->angleVel[1] = omm_clamp_s(m->angleVel[1] + angleAccel, -0x1000, +0x1000);
            }
        } else {
            m->actionArg = 0;
            m->angleVel[1] = approach_s32(m->angleVel[1], 0, 0x40, 0x40);
        }
    }

    // Update face angle
    s16 prevAngle = m->faceAngle[1];
    m->faceAngle[1] += m->angleVel[1];
    if ((m->angleVel[1] < 0 && prevAngle < m->faceAngle[1]) ||
        (m->angleVel[1] > 0 && prevAngle > m->faceAngle[1])) {
        play_sound(SOUND_OBJ_BOWSER_SPINNING, m->marioObj->oCameraToObject);
    }
    stationary_ground_step(m);

    // Update Bowser's pos and angle
    struct Object *bowser = obj_get_first_with_behavior(omm_bhv_bowser);
    f32 distBowserFromCenter = sqrtf(omm_sqr_f(bowser->oPosX) + omm_sqr_f(bowser->oPosZ));
    bowser->oPosX = distBowserFromCenter * sins(m->faceAngle[1]);
    bowser->oPosZ = distBowserFromCenter * coss(m->faceAngle[1]);
    bowser->oFaceAngleYaw = m->faceAngle[1];
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_release_bowser(struct MarioState *m) {
    if (omm_mario_is_anim_at_end(m)) {
        omm_mario_set_action(m, ACT_IDLE, 0, 0);
    }
    stationary_ground_step(m);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

//
// Automatic
//

static bool omm_check_common_automatic_cancels(struct MarioState *m) {

    // Plunge
    if (m->pos[1] < m->waterLevel - 100) {
        set_water_plunge_action(m);
        return true;
    }

    return false;
}

s32 omm_mario_execute_automatic_action(struct MarioState *m) {
    gOmmData->mario->wallSlide.jumped = false;
    gOmmData->mario->peach.floated = false;
    gOmmData->mario->cappy.bounced = false;
    gOmmData->mario->state.airCombo = 0;
    gOmmData->mario->midairSpin.counter = 0;
    m->quicksandDepth = 0.0f;

    // Cancels
    if (omm_check_common_automatic_cancels(m)) {
        return OMM_MARIO_ACTION_RESULT_CANCEL;
    }

    // Actions
    switch (m->action) {
        case ACT_IN_CANNON:             return omm_act_in_cannon(m);
        case ACT_HOLDING_POLE:          return omm_act_holding_pole(m);
        case ACT_CLIMBING_POLE:         return omm_act_climbing_pole(m);

        case ACT_OMM_GRAB_BOWSER:       return omm_act_grab_bowser(m);
        case ACT_OMM_HOLD_BOWSER:       return omm_act_hold_bowser(m);
        case ACT_OMM_RELEASE_BOWSER:    return omm_act_release_bowser(m);
    }

    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}
