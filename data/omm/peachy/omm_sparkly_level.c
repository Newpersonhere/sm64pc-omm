#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS
extern struct Object **sOmmSparklyStar;

#pragma GCC push_options
#pragma GCC diagnostic ignored "-Wunused-parameter"

//
// Timer
//

static void omm_sparkly_level_update_timer(struct MarioState *m, const OmmSparklyStarsData *data, bool run) {

    // PSS / Koopa the Quick timer
    if (data->timer < 0) {
        if ((gHudDisplay.flags & HUD_DISPLAY_FLAG_TIMER) && gHudDisplay.timer >= -data->timer) {
            omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_FAIL, 1);
        }
        return;
    }

    // Countdown
    if (data->timer > 0) {
        if (omm_sparkly_context_is_state_ok()) {

            // Start the timer...
            if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED)) {
                if (run) {
                    s32 cap = 0;
                    switch (data->flags & OMM_SPARKLY_DATA_FLAG_CAPS) {
                        case OMM_SPARKLY_DATA_FLAG_WING_CAP: cap = MARIO_WING_CAP; audio_play_wing_cap_music(); break;
                        case OMM_SPARKLY_DATA_FLAG_METAL_CAP: cap = MARIO_METAL_CAP; audio_play_metal_cap_music(); break;
                        case OMM_SPARKLY_DATA_FLAG_VANISH_CAP: cap = MARIO_VANISH_CAP; audio_play_vanish_cap_music(); break;
                    }
                    if (cap) {
                        m->flags &= ~(MARIO_CAP_IN_HAND | MARIO_WING_CAP | MARIO_METAL_CAP | MARIO_VANISH_CAP);
                        m->flags |= (MARIO_CAP_ON_HEAD | cap);
                        m->capTimer = data->timer;
                        m->marioObj->oHomeX = m->pos[0];
                        m->marioObj->oHomeY = m->pos[1];
                        m->marioObj->oHomeZ = m->pos[2];
                    }
                    gHudDisplay.timer = data->timer;
                    gHudDisplay.flags |= HUD_DISPLAY_FLAG_TIMER;
                    omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED, 1, 0);
                }
                return;
            }

            // ...and grab the star before it runs out
            if (omm_sparkly_flags_get(OMM_SPARKLY_FLAG_MARIO_UPDATED) &&
                !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_GAME_PAUSED) &&
                !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_TRANSITION) &&
                !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_TIME_STOP) &&
                !omm_mario_is_reading(m) && (
                !omm_sparkly_is_mode_selected(OMM_SPARKLY_MODE_NORMAL) ||
                !omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_STAR_SPAWNED))) {
                if (gHudDisplay.timer > 0) {
                    gHudDisplay.timer -= run;
                    gHudDisplay.flags |= HUD_DISPLAY_FLAG_TIMER;
                } else {
                    omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_FAIL, 1);
                }
            }
        } else {
            gHudDisplay.flags &= ~HUD_DISPLAY_FLAG_TIMER;
        }
    }
}

//
// Per level
//

static void common_lll(struct MarioState *m, const OmmSparklyStarsData *data, f32 lavaSpeed, const f32 *shardsPos) {
    s32 starShards = omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_RED_COINS);
    struct Object *lava = obj_get_first_with_behavior(omm_bhv_rising_lava);

    // Spawn lava object and star shards
    if (lava == NULL) {
        lava = omm_spawn_rising_lava(m->marioObj, 0, 0, 0, 0, 3570, lavaSpeed, 4096, 16, 10);
        obj_set_dormant(lava, true);
        for (s32 i = 0; i != 8; ++i) {
            struct Object *starShard = spawn_object_abs_with_rot(m->marioObj, 0,
                MODEL_SPARKLES, bhvRedCoin,
                shardsPos[3 * i + 0],
                shardsPos[3 * i + 1],
                shardsPos[3 * i + 2],
                0, 0, 0
            );
            starShard->parentObj = NULL;
            obj_set_dormant(starShard, true);
        }
        return;
    }

    // Update rising lava
    if (!obj_is_dormant(lava)) {
        if (omm_sparkly_context_is_state_ok() && starShards > 0 && omm_sparkly_context_dif_counter(OMM_SPARKLY_CONTEXT_COUNTER_RED_COINS) > 0) {
            lava->oAction = 1;
            spawn_object_relative(starShards, 0, 0, 0, m->marioObj, MODEL_NUMBER, bhvOrangeNumber)->oPosY += 80;
            play_sound(SOUND_MENU_COLLECT_RED_COIN + (((u8) (starShards - 1)) << 16), gGlobalSoundArgs);
        } else if (m->prevAction == ACT_OMM_SPARKLY_STAR_DANCE) {
            lava->oAction = 2;
        }
    }
}

