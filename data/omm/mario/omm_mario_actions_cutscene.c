#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// Lost Coins
//

typedef union { struct Object *obj; Vec3f pos; } OmmLostCoin;
static OmmLostCoin sOmmLostCoins[20];
static s32 sOmmLostCoinsCount = 0;
static u8 sOmmCurrStarBits = 0;

static void omm_lost_coins_spawn(struct MarioState *m) {
    sOmmLostCoinsCount = (s32) omm_clamp_s(m->numCoins, 0, 20);
    for (s32 i = 0; i != sOmmLostCoinsCount; ++i) {
        struct Object *coin   = spawn_object(m->marioObj, MODEL_YELLOW_COIN, bhvYellowCoin);
        coin->oHomeX          = m->pos[0];
        coin->oHomeY          = m->pos[1] + 80;
        coin->oHomeZ          = m->pos[2];
        coin->oFaceAnglePitch = 0;
        coin->oFaceAngleYaw   = 0;
        coin->oFaceAngleRoll  = 0;
        coin->oMoveAnglePitch = 0;
        coin->oMoveAngleYaw   = 0;
        coin->oMoveAngleRoll  = 0;
        coin->oVelY           = 52.f;
        coin->oForwardVel     = 8.f;
        sOmmLostCoins[i].obj  = coin;
    }
}

static void omm_lost_coins_update() {
    for (s32 i = 0; i != sOmmLostCoinsCount; ++i) {
        OmmLostCoin *coin = &sOmmLostCoins[i];
        f32 angle = 65536.f * (f32) i / (f32) sOmmLostCoinsCount;
        coin->obj->oVelX = coin->obj->oForwardVel * coss(angle);
        coin->obj->oVelY = omm_max_f(coin->obj->oVelY - 4.f, -72.f);
        coin->obj->oVelZ = coin->obj->oForwardVel * sins(angle);

        // Move coin
        u8 isUnderwater = (find_water_level(coin->obj->oPosX, coin->obj->oPosZ) > coin->obj->oPosY);
        u8 numSteps = (isUnderwater ? 5 : 8);
        for (s32 step = 0; step != numSteps; ++step) {
            f32 x = coin->obj->oPosX;
            f32 y = coin->obj->oPosY;
            f32 z = coin->obj->oPosZ;
            coin->obj->oPosX += (coin->obj->oVelX / 8);
            coin->obj->oPosY += (coin->obj->oVelY / 8);
            coin->obj->oPosZ += (coin->obj->oVelZ / 8);

            // Walls
            struct WallCollisionData hitbox;
            hitbox.x = coin->obj->oPosX;
            hitbox.y = coin->obj->oPosY;
            hitbox.z = coin->obj->oPosZ;
            hitbox.offsetY = 0;
            hitbox.radius = 40;
            if (find_wall_collisions(&hitbox) != 0) {
                coin->obj->oPosX = hitbox.x;
                coin->obj->oPosY = hitbox.y;
                coin->obj->oPosZ = hitbox.z;
            }

            // Floor
            struct Surface *floor = NULL;
            f32 floorY = find_floor(coin->obj->oPosX, coin->obj->oPosY, coin->obj->oPosZ, &floor);
            if (floor != NULL && coin->obj->oPosY <= floorY) {
                coin->obj->oPosY = floorY;
                coin->obj->oVelY *= -0.85f;
                coin->obj->oForwardVel *= 0.8f;
                if (floor->type == SURFACE_DEATH_PLANE) {
                    coin->obj->oNodeFlags |= GRAPH_RENDER_INVISIBLE;
                }
                break;
            } else if (floor == NULL) {
                coin->obj->oPosX = x;
                coin->obj->oPosY = y;
                coin->obj->oPosZ = z;
                coin->obj->oForwardVel = 0;
                break;
            }
        }
    }
}

static void omm_lost_coins_save() {
    sOmmCurrStarBits = omm_stars_get_bits();
    for (s32 i = 0; i != sOmmLostCoinsCount; ++i) {
        struct Object *coin = sOmmLostCoins[i].obj;
        f32 x = coin->oPosX;
        f32 y = coin->oPosY;
        f32 z = coin->oPosZ;
        sOmmLostCoins[i].pos[0] = x;
        sOmmLostCoins[i].pos[1] = y;
        sOmmLostCoins[i].pos[2] = z;
    }
}

