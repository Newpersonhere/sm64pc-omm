#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// Common actions
//

s32 omm_act_peach_float(struct MarioState *m) {
    if (m->actionState == 0) {
        action_condition(gOmmData->mario->peach.floated, ACT_OMM_PEACH_GLIDE, 0, RETURN_CANCEL);
        gOmmData->mario->peach.floatTimer = 0;
        gOmmData->mario->peach.floated = true;
        m->actionState = 1;
    }

    // Cancels
    action_condition(!OMM_PLAYER_IS_PEACH, ACT_FREEFALL, 0, RETURN_CANCEL);
    action_cappy(1, ACT_OMM_CAPPY_THROW_AIRBORNE, 0, RETURN_CANCEL);
    action_zb_pressed(OMM_MOVESET_ODYSSEY, ACT_DIVE, 0, RETURN_CANCEL);
    action_b_pressed(OMM_MOVESET_ODYSSEY, ACT_JUMP_KICK, 0, RETURN_CANCEL);
    action_z_pressed(OMM_MOVESET_ODYSSEY, ACT_GROUND_POUND, 0, RETURN_CANCEL);
    if (!(m->controller->buttonDown & A_BUTTON) || (gOmmData->mario->peach.floatTimer >= OMM_PEACH_FLOAT_DURATION)) {
        if (m->prevAction == ACT_DOUBLE_JUMP) {
            set_mario_animation(m, MARIO_ANIM_DOUBLE_JUMP_FALL);
            m->prevAction = m->action;
            m->action = ACT_DOUBLE_JUMP;
            m->actionArg = 0;
            m->actionState = 1;
            m->actionTimer = 1;
            m->flags |= (MARIO_ACTION_SOUND_PLAYED | MARIO_MARIO_SOUND_PLAYED);
        } else {
            set_mario_animation(m, MARIO_ANIM_GENERAL_FALL);
            omm_mario_set_action(m, ACT_FREEFALL, 0, 0);
        }
        return OMM_MARIO_ACTION_RESULT_CANCEL;
    }

    // Step
    m->vel[1] = 0.f;
    omm_mario_update_air_with_turn(m);
    s32 step = perform_air_step(m, OMM_MOVESET_ODYSSEY * AIR_STEP_CHECK_LEDGE_GRAB);
    action_condition(step == AIR_STEP_LANDED, ACT_FREEFALL_LAND, 0, RETURN_BREAK);
    action_condition(step == AIR_STEP_GRABBED_LEDGE, ACT_LEDGE_GRAB, 0, RETURN_BREAK, set_mario_animation(m, MARIO_ANIM_IDLE_ON_LEDGE););
    action_condition(step == AIR_STEP_HIT_WALL && omm_mario_check_and_perform_wall_slide(m), ACT_OMM_WALL_SLIDE, 0, RETURN_BREAK);
    action_condition(step == AIR_STEP_HIT_LAVA_WALL && lava_boost_on_wall(m), ACT_LAVA_BOOST, 1, RETURN_BREAK);

    // Gfx
    if (omm_mario_is_anim_at_end(m)) m->marioObj->header.gfx.mAnimInfo.animID = -1;
    set_mario_animation(m, MARIO_ANIM_BEND_KNESS_RIDING_SHELL);
    m->particleFlags |= PARTICLE_SPARKLES;
    gOmmData->mario->peach.floatTimer++;
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

s32 omm_act_peach_glide(struct MarioState *m) {
    if (m->actionState == 0) {
        omm_sound_play(OMM_SOUND_EFFECT_PEACH_GLIDE, m->marioObj->oCameraToObject);
        set_mario_anim_with_accel(m, MARIO_ANIM_GRAB_POLE_SHORT, 0x20000);
        m->actionState = 1;
    }

    // Cancels
    action_condition(!OMM_PLAYER_IS_PEACH, ACT_FREEFALL, 0, RETURN_CANCEL);
    action_cappy(1, ACT_OMM_CAPPY_THROW_AIRBORNE, 0, RETURN_CANCEL);
    action_zb_pressed(OMM_MOVESET_ODYSSEY, ACT_DIVE, 0, RETURN_CANCEL);
    action_b_pressed(OMM_MOVESET_ODYSSEY, ACT_JUMP_KICK, 0, RETURN_CANCEL);
    action_z_pressed(OMM_MOVESET_ODYSSEY, ACT_GROUND_POUND, 0, RETURN_CANCEL);
    action_condition(!(m->controller->buttonDown & A_BUTTON), ACT_FREEFALL, 0, RETURN_CANCEL);

    // Step
    omm_mario_update_air_with_turn(m);
    s32 step = perform_air_step(m, OMM_MOVESET_ODYSSEY * AIR_STEP_CHECK_LEDGE_GRAB);
    action_condition(step == AIR_STEP_LANDED, ACT_FREEFALL_LAND, 0, RETURN_BREAK);
    action_condition(step == AIR_STEP_GRABBED_LEDGE, ACT_LEDGE_GRAB, 0, RETURN_BREAK, set_mario_animation(m, MARIO_ANIM_IDLE_ON_LEDGE););
    action_condition(step == AIR_STEP_HIT_LAVA_WALL && lava_boost_on_wall(m), ACT_LAVA_BOOST, 1, RETURN_BREAK);

    // Gfx
    if (++m->actionTimer > 6) set_mario_animation(m, MARIO_ANIM_IDLE_ON_POLE);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static void omm_peach_set_animation_and_play_sound(struct MarioState *m, s32 animId, s32 animAccel, s32 sound) {
    if (m->marioObj->header.gfx.mAnimInfo.animID != animId) {
        obj_play_sound(m->marioObj, sound);
    }
    set_mario_anim_with_accel(m, animId, animAccel);
}

s32 omm_act_peach_attack_ground(struct MarioState *m) {
    action_condition(!OMM_PLAYER_IS_PEACH, ACT_FREEFALL, 0, RETURN_CANCEL);
    action_condition((m->actionState == 0) && (m->controller->buttonDown & A_BUTTON), ACT_JUMP_KICK, 0, RETURN_CANCEL);
    action_init(((omm_abs_f(m->forwardVel) < 12.f) ? 12.f : m->forwardVel), 0.f, 0, 0);
    action_cappy(1, ACT_OMM_CAPPY_THROW_GROUND, 0, RETURN_CANCEL);
    action_a_pressed(1, ACT_JUMP, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_FREEFALL, 0, RETURN_CANCEL);
    action_condition((m->forwardVel > 28.f) && (m->controller->stickMag > 56.f), ACT_OMM_PEACH_ATTACK_FAST, 0, RETURN_CANCEL);

    // Spawn shockwave
    if (m->actionTimer == 0 && (omm_health_is_at_max(m) || omm_peach_vibe_is_active())) {
        omm_spawn_perry_shockwave(m->marioObj, 6 + 2 * (m->actionArg == 4), gOmmData->mario->peach.vibeType, m->actionArg == 4);
    }

    // Update vel
    if (m->forwardVel > 0.f) {
        apply_slope_decel(m, 0.5f);
    } else {
        m->forwardVel = omm_min_f(m->forwardVel + 8.f, 0.f);
        apply_slope_accel(m);
    }
    mario_set_forward_vel(m, m->forwardVel);

    // Step
    s32 step = perform_ground_step(m);
    action_condition(step == GROUND_STEP_LEFT_GROUND, ACT_FREEFALL, 0, RETURN_BREAK);

    // Update attack sequence
    switch (m->actionArg) {

        // First attack (part 1)
        case 0: {
            omm_peach_set_animation_and_play_sound(m, MARIO_ANIM_FIRST_PUNCH, 0x10000, SOUND_MARIO_PUNCH_WAH);
            action_condition(is_anim_past_end(m), 0, 0, NO_RETURN, m->actionArg = 1;);
            action_condition(mario_check_object_grab(m), 0, 0, RETURN_BREAK);
            m->actionState = 2;
        } break;

        // First attack (part 2)
        case 1: {
            omm_peach_set_animation_and_play_sound(m, MARIO_ANIM_FIRST_PUNCH_FAST, 0x10000, 0);
            action_b_pressed(1, 0, 0, NO_RETURN, m->actionArg = 2;);
            action_condition(is_anim_at_end(m), ACT_IDLE, 0, RETURN_BREAK);
            m->actionState = 1 + ((m->marioObj->header.gfx.mAnimInfo.animFrameAccelAssist >> 0x10) < 2);
        } break;

        // Second attack (part 1)
        case 2: {
            omm_peach_set_animation_and_play_sound(m, MARIO_ANIM_SECOND_PUNCH, 0x10000, SOUND_MARIO_PUNCH_YAH);
            action_condition(is_anim_past_end(m), 0, 0, NO_RETURN, m->actionArg = 3;);
            m->actionState = 2;
        } break;

        // Second attack (part 2)
        case 3: {
            omm_peach_set_animation_and_play_sound(m, MARIO_ANIM_SECOND_PUNCH_FAST, 0x10000, 0);
            action_b_pressed(1, 0, 0, NO_RETURN, m->actionArg = 4;);
            action_condition(is_anim_at_end(m), ACT_IDLE, 0, RETURN_BREAK);
            m->actionState = 1 + ((m->marioObj->header.gfx.mAnimInfo.animFrameAccelAssist >> 0x10) < 2);
        } break;

        // Spin attack
        case 4: {
            omm_peach_set_animation_and_play_sound(m, MARIO_ANIM_FINAL_BOWSER_RAISE_HAND_SPIN, 0x20000, SOUND_MARIO_PUNCH_HOO);
            m->marioObj->header.gfx.mAnimInfo.animFrameAccelAssist = omm_clamp_s(m->marioObj->header.gfx.mAnimInfo.animFrameAccelAssist, (70 << 16), (94 << 16));
            action_condition(m->marioObj->header.gfx.mAnimInfo.animFrameAccelAssist == (94 << 16), ACT_IDLE, 0, RETURN_BREAK);
            m->actionState = 2;
        } break;
    }

    // Gfx
    m->actionTimer++;
    m->flags |= MARIO_PUNCHING;
    m->particleFlags |= PARTICLE_DUST * (omm_abs_f(m->forwardVel) > 16.f);
    m->marioBodyState->punchState = 0;
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static void omm_peach_play_random_attack_sound(struct MarioState *m) {
    static s32 prevSound = 0;
    s32 nextSound;
    do {
        switch (random_u16() % 3) {
            case 0: nextSound = SOUND_MARIO_PUNCH_WAH; break;
            case 1: nextSound = SOUND_MARIO_PUNCH_YAH; break;
            case 2: nextSound = SOUND_MARIO_PUNCH_HOO; break;
        }
    } while (nextSound == prevSound);
    play_sound(nextSound, m->marioObj->oCameraToObject); 
    prevSound = nextSound;
}

s32 omm_act_peach_attack_fast(struct MarioState *m) {
    static s32 sAttackTimer = 0;
    action_condition(!OMM_PLAYER_IS_PEACH, ACT_FREEFALL, 0, RETURN_CANCEL);
    action_init(m->forwardVel * 1.2f, 0.f, PARTICLE_HORIZONTAL_STAR, 0, sAttackTimer = 0;);
    action_cappy(1, ACT_OMM_CAPPY_THROW_GROUND, 0, RETURN_CANCEL);
    action_a_pressed(1, ACT_JUMP, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_FREEFALL, 0, RETURN_CANCEL);

    // Spawn shockwave
    if (m->actionTimer == 0 && (omm_health_is_at_max(m) || omm_peach_vibe_is_active())) {
        omm_spawn_perry_shockwave(m->marioObj, 4, gOmmData->mario->peach.vibeType, false);
    }

    // Step
    omm_mario_update_walking_speed(m);
    s32 step = perform_ground_step(m);
    action_condition(step == GROUND_STEP_LEFT_GROUND, ACT_FREEFALL, 0, RETURN_BREAK);

    // Update attack sequence
    switch (m->actionArg) {

        // First attack (part 1)
        case 0: {
            if (sAttackTimer++ == 0) omm_peach_play_random_attack_sound(m);
            set_mario_anim_with_accel(m, MARIO_ANIM_FIRST_PUNCH, 0x10000);
            action_condition(is_anim_past_end(m), 0, 0, NO_RETURN, m->actionArg = 1; sAttackTimer = 0;);
        } break;

        // First attack (part 2)
        case 1: {
            set_mario_anim_with_accel(m, MARIO_ANIM_FIRST_PUNCH_FAST, 0x10000);
            action_condition(++sAttackTimer > 3, 0, 0, NO_RETURN, m->actionArg = 2; sAttackTimer = 0;);
        } break;

        // Second attack (part 1)
        case 2: {
            if (sAttackTimer++ == 0) omm_peach_play_random_attack_sound(m);
            set_mario_anim_with_accel(m, MARIO_ANIM_SECOND_PUNCH, 0x10000);
            action_condition(is_anim_past_end(m), 0, 0, NO_RETURN, m->actionArg = 3; sAttackTimer = 0;);
        } break;

        // Second attack (part 2)
        case 3: {
            set_mario_anim_with_accel(m, MARIO_ANIM_SECOND_PUNCH_FAST, 0x10000);
            action_condition(++sAttackTimer > 3, ACT_WALKING, 0, RETURN_BREAK);
        } break;
    }

    // Gfx
    m->actionTimer++;
    m->particleFlags |= PARTICLE_DUST;
    m->marioBodyState->punchState = 0;
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

s32 omm_act_peach_attack_air(struct MarioState *m) {
    action_condition(!OMM_PLAYER_IS_PEACH, ACT_FREEFALL, 0, RETURN_CANCEL);
    action_cappy(1, ACT_OMM_CAPPY_THROW_AIRBORNE, 0, RETURN_CANCEL);
    action_zb_pressed(OMM_MOVESET_ODYSSEY, ACT_DIVE, 0, RETURN_CANCEL);
    action_z_pressed(OMM_MOVESET_ODYSSEY, ACT_GROUND_POUND, 0, RETURN_CANCEL);
    action_b_pressed(omm_mario_has_wing_cap(m), ACT_FLYING, 0, RETURN_CANCEL);
    action_midair_spin(OMM_MOVESET_ODYSSEY && (m->actionTimer >= 16), ACT_OMM_MIDAIR_SPIN, 0, RETURN_CANCEL);
    m->controller->buttonPressed &= ~B_BUTTON;

    // Spawn shockwave
    if (m->actionTimer == 0 && (omm_health_is_at_max(m) || omm_peach_vibe_is_active())) {
        omm_spawn_perry_shockwave(m->marioObj, 8, gOmmData->mario->peach.vibeType, true);
    }

    // Step
    omm_mario_update_air_without_turn(m);
    s32 step = perform_air_step(m, OMM_MOVESET_ODYSSEY * AIR_STEP_CHECK_LEDGE_GRAB);
    action_condition(step == AIR_STEP_LANDED, ACT_FREEFALL_LAND, 0, RETURN_BREAK);
    action_condition(step == AIR_STEP_GRABBED_LEDGE, ACT_LEDGE_GRAB, 0, RETURN_BREAK, set_mario_animation(m, MARIO_ANIM_IDLE_ON_LEDGE););
    action_condition(step == AIR_STEP_HIT_WALL && omm_mario_check_and_perform_wall_slide(m), ACT_OMM_WALL_SLIDE, 0, RETURN_BREAK);
    action_condition(step == AIR_STEP_HIT_LAVA_WALL && lava_boost_on_wall(m), ACT_LAVA_BOOST, 1, RETURN_BREAK);

    // Gfx
    m->actionTimer++;
    set_mario_anim_with_accel(m, MARIO_ANIM_FINAL_BOWSER_RAISE_HAND_SPIN, 0x20000);
    m->marioObj->header.gfx.mAnimInfo.animFrameAccelAssist = omm_clamp_s(m->marioObj->header.gfx.mAnimInfo.animFrameAccelAssist, (70 << 16), (94 << 16));
    m->flags |= MARIO_KICKING * (m->marioObj->header.gfx.mAnimInfo.animFrameAccelAssist < (94 << 16));
    m->marioBodyState->punchState = 0;
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

//
// Joy actions
//

static s32 omm_peach_vibe_joy_update(struct MarioState *m, s32 yawVel, bool fly) {

    // Update forward vel
    f32 fVelTarget = omm_relerp_0_1_f(m->intendedMag, 0.f, 32.f, 0.f, 30.f);
    mario_set_forward_vel(m, approach_f32(m->forwardVel, fVelTarget, 1.5f, 3.f));

    // Update vertical vel
    f32 yVelInc = fly * omm_clamp_f((31.f - m->vel[1]) / 2.f, 1.f, 4.f);
    m->vel[1] = approach_f32(m->vel[1], 30.f, yVelInc, omm_sign_0_f(yVelInc));

    // Update angle
    s32 handling = omm_relerp_0_1_f(m->forwardVel, 8.f, 30.f, 0x2000, 0x800);
    m->faceAngle[1] = m->intendedYaw - approach_s32((s16) (m->intendedYaw - m->faceAngle[1]), 0, handling, handling);

    // Perform step
    s32 step = (!fly && m->pos[1] < find_floor_height(m->pos[0], m->pos[1], m->pos[2]) + 4.f) ? perform_ground_step(m) : perform_air_step(m, 0);
    action_condition(step == AIR_STEP_LANDED && fly, ACT_OMM_PEACH_VIBE_JOY_MOVE, 0, RETURN_BREAK);
    action_condition(step == AIR_STEP_HIT_LAVA_WALL && lava_boost_on_wall(m), ACT_LAVA_BOOST, 1, RETURN_BREAK);

    // Update Gfx
    set_mario_animation(m, MARIO_ANIM_TWIRL);
    gOmmData->mario->peach.joySpinYaw += yawVel;
    gOmmData->mario->state.peakHeight = m->pos[1];
    m->peakHeight = m->pos[1];
    m->quicksandDepth = 0.f;
    m->particleFlags |= fly * PARTICLE_DUST;
    m->marioObj->header.gfx.angle[1] = gOmmData->mario->peach.joySpinYaw;
    m->marioBodyState->handState = MARIO_HAND_OPEN;
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

s32 omm_act_peach_vibe_joy_move(struct MarioState *m) {
    action_condition(!omm_peach_vibe_is_joy(), ACT_FREEFALL, 0, RETURN_CANCEL);
    action_a_pressed(1, ACT_OMM_PEACH_VIBE_JOY_FLY, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_PEACH_VIBE_JOY_ATTACK, 0, RETURN_CANCEL);
    return omm_peach_vibe_joy_update(m, 0x1000, false);
}

s32 omm_act_peach_vibe_joy_fly(struct MarioState *m) {
    action_init(m->forwardVel, m->vel[1], 0, SOUND_ACTION_INTRO_UNK45E);
    action_condition(!omm_peach_vibe_is_joy(), ACT_FREEFALL, 0, RETURN_CANCEL);
    action_condition(!(m->controller->buttonDown & A_BUTTON), ACT_OMM_PEACH_VIBE_JOY_MOVE, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_PEACH_VIBE_JOY_ATTACK, 0, RETURN_CANCEL);
    return omm_peach_vibe_joy_update(m, 0x2000, true);
}

s32 omm_act_peach_vibe_joy_attack(struct MarioState *m) {
    action_init(m->forwardVel, omm_max_f(m->vel[1], 10.f), 0, SOUND_ACTION_FLYING_FAST, omm_spawn_peach_vibe_joy_gust(m->marioObj, 60.f, m->faceAngle[1]););
    action_condition(!omm_peach_vibe_is_joy(), ACT_FREEFALL, 0, RETURN_CANCEL);
    action_condition(m->actionTimer++ >= 12, ACT_OMM_PEACH_VIBE_JOY_MOVE, 0, RETURN_CANCEL);
    return omm_peach_vibe_joy_update(m, omm_relerp_0_1_f(m->actionTimer, 8, 12, 0x3000, 0x1000), false);
}