static void common_sl(struct MarioState *m, const OmmSparklyStarsData *data, s32 numSnowmen) {
    if (omm_sparkly_context_is_state_ok()) {
        s32 numSnowmenAtTop = 0;
        for_each_object_with_behavior(mrBlizzard, bhvMrBlizzard) {
            f32 dx = mrBlizzard->oPosX - 0;
            f32 dy = mrBlizzard->oPosY - 4860;
            f32 dz = mrBlizzard->oPosZ - 0;
            f32 dr = sqrtf(omm_sqr_f(dx) + omm_sqr_f(dz));
            numSnowmenAtTop += (dy > 0.f && dr < 512);
        }
        omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, numSnowmenAtTop >= numSnowmen, 1);
    }
}

static void common_mushrooms(struct MarioState *m, const OmmSparklyStarsData *data, s32 numMushrooms, bool valid) {
    if (omm_sparkly_context_is_state_ok() && omm_sparkly_context_dif_counter(OMM_SPARKLY_CONTEXT_COUNTER_1UP_MUSHROOMS) > 0) {
        if (valid) {
            s32 mushrooms = omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_1UP_MUSHROOMS);
            spawn_object_relative(mushrooms, 0, 0, 0, m->marioObj, MODEL_NUMBER, bhvOrangeNumber)->oPosY += 80;
            play_sound(SOUND_GENERAL_COLLECT_1UP, gGlobalSoundArgs);
            omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, mushrooms == numMushrooms, 0);
        } else {
            omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_FAIL, 1);
        }
    }
}

static void normal_castle(struct MarioState *m, const OmmSparklyStarsData *data, const f32 *shardsPos) {
    if (omm_sparkly_context_is_state_ok()) {

        // Spawn shards
        if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_OBJECTS_SPAWNED)) {
            for (s32 i = 0; i != 8; ++i) {
                spawn_object_abs_with_rot(m->marioObj, 0,
                    MODEL_SPARKLES, bhvRedCoin,
                    shardsPos[3 * i + 0],
                    shardsPos[3 * i + 1],
                    shardsPos[3 * i + 2],
                    0, 0, 0
                )->parentObj = NULL;
            }
            omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_OBJECTS_SPAWNED, 1, 0);
            return;
        }

        // A shard was collected
        s32 starShards = omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_RED_COINS);
        if (starShards > 0 && omm_sparkly_context_dif_counter(OMM_SPARKLY_CONTEXT_COUNTER_RED_COINS) > 0) {
            spawn_object_relative(starShards, 0, 0, 0, m->marioObj, MODEL_NUMBER, bhvOrangeNumber)->oPosY += 80;
            play_sound(SOUND_MENU_COLLECT_RED_COIN + (((u8) (starShards - 1)) << 16), gGlobalSoundArgs);
            omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, starShards == 8, 0);
        }
    }
}

static void hard_castle(struct MarioState *m, const OmmSparklyStarsData *data, const s32 *toadsParam) {
    static const BehaviorScript bhvToadMessage2[] = {
        0x00040000,
        0x11014049,
        0x27260000, (uintptr_t) toad_seg6_anims_0600FB58,
        0x28060000,
        0x2F000000, (uintptr_t) INTERACT_TEXT,
        0x10050000,
        0x0C000000, (uintptr_t) bhv_toad_message_init,
        0x08000000,
        0x0C000000, (uintptr_t) bhv_toad_message_loop,
        0x09000000
    };

    // Spawn toads
    if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_OBJECTS_SPAWNED)) {
        obj_deactivate_all_with_behavior(bhvToadMessage);
        for (s32 i = 0; i != 2; ++i) {
            struct Object *toad = spawn_object(m->marioObj, MODEL_TOAD, bhvToadMessage2);
            toad->behavior = bhvToadMessage;
            toad->oBehParams = toadsParam[5 * i + 4] << 24;
            obj_set_pos(toad, toadsParam[5 * i + 0], toadsParam[5 * i + 1], toadsParam[5 * i + 2]);
            obj_set_home(toad, toadsParam[5 * i + 0], toadsParam[5 * i + 1], toadsParam[5 * i + 2]);
            obj_set_angle(toad, 0, toadsParam[5 * i + 3], 0);
            obj_reset_hitbox(toad, 20, 100, 0, 0, 50, 0);
        }
        omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_OBJECTS_SPAWNED, 1, 0);
        return;
    }
}

//
// Normal
//

static bool normal_bob(struct MarioState *m, const OmmSparklyStarsData *data) {
    omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_SECRETS) == 0, 0);
    return true;
}

static bool normal_jrb(struct MarioState *m, const OmmSparklyStarsData *data) {
    if (!omm_world_is_flooded()) {
        omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 0);
    }
    return true;
}

static bool normal_lll(struct MarioState *m, const OmmSparklyStarsData *data) {
    common_lll(m, data, 2.f, OMM_ARRAY_OF(f32) {
        -1700,   60,  -440,
         2550,  630,  1120,
          250, 1210,  2640,
        -2080, 1915,  -300,
        -1600, 2440, -2150,
         1300, 3705, -2500,
          420, 2365,   575,
          662, 3300,   708,
    });
    return true;
}