static void omm_lost_coins_respawn() {
    if (OMM_STARS_NON_STOP) {
#if OMM_CODE_TIME_TRIALS
        if (!time_trials_enabled()) omm_stars_set_bits(sOmmCurrStarBits);
#else
        omm_stars_set_bits(sOmmCurrStarBits);
#endif
        struct MarioState *m = gMarioState;
        for (s32 i = 0; i != sOmmLostCoinsCount; ++i) {
            f32 x = sOmmLostCoins[i].pos[0];
            f32 y = sOmmLostCoins[i].pos[1];
            f32 z = sOmmLostCoins[i].pos[2];

            // Respawn coin only if not above OoB or death barrier
            struct Surface *floor = NULL;
            f32 floorHeight = find_floor(x, y, z, &floor);
            if (floor       == NULL ||
                floor->type == SURFACE_DEATH_PLANE ||
                floor->type == SURFACE_VERTICAL_WIND) {
                continue;
            }

            // Respawn coin
            struct Object *coin   = spawn_object(m->marioObj, MODEL_YELLOW_COIN, bhvYellowCoin);
            coin->respawnInfoType = OMM_OBJECT_LOST_COIN;
            coin->oPosX           = x;
            coin->oPosY           = floorHeight;
            coin->oPosZ           = z;
            coin->oHomeX          = x;
            coin->oHomeY          = floorHeight;
            coin->oHomeZ          = z;
            coin->oFaceAnglePitch = 0;
            coin->oFaceAngleYaw   = 0;
            coin->oFaceAngleRoll  = 0;
            coin->oMoveAnglePitch = 0;
            coin->oMoveAngleYaw   = 0;
            coin->oMoveAngleRoll  = 0;
        }
    }
    sOmmLostCoinsCount = 0;
    sOmmCurrStarBits = 0;
}

//
// Deaths
//

struct OmmActDeathAnim1Params {
    u32 animation;
    s32 accel;
    s16 anglePitch;
    s32 frameMin;
    s32 frameMax;
    f32 yPos;
};

struct OmmActDeathAnim2Params {
    u32 animation;
    s32 accel;
    s16 anglePitch;
    s32 frameMin;
    s32 frameMax;
    f32 yVelInit;
    f32 yVelDec;
    f32 yVelLimit;
};

static void omm_play_death_sound(struct MarioState *m, s32 type) {
    switch (type) {
        case 0: omm_sound_play(OMM_SOUND_EVENT_DEATH_MARIO, m->marioObj->oCameraToObject); break;
        case 1: omm_sound_play(OMM_SOUND_EVENT_DEATH_MARIO_WATER, m->marioObj->oCameraToObject); break;
        case 2: omm_sound_play(OMM_SOUND_EVENT_DEATH_MARIO_FALL, m->marioObj->oCameraToObject); break;
    }
}

