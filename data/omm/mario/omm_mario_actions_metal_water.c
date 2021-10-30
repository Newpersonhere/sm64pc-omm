#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

static void omm_metal_water_update_walking_speed(struct MarioState *m) {
    mario_set_forward_vel(m, approach_f32(m->forwardVel, OMM_MARIO_METAL_WATER_MAX_WALKING_SPEED * m->intendedMag / 32.f, 1.f, 3.f));
    if (m->forwardVel < 0.f && !(m->input & INPUT_NONZERO_ANALOG)) {
        mario_set_forward_vel(m, 0.f);
    }
    if (OMM_MOVESET_ODYSSEY || (gOmmData->mario->metalWater.jumpNext != ACT_OMM_METAL_WATER_LONG_JUMP)) {
        m->faceAngle[1] = m->intendedYaw - approach_s32((s16) (m->intendedYaw - m->faceAngle[1]), 0, 0x800, 0x800);
    }
}

static void omm_metal_water_play_step_sound(struct MarioState *m, s16 frame1, s16 frame2) {
    if (is_anim_past_frame(m, frame1) || is_anim_past_frame(m, frame2)) {
        play_sound(SOUND_ACTION_METAL_STEP_WATER, m->marioObj->oCameraToObject);
        m->particleFlags |= PARTICLE_DUST;
    }
}

static void omm_metal_water_set_next_jump(u32 jumpAction) {
    gOmmData->mario->metalWater.jumpNext = jumpAction;
    gOmmData->mario->metalWater.jumpTimer = 8;
}

static bool omm_metal_water_check_water_jump(struct MarioState *m) {
    if (m->vel[1] > 0.0f && m->pos[1] > m->waterLevel - 100) {
        play_sound(SOUND_ACTION_UNKNOWN430, m->marioObj->oCameraToObject);
        m->particleFlags |= PARTICLE_WATER_SPLASH;
        return true;
    }
    return false;
}

static bool omm_metal_water_common_stationary_step(struct MarioState* m, s32 animation) {
    set_mario_animation(m, animation);
    stationary_ground_step(m);
    return is_anim_at_end(m);
}

static bool omm_metal_water_common_air_action_step(struct MarioState *m, u32 landAction, s32 animation) {
    omm_mario_update_air_without_turn(m);

    if (m->action != ACT_OMM_METAL_WATER_LONG_JUMP) {
        mario_set_forward_vel(m, omm_min_f(m->forwardVel, 16.f));
    }

    switch (perform_air_step(m, 0)) {
        case AIR_STEP_NONE:
            set_mario_animation(m, animation);
            break;

        case AIR_STEP_LANDED:
            omm_mario_set_action(m, landAction, 0, 0);
            break;

        case AIR_STEP_HIT_WALL:
            set_mario_animation(m, animation);
            if (omm_mario_check_wall_slide(m)) {
                if (omm_mario_can_perform_wall_slide(m)) {
                    omm_mario_set_action(m, ACT_OMM_METAL_WATER_WALL_SLIDE, 0, 0);
                    return true;
                }
                break;
            }
            if (m->forwardVel > 12.f) {
                mario_bonk_reflection(m, false);
                m->faceAngle[1] += 0x8000;
                m->particleFlags |= PARTICLE_VERTICAL_STAR;
                omm_mario_set_action(m, ACT_OMM_METAL_WATER_BACKWARD_AIR_KB, 0, 0);
                return true;
            } else if (m->wall) {
                mario_set_forward_vel(m, 0.f);
            }
            break;
    }
    return false;
}

static u32 omm_metal_water_update_punch_sequence(struct MarioState *m) {
    switch (gOmmData->mario->metalWater.punchType) {

        // First punch, first half
        case 0: {
            m->flags |= MARIO_PUNCHING;
            play_mario_action_sound(m, SOUND_ACTION_METAL_STEP_WATER, 0);
            set_mario_animation(m, MARIO_ANIM_FIRST_PUNCH);
            if (is_anim_past_end(m)) {
                gOmmData->mario->metalWater.punchType = 1;
                m->marioBodyState->punchState = (0 << 6) | 4;
            }
        } break;

        // First punch, second half
        case 1: {
            m->flags |= MARIO_PUNCHING;
            set_mario_animation(m, MARIO_ANIM_FIRST_PUNCH_FAST);
            if (m->controller->buttonPressed & B_BUTTON) {
                gOmmData->mario->metalWater.punchType = 2;
                m->flags &= ~MARIO_ACTION_SOUND_PLAYED;
            } else if (is_anim_at_end(m)) {
                return ACT_OMM_METAL_WATER_IDLE;
            }
        } break;

        // Second punch, first half
        case 2: {
            m->flags |= MARIO_PUNCHING;
            play_mario_action_sound(m, SOUND_ACTION_METAL_STEP_WATER, 0);
            set_mario_animation(m, MARIO_ANIM_SECOND_PUNCH);
            if (is_anim_past_end(m)) {
                gOmmData->mario->metalWater.punchType = 3;
                m->marioBodyState->punchState = (1 << 6) | 4;
            }
        } break;

        // Second punch, second half
        case 3: {
            m->flags |= MARIO_PUNCHING;
            set_mario_animation(m, MARIO_ANIM_SECOND_PUNCH_FAST);
            if (m->controller->buttonPressed & B_BUTTON) {
                gOmmData->mario->metalWater.punchType = 4;
                m->flags &= ~MARIO_ACTION_SOUND_PLAYED;
            } else if (is_anim_at_end(m)) {
                return ACT_OMM_METAL_WATER_IDLE;
            }
        } break;

        // Ground kick
        case 4: {
            if (!(m->flags & MARIO_ACTION_SOUND_PLAYED)) {
                play_mario_action_sound(m, SOUND_ACTION_METAL_JUMP_WATER, 0);
                m->marioBodyState->punchState = (2 << 6) | 6;
            }
            if (set_mario_animation(m, MARIO_ANIM_GROUND_KICK) < 8) {
                m->flags |= MARIO_KICKING;
            } else if (is_anim_at_end(m)) {
                return ACT_OMM_METAL_WATER_IDLE;
            }
        } break;
    }
    return 0;
}

//
// Moves
//