static bool normal_sl(struct MarioState *m, const OmmSparklyStarsData *data) {
    common_sl(m, data, 1);
    return true;
}

static bool normal_thi(struct MarioState *m, const OmmSparklyStarsData *data) {
    if (omm_sparkly_context_is_state_ok() &&
        m->action == ACT_OMM_POSSESSION &&
        gOmmData->mario->capture.obj->behavior == bhvGoomba &&
        gOmmData->object->goomba.stackCount >= 9) {
        omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, 1, 0);
    }
    return true;
}

static bool normal_ttc(struct MarioState *m, const OmmSparklyStarsData *data) {
    s32 boxes = omm_sparkly_context_get_remaining_boxes();
    omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, boxes == 0, 0);
    return (boxes > 0 && boxes < 13);
}

static bool normal_bitdw(struct MarioState *m, const OmmSparklyStarsData *data) {

    // Verifying conditions...
    // Mario must be possessing a goomba or standing inside the safe zone without touching a coin
    if (m->action != ACT_OMM_POSSESSION || gOmmData->mario->capture.obj->behavior != bhvGoomba) {
        if ((m->pos[0] < -7925) || (m->pos[0] > -6975) ||
            (m->pos[2] < +3350) || (m->pos[2] > +4425) ||
            (m->numCoins != 0 && !omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_STAR_SPAWNED))) {
            omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_FAIL, 1);
        }
    }

    // Spawns a dormant goomba at the center of the safe zone
    static struct Object *sBitdwGoomba = NULL;
    if (!obj_get_first_with_behavior(omm_bhv_sparkly_star_hint) && omm_sparkly_context_is_state_ok()) {
        f32 goombaX = (-7925 + -6975) / 2;
        f32 goombaZ = (+3350 + +4425) / 2;
        sBitdwGoomba = spawn_object_abs_with_rot(m->marioObj, 0, MODEL_GOOMBA, bhvGoomba, goombaX, +20000.f, goombaZ, 0, 0, 0);
        sBitdwGoomba->oBehParams = GOOMBA_SIZE_REGULAR << 16;
        sBitdwGoomba->oBehParams2ndByte = GOOMBA_SIZE_REGULAR;
        sBitdwGoomba->parentObj = sBitdwGoomba;
        sBitdwGoomba->prevObj = NULL;
        obj_drop_to_floor(sBitdwGoomba);
        obj_set_dormant(sBitdwGoomba, true);
    }

    // Possess the goomba as soon as it awakes
    if (omm_sparkly_context_is_state_ok() && m->action != ACT_READING_SIGN) {
        struct Object *goomba = obj_get_nearest_with_behavior(m->marioObj, bhvGoomba);
        if (goomba != NULL && goomba == sBitdwGoomba && !obj_is_dormant(goomba)) {
            if (omm_mario_possess_object(m, goomba, false)) {
                sBitdwGoomba = NULL;
            }
        }
    }
    return true;
}

static bool normal_bitfs(struct MarioState *m, const OmmSparklyStarsData *data) {
    common_mushrooms(m, data, 7, m->action == ACT_OMM_POSSESSION);
    return true;
}

static bool normal_vcutm(struct MarioState *m, const OmmSparklyStarsData *data) {
    static const Vec2f sVcutmZones[] = {
        { -6500, -6500 },
        { -6500, +6500 },
        { +4865, +6500 },
        { +4865, -6500 },
        { -2400, -6500 }
    };
    bool newZone = false;
    s32 numZones = 0;

    // Zones
    for (s32 i = 0; i != 5; ++i) {
        if (omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_ZONES << i)) {
            numZones++;
        } else {
            f32 x = sVcutmZones[i][0];
            f32 z = sVcutmZones[i][1];
            f32 d = sqrtf(omm_sqr_f(m->pos[0] - x) + omm_sqr_f(m->pos[2] - z));
            if (d < 300) {
                omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_ZONES << i, 1, 0);
                newZone = true;
                numZones++;
            }
        }
    }
    omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, numZones == 5, 0);

    // Spawn secret effect if Mario enters a new zone
    if (newZone && omm_sparkly_context_is_state_ok()) {
        spawn_object_relative(numZones, 0, 0, 0, m->marioObj, MODEL_NUMBER, bhvOrangeNumber)->oPosY += 80;
        play_sound(SOUND_MENU_COLLECT_SECRET + (((u8) numZones - 1) << 16), gGlobalSoundArgs);
    }
    return true;
}