#define marioGfx m->marioObj->header.gfx
#define marioAnm m->marioObj->header.gfx.mAnimInfo
static void omm_act_death_handler(struct MarioState *m, s32 type, bool lookAtCamera, u16 tAnim2, u16 tBowserFace, u16 tEnd,
                                  struct OmmActDeathAnim1Params anim1, struct OmmActDeathAnim2Params anim2) {
    static f32 sMarioPosY;
    static f32 sAnim2Vel;

    // Init
    // Background music is reduced
    // Odyssey Death sound is played
    // Mario looks at the camera
    // Spawn lost coins around Mario
    if (m->actionTimer == 0) {
        music_lower_volume(SEQ_PLAYER_LEVEL, 60, 40);
        omm_play_death_sound(m, type);
        if (lookAtCamera) {
            m->faceAngle[1] = m->area->camera->yaw;
            marioGfx.angle[1] = m->area->camera->yaw;
        }
        m->health = OMM_HEALTH_DEAD;
        omm_lost_coins_spawn(m);
        sAnim2Vel = anim2.yVelInit;
        sMarioPosY = anim1.yPos;
    }

    // Fix the camera during the animation
    omm_mario_lock_camera(m, false);

    // Mario progressively loses all of his coins
    omm_lost_coins_update();
    if (m->numCoins > 0) {
        m->numCoins--;
        gHudDisplay.coins = m->numCoins;
    }

    // Animations
    if (m->actionTimer < tAnim2) {
        if (marioAnm.animAccel != anim1.accel) marioAnm.animID = -1;
        set_mario_anim_with_accel(m, anim1.animation, anim1.accel);
        marioAnm.animFrame            = (s16) omm_clamp_s(marioAnm.animFrame,            anim1.frameMin,       anim1.frameMax      );
        marioAnm.animFrameAccelAssist = (s32) omm_clamp_s(marioAnm.animFrameAccelAssist, anim1.frameMin << 16, anim1.frameMax << 16);
        marioGfx.angle[0]             = anim1.anglePitch;
    } else {
        if (marioAnm.animAccel != anim2.accel) marioAnm.animID = -1;
        set_mario_anim_with_accel(m, anim2.animation, anim2.accel);
        marioAnm.animFrame            = (s16) omm_clamp_s(marioAnm.animFrame,            anim2.frameMin,       anim2.frameMax      );
        marioAnm.animFrameAccelAssist = (s32) omm_clamp_s(marioAnm.animFrameAccelAssist, anim2.frameMin << 16, anim2.frameMax << 16);
        marioGfx.angle[0]             = anim2.anglePitch;
        sMarioPosY += sAnim2Vel;
        sAnim2Vel = omm_max_f(sAnim2Vel + anim2.yVelDec, anim2.yVelLimit);
    }
    m->marioBodyState->eyeState = MARIO_EYES_DEAD;
    m->marioObj->oNodeFlags &= ~GRAPH_RENDER_INVISIBLE;
    marioGfx.angle[2] = 0;
    marioGfx.pos[1] = sMarioPosY;

    // Fade-out, Bowser face
    // 30 frames long, black screen until frame 150
    if (m->actionTimer == tBowserFace) {
        play_transition(WARP_TRANSITION_FADE_INTO_BOWSER, 30, 0x00, 0x00, 0x00);
    }

    // Send the signal to restart the level
    if (m->actionTimer == tEnd) {
#if OMM_CODE_TIME_TRIALS
        if (time_trials_enabled()) {
            omm_restart_level();
        } else {
            omm_lost_coins_save();
            omm_add_routine(OMM_ROUTINE_TYPE_LEVEL_ENTRY, omm_lost_coins_respawn);
            omm_restart_area();
        }
#else
        omm_lost_coins_save();
        omm_add_routine(OMM_ROUTINE_TYPE_LEVEL_ENTRY, omm_lost_coins_respawn);
        omm_restart_area();
#endif

        // Restore Mario's health
        if (OMM_MOVESET_ODYSSEY) {
            m->health = OMM_HEALTH_DEAD;
            omm_health_fully_heal_mario(m);
            omm_health_set(m, m->health);
        } else {
            m->health = 0x880;
        }
    }

    m->actionTimer++;
}