static s32 omm_act_metal_water_idle(struct MarioState *m) {
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_GROUND, 0, RETURN_CANCEL);
    action_a_pressed(1, gOmmData->mario->metalWater.jumpNext, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_PUNCHING, 0, RETURN_CANCEL);
    action_z_pressed(1, ACT_OMM_METAL_WATER_START_CROUCHING, 0, RETURN_CANCEL);
    action_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_GROUND, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);
    action_moving(1, ACT_OMM_METAL_WATER_WALKING, 0, RETURN_CANCEL);
    if (omm_metal_water_common_stationary_step(m, MARIO_ANIM_IDLE_HEAD_LEFT + m->actionState)) m->actionState = (m->actionState + 1) % 3;
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_walking(struct MarioState *m) {
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_GROUND, 0, RETURN_CANCEL);
    action_a_pressed(analog_stick_held_back(m) && m->forwardVel >= OMM_MARIO_METAL_WATER_MAX_WALKING_SPEED / 2.f, ACT_OMM_METAL_WATER_SIDE_FLIP, 0, RETURN_CANCEL);
    action_a_pressed((m->forwardVel >= OMM_MARIO_METAL_WATER_MAX_WALKING_SPEED / 2.f || gOmmData->mario->metalWater.jumpNext == ACT_OMM_METAL_WATER_LONG_JUMP) && (m->controller->buttonDown & Z_TRIG), ACT_OMM_METAL_WATER_LONG_JUMP, 0, RETURN_CANCEL);
    action_a_pressed(1, gOmmData->mario->metalWater.jumpNext, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_PUNCHING, 0, RETURN_CANCEL);
    action_z_pressed(1, ACT_OMM_METAL_WATER_START_CROUCHING, 0, RETURN_CANCEL);
    action_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_GROUND, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);

    omm_metal_water_update_walking_speed(m);
    Vec3f pos = { m->pos[0], m->pos[1], m->pos[2] };
    s32 step = perform_ground_step(m);
    action_condition(step == GROUND_STEP_LEFT_GROUND, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_BREAK);
    action_condition(m->forwardVel == 0, ACT_OMM_METAL_WATER_IDLE, 0, RETURN_BREAK);

    // Push / Side-step
    if (step == GROUND_STEP_HIT_WALL) {
        mario_set_forward_vel(m, omm_min_f(m->forwardVel, OMM_MARIO_METAL_WATER_MAX_WALKING_SPEED / 4.f));
        if (m->wall == NULL) {
            set_mario_animation(m, MARIO_ANIM_PUSHING);
            omm_metal_water_play_step_sound(m, 6, 18);
        } else {
            s16 wallAngle = atan2s(m->wall->normal.z, m->wall->normal.x);
            s16 dWallAngle = wallAngle - m->faceAngle[1];
            if (omm_abs_s(dWallAngle) >= 0x7200) {
                set_mario_animation(m, MARIO_ANIM_PUSHING);
                omm_metal_water_play_step_sound(m, 6, 18);
            } else {
                s32 animAccel = (s32) (sqrtf(omm_sqr_f(m->pos[0] - pos[0]) + omm_sqr_f(m->pos[2] - pos[2])) * 2.f * 0x10000);
                set_mario_anim_with_accel(m, MARIO_ANIM_SIDESTEP_LEFT + (dWallAngle < 0), animAccel);
                omm_metal_water_play_step_sound(m, 20, 20);
                m->marioObj->header.gfx.angle[1] = wallAngle + 0x8000;
            }
        }
    } else {
        set_mario_anim_with_accel(m, MARIO_ANIM_WALKING, omm_max_s(0x1000, (m->forwardVel / 4.0f) * 0x10000));
        omm_metal_water_play_step_sound(m, 10, 49);
    }
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_start_crouching(struct MarioState *m) {
    action_a_pressed(m->forwardVel >= OMM_MARIO_METAL_WATER_MAX_WALKING_SPEED / 2.f && m->controller->buttonDown & Z_TRIG, ACT_OMM_METAL_WATER_LONG_JUMP, 0, RETURN_CANCEL);
    action_a_pressed(1, ACT_OMM_METAL_WATER_BACKFLIP, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);

    mario_set_forward_vel(m, omm_max_f(0.f, m->forwardVel - 2.f));
    perform_ground_step(m);
    m->pos[1] = m->floorHeight;
    m->vel[1] = 0.f;
    vec3f_copy(m->marioObj->header.gfx.pos, m->pos);
    vec3s_set(m->marioObj->header.gfx.angle, 0, m->faceAngle[1], 0);

    set_mario_animation(m, MARIO_ANIM_START_CROUCHING);
    if (m->forwardVel > 0.f) m->particleFlags |= PARTICLE_DUST;
    action_condition(m->forwardVel == 0.f && is_anim_at_end(m), ACT_OMM_METAL_WATER_CROUCHING, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_crouching(struct MarioState *m) {
    action_condition(!(m->controller->buttonDown & Z_TRIG), ACT_OMM_METAL_WATER_STOP_CROUCHING, 0, RETURN_CANCEL);
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_GROUND, 0, RETURN_CANCEL);
    action_a_pressed(1, ACT_OMM_METAL_WATER_BACKFLIP, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_PUNCHING, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);
    action_moving(1, ACT_OMM_METAL_WATER_START_CRAWLING, 0, RETURN_CANCEL);
    omm_metal_water_common_stationary_step(m, MARIO_ANIM_CROUCHING);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_stop_crouching(struct MarioState *m) {
    action_a_pressed(1, ACT_OMM_METAL_WATER_BACKFLIP, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_stationary_step(m, MARIO_ANIM_STOP_CROUCHING), ACT_OMM_METAL_WATER_IDLE, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_start_crawling(struct MarioState *m) {
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_stationary_step(m, MARIO_ANIM_START_CRAWLING), ACT_OMM_METAL_WATER_CRAWLING, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_crawling(struct MarioState *m) {
    action_condition(!(m->controller->buttonDown & Z_TRIG), ACT_OMM_METAL_WATER_STOP_CRAWLING, 0, RETURN_CANCEL);
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_GROUND, 0, RETURN_CANCEL);
    action_a_pressed(1, gOmmData->mario->metalWater.jumpNext, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_PUNCHING, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);

    m->intendedMag *= 0.125f;
    omm_metal_water_update_walking_speed(m);
    s32 step = perform_ground_step(m);
    action_condition(step == GROUND_STEP_LEFT_GROUND, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_BREAK);
    action_condition(m->forwardVel == 0, ACT_OMM_METAL_WATER_STOP_CRAWLING, 0, RETURN_BREAK);

    set_mario_anim_with_accel(m, MARIO_ANIM_CRAWLING, (s32) (m->forwardVel * 2.f * 0x10000));
    omm_metal_water_play_step_sound(m, 26, 79);

    // Align Mario with the ground below
    static Mat4 sFloorAlignMatrix;
    m->pos[1] = m->floorHeight;
    mtxf_align_terrain_triangle(sFloorAlignMatrix, m->pos, m->faceAngle[1], 40.f);
    m->marioObj->header.gfx.throwMatrix = &sFloorAlignMatrix;
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_stop_crawling(struct MarioState *m) {
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_stationary_step(m, MARIO_ANIM_STOP_CRAWLING), ACT_OMM_METAL_WATER_CROUCHING, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

//
// Jumps
//

static s32 omm_act_metal_water_jump(struct MarioState *m) {
    action_init(m->forwardVel * 0.9f, 30.f, PARTICLE_MIST_CIRCLE, SOUND_ACTION_METAL_JUMP_WATER);
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_AIRBORNE, 0, RETURN_CANCEL);
    action_zb_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_DIVE, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_JUMP_KICK, 0, RETURN_CANCEL);
    action_z_pressed(1, ACT_OMM_METAL_WATER_GROUND_POUND, 0, RETURN_CANCEL);
    action_air_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_AIR, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_check_water_jump(m), ACT_WATER_JUMP, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_air_action_step(m, ACT_OMM_METAL_WATER_JUMP_LAND, MARIO_ANIM_SINGLE_JUMP), 0, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_jump_land(struct MarioState *m) {
    action_init(m->forwardVel, 0.f, PARTICLE_MIST_CIRCLE, SOUND_ACTION_METAL_LAND_WATER, omm_metal_water_set_next_jump(ACT_OMM_METAL_WATER_DOUBLE_JUMP););
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_GROUND, 0, RETURN_CANCEL);
    action_a_pressed(1, gOmmData->mario->metalWater.jumpNext, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_PUNCHING, 0, RETURN_CANCEL);
    action_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_GROUND, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);
    action_moving(1, ACT_OMM_METAL_WATER_WALKING, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_stationary_step(m, MARIO_ANIM_LAND_FROM_SINGLE_JUMP), ACT_OMM_METAL_WATER_IDLE, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_double_jump(struct MarioState *m) {
    action_init(m->forwardVel * 0.9f, 38.f, PARTICLE_MIST_CIRCLE, SOUND_ACTION_METAL_JUMP_WATER);
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_AIRBORNE, 0, RETURN_CANCEL);
    action_zb_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_DIVE, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_JUMP_KICK, 0, RETURN_CANCEL);
    action_z_pressed(1, ACT_OMM_METAL_WATER_GROUND_POUND, 0, RETURN_CANCEL);
    action_air_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_AIR, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_check_water_jump(m), ACT_WATER_JUMP, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_air_action_step(m, ACT_OMM_METAL_WATER_DOUBLE_JUMP_LAND, m->vel[1] > 0.f ? MARIO_ANIM_DOUBLE_JUMP_RISE : MARIO_ANIM_DOUBLE_JUMP_FALL), 0, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_double_jump_land(struct MarioState *m) {
    action_init(m->forwardVel, 0.f, PARTICLE_MIST_CIRCLE, SOUND_ACTION_METAL_LAND_WATER, omm_metal_water_set_next_jump(ACT_OMM_METAL_WATER_TRIPLE_JUMP););
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_GROUND, 0, RETURN_CANCEL);
    action_a_pressed(1, gOmmData->mario->metalWater.jumpNext, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_PUNCHING, 0, RETURN_CANCEL);
    action_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_GROUND, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);
    action_moving(1, ACT_OMM_METAL_WATER_WALKING, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_stationary_step(m, MARIO_ANIM_LAND_FROM_DOUBLE_JUMP), ACT_OMM_METAL_WATER_IDLE, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_triple_jump(struct MarioState *m) {
    action_init(m->forwardVel * 0.9f, 46.f, PARTICLE_MIST_CIRCLE, SOUND_ACTION_METAL_JUMP_WATER);
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_AIRBORNE, 0, RETURN_CANCEL);
    action_zb_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_DIVE, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_JUMP_KICK, 0, RETURN_CANCEL);
    action_z_pressed(1, ACT_OMM_METAL_WATER_GROUND_POUND, 0, RETURN_CANCEL);
    action_air_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_AIR, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_check_water_jump(m), ACT_WATER_JUMP, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_air_action_step(m, ACT_OMM_METAL_WATER_TRIPLE_JUMP_LAND, MARIO_ANIM_TRIPLE_JUMP), 0, 0, RETURN_BREAK);
    play_flip_sounds(m, 2, 8, 20);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_triple_jump_land(struct MarioState *m) {
    action_init(m->forwardVel, 0.f, PARTICLE_MIST_CIRCLE, SOUND_ACTION_METAL_LAND_WATER, omm_metal_water_set_next_jump(ACT_OMM_METAL_WATER_JUMP););
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_GROUND, 0, RETURN_CANCEL);
    action_a_pressed(1, gOmmData->mario->metalWater.jumpNext, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_PUNCHING, 0, RETURN_CANCEL);
    action_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_GROUND, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);
    action_moving(1, ACT_OMM_METAL_WATER_WALKING, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_stationary_step(m, MARIO_ANIM_TRIPLE_JUMP_LAND), ACT_OMM_METAL_WATER_IDLE, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_backflip(struct MarioState *m) {
    action_init(-12.f, 42.f, PARTICLE_MIST_CIRCLE, SOUND_ACTION_METAL_JUMP_WATER);
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_AIRBORNE, 0, RETURN_CANCEL);
    action_zb_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_DIVE, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_JUMP_KICK, 0, RETURN_CANCEL);
    action_z_pressed(1, ACT_OMM_METAL_WATER_GROUND_POUND, 0, RETURN_CANCEL);
    action_air_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_AIR, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_check_water_jump(m), ACT_WATER_JUMP, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_air_action_step(m, ACT_OMM_METAL_WATER_JUMP_LAND, MARIO_ANIM_BACKFLIP), 0, 0, RETURN_BREAK);
    play_flip_sounds(m, 2, 3, 17);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_backflip_land(struct MarioState *m) {
    action_init(m->forwardVel, 0.f, PARTICLE_MIST_CIRCLE, SOUND_ACTION_METAL_LAND_WATER, omm_metal_water_set_next_jump(ACT_OMM_METAL_WATER_DOUBLE_JUMP););
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_GROUND, 0, RETURN_CANCEL);
    action_a_pressed(1, gOmmData->mario->metalWater.jumpNext, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_PUNCHING, 0, RETURN_CANCEL);
    action_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_GROUND, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);
    action_moving(1, ACT_OMM_METAL_WATER_WALKING, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_stationary_step(m, MARIO_ANIM_TRIPLE_JUMP_LAND), ACT_OMM_METAL_WATER_IDLE, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_side_flip(struct MarioState *m) {
    action_init(m->forwardVel, 42.f, PARTICLE_MIST_CIRCLE, SOUND_ACTION_METAL_JUMP_WATER, m->faceAngle[1] += 0x8000;);
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_AIRBORNE, 0, RETURN_CANCEL);
    action_zb_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_DIVE, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_JUMP_KICK, 0, RETURN_CANCEL);
    action_z_pressed(1, ACT_OMM_METAL_WATER_GROUND_POUND, 0, RETURN_CANCEL);
    action_air_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_AIR, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_check_water_jump(m), ACT_WATER_JUMP, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_air_action_step(m, ACT_OMM_METAL_WATER_JUMP_LAND, MARIO_ANIM_SLIDEFLIP), 0, 0, RETURN_BREAK);
    if (m->marioObj->header.gfx.mAnimInfo.animFrame == 6) play_sound(SOUND_ACTION_SIDE_FLIP_UNK, m->marioObj->oCameraToObject);
    m->marioObj->header.gfx.angle[1] = m->faceAngle[1] + 0x8000;
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_side_flip_land(struct MarioState *m) {
    action_init(m->forwardVel, 0.f, PARTICLE_MIST_CIRCLE, SOUND_ACTION_METAL_LAND_WATER, omm_metal_water_set_next_jump(ACT_OMM_METAL_WATER_DOUBLE_JUMP););
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_GROUND, 0, RETURN_CANCEL);
    action_a_pressed(1, gOmmData->mario->metalWater.jumpNext, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_PUNCHING, 0, RETURN_CANCEL);
    action_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_GROUND, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);
    action_moving(1, ACT_OMM_METAL_WATER_WALKING, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_stationary_step(m, MARIO_ANIM_SLIDEFLIP_LAND), ACT_OMM_METAL_WATER_IDLE, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_long_jump(struct MarioState *m) {
    action_init((m->forwardVel *= 1.5f), 26.f, PARTICLE_MIST_CIRCLE, SOUND_ACTION_METAL_JUMP_WATER);
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_AIRBORNE, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_JUMP_KICK, 0, RETURN_CANCEL);
    action_air_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_AIR, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_check_water_jump(m), ACT_WATER_JUMP, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_air_action_step(m, ACT_OMM_METAL_WATER_LONG_JUMP_LAND, MARIO_ANIM_SLOW_LONGJUMP), 0, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_long_jump_land(struct MarioState *m) {
    action_init(m->forwardVel, 0.f, PARTICLE_MIST_CIRCLE, SOUND_ACTION_METAL_LAND_WATER, omm_metal_water_set_next_jump(ACT_OMM_METAL_WATER_LONG_JUMP););
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_GROUND, 0, RETURN_CANCEL);
    action_a_pressed(m->controller->buttonDown & Z_TRIG, gOmmData->mario->metalWater.jumpNext, 0, NO_RETURN);
    action_a_pressed(1, ACT_OMM_METAL_WATER_JUMP, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_PUNCHING, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);
    action_moving(1, ACT_OMM_METAL_WATER_WALKING, 0, RETURN_CANCEL);

    if (!(m->input & INPUT_NONZERO_ANALOG)) {
        mario_set_forward_vel(m, omm_max_f(0.f, m->forwardVel - 2.f));
    } else {
        mario_set_forward_vel(m, approach_f32(m->forwardVel, 0.f, 1.5f, 1.5f));
    }
    perform_ground_step(m);
    m->pos[1] = m->floorHeight;
    m->vel[1] = 0.f;
    vec3f_copy(m->marioObj->header.gfx.pos, m->pos);
    vec3s_set(m->marioObj->header.gfx.angle, 0, m->faceAngle[1], 0);

    set_mario_animation(m, MARIO_ANIM_CROUCHING);
    if (m->forwardVel > 0.f) m->particleFlags |= PARTICLE_DUST;
    action_condition(m->forwardVel == 0.f, ACT_OMM_METAL_WATER_STOP_CROUCHING, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_freefall(struct MarioState *m) {
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_AIRBORNE, 0, RETURN_CANCEL);
    action_zb_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_DIVE, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_JUMP_KICK, 0, RETURN_CANCEL);
    action_z_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_GROUND_POUND, 0, RETURN_CANCEL);
    action_air_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_AIR, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_check_water_jump(m), ACT_WATER_JUMP, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_air_action_step(m, ACT_OMM_METAL_WATER_FREEFALL_LAND, MARIO_ANIM_GENERAL_FALL), 0, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_freefall_land(struct MarioState *m) {
    action_init(m->forwardVel, 0.f, PARTICLE_MIST_CIRCLE, SOUND_ACTION_METAL_LAND_WATER, omm_metal_water_set_next_jump(ACT_OMM_METAL_WATER_DOUBLE_JUMP););
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_GROUND, 0, RETURN_CANCEL);
    action_a_pressed(1, gOmmData->mario->metalWater.jumpNext, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_PUNCHING, 0, RETURN_CANCEL);
    action_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_GROUND, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);
    action_moving(1, ACT_OMM_METAL_WATER_WALKING, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_stationary_step(m, MARIO_ANIM_GENERAL_LAND), ACT_OMM_METAL_WATER_IDLE, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s16 omm_metal_water_wall_slide_get_jump_angle(struct MarioState *m) {
    s16 wAngle = atan2s(m->wall->normal.z, m->wall->normal.x);
    s16 dAngle = m->faceAngle[1] - wAngle;
    if (m->controller->stickMag > 32.f) {
        dAngle = m->intendedYaw - wAngle;
        if (-0x4000 < dAngle && dAngle < +0x4000) {
            dAngle = 0x8000 - dAngle;
        }
        if (dAngle < 0) {
            dAngle = omm_min_s(dAngle, -0x8000 + OMM_MARIO_WALL_SLIDE_JUMP_ANGLE_MAX);
        } else {
            dAngle = omm_max_s(dAngle, +0x8000 - OMM_MARIO_WALL_SLIDE_JUMP_ANGLE_MAX);
        }
    }
    return (0x8000 + wAngle - dAngle);
}

static s32 omm_act_metal_water_wall_slide(struct MarioState *m) {
    action_condition(!m->wall, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL, SFX(SOUND_ACTION_METAL_STEP_WATER););
    action_a_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_WALL_KICK_AIR, 0, RETURN_CANCEL, m->faceAngle[1] = omm_metal_water_wall_slide_get_jump_angle(m););
    action_z_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL, SFX(SOUND_ACTION_METAL_STEP_WATER););

    mario_set_forward_vel(m, 0);
    set_mario_animation(m, MARIO_ANIM_START_WALLKICK);
    play_sound(SOUND_MOVING_TERRAIN_SLIDE + SOUND_TERRAIN_STONE, m->marioObj->oCameraToObject);
    m->particleFlags |= PARTICLE_DUST;

    // Cling Mario to the wall before performing the air step,
    // to avoid missing slightly slanted walls (normal.y near 0, but not 0)
    m->pos[0] -= m->wall->normal.x * 4.f;
    m->pos[2] -= m->wall->normal.z * 4.f;
    s32 step = perform_air_step(m, 0);
    action_condition(step == AIR_STEP_LANDED, ACT_OMM_METAL_WATER_IDLE, 0, RETURN_BREAK);
    action_condition(step == AIR_STEP_NONE, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_BREAK);
    action_condition(step == AIR_STEP_HIT_WALL && !omm_mario_check_wall_slide(m), ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_BREAK);

    // Turn Mario away from the wall
    m->marioObj->header.gfx.angle[1] = atan2s(m->wall->normal.z, m->wall->normal.x);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_wall_kick_air(struct MarioState *m) {
    action_init(16.f, 36.f, 0, SOUND_ACTION_METAL_JUMP_WATER);
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_AIRBORNE, 0, RETURN_CANCEL);
    action_zb_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_DIVE, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_JUMP_KICK, 0, RETURN_CANCEL);
    action_z_pressed(1, ACT_OMM_METAL_WATER_GROUND_POUND, 0, RETURN_CANCEL);
    action_air_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_AIR, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_check_water_jump(m), ACT_WATER_JUMP, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_air_action_step(m, ACT_OMM_METAL_WATER_JUMP_LAND, MARIO_ANIM_SLIDEJUMP), 0, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