static bool normal_cotmc(struct MarioState *m, const OmmSparklyStarsData *data) {
    bool onGround = !(m->action & ACT_FLAG_AIR);

    // When warp-landing, trigger the dialog box...
    if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_ENTRY_DIALOG)) {
        return false;
    }

    // ...then, enter the starting action...
    if (m->prevAction == ACT_READING_AUTOMATIC_DIALOG) {
        omm_mario_set_action(m, ACT_LONG_JUMP, 0, 0xFFFF);
        mario_set_forward_vel(m, 48.f);
        return true;
    }

    // ...and never touch the floor again
    if (onGround && ((m->action & ACT_GROUP_MASK) != ACT_GROUP_CUTSCENE)) {
        omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_FAIL, 1);
    }
    return true;
}

static bool normal_sa(struct MarioState *m, const OmmSparklyStarsData *data) {
    if (!omm_world_is_frozen()) {
        omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 0);
    }
    return true;
}

static bool normal_inside(struct MarioState *m, const OmmSparklyStarsData *data) {
    normal_castle(m, data, OMM_ARRAY_OF(f32) {
        -1024,  400,  -250,
        -1024,  800,  1400,
        -4500, 1200,  -450,
        -2280, -400, -3040,
         -128, -300, -2640,
         2000,  900, -1700,
         1910,  850, -2120,
        -5500,  600, -4300
    });
    return true;
}

static bool normal_basement(struct MarioState *m, const OmmSparklyStarsData *data) {
    if (m->action != ACT_OMM_POSSESSION && omm_sparkly_context_is_state_ok()) {
        struct Object *toad1 = obj_get_first_with_behavior(bhvToadMessage);
        struct Object *toad2 = obj_get_next_with_behavior(toad1, bhvToadMessage);
        if (toad1 && toad2) {
            struct Object *toad1Pillar = obj_get_nearest_with_behavior(toad1, bhvWaterLevelPillar);
            struct Object *toad2Pillar = obj_get_nearest_with_behavior(toad2, bhvWaterLevelPillar);
            if (toad1Pillar && toad2Pillar && (toad1Pillar != toad2Pillar)) {
                f32 heightToad1 = toad1->oPosY - toad1Pillar->oPosY;
                f32 heightToad2 = toad2->oPosY - toad2Pillar->oPosY;
                if (omm_abs_f(heightToad1) < 20.f && omm_abs_f(heightToad2) < 20.f) {
                    f32 distToad1 = obj_get_horizontal_distance(toad1, toad1Pillar);
                    f32 distToad2 = obj_get_horizontal_distance(toad2, toad2Pillar);
                    omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, distToad1 < 200 && distToad2 < 200, 0);
                }
            }
        }
    }
    return true;
}

static bool normal_upstairs(struct MarioState *m, const OmmSparklyStarsData *data) {
    normal_castle(m, data, OMM_ARRAY_OF(f32) {
        -1000, 1210,  1480,
        -4780, 3200, -3240,
         3538, 1680,  3880,
         -610, 1900,  2500,
         -200, 2800,  5000,
        -3400, 3600,  5886,
         3000, 2900,  5886,
         -200, 5400, -3700
    });
    return true;
}

static bool normal_courtyard(struct MarioState *m, const OmmSparklyStarsData *data) {

    // Spawn secrets in order
    if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_OBJECTS_SPAWNED)) {
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvHiddenStarTrigger,  1500,    0, -1740, 0, 0, 0)->oBehParams = 1;
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvHiddenStarTrigger,  -820,   20, -3630, 0, 0, 0)->oBehParams = 2;
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvHiddenStarTrigger,   820,   20, -3630, 0, 0, 0)->oBehParams = 3;
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvHiddenStarTrigger,  -820,   20,   200, 0, 0, 0)->oBehParams = 4;
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvHiddenStarTrigger,   820,   20,   200, 0, 0, 0)->oBehParams = 5;
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvHiddenStarTrigger, -1500,    0, -1740, 0, 0, 0)->oBehParams = 6;
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvHiddenStarTrigger,  2040, -200,  -480, 0, 0, 0)->oBehParams = 7;
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvHiddenStarTrigger, -2240, -200, -2815, 0, 0, 0)->oBehParams = 8;
        omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_OBJECTS_SPAWNED, 1, 0);
        return true;
    }
    
    // A secret was collected
    if (omm_sparkly_context_is_state_ok() && omm_sparkly_context_dif_counter(OMM_SPARKLY_CONTEXT_COUNTER_SECRETS) < 0) {
        s32 expectedIndex = 8 - omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_SECRETS);
        for_each_object_with_behavior(trigger, bhvHiddenStarTrigger) {
            if (trigger->oBehParams == expectedIndex) {
                expectedIndex = -1;
                omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_FAIL, 1);
                break;
            }
        }
        if (expectedIndex != -1) {
            spawn_object_relative(expectedIndex, 0, 0, 0, m->marioObj, MODEL_NUMBER, bhvOrangeNumber)->oPosY += 80;
            play_sound(SOUND_MENU_COLLECT_SECRET + (((u8) expectedIndex - 1) << 16), gGlobalSoundArgs);
        }
    }
    omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_SECRETS) == 0, 0);
    return true;
}