static s32 omm_act_shocked(struct MarioState *m) {
    action_condition(OMM_MOVESET_ODYSSEY && m->actionArg == 0 && ++m->actionState > 45, ACT_FREEFALL, 0, RETURN_CANCEL);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_disappeared(struct MarioState *m) {
    action_condition(m->prevAction == ACT_OMM_DEATH, ACT_OMM_DEATH_SQUISHED, 0, RETURN_CANCEL); // Squished
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_death(struct MarioState *m) {
    struct OmmActDeathAnim1Params anim1 = { MARIO_ANIM_AIR_FORWARD_KB,     0x10000, 0xE000, 0,  0,  m->pos[1] + 16.f };
    struct OmmActDeathAnim2Params anim2 = { MARIO_ANIM_BACKWARDS_WATER_KB, 0x10000, 0x0000, 0, 20, 40.f, -2.f, -40.f };
    omm_act_death_handler(m, 0, true, 24, 84, 150, anim1, anim2);
    return OMM_MARIO_ACTION_RESULT_BREAK;
}

static s32 omm_act_death_water(struct MarioState *m) {
    struct OmmActDeathAnim1Params anim1 = { MARIO_ANIM_DROWNING_PART2,     0x10000, 0x0000, 0,  0,   m->pos[1] + 16.f };
    struct OmmActDeathAnim2Params anim2 = { MARIO_ANIM_BACKWARDS_WATER_KB, 0x0C000, 0x0000, 0, 20, 20.f, -0.8f, -20.f };
    omm_act_death_handler(m, 1, true, 24, 84, 150, anim1, anim2);
    return OMM_MARIO_ACTION_RESULT_BREAK;
}

static s32 omm_act_death_fall(struct MarioState *m) {
    struct OmmActDeathAnim1Params anim1 = { MARIO_ANIM_BEING_GRABBED, 0x20000, 0x0000,                  0, 30, m->pos[1]              };
    struct OmmActDeathAnim2Params anim2 = { MARIO_ANIM_BEING_GRABBED, 0x20000, 0x0000, marioAnm.animFrame, 30, m->vel[1], -2.f, -90.f };
    omm_act_death_handler(m, 2, false, 0, 60, 120, anim1, anim2);
    return OMM_MARIO_ACTION_RESULT_BREAK;
}

static s32 omm_act_death_frozen(struct MarioState *m) {
    struct OmmActDeathAnim1Params anim1 = { 0,               0x00000,            0x0000,                  0,                  0,     m->pos[1] };
    struct OmmActDeathAnim2Params anim2 = { marioAnm.animID, 0x00001, marioGfx.angle[0], marioAnm.animFrame, marioAnm.animFrame, 0.f, 0.f, 0.f };
    omm_act_death_handler(m, 0, false, 0, 84, 150, anim1, anim2);
    return OMM_MARIO_ACTION_RESULT_BREAK;
}

static s32 omm_act_death_eaten_by_bubba(struct MarioState *m) {
    struct OmmActDeathAnim1Params anim1 = { 0,                          0x00000,            0x0000,                  0,                  0,     m->pos[1] };
    struct OmmActDeathAnim2Params anim2 = { marioAnm.animID, marioAnm.animAccel, marioGfx.angle[0], marioAnm.animFrame, marioAnm.animFrame, 0.f, 0.f, 0.f };
    omm_act_death_handler(m, 0, false, 0, 84, 150, anim1, anim2);
    m->marioObj->oNodeFlags |= GRAPH_RENDER_INVISIBLE;
    return OMM_MARIO_ACTION_RESULT_BREAK;
}

static s32 omm_act_death_quicksand(struct MarioState *m) {
    struct OmmActDeathAnim1Params anim1 = { 0,                             0x00000, 0x0000, 0,  0, m->pos[1] + 8.f };
    struct OmmActDeathAnim2Params anim2 = { MARIO_ANIM_DYING_IN_QUICKSAND, 0x10000, 0x0000, 0, 60, 0.f, -3.f, -3.f };
    omm_act_death_handler(m, 2, false, 0, 60, 120, anim1, anim2);
    if (m->actionTimer < 75) play_sound(SOUND_MOVING_QUICKSAND_DEATH, m->marioObj->oCameraToObject);
    return OMM_MARIO_ACTION_RESULT_BREAK;
}

// Pancake man
static s32 omm_act_death_squished(struct MarioState *m) {
    struct OmmActDeathAnim1Params anim1 = { 0,                 0x00000, 0x0000, 0, 0,     m->pos[1] };
    struct OmmActDeathAnim2Params anim2 = { MARIO_ANIM_A_POSE, 0x00001, 0x0000, 0, 0, 0.f, 0.f, 0.f };
    omm_act_death_handler(m, 0, false, 0, 84, 150, anim1, anim2);
    m->squishTimer = 0xFF;
    marioGfx.scale[0] = 2.00f;
    marioGfx.scale[1] = 0.05f;
    marioGfx.scale[2] = 2.00f;
    return OMM_MARIO_ACTION_RESULT_BREAK;
}

//
// Odyssey star dance
//

// Animation, Hands, Yaw, PosY
static s32 sOmmStarDanceAnimData[][4] = {
    { MARIO_ANIM_TRIPLE_JUMP_LAND, MARIO_HAND_FISTS, 0x0000, 0 }, // 00
    { MARIO_ANIM_TRIPLE_JUMP_LAND, MARIO_HAND_FISTS, 0x0000, 0 }, // 01
    { MARIO_ANIM_TRIPLE_JUMP_LAND, MARIO_HAND_FISTS, 0x0000, 0 }, // 02
    { MARIO_ANIM_TRIPLE_JUMP_LAND, MARIO_HAND_FISTS, 0x0080, 0 }, // 03
    { MARIO_ANIM_TRIPLE_JUMP_LAND, MARIO_HAND_FISTS, 0x0100, 0 }, // 04
    { MARIO_ANIM_TRIPLE_JUMP_LAND, MARIO_HAND_FISTS, 0x0200, 0 }, // 05
    { MARIO_ANIM_TRIPLE_JUMP_LAND, MARIO_HAND_FISTS, 0x0400, 0 }, // 06
    { MARIO_ANIM_TRIPLE_JUMP_LAND, MARIO_HAND_FISTS, 0x0800, 0 }, // 07
    { MARIO_ANIM_TWIRL, MARIO_HAND_FISTS, 0x1000, 0 }, // 08
    { MARIO_ANIM_TWIRL, MARIO_HAND_FISTS, 0x2000, 0 }, // 09
    { MARIO_ANIM_TWIRL, MARIO_HAND_FISTS, 0x4000, 0 }, // 10
    { MARIO_ANIM_TWIRL, MARIO_HAND_FISTS, 0x6000, 0 }, // 11
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0x8000, 0 },  // 12
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0xA000, 0 },  // 13
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0xC000, 0 },  // 14
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0xE000, 0 },  // 15
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0x0000, 0 }, // 16
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0x2000, 0 }, // 17
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0x4000, 0 }, // 18
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0x6000, 0 }, // 19
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0x8000, 0 }, // 20
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0xA000, 0 }, // 21
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0xC000, 0 }, // 22
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0xE000, 0 }, // 23
    { MARIO_ANIM_TWIRL_LAND, MARIO_HAND_FISTS, 0xF000, 0 }, // 24
    { MARIO_ANIM_TWIRL_LAND, MARIO_HAND_FISTS, 0xF800, 0 }, // 25
    { MARIO_ANIM_TWIRL_LAND, MARIO_HAND_FISTS, 0xFC00, 0 }, // 26
    { MARIO_ANIM_TWIRL_LAND, MARIO_HAND_FISTS, 0xFE00, 0 }, // 27
    { MARIO_ANIM_TWIRL_LAND, MARIO_HAND_FISTS, 0xFF00, 0 }, // 28
    { MARIO_ANIM_TWIRL_LAND, MARIO_HAND_FISTS, 0x0000, 0 }, // 29
    { MARIO_ANIM_TWIRL_LAND, MARIO_HAND_FISTS, 0x0000, 0 }, // 30
    { MARIO_ANIM_TWIRL_LAND, MARIO_HAND_FISTS, 0x0000, 0 }, // 31
    { MARIO_ANIM_SINGLE_JUMP, MARIO_HAND_FISTS, 0x0000, 16 }, // 32
    { MARIO_ANIM_SINGLE_JUMP, MARIO_HAND_FISTS, 0x0000, 24 }, // 33
    { MARIO_ANIM_SINGLE_JUMP, MARIO_HAND_FISTS, 0x0000, 28 }, // 34
    { MARIO_ANIM_SINGLE_JUMP, MARIO_HAND_FISTS, 0x0000, 30 }, // 35
    { MARIO_ANIM_SINGLE_JUMP, MARIO_HAND_PEACE_SIGN, 0x0000, 31 }, // 36
    { MARIO_ANIM_SINGLE_JUMP, MARIO_HAND_PEACE_SIGN, 0x0000, 32 }, // 37
    { MARIO_ANIM_SINGLE_JUMP, MARIO_HAND_PEACE_SIGN, 0x0000, 32 }, // 38
    { MARIO_ANIM_SINGLE_JUMP, MARIO_HAND_PEACE_SIGN, 0x0000, 32 }, // 39
};