//
// Attacks
//

static s32 omm_act_metal_water_punching(struct MarioState *m) {
    action_init(omm_max_f(12.f, m->forwardVel), 0.f, 0, 0, omm_metal_water_set_next_jump(ACT_OMM_METAL_WATER_JUMP););
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_GROUND, 0, RETURN_CANCEL);
    action_a_pressed(1, gOmmData->mario->metalWater.jumpNext, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);

    mario_set_forward_vel(m, approach_f32(omm_clamp_f(m->forwardVel, -OMM_MARIO_METAL_WATER_MAX_WALKING_SPEED, OMM_MARIO_METAL_WATER_MAX_WALKING_SPEED), 0, 1.5f, 1.f));
    perform_ground_step(m);
    u32 action = omm_metal_water_update_punch_sequence(m);
    action_condition(action != 0, action, 0, RETURN_BREAK);
    if (m->forwardVel != 0.f) m->particleFlags |= PARTICLE_DUST;
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_jump_kick(struct MarioState *m) {
    bool cappyTwirl = (OMM_MOVESET_ODYSSEY && (omm_cappy_get_object() != NULL));
    action_init(omm_min_f(m->forwardVel, 6.f), (cappyTwirl ? 26.f : 22.f), PARTICLE_MIST_CIRCLE, SOUND_ACTION_METAL_JUMP_WATER, m->actionArg = cappyTwirl;);
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_AIRBORNE, 0, RETURN_CANCEL);
    action_zb_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_DIVE, 0, RETURN_CANCEL);
    action_z_pressed(1, ACT_OMM_METAL_WATER_GROUND_POUND, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_check_water_jump(m), ACT_WATER_JUMP, 0, RETURN_CANCEL);

    // Twirl
    if (m->actionArg == 1) {
        set_mario_anim_with_accel(m, MARIO_ANIM_FINAL_BOWSER_RAISE_HAND_SPIN, 0x18000);
        m->marioObj->header.gfx.mAnimInfo.animFrameAccelAssist = omm_clamp_s(m->marioObj->header.gfx.mAnimInfo.animFrameAccelAssist, (74 << 16), (94 << 16));
        m->flags |= MARIO_KICKING * (m->marioObj->header.gfx.mAnimInfo.animFrameAccelAssist < (94 << 16));
        m->marioBodyState->punchState = 0;

        omm_mario_update_air_without_turn(m);
        switch (perform_air_step(m, 0)) {
            case AIR_STEP_LANDED: omm_mario_set_action(m, ACT_OMM_METAL_WATER_FREEFALL_LAND, 0, 0); break;
            case AIR_STEP_HIT_WALL: mario_set_forward_vel(m, 0.0f); break;
        }
        return OMM_MARIO_ACTION_RESULT_CONTINUE;
    }

    // Kick
    if (set_mario_animation(m, MARIO_ANIM_AIR_KICK) == 0) m->marioBodyState->punchState = (2 << 6) | 6;
    if (!is_anim_past_end(m)) m->flags |= MARIO_KICKING;
    action_condition(omm_metal_water_common_air_action_step(m, ACT_OMM_METAL_WATER_FREEFALL_LAND, MARIO_ANIM_AIR_KICK), 0, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_dive(struct MarioState *m) {
    action_init(12.f, 24.f, PARTICLE_MIST_CIRCLE, SOUND_ACTION_METAL_JUMP_WATER);
    action_condition(omm_metal_water_check_water_jump(m), ACT_WATER_JUMP, 0, RETURN_CANCEL);

    set_mario_animation(m, MARIO_ANIM_DIVE);
    omm_mario_update_air_without_turn(m);
    switch (perform_air_step(m, 0)) {
        case AIR_STEP_NONE:
            m->faceAngle[0] = omm_max_s(m->faceAngle[0] - 0x200 * (m->vel[1] < 0.0f), -0x2AAA);
            m->marioObj->header.gfx.angle[0] = -m->faceAngle[0];
            break;

        case AIR_STEP_LANDED:
            m->faceAngle[0] = 0;
            play_sound(SOUND_ACTION_METAL_LAND_WATER, m->marioObj->oCameraToObject);
            omm_mario_set_action(m, ACT_OMM_METAL_WATER_FREEFALL_LAND, 0, 0);
            break;

        case AIR_STEP_HIT_WALL:
            mario_bonk_reflection(m, true);
            m->faceAngle[0] = 0;
            m->vel[1] = omm_min_f(0, m->vel[1]);
            m->particleFlags |= PARTICLE_VERTICAL_STAR;
            omm_mario_set_action(m, ACT_OMM_METAL_WATER_BACKWARD_AIR_KB, 0, 0);
            break;
    }
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_ground_pound(struct MarioState *m) {
    action_b_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_DIVE, 0, RETURN_CANCEL);

    if (m->actionState == 0) {

        m->pos[1] = omm_min_f(m->pos[1] + omm_max_s(0, 20 - 2 * m->actionTimer), m->ceilHeight - 160.f);
        vec3f_copy(m->marioObj->header.gfx.pos, m->pos);
        mario_set_forward_vel(m, 0.0f);
        set_mario_animation(m, m->actionArg == 0 ? MARIO_ANIM_START_GROUND_POUND : MARIO_ANIM_TRIPLE_JUMP_GROUND_POUND);
        if (m->actionTimer == 0) play_sound(SOUND_ACTION_SPIN, m->marioObj->oCameraToObject);

        if (++m->actionTimer >= m->marioObj->header.gfx.mAnimInfo.curAnim->mLoopEnd + 6) {
            m->vel[1] = -32.f;
            m->actionState = 1;
        }

    } else {

        m->vel[1] = -32.f;
        mario_set_forward_vel(m, 0);
        set_mario_animation(m, MARIO_ANIM_GROUND_POUND);
        m->particleFlags |= PARTICLE_PLUNGE_BUBBLE;
        switch (perform_air_step(m, 0)) {
            case AIR_STEP_LANDED:
                play_sound(SOUND_ACTION_METAL_LAND_WATER, m->marioObj->oCameraToObject);
                m->particleFlags |= PARTICLE_MIST_CIRCLE | PARTICLE_HORIZONTAL_STAR;
                omm_mario_set_action(m, ACT_OMM_METAL_WATER_GROUND_POUND_LAND, 0, 0);
                set_camera_shake_from_hit(SHAKE_GROUND_POUND);
                break;
        }
    }

    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_ground_pound_land(struct MarioState *m) {
    action_init(0, 0, 0, 0, omm_metal_water_set_next_jump(ACT_OMM_METAL_WATER_JUMP););
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_GROUND, 0, RETURN_CANCEL);
    action_a_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_GROUND_POUND_JUMP, 0, RETURN_CANCEL);
    action_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_GROUND, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_stationary_step(m, MARIO_ANIM_GROUND_POUND_LANDING), ACT_OMM_METAL_WATER_GROUND_POUND_LAND_STOP, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_ground_pound_land_stop(struct MarioState *m) {
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_GROUND, 0, RETURN_CANCEL);
    action_a_pressed(1, gOmmData->mario->metalWater.jumpNext, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_PUNCHING, 0, RETURN_CANCEL);
    action_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_GROUND, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_stationary_step(m, MARIO_ANIM_STOP_SLIDE), ACT_OMM_METAL_WATER_IDLE, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_ground_pound_jump(struct MarioState *m) {
    action_init(0, 44.f, 0, SOUND_ACTION_METAL_JUMP_WATER, gOmmData->mario->spin.yaw = 1;);
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_AIRBORNE, 0, RETURN_CANCEL);
    action_zb_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_DIVE, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_JUMP_KICK, 0, RETURN_CANCEL);
    action_z_pressed(1, ACT_OMM_METAL_WATER_GROUND_POUND, 0, RETURN_CANCEL);
    action_air_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_AIR, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_check_water_jump(m), ACT_WATER_JUMP, 0, RETURN_CANCEL);

    if (m->vel[1] >= 30.f) {
        s32 step = perform_air_step(m, 0);
        action_condition(step == AIR_STEP_LANDED, ACT_OMM_METAL_WATER_JUMP_LAND, 0, RETURN_BREAK);
    } else {
        bool shouldBreak = omm_metal_water_common_air_action_step(m, ACT_OMM_METAL_WATER_JUMP_LAND, MARIO_ANIM_SINGLE_JUMP);
        action_condition(shouldBreak, 0, 0, RETURN_BREAK);
    }

    set_mario_animation(m, MARIO_ANIM_SINGLE_JUMP);
    if (m->actionTimer++ > 10) set_anim_to_frame(m, 10); // Luigi's freaking flutter jump (again)
    s16 prevSpinYaw = gOmmData->mario->spin.yaw;
    gOmmData->mario->spin.yaw += (0x80 * m->vel[1]) * (prevSpinYaw != 0) * (omm_mario_has_vanish_cap(m) ? 0.8f : 1.f) / omm_sqr_f(omm_player_get_selected_jump_multiplier());
    gOmmData->mario->spin.yaw *= ((u16) prevSpinYaw < (u16) gOmmData->mario->spin.yaw) * (m->vel[1] > 0.f);
    m->marioObj->header.gfx.angle[1] = m->faceAngle[1] + gOmmData->mario->spin.yaw;
    m->particleFlags |= (m->vel[1] > 0.f ? PARTICLE_DUST : 0);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_backward_ground_kb(struct MarioState *m) {
    mario_set_forward_vel(m, m->forwardVel * 0.94f);
    set_mario_animation(m, MARIO_ANIM_BACKWARD_KB);

    s32 step = perform_ground_step(m);
    action_condition(step == GROUND_STEP_LEFT_GROUND, ACT_OMM_METAL_WATER_BACKWARD_AIR_KB, 0, RETURN_BREAK);
    action_condition(is_anim_at_end(m), ACT_OMM_METAL_WATER_IDLE, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_backward_air_kb(struct MarioState *m) {
    action_init((m->prevAction == ACT_OMM_METAL_WATER_BACKWARD_GROUND_KB ? m->forwardVel : -16.f), +16.f, 0, 0);
    mario_set_forward_vel(m, m->forwardVel * 0.97f);
    set_mario_animation(m, MARIO_ANIM_BACKWARD_AIR_KB);

    switch (perform_air_step(m, 0)) {
        case AIR_STEP_LANDED:
            play_sound(SOUND_ACTION_METAL_LAND_WATER, m->marioObj->oCameraToObject);
            omm_mario_set_action(m, ACT_OMM_METAL_WATER_BACKWARD_GROUND_KB, 0, 0);
            break;

        case AIR_STEP_HIT_WALL:
            mario_bonk_reflection(m, false);
            m->vel[1] = omm_min_f(0, m->vel[1]);
            mario_set_forward_vel(m, m->forwardVel);
            break;
    }
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

//
// Cappy
//

static s32 omm_act_metal_water_cappy_throw_ground(struct MarioState *m) {
    f32 f = coss(omm_abs_s(m->faceAngle[1] - m->intendedYaw)) * m->controller->stickMag / 64.f;
    mario_set_forward_vel(m, m->forwardVel * omm_clamp_f(f, 0.80f, 0.98f));

    s32 step = perform_ground_step(m);
    action_condition(step == GROUND_STEP_LEFT_GROUND, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_BREAK);
    action_condition(step == GROUND_STEP_HIT_WALL, ACT_OMM_METAL_WATER_IDLE, 0, RETURN_BREAK, m->forwardVel = 0;);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_cappy_throw_airborne(struct MarioState *m) {
    action_init(omm_min_f(m->forwardVel, 4.f), 10.f, 0, 0);
    action_zb_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_DIVE, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_JUMP_KICK, 0, RETURN_CANCEL);
    action_z_pressed(1, ACT_OMM_METAL_WATER_GROUND_POUND, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_check_water_jump(m), ACT_WATER_JUMP, 0, RETURN_CANCEL);

    omm_metal_water_common_air_action_step(m, ACT_OMM_METAL_WATER_FREEFALL_LAND, m->marioObj->header.gfx.mAnimInfo.animID);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_cappy_bounce(struct MarioState *m) {
    action_init(m->forwardVel * 0.9f, 40.f, 0, SOUND_ACTION_METAL_JUMP_WATER);
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_AIRBORNE, 0, RETURN_CANCEL);
    action_zb_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_DIVE, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_JUMP_KICK, 0, RETURN_CANCEL);
    action_z_pressed(1, ACT_OMM_METAL_WATER_GROUND_POUND, 0, RETURN_CANCEL);
    action_air_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_AIR, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_check_water_jump(m), ACT_WATER_JUMP, 0, RETURN_CANCEL);

    omm_metal_water_common_air_action_step(m, ACT_OMM_METAL_WATER_DOUBLE_JUMP_LAND, m->vel[1] >= 0.f ? MARIO_ANIM_DOUBLE_JUMP_RISE : MARIO_ANIM_DOUBLE_JUMP_FALL);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

//
// Spin
//

static s32 omm_act_metal_water_spin_ground(struct MarioState *m) {
    action_init(m->forwardVel, 0.f, 0, 0);
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_GROUND, 0, RETURN_CANCEL);
    action_a_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_JUMP, 0, RETURN_CANCEL);
    action_b_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_PUNCHING, 0, RETURN_CANCEL);
    action_off_floor(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_AIR, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);
    action_condition(gOmmData->mario->spin.timer == 0, ACT_OMM_METAL_WATER_IDLE, 0, RETURN_CANCEL);

    s32 step = perform_ground_step(m);
    action_condition(step == GROUND_STEP_LEFT_GROUND, ACT_OMM_METAL_WATER_SPIN_AIR, 0, RETURN_BREAK);

    mario_set_forward_vel(m, omm_max_f(0, m->forwardVel - 0.8f));
    set_mario_animation(m, MARIO_ANIM_TWIRL);
    play_sound(SOUND_MOVING_TERRAIN_SLIDE + SOUND_TERRAIN_STONE, m->marioObj->oCameraToObject);
    gOmmData->mario->spin.yaw += omm_min_s(0x238A, 0x200 * gOmmData->mario->spin.timer);
    m->marioObj->header.gfx.angle[1] = m->faceAngle[1] - gOmmData->mario->spin.yaw;
    m->marioBodyState->handState = MARIO_HAND_OPEN;
    m->particleFlags |= (PARTICLE_DUST | PARTICLE_PLUNGE_BUBBLE);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_spin_air(struct MarioState *m) {
    action_init(m->forwardVel, 12.f, 0, 0);
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_AIRBORNE, 0, RETURN_CANCEL);
    action_zb_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_DIVE, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_JUMP_KICK, 0, RETURN_CANCEL);
    action_z_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_POUND, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_check_water_jump(m), ACT_WATER_JUMP, 0, RETURN_CANCEL);
    action_condition(gOmmData->mario->spin.timer == 0, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);

    bool shouldBreak = omm_metal_water_common_air_action_step(m, ACT_OMM_METAL_WATER_JUMP_LAND, MARIO_ANIM_TWIRL);
    action_condition(shouldBreak, 0, 0, RETURN_BREAK);

    set_mario_animation(m, MARIO_ANIM_TWIRL);
    s16 prevSpinYaw = gOmmData->mario->spin.yaw;
    gOmmData->mario->spin.yaw += omm_min_s(0x4139, 0x300 * gOmmData->mario->spin.timer);
    if (gOmmData->mario->spin.yaw < prevSpinYaw) play_sound(SOUND_ACTION_SPIN, m->marioObj->oCameraToObject);
    m->marioObj->header.gfx.angle[1] = m->faceAngle[1] - gOmmData->mario->spin.yaw;
    m->marioBodyState->handState = MARIO_HAND_OPEN;
    m->particleFlags |= PARTICLE_PLUNGE_BUBBLE;
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_spin_jump(struct MarioState *m) {
    action_init(m->forwardVel, 48.f, 0, SOUND_ACTION_METAL_JUMP_WATER);
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_AIRBORNE, 0, RETURN_CANCEL);
    action_zb_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_DIVE, 0, RETURN_CANCEL);
    action_b_pressed(1, ACT_OMM_METAL_WATER_JUMP_KICK, 0, RETURN_CANCEL);
    action_z_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_POUND, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_check_water_jump(m), ACT_WATER_JUMP, 0, RETURN_CANCEL);
    
    bool shouldBreak = omm_metal_water_common_air_action_step(m, ACT_OMM_METAL_WATER_JUMP_LAND, MARIO_ANIM_TWIRL);
    action_condition(shouldBreak, 0, 0, RETURN_BREAK);
    action_condition(m->vel[1] <= 0.f, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_BREAK);

    set_mario_animation(m, MARIO_ANIM_TWIRL);
    s16 prevSpinYaw = gOmmData->mario->spin.yaw;
    gOmmData->mario->spin.yaw += omm_min_s(0x44CD, 0x300 * m->vel[1]);
    if (gOmmData->mario->spin.yaw < prevSpinYaw) play_sound(SOUND_ACTION_SPIN, m->marioObj->oCameraToObject);
    m->marioObj->header.gfx.angle[1] = m->faceAngle[1] - gOmmData->mario->spin.yaw;
    m->marioBodyState->handState = MARIO_HAND_OPEN;
    m->particleFlags |= PARTICLE_SPARKLES | PARTICLE_PLUNGE_BUBBLE;
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_spin_pound(struct MarioState *m) {
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_AIRBORNE, 0, RETURN_CANCEL);
    action_b_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_DIVE, 0, RETURN_CANCEL);

    m->vel[1] = -40.f;
    mario_set_forward_vel(m, 0);
    s32 step = perform_air_step(m, 0);
    s32 soundBits = (SOUND_ACTION_METAL_LAND_WATER | 0x0000FF00);
    u32 particles = (PARTICLE_MIST_CIRCLE | PARTICLE_HORIZONTAL_STAR);
    action_condition(step == AIR_STEP_LANDED, ACT_OMM_METAL_WATER_SPIN_POUND_LAND, 0, RETURN_BREAK, SFX(soundBits), PFX(particles), set_camera_shake_from_hit(SHAKE_GROUND_POUND););

    set_mario_animation(m, MARIO_ANIM_TWIRL);
    s16 prevSpinYaw = gOmmData->mario->spin.yaw;
    gOmmData->mario->spin.yaw += 0x3000;
    if (gOmmData->mario->spin.yaw < prevSpinYaw) play_sound(SOUND_ACTION_SIDE_FLIP_UNK, m->marioObj->oCameraToObject);
    m->marioObj->header.gfx.angle[1] = m->faceAngle[1] - gOmmData->mario->spin.yaw;
    m->marioBodyState->handState = MARIO_HAND_OPEN;
    m->particleFlags |= (PARTICLE_DUST | PARTICLE_PLUNGE_BUBBLE);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_metal_water_spin_pound_land(struct MarioState *m) {
    action_init(0, 0, 0, 0);
    action_cappy(1, ACT_OMM_METAL_WATER_CAPPY_THROW_GROUND, 0, RETURN_CANCEL);
    action_a_pressed(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_GROUND_POUND_JUMP, 0, RETURN_CANCEL);
    action_spin(OMM_MOVESET_ODYSSEY, ACT_OMM_METAL_WATER_SPIN_GROUND, 0, RETURN_CANCEL);
    action_off_floor(1, ACT_OMM_METAL_WATER_FREEFALL, 0, RETURN_CANCEL);
    action_condition(omm_metal_water_common_stationary_step(m, MARIO_ANIM_GENERAL_LAND), ACT_OMM_METAL_WATER_IDLE, 0, RETURN_BREAK);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

//
// Metal Water
//

s32 omm_mario_execute_metal_water_action(struct MarioState *m) {

    // Drown Metal Mario if 0 HP
    if (m->health <= OMM_HEALTH_DEAD) {
        omm_mario_set_action(m, ACT_DROWNING, 0, 0);
        return OMM_MARIO_ACTION_RESULT_CANCEL;
    }

    // Return to Submerged action if not Metal
    if (!omm_mario_has_metal_cap(m)) {
        omm_mario_set_action(m, ACT_WATER_IDLE, 0, 0);
        return OMM_MARIO_ACTION_RESULT_CANCEL;
    }

    // Return to vanilla Metal if not Improved Metal cap
    if (!OMM_POWER_UPS_IMPROVED) {
        omm_mario_set_action(m, ACT_METAL_WATER_FALLING, 0, 0);
        return OMM_MARIO_ACTION_RESULT_CANCEL;
    }

    // Water surface
    if (m->pos[1] > m->waterLevel - 80) {
        if (m->floorHeight >= m->waterLevel - 80) {
            transition_submerged_to_walking(m);
            return OMM_MARIO_ACTION_RESULT_CANCEL;
        }
        m->pos[1] = m->waterLevel - 80;
    }

    // Jump timer
    if (gOmmData->mario->metalWater.jumpTimer == 0) {
        gOmmData->mario->metalWater.jumpNext = ACT_OMM_METAL_WATER_JUMP;
    } else {
        gOmmData->mario->metalWater.jumpTimer--;
    }

    // Update
    gOmmData->mario->metalWater.punchType *= (m->action == ACT_OMM_METAL_WATER_PUNCHING);
    gOmmData->mario->wallSlide.jumped = false;
    gOmmData->mario->peach.floated = false;
    gOmmData->mario->cappy.bounced = false;
    gOmmData->mario->state.airCombo = 0;
    gOmmData->mario->midairSpin.counter = 0;
    m->quicksandDepth = 0.0f;
    m->marioBodyState->headAngle[1] = 0;
    m->marioBodyState->headAngle[2] = 0;
    m->particleFlags |= PARTICLE_BUBBLE;
    m->flags &= ~(MARIO_PUNCHING | MARIO_KICKING | MARIO_TRIPPING);

    // Actions
    switch (m->action) {

        // Moves
        case ACT_OMM_METAL_WATER_IDLE:                      return omm_act_metal_water_idle(m);
        case ACT_OMM_METAL_WATER_WALKING:                   return omm_act_metal_water_walking(m);
        case ACT_OMM_METAL_WATER_START_CROUCHING:           return omm_act_metal_water_start_crouching(m);
        case ACT_OMM_METAL_WATER_CROUCHING:                 return omm_act_metal_water_crouching(m);
        case ACT_OMM_METAL_WATER_STOP_CROUCHING:            return omm_act_metal_water_stop_crouching(m);
        case ACT_OMM_METAL_WATER_START_CRAWLING:            return omm_act_metal_water_start_crawling(m);
        case ACT_OMM_METAL_WATER_CRAWLING:                  return omm_act_metal_water_crawling(m);
        case ACT_OMM_METAL_WATER_STOP_CRAWLING:             return omm_act_metal_water_stop_crawling(m);

        // Jumps
        case ACT_OMM_METAL_WATER_JUMP:                      return omm_act_metal_water_jump(m);
        case ACT_OMM_METAL_WATER_JUMP_LAND:                 return omm_act_metal_water_jump_land(m);
        case ACT_OMM_METAL_WATER_DOUBLE_JUMP:               return omm_act_metal_water_double_jump(m);
        case ACT_OMM_METAL_WATER_DOUBLE_JUMP_LAND:          return omm_act_metal_water_double_jump_land(m);
        case ACT_OMM_METAL_WATER_TRIPLE_JUMP:               return omm_act_metal_water_triple_jump(m);
        case ACT_OMM_METAL_WATER_TRIPLE_JUMP_LAND:          return omm_act_metal_water_triple_jump_land(m);
        case ACT_OMM_METAL_WATER_BACKFLIP:                  return omm_act_metal_water_backflip(m);
        case ACT_OMM_METAL_WATER_BACKFLIP_LAND:             return omm_act_metal_water_backflip_land(m);
        case ACT_OMM_METAL_WATER_SIDE_FLIP:                 return omm_act_metal_water_side_flip(m);
        case ACT_OMM_METAL_WATER_SIDE_FLIP_LAND:            return omm_act_metal_water_side_flip_land(m);
        case ACT_OMM_METAL_WATER_LONG_JUMP:                 return omm_act_metal_water_long_jump(m);
        case ACT_OMM_METAL_WATER_LONG_JUMP_LAND:            return omm_act_metal_water_long_jump_land(m);
        case ACT_OMM_METAL_WATER_FREEFALL:                  return omm_act_metal_water_freefall(m);
        case ACT_OMM_METAL_WATER_FREEFALL_LAND:             return omm_act_metal_water_freefall_land(m);
        case ACT_OMM_METAL_WATER_WALL_SLIDE:                return omm_act_metal_water_wall_slide(m);
        case ACT_OMM_METAL_WATER_WALL_KICK_AIR:             return omm_act_metal_water_wall_kick_air(m);

        // Attacks
        case ACT_OMM_METAL_WATER_PUNCHING:                  return omm_act_metal_water_punching(m);
        case ACT_OMM_METAL_WATER_JUMP_KICK:                 return omm_act_metal_water_jump_kick(m);
        case ACT_OMM_METAL_WATER_DIVE:                      return omm_act_metal_water_dive(m);
        case ACT_OMM_METAL_WATER_GROUND_POUND:              return omm_act_metal_water_ground_pound(m);
        case ACT_OMM_METAL_WATER_GROUND_POUND_LAND:         return omm_act_metal_water_ground_pound_land(m);
        case ACT_OMM_METAL_WATER_GROUND_POUND_LAND_STOP:    return omm_act_metal_water_ground_pound_land_stop(m);
        case ACT_OMM_METAL_WATER_GROUND_POUND_JUMP:         return omm_act_metal_water_ground_pound_jump(m);
        case ACT_OMM_METAL_WATER_BACKWARD_GROUND_KB:        return omm_act_metal_water_backward_ground_kb(m);
        case ACT_OMM_METAL_WATER_BACKWARD_AIR_KB:           return omm_act_metal_water_backward_air_kb(m);

        // Cappy
        case ACT_OMM_METAL_WATER_CAPPY_THROW_GROUND:        return omm_act_metal_water_cappy_throw_ground(m);
        case ACT_OMM_METAL_WATER_CAPPY_THROW_AIRBORNE:      return omm_act_metal_water_cappy_throw_airborne(m);
        case ACT_OMM_METAL_WATER_CAPPY_BOUNCE:              return omm_act_metal_water_cappy_bounce(m);

        // Spin
        case ACT_OMM_METAL_WATER_SPIN_GROUND:               return omm_act_metal_water_spin_ground(m);
        case ACT_OMM_METAL_WATER_SPIN_AIR:                  return omm_act_metal_water_spin_air(m);
        case ACT_OMM_METAL_WATER_SPIN_JUMP:                 return omm_act_metal_water_spin_jump(m);
        case ACT_OMM_METAL_WATER_SPIN_POUND:                return omm_act_metal_water_spin_pound(m);
        case ACT_OMM_METAL_WATER_SPIN_POUND_LAND:           return omm_act_metal_water_spin_pound_land(m);
    }

    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}