//
// Hard
//

static bool hard_bob(struct MarioState *m, const OmmSparklyStarsData *data) {
    return normal_bob(m, data);
}

static bool hard_jrb(struct MarioState *m, const OmmSparklyStarsData *data) {
    return normal_jrb(m, data);
}

static bool hard_hmc(struct MarioState *m, const OmmSparklyStarsData *data) {
    static struct { struct Object *obj; Vec3f pos; } sFlames[64];
    static s32 sNumFlames;

    // Make the flames bigger
    if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_OBJECTS_SPAWNED)) {
        sNumFlames = 0;
        for_each_object_with_behavior(flame, bhvFlame) {
            flame->oScaleX *= 3.f;
            flame->oScaleY *= 3.f;
            flame->oScaleZ *= 3.f;
            flame->hitboxRadius *= 3.f;
            flame->hitboxHeight *= 12.f;
            flame->hurtboxRadius *= 3.f;
            flame->hurtboxHeight *= 12.f;
            flame->hitboxDownOffset *= 6.f;
            sFlames[sNumFlames].obj = flame;
            sFlames[sNumFlames].pos[0] = flame->oPosX;
            sFlames[sNumFlames].pos[1] = flame->oPosY;
            sFlames[sNumFlames].pos[2] = flame->oPosZ;
            sNumFlames++;
        }
        omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_OBJECTS_SPAWNED, 1, 0);
        return true;
    }

    // Extinguish a flame by touching it
    if (omm_sparkly_context_is_state_ok()) {

        // Unload the flame that burned Mario
        struct Object *o = m->interactObj;
        if (m->marioObj->oMarioBurnTimer != 0 && o != NULL && o->behavior == bhvFlame) {
            obj_spawn_white_puff(o, SOUND_GENERAL_FLAME_OUT);
            obj_mark_for_deletion(o);
            m->interactObj = NULL;
        }

        // Check if some flame(s) has(ve) been unloaded recently
        for (s32 i = 0; i != sNumFlames; ++i) {
            if (sFlames[i].obj != NULL && (!sFlames[i].obj->activeFlags || sFlames[i].obj->behavior != bhvFlame)) {
                static const BehaviorScript bhvFlame2[] = { 0x00040000, 0x04000000, (uintptr_t) bhvFlame };
                play_sound(SOUND_GENERAL2_RIGHT_ANSWER, gGlobalSoundArgs);
                o = spawn_object(m->marioObj, MODEL_BLUE_FLAME, bhvFlame2);
                o->oPosX = sFlames[i].pos[0];
                o->oPosY = sFlames[i].pos[1];
                o->oPosZ = sFlames[i].pos[2];
                sFlames[i].obj = NULL;
            }
        }
        omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, omm_sparkly_context_get_remaining_flames() == 0, 0);
    }
    return true;
}

static bool hard_lll(struct MarioState *m, const OmmSparklyStarsData *data) {
    common_lll(m, data, 3.f, OMM_ARRAY_OF(f32) {
        -1700,   60,  -440,
          180, 1050, -1360,
         2550,  630,  1120,
         -480, 1210,  2630,
          705, 2160,   330,
            0, 4690, -2800,
          662, 3900,   708,
         2800, 3000,   250
    });
    return true;
}

static bool hard_ssl(struct MarioState *m, const OmmSparklyStarsData *data) {
    if (omm_sparkly_context_is_state_ok()) {
        s32 difY = omm_sparkly_context_dif_counter(OMM_SPARKLY_CONTEXT_COUNTER_YELLOW_COINS);
        s32 difB = omm_sparkly_context_dif_counter(OMM_SPARKLY_CONTEXT_COUNTER_BLUE_COINS);
        s32 difR = omm_sparkly_context_dif_counter(OMM_SPARKLY_CONTEXT_COUNTER_RED_COINS);
        s32 difC = omm_max_s(difY, 0) * 1 + omm_max_s(difR, 0) * 2 + omm_max_s(difB, 0) * 5;
        gHudDisplay.timer += difC * 45;
    }
    return true;
}

static bool hard_sl(struct MarioState *m, const OmmSparklyStarsData *data) {
    common_sl(m, data, 4);
    return true;
}