static bool omm_update_star_dance(struct MarioState *m) {
    static struct Object *celebStar;

    // Enable time stop and spawn the celebration star
    if (m->actionTimer == 0) {
        m->riddenObj = (void *) m->interactObj->behavior; // Mario stops ridding when collecting a star, so we can safely use this slot to store the star behavior
        m->faceAngle[1] = m->area->camera->yaw;
        vec3s_set(m->marioObj->header.gfx.angle, 0, m->area->camera->yaw, 0);
        disable_background_sound();
        play_course_clear();
        f32 radius =  60.f * m->marioObj->oScaleX;
        f32 height = 160.f * m->marioObj->oScaleY;
        celebStar = omm_spawn_star_celebration(m->marioObj, radius, height);
        set_time_stop_flags(TIME_STOP_ENABLED | TIME_STOP_MARIO_AND_DOORS);
        m->marioObj->activeFlags |= ACTIVE_FLAG_INITIATED_TIME_STOP;
    }

    // Display the text box "You got a star"
    else if (m->actionTimer == 39) {
        omm_render_start_you_got_a_star(OMM_TEXT_YOU_GOT_A_STAR, omm_level_get_name(gCurrLevelNum, false, false), omm_level_get_act_name(gCurrLevelNum, gLastCompletedStarNum, false, false));
    }
    
    // Here we go!
    else if (m->actionTimer == 40) {
        play_sound(SOUND_MARIO_HERE_WE_GO, m->marioObj->oCameraToObject);
        set_camera_shake_from_hit(SHAKE_GROUND_POUND);
    }
    
    // Resume action
    else if (m->actionTimer == 80) {
        obj_mark_for_deletion(celebStar);
        m->marioObj->activeFlags &= ~ACTIVE_FLAG_INITIATED_TIME_STOP;
        clear_time_stop_flags(TIME_STOP_ENABLED | TIME_STOP_MARIO_AND_DOORS);
        enable_background_sound();
        omm_render_stop_you_got_a_star();
        omm_health_fully_heal_mario(m);
        m->healCounter = OMM_O2_REFILL;
        m->riddenObj = NULL;
        return true;
    }

    // Fix the camera during the animation
    omm_mario_lock_camera(m, true);

    // Animation
    s32 *animData = sOmmStarDanceAnimData[omm_min_s(m->actionTimer, 39)];
    omm_mario_set_animation(m, animData[0], 1.f, -1);
    if (m->actionTimer >= 40) set_anim_to_frame(m, 10); // Luigi's freaking flutter jump
    m->marioBodyState->handState = animData[1];
    m->marioObj->header.gfx.angle[1] = m->faceAngle[1] - (s16) animData[2];
    m->marioObj->header.gfx.pos[1] = m->pos[1] + animData[3];
    m->actionTimer++;
    return false;
}