static bool hard_wdw(struct MarioState *m, const OmmSparklyStarsData *data) {
    if (omm_sparkly_context_is_state_ok()) {

        // Make the Chuckya follow Mario after some point
        for_each_object_with_behavior(obj, bhvChuckya) {
            if (gCurrAreaIndex == 2 || obj->oPosY < 500.f) {
                obj_set_home(obj, obj->oPosX, obj->oPosY, obj->oPosZ);
            }
        }

        // Update the Chuckya so Mario never ends up with an unloaded object in his hands
        s32 chuckyaFlagCurr = OMM_SPARKLY_CONTEXT_FLAG_ZONES << (gCurrAreaIndex - 1);
        s32 chuckyaFlagPrev = OMM_SPARKLY_CONTEXT_FLAG_ZONES << (2 - gCurrAreaIndex);
        if (omm_sparkly_context_get_flag(chuckyaFlagPrev) && omm_mario_is_holding(m)) {
            struct Object *chuckya = obj_get_first_with_behavior(bhvChuckya);
            if (!chuckya)  chuckya = spawn_object(m->marioObj, MODEL_CHUCKYA, bhvChuckya);
            chuckya->oHeldState = HELD_HELD;
            m->heldObj = chuckya;
        }
        omm_sparkly_context_set_flag(chuckyaFlagCurr, omm_mario_is_holding(m) && m->heldObj && m->heldObj->behavior == bhvChuckya, 1);
        omm_sparkly_context_set_flag(chuckyaFlagPrev, 0, 1);

        // Check if Mario is inside the opening where the red coins star is
        if ((gCurrAreaIndex == 2) &&
            (-1024 < m->pos[0]) && (m->pos[0] <  -512) &&
            (-1800 < m->pos[1]) && (m->pos[1] < -1400) &&
            (+3200 < m->pos[2]) && (m->pos[2] < +3712)) {

            // If Mario's holding a Chuckya and the Crystal Star isn't spawned yet, spawn it and replace the Chuckya
            if (omm_sparkly_context_get_flag(chuckyaFlagCurr) &&
                !omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL)) {
                obj_set_dormant(m->heldObj, true);
                omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, 1, 0);
                omm_sparkly_context_spawn_star(m);
                (*sOmmSparklyStar)->oAction = 4;
                m->usedObj = spawn_object(m->marioObj, MODEL_NONE, omm_bhv_dummy);
                m->heldObj = NULL;
                mario_grab_used_object(m);
            }

            // If Mario released the Star, collect it
            if (*sOmmSparklyStar && !m->heldObj) {
                (*sOmmSparklyStar)->oAction = 2;
            }
        }
        
        // If not, remove the Star from Mario's hands and replace it by the Chuckya
        else if (omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL)) {
            (*sOmmSparklyStar) = NULL;
            omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, 0, 1);
            if (m->heldObj) {
                obj_mark_for_deletion(m->heldObj);
                struct Object *chuckya = obj_get_first_with_behavior(bhvChuckya);
                if (!chuckya)  chuckya = spawn_object(m->marioObj, MODEL_CHUCKYA, bhvChuckya);
                chuckya->oHeldState = HELD_HELD;
                m->heldObj = chuckya;
                obj_set_dormant(m->heldObj, false);
            }
        }
    } else if (*sOmmSparklyStar) {
        *sOmmSparklyStar = NULL;
    }
    return true;
}

static bool hard_thi(struct MarioState *m, const OmmSparklyStarsData *data) {
    return normal_thi(m, data);
}

static bool hard_ttc(struct MarioState *m, const OmmSparklyStarsData *data) {

    // Don't land after leaving the floor, Super Mario Maker 2 style
    bool onGround = !(m->action & ACT_FLAG_AIR);
    if (omm_sparkly_context_is_state_ok()) {
        if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED)) {
            omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED, (m->action == ACT_OMM_POSSESSION) || (((m->action & ACT_GROUP_MASK) != ACT_GROUP_CUTSCENE) && !onGround), 0);
        } else if (onGround) {
            omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_FAIL, 1);
        }
    }
    return true;
}

static bool hard_rr(struct MarioState *m, const OmmSparklyStarsData *data) {
    common_mushrooms(m, data, 9, true);
    return true;
}

static bool hard_bitdw(struct MarioState *m, const OmmSparklyStarsData *data) {
    normal_bitdw(m, data);

    // 7 Goombas stack and 74 coins
    if (omm_sparkly_context_is_state_ok()) {
        if (m->action == ACT_OMM_POSSESSION &&
            gOmmData->mario->capture.obj->behavior == bhvGoomba &&
            gOmmData->object->goomba.stackCount >= 6) {
            s32 collectedCoins = (
                omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_YELLOW_COINS) * 1 +
                omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_BLUE_COINS) * 5 +
                omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_RED_COINS) * 2
            );
            if (collectedCoins >= 74) {
                omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, 1, 0);
            }
        }
    }
    return (m->action == ACT_OMM_POSSESSION || omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED));
}

static bool hard_bitfs(struct MarioState *m, const OmmSparklyStarsData *data) {
    static f32 sMarioLastGroundedY = 0.f;
    if (!(m->action & ACT_FLAG_AIR) && (m->action != ACT_OMM_POSSESSION || obj_is_on_ground(gOmmData->mario->capture.obj))) {
        sMarioLastGroundedY = m->pos[1];
    }

    // Spawn pillars, secrets and flamethrowers
    if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_OBJECTS_SPAWNED)) {
        omm_spawn_bitfs_pillar(m->marioObj, -5528, -3100, -572);
        omm_spawn_bitfs_pillar(m->marioObj, -5528, -3100, +572);
        omm_spawn_bitfs_pillar(m->marioObj, +4300,  +250, -572);
        omm_spawn_bitfs_pillar(m->marioObj, +4300,  +250, +572);
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvHiddenStarTrigger, -8000, -2800, -8000, 0, 0, 0);
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvHiddenStarTrigger, +8000, -2800, -8000, 0, 0, 0);
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvHiddenStarTrigger, +8000, -2800, +8000, 0, 0, 0);
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvHiddenStarTrigger, -8000, -2800, +8000, 0, 0, 0);
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvHiddenStarTrigger, +4300,  3000,     0, 0, 0, 0);
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvHiddenStarTrigger, -5528,  -300,     0, 0, 0, 0);
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvFlamethrower, -5528,  -250, +600, 0x0000, 0x8000, 0x0000);
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvFlamethrower, -5528,  -350, -600, 0x0000, 0x0000, 0x0000);
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvFlamethrower, -5528, -1250, +600, 0x0000, 0x8000, 0x0000);
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvFlamethrower, -5528, -2150, -600, 0x0000, 0x0000, 0x0000);
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvFlamethrower, +4300, +3050, -600, 0x0000, 0x0000, 0x0000);
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvFlamethrower, +4300, +2950, +600, 0x0000, 0x8000, 0x0000);
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvFlamethrower, +4300, +2050, -600, 0x0000, 0x0000, 0x0000);
        spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvFlamethrower, +4300, +1150, +600, 0x0000, 0x8000, 0x0000);
        omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_OBJECTS_SPAWNED, 1, 0);
        return true;
    }
    
    // Update secrets hitboxes
    for_each_object_with_behavior(trigger, bhvHiddenStarTrigger) {
        if (trigger->oPosY < -2500.f) {
            trigger->hitboxRadius = 200.f;
            trigger->hitboxHeight = 300.f;
            trigger->hitboxDownOffset = 150.f;
        } else {
            trigger->hitboxRadius = 100.f;
            trigger->hitboxHeight = 150.f;
            trigger->hitboxDownOffset = 75.f;
        }
    }

    // Anti-cheese
    if (m->action != ACT_SHOCKED) {
        struct Object *nearestSecretToMario = obj_get_nearest_with_behavior_and_radius(m->marioObj, bhvHiddenStarTrigger, 300.f);
        struct Object *nearestSecretToCappy = obj_get_nearest_with_behavior_and_radius(omm_cappy_get_object(), bhvHiddenStarTrigger, 400.f);
        if ((nearestSecretToMario && nearestSecretToMario->oPosY > -2500.f && nearestSecretToMario->oPosY < sMarioLastGroundedY) ||
            (nearestSecretToCappy && nearestSecretToCappy->oPosY > -2500.f && nearestSecretToCappy->oPosY < sMarioLastGroundedY)) {
            omm_cappy_unload();
            omm_spawn_damage_mario(m->marioObj, INTERACT_SHOCK, 1);
        }
    }
    
    // A secret was collected
    if (omm_sparkly_context_is_state_ok() && omm_sparkly_context_dif_counter(OMM_SPARKLY_CONTEXT_COUNTER_SECRETS) < 0) {
        s32 index = 6 - omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_SECRETS);
        spawn_object_relative(index, 0, 0, 0, m->marioObj, MODEL_NUMBER, bhvOrangeNumber)->oPosY += 80;
        play_sound(SOUND_MENU_COLLECT_SECRET + (((u8) index - 1) << 16), gGlobalSoundArgs);
    }
    omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_SECRETS) == 0, 0);
    return true;
}

static bool hard_vcutm(struct MarioState *m, const OmmSparklyStarsData *data) {
    return normal_vcutm(m, data);
}

static bool hard_cotmc(struct MarioState *m, const OmmSparklyStarsData *data) {
    return normal_cotmc(m, data);
}

static bool hard_sa(struct MarioState *m, const OmmSparklyStarsData *data) {
    return normal_sa(m, data);
}

static bool hard_inside(struct MarioState *m, const OmmSparklyStarsData *data) {
    hard_castle(m, data, OMM_ARRAY_OF(s32) {
         -670,    0,  -320, 0xA000, DIALOG_134,
        -2300, -204, -2100, 0x0000, DIALOG_135,
    });
    return true;
}