//
// Star dances
//

static s32 omm_act_star_dance_exit(struct MarioState *m) {
    if (OMM_ALL_STARS) return OMM_MARIO_ACTION_RESULT_CONTINUE;
    u32 actionArg = (m->prevAction & ACT_FLAG_METAL_WATER) || ((m->prevAction & ACT_GROUP_MASK) == ACT_GROUP_SUBMERGED);
    action_condition(OMM_STARS_NON_STOP, ACT_OMM_STAR_DANCE, actionArg, RETURN_CANCEL);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_star_dance_no_exit(struct MarioState *m) {
    if (OMM_ALL_STARS) return OMM_MARIO_ACTION_RESULT_CONTINUE;
    u32 actionArg = (m->prevAction & ACT_FLAG_METAL_WATER) || ((m->prevAction & ACT_GROUP_MASK) == ACT_GROUP_SUBMERGED);
    action_condition(OMM_STARS_NON_STOP, ACT_OMM_STAR_DANCE, actionArg, RETURN_CANCEL);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_star_dance_water(struct MarioState *m) {
    if (OMM_ALL_STARS) return OMM_MARIO_ACTION_RESULT_CONTINUE;
    u32 actionArg = (m->prevAction & ACT_FLAG_METAL_WATER) || ((m->prevAction & ACT_GROUP_MASK) == ACT_GROUP_SUBMERGED);
    action_condition(OMM_STARS_NON_STOP, ACT_OMM_STAR_DANCE, actionArg, RETURN_CANCEL);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_fall_after_star_grab(struct MarioState *m) {
    if (OMM_ALL_STARS) return OMM_MARIO_ACTION_RESULT_CONTINUE;
    u32 actionArg = (m->prevAction & ACT_FLAG_METAL_WATER) || ((m->prevAction & ACT_GROUP_MASK) == ACT_GROUP_SUBMERGED);
    action_condition(OMM_STARS_NON_STOP, ACT_OMM_STAR_DANCE, actionArg, RETURN_CANCEL);
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_star_dance(struct MarioState *m) {
    if (omm_update_star_dance(m)) {

#if OMM_CODE_VANILLA
        // WF Tower transition
        if (gCurrCourseNum == COURSE_WF && gLastCompletedStarNum == 1) {
            omm_mario_set_action(m, ACT_OMM_TRANSITION_WF_TOWER, 0, 0);
            return OMM_MARIO_ACTION_RESULT_CANCEL;
        }
#endif

        // Resume action
        mario_set_forward_vel(m, 0.f);
        omm_mario_set_action(m, m->actionArg ? ACT_WATER_IDLE : ACT_FREEFALL, 0, 0);
        return OMM_MARIO_ACTION_RESULT_CANCEL;
    }
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

static s32 omm_act_transition_wf_tower(struct MarioState *m) {
    switch (m->actionTimer) {
        case 15:
            play_transition(WARP_TRANSITION_FADE_INTO_COLOR, 30, 0xFF, 0xFF, 0xFF);
            break;

        case 45:
            play_sound(SOUND_MENU_STAR_SOUND, gGlobalSoundArgs);
            struct Object *whompKingBoss = obj_get_first_with_behavior(bhvWhompKingBoss);
            if (whompKingBoss) {
                stop_background_music(SEQ_EVENT_BOSS);
                obj_mark_for_deletion(whompKingBoss);
            }
            break;

        case 75:
            m->pos[0] = +840;
            m->pos[1] = +3584;
            m->pos[2] = +1230;
            m->intendedYaw = 0xA000;
            m->faceAngle[1] = 0xA000;
            m->marioObj->oFaceAngleYaw = 0xA000;
            m->marioObj->header.gfx.angle[1] = 0xA000;
            mario_set_forward_vel(m, 0.f);
            omm_mario_set_action(m, ACT_FREEFALL, 0, 0xFFFF);
            play_transition(WARP_TRANSITION_FADE_FROM_COLOR, 30, 0xFF, 0xFF, 0xFF);
            break;
    }
    m->actionTimer++;
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

//
// Exits
//

static bool omm_mario_launch_until_land(struct MarioState *m, s32 endAction, s32 animation, f32 forwardVel, u16 delay) {
    if (m->actionTimer++ >= delay) {
        m->marioObj->oNodeFlags |= GRAPH_RENDER_ACTIVE;
        mario_set_forward_vel(m, forwardVel);
        set_mario_animation(m, animation);
        if (perform_air_step(m, 0) == AIR_STEP_LANDED) {
            omm_mario_set_action(m, endAction, 0, 0);
            return true;
        }
        return false;
    }
    m->marioObj->oNodeFlags &= ~GRAPH_RENDER_ACTIVE;
    return false;
}

static s32 omm_act_exit_airborne(struct MarioState *m) {
    if (omm_mario_launch_until_land(m, ACT_EXIT_LAND_SAVE_DIALOG, MARIO_ANIM_GENERAL_FALL, -32.f, 15)) {
        omm_health_fully_heal_mario(m);
        m->healCounter = 31;
    }
    m->marioObj->header.gfx.angle[1] += 0x8000;
    m->particleFlags |= PARTICLE_SPARKLES;
    return OMM_MARIO_ACTION_RESULT_BREAK;
}

static s32 omm_act_falling_exit_airborne(struct MarioState *m) {
    if (omm_mario_launch_until_land(m, ACT_EXIT_LAND_SAVE_DIALOG, MARIO_ANIM_GENERAL_FALL, 0.f, 0)) {
        omm_health_fully_heal_mario(m);
        m->healCounter = 31;
    }
    m->marioObj->header.gfx.angle[1] += 0x8000;
    m->particleFlags |= PARTICLE_SPARKLES;
    return OMM_MARIO_ACTION_RESULT_BREAK;
}

static s32 omm_act_special_exit_airborne(struct MarioState *m) {
    play_sound_if_no_flag(m, SOUND_MARIO_YAHOO, MARIO_MARIO_SOUND_PLAYED);
    if (omm_mario_launch_until_land(m, ACT_EXIT_LAND_SAVE_DIALOG, MARIO_ANIM_SINGLE_JUMP, -24.f, 11)) {
        omm_health_fully_heal_mario(m);
        m->healCounter = 31;
        m->actionArg = 1;
    }
    m->marioObj->header.gfx.angle[1] += 0x8000;
    m->particleFlags |= PARTICLE_SPARKLES;
    return OMM_MARIO_ACTION_RESULT_BREAK;
}

static s32 omm_act_death_exit(struct MarioState *m) {
    if (omm_mario_launch_until_land(m, ACT_DEATH_EXIT_LAND, MARIO_ANIM_GENERAL_FALL, -32.f, 15)) {
        play_sound(SOUND_MARIO_OOOF2, m->marioObj->oCameraToObject);
        omm_health_fully_heal_mario(m);
        m->healCounter = 31;
    } else {
        omm_health_set(m, OMM_HEALTH_DEAD + 1);
    }
    return OMM_MARIO_ACTION_RESULT_BREAK;
}

static s32 omm_act_unused_death_exit(struct MarioState *m) {
    if (omm_mario_launch_until_land(m, ACT_FREEFALL_LAND_STOP, MARIO_ANIM_GENERAL_FALL, 0.0f, 0)) {
        play_sound(SOUND_MARIO_OOOF2, m->marioObj->oCameraToObject);
        omm_health_fully_heal_mario(m);
        m->healCounter = 31;
    } else {
        omm_health_set(m, OMM_HEALTH_DEAD + 1);
    }
    return OMM_MARIO_ACTION_RESULT_BREAK;
}

static s32 omm_act_falling_death_exit(struct MarioState *m) {
    if (omm_mario_launch_until_land(m, ACT_DEATH_EXIT_LAND, MARIO_ANIM_GENERAL_FALL, 0.0f, 0)) {
        play_sound(SOUND_MARIO_OOOF2, m->marioObj->oCameraToObject);
        omm_health_fully_heal_mario(m);
        m->healCounter = 31;
    } else {
        omm_health_set(m, OMM_HEALTH_DEAD + 1);
    }
    return OMM_MARIO_ACTION_RESULT_BREAK;
}

static s32 omm_act_special_death_exit(struct MarioState *m) {
    if (omm_mario_launch_until_land(m, ACT_HARD_BACKWARD_GROUND_KB, MARIO_ANIM_BACKWARD_AIR_KB, -24.0f, 11)) {
        omm_health_fully_heal_mario(m);
        m->healCounter = 31;
    } else {
        omm_health_set(m, OMM_HEALTH_DEAD + 1);
    }
    return OMM_MARIO_ACTION_RESULT_BREAK;
}

//
// Other
//

static s32 omm_act_warping(struct MarioState *m) {
    m->marioObj->oNodeFlags |= GRAPH_RENDER_INVISIBLE;
    return OMM_MARIO_ACTION_RESULT_BREAK;
}

//
// Cutscene
//

s32 omm_mario_execute_cutscene_action(struct MarioState *m) {
    gOmmData->mario->wallSlide.jumped = false;
    gOmmData->mario->peach.floated = false;
    gOmmData->mario->cappy.bounced = false;
    gOmmData->mario->state.airCombo = 0;
    gOmmData->mario->midairSpin.counter = 0;

    // Actions
    switch (m->action) {
        case ACT_SHOCKED:                   return omm_act_shocked(m);
        case ACT_DISAPPEARED:               return omm_act_disappeared(m);
        case ACT_STAR_DANCE_EXIT:           return omm_act_star_dance_exit(m);
        case ACT_STAR_DANCE_NO_EXIT:        return omm_act_star_dance_no_exit(m);
        case ACT_STAR_DANCE_WATER:          return omm_act_star_dance_water(m);
        case ACT_FALL_AFTER_STAR_GRAB:      return omm_act_fall_after_star_grab(m);
        case ACT_EXIT_AIRBORNE:             return omm_act_exit_airborne(m);
        case ACT_FALLING_EXIT_AIRBORNE:     return omm_act_falling_exit_airborne(m);
        case ACT_SPECIAL_EXIT_AIRBORNE:     return omm_act_special_exit_airborne(m);
        case ACT_DEATH_EXIT:                return omm_act_death_exit(m);
        case ACT_UNUSED_DEATH_EXIT:         return omm_act_unused_death_exit(m);
        case ACT_FALLING_DEATH_EXIT:        return omm_act_falling_death_exit(m);
        case ACT_SPECIAL_DEATH_EXIT:        return omm_act_special_death_exit(m);
        case ACT_END_PEACH_CUTSCENE:        return omm_sparkly_act_end_cutscene_1(m);
        case ACT_END_WAVING_CUTSCENE:       return omm_sparkly_act_end_cutscene_2(m);

        case ACT_OMM_POSSESSION:            return omm_act_possession(m);
        case ACT_OMM_DEATH:                 return omm_act_death(m);
        case ACT_OMM_DEATH_WATER:           return omm_act_death_water(m);
        case ACT_OMM_DEATH_FALL:            return omm_act_death_fall(m);
        case ACT_OMM_DEATH_FROZEN:          return omm_act_death_frozen(m);
        case ACT_OMM_DEATH_EATEN_BY_BUBBA:  return omm_act_death_eaten_by_bubba(m);
        case ACT_OMM_DEATH_QUICKSAND:       return omm_act_death_quicksand(m);
        case ACT_OMM_DEATH_SQUISHED:        return omm_act_death_squished(m);
        case ACT_OMM_STAR_DANCE:            return omm_act_star_dance(m);
        case ACT_OMM_SPARKLY_STAR_DANCE:    return omm_sparkly_act_star_dance(m);
        case ACT_OMM_TRANSITION_WF_TOWER:   return omm_act_transition_wf_tower(m);
        case ACT_OMM_WARPING:               return omm_act_warping(m);
    }

    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}