static bool hard_basement(struct MarioState *m, const OmmSparklyStarsData *data) {

    // Unspawn the regular Mips and spawn the super fast Mips
    if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_OBJECTS_SPAWNED)) {
        obj_deactivate_all_with_behavior(bhvMips);
        omm_spawn_mips(m->marioObj, -1509, -1177, -1564, 60.f);
        omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_OBJECTS_SPAWNED, 1, 0);
        return true;
    }
    
    // Mips must be caught 3 times
    struct Object *mips = obj_get_first_with_behavior(omm_bhv_mips);
    if (mips && omm_sparkly_context_is_state_ok()) {
        omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, mips->oMipsGrabbedCounter >= 3, 0);
    }
    return true;
}

static bool hard_upstairs(struct MarioState *m, const OmmSparklyStarsData *data) {
    hard_castle(m, data, OMM_ARRAY_OF(s32) {
        -2260, 2253, 4850, 0xA000, DIALOG_076,
         -205, 2253, 7200, 0x7000, DIALOG_083,
    });
    return true;
}

static bool hard_courtyard(struct MarioState *m, const OmmSparklyStarsData *data) {
    static Vec3f sCappyStarPos = { 9, 433, -1735 };
    static s32 sCappyTimer = 0;
    if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL) &&
        !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_GAME_PAUSED) &&
        !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_TRANSITION)) {
        struct Object *cappy = omm_cappy_get_object();
        if (cappy && vec3f_dist(&cappy->oPosX, sCappyStarPos) < 20.f && omm_sparkly_context_is_state_ok()) {
            switch (sCappyTimer++) {
                case  0: case 14: case  27: case  39: case  50:
                case 60: case 69: case  77: case  84: case  90:
                case 95: case 99: case 102: case 104: case 105: {
                    play_sound(SOUND_GENERAL_SHORT_STAR, gGlobalSoundArgs);
                    omm_spawn_sparkly_star_sparkle(cappy, OMM_SPARKLY_MODE_HARD, 0, 10.f, 0.4f, 50.f);
                    omm_spawn_sparkly_star_sparkle(cappy, OMM_SPARKLY_MODE_HARD, 0, 10.f, 0.4f, 50.f);
                    omm_spawn_sparkly_star_sparkle(cappy, OMM_SPARKLY_MODE_HARD, 0, 10.f, 0.4f, 50.f);
                    omm_spawn_sparkly_star_sparkle(cappy, OMM_SPARKLY_MODE_HARD, 0, 10.f, 0.4f, 50.f);
                    omm_spawn_sparkly_star_sparkle(cappy, OMM_SPARKLY_MODE_HARD, 0, 10.f, 0.4f, 50.f);
                } break;

                case 106: {
                    play_sound(SOUND_MENU_STAR_SOUND, gGlobalSoundArgs);
                    omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, 1, 0);
                } break;
            }
        } else {
            sCappyTimer = 0;
        }
    }
    return true;
}

//
// Extreme
//

// Coming soon...

//
// Update per difficulty and level
//

void omm_sparkly_level_update(struct MarioState *m, const OmmSparklyStarsData *data) {
    static bool (*sOmmSparklyLevelUpdate[29][3])(struct MarioState *m, const OmmSparklyStarsData *data) = {
        { normal_bob,       hard_bob,       NULL },
        { NULL,             NULL,           NULL },
        { normal_jrb,       hard_jrb,       NULL },
        { NULL,             NULL,           NULL },
        { NULL,             NULL,           NULL },
        { NULL,             hard_hmc,       NULL },
        { normal_lll,       hard_lll,       NULL },
        { NULL,             hard_ssl,       NULL },
        { NULL,             NULL,           NULL },
        { normal_sl,        hard_sl,        NULL },
        { NULL,             hard_wdw,       NULL },
        { NULL,             NULL,           NULL },
        { normal_thi,       hard_thi,       NULL },
        { normal_ttc,       hard_ttc,       NULL },
        { NULL,             hard_rr,        NULL },
        { normal_bitdw,     hard_bitdw,     NULL },
        { normal_bitfs,     hard_bitfs,     NULL },
        { NULL,             NULL,           NULL },
        { NULL,             NULL,           NULL },
        { normal_vcutm,     hard_vcutm,     NULL },
        { normal_cotmc,     hard_cotmc,     NULL },
        { NULL,             NULL,           NULL },
        { NULL,             NULL,           NULL },
        { normal_sa,        hard_sa,        NULL },
        { NULL,             NULL,           NULL },
        { normal_inside,    hard_inside,    NULL },
        { normal_basement,  hard_basement,  NULL },
        { normal_upstairs,  hard_upstairs,  NULL },
        { normal_courtyard, hard_courtyard, NULL },
    };

    // Update the context for the current star and mode
    s32 star = data->index;
    s32 mode = omm_sparkly_get_current_mode() - 1;
    bool run = true;
    if (sOmmSparklyLevelUpdate[star][mode]) {
        run = sOmmSparklyLevelUpdate[star][mode](m, data);
    }
    omm_sparkly_level_update_timer(m, data, run);
}

#pragma GCC pop_options
