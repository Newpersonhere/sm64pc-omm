#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS
#if OMM_CODE_SPARKLY

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
    gOmmData->mario->sparkly.mode = (omm_sparkly_check_cheats(gMarioState) ? OMM_SPARKLY_MODE_DISABLED : gOmmSparklyStarsMode);
    if (omm_sparkly_is_enabled()) {
        omm_sparkly_context_restart();
    }
}

//
// Objects
//

static s32 omm_sparkly_context_count_objects(const BehaviorScript *behavior, u8 field, s32 value, bool countDormants) {
    if (!behavior) return 0;
    s32 count = 0;
    for_each_object_in_list(obj, get_object_list_from_behavior(behavior)) {
        count += (
            (countDormants || !obj_is_dormant(obj)) &&
            (obj->behavior == behavior) &&
            ((field == 0) || (obj->OBJECT_FIELD_S32(field) == value)) &&
            (obj->respawnInfoType != OMM_OBJECT_LOST_COIN)
        );
    }
    return count;
}

static struct Object *omm_sparkly_context_spawn_object_dormant(struct Object *parent, s16 uselessArg, u32 model, const BehaviorScript *behavior, s16 x, s16 y, s16 z, s16 rx, s16 ry, s16 rz) {
    struct Object *o = spawn_object_abs_with_rot(parent, uselessArg, model, behavior, x, y, z, rx, ry, rz);
    obj_set_dormant(o, true);
    return o;
}

static void omm_sparkly_context_awaken_objects(const BehaviorScript *behavior) {
    for_each_object_with_behavior(obj, behavior) {
        if (obj->behavior == behavior) {
            obj_set_dormant(obj, false);
        }
    }
}

//
// Per level
//

static void omm_sparkly_context_decrease_timer(struct MarioState *m, bool decrease) {
    if (omm_sparkly_flags_get(OMM_SPARKLY_FLAG_MARIO_UPDATED) &&
        !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_GAME_PAUSED) &&
        !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_TRANSITION) &&
        !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_TIME_STOP) &&
        !omm_mario_is_reading(m)) {
        if (omm_sparkly_context_is_state_ok()) {
            if (gHudDisplay.timer > 0) {
                gHudDisplay.timer -= decrease;
                gHudDisplay.flags |= HUD_DISPLAY_FLAG_TIMER;
            } else {
                omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_FAIL, 1);
            }
        } else {
            gHudDisplay.flags &= ~HUD_DISPLAY_FLAG_TIMER;
        }
    }
}

static void omm_sparkly_context_update_timer(struct MarioState *m, s32 timer, bool start) {

    // Start the timer...
    if (omm_sparkly_context_is_state_ok() && !omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED)) {
        if (start) {
            gHudDisplay.timer = timer;
            gHudDisplay.flags |= HUD_DISPLAY_FLAG_TIMER;
            omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED, 1, 0);
        }
    }

    // ...and grab the star before it runs out
    else if (!omm_sparkly_is_mode_selected(OMM_SPARKLY_MODE_NORMAL) || !omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_STAR_SPAWNED)) {
        omm_sparkly_context_decrease_timer(m, 1);
    }
}

static void omm_sparkly_context_update_cap_course(struct MarioState *m, s32 cap, s32 timer) {

    // Start with a cap...
    if (omm_sparkly_context_is_state_ok() && !omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED)) {
        m->flags &= ~(MARIO_CAP_IN_HAND | MARIO_WING_CAP | MARIO_METAL_CAP | MARIO_VANISH_CAP);
        m->flags |= (MARIO_CAP_ON_HEAD | cap);
        m->capTimer = timer;
        m->marioObj->oHomeX = m->pos[0];
        m->marioObj->oHomeY = m->pos[1];
        m->marioObj->oHomeZ = m->pos[2];
        switch (cap) {
            case MARIO_NORMAL_CAP: audio_stop_cap_music();        m->capTimer = 1; break;
            case MARIO_WING_CAP:   audio_play_wing_cap_music();   m->capTimer = omm_max_s(m->capTimer, OMM_IMPROVED_WING_CAP_DURATION); break;
            case MARIO_METAL_CAP:  audio_play_metal_cap_music();  m->capTimer = omm_max_s(m->capTimer, OMM_IMPROVED_METAL_CAP_DURATION); break;
            case MARIO_VANISH_CAP: audio_play_vanish_cap_music(); m->capTimer = omm_max_s(m->capTimer, OMM_IMPROVED_VANISH_CAP_DURATION); break;
        }
        gHudDisplay.timer = timer;
        gHudDisplay.flags |= HUD_DISPLAY_FLAG_TIMER;
        omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED, 1, 0);
    }

    // ...and grab the star before it runs out
    else if (!omm_sparkly_is_mode_selected(OMM_SPARKLY_MODE_NORMAL) || !omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_STAR_SPAWNED)) {
        omm_sparkly_context_decrease_timer(m, 1);
    }
}

static void omm_sparkly_context_update_shards(struct MarioState *m, const f32 *positions, s32 redCoins) {
    if (omm_sparkly_context_is_state_ok()) {

        // Spawn shards
        if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED)) {
            for (s32 i = 0; i != 8; ++i) {
                spawn_object_abs_with_rot(m->marioObj, 0,
                    MODEL_SPARKLES, bhvRedCoin,
                    positions[3 * i + 0],
                    positions[3 * i + 1],
                    positions[3 * i + 2],
                    0, 0, 0
                )->parentObj = NULL;
            }
            omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED, 1, 0);
        }

        // A shard was collected
        else if (redCoins > 0 && omm_sparkly_context_dif_counter(OMM_SPARKLY_CONTEXT_COUNTER_RED_COINS) > 0) {
            spawn_object_relative(redCoins, 0, 0, 0, m->marioObj, MODEL_NUMBER, bhvOrangeNumber)->oPosY += 80;
            play_sound(SOUND_MENU_COLLECT_RED_COIN + (((u8) (redCoins - 1)) << 16), gGlobalSoundArgs);
            omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, redCoins == 8, 0);
        }
    }
}

static void omm_sparkly_context_spawn_toad_at(struct MarioState *m, f32 x, f32 y, f32 z, s16 a, s32 dialogId) {
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
    struct Object *toad = spawn_object(m->marioObj, MODEL_TOAD, bhvToadMessage2);
    toad->behavior = bhvToadMessage;
    toad->oBehParams = dialogId << 24;
    obj_set_pos(toad, x, y, z);
    obj_set_home(toad, x, y, z);
    obj_set_angle(toad, 0, a, 0);
    obj_reset_hitbox(toad, 20, 100, 0, 0, 50, 0);
}

#define LEVEL(mode, level, area) ((mode << 16) + (level << 8) + (area << 0))
#define COIN_ARROW(x, y, z, a)   ((((u64) ((u16) ((s16) x))) << 48llu) | (((u64) ((u16) ((s16) y))) << 32llu) | (((u64) ((u16) ((s16) z))) << 16llu) | (((u64) ((u16) ((s16) a))) << 0llu))

static void omm_sparkly_update_context_per_level(struct MarioState *m) {
    s32 level = LEVEL(omm_sparkly_get_current_mode(), gCurrLevelNum, gCurrAreaIndex);
    s32 redCoins = omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_RED_COINS);
    s32 mushrooms = omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_1UP_MUSHROOMS);
    u64 coinArrow = 0;

    switch (level) {

        // BOB
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_BOB, 1):
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_BOB, 1): {
            omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_SECRETS) == 0, 0);
        } break;

        // WF
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_WF, 1): {
        } break;
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_WF, 1): {
            omm_sparkly_context_update_timer(m, OMM_SPARKLY_WF_TIMER, true);
        } break;

        // JRB
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_JRB, 1):
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_JRB, 1): {
            if (omm_world_is_flooded()) {
                if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_DIALOG_STARTED) &&
                    !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_GAME_PAUSED) &&
                    !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_TRANSITION)) {
                    level_set_transition(-1, NULL);
                    create_dialog_box(OMM_SPARKLY_JRB_START_DIALOG);
                    omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_DIALOG_STARTED, 1, 0);
                }
            } else {
                omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 0);
            }
        } break;

        // CCM
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_CCM, 2): {
            coinArrow = COIN_ARROW(-4890, 0, 6000, 0x0000);
        } break;
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_CCM, 1): {
            coinArrow = COIN_ARROW(-55, 5800, -150, 0x0000);
        } break;

        // BBH
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_BBH, 1): {
        } break;
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_BBH, 1): {
            omm_sparkly_context_update_timer(m, OMM_SPARKLY_BBH_TIMER, true);
        } break;

        // HMC
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_HMC, 1): {
            coinArrow = COIN_ARROW(7000, 2000, 7100, 0x6000);
        } break;
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_HMC, 1): {
            static struct { struct Object *obj; Vec3f pos; } sFlames[64];
            static s32 sNumFlames;
            omm_sparkly_context_update_timer(m, OMM_SPARKLY_HMC_TIMER, true);

            // Make the flames bigger
            if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED)) {
                sNumFlames = 0;
                for_each_object_with_behavior(flame, bhvFlame) {
                    flame->oScaleX *= OMM_SPARKLY_HMC_FLAME_MULT;
                    flame->oScaleY *= OMM_SPARKLY_HMC_FLAME_MULT;
                    flame->oScaleZ *= OMM_SPARKLY_HMC_FLAME_MULT;
                    flame->hitboxRadius *= OMM_SPARKLY_HMC_FLAME_MULT;
                    flame->hitboxHeight *= OMM_SPARKLY_HMC_FLAME_MULT * 4.f;
                    flame->hurtboxRadius *= OMM_SPARKLY_HMC_FLAME_MULT;
                    flame->hurtboxHeight *= OMM_SPARKLY_HMC_FLAME_MULT * 4.f;
                    flame->hitboxDownOffset *= OMM_SPARKLY_HMC_FLAME_MULT * 2.f;
                    sFlames[sNumFlames].obj = flame;
                    sFlames[sNumFlames].pos[0] = flame->oPosX;
                    sFlames[sNumFlames].pos[1] = flame->oPosY;
                    sFlames[sNumFlames].pos[2] = flame->oPosZ;
                    sNumFlames++;
                }
                omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED, 1, 0);
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
        } break;

        // LLL
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_LLL, 2):
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_LLL, 2): {
            struct Object *lava = obj_get_first_with_behavior(omm_bhv_rising_lava);

            // Spawn lava object and star shards
            if (lava == NULL) {
                lava = omm_spawn_rising_lava(m->marioObj, OMM_SPARKLY_LLL_LAVA_PARAMS);
                obj_set_dormant(lava, true);
                for (s32 i = 0; i != 8; ++i) {
                    omm_sparkly_context_spawn_object_dormant(m->marioObj, 0,
                        MODEL_SPARKLES, bhvRedCoin,
                        OMM_SPARKLY_LLL_STAR_SHARDS_POS[3 * i + 0],
                        OMM_SPARKLY_LLL_STAR_SHARDS_POS[3 * i + 1],
                        OMM_SPARKLY_LLL_STAR_SHARDS_POS[3 * i + 2],
                        0, 0, 0
                    )->parentObj = NULL;
                }
            }

            // Update rising lava
            else if (!obj_is_dormant(lava)) {
                if (omm_sparkly_context_is_state_ok() && redCoins > 0 && omm_sparkly_context_dif_counter(OMM_SPARKLY_CONTEXT_COUNTER_RED_COINS) > 0) {
                    lava->oAction = 1;
                    spawn_object_relative(redCoins, 0, 0, 0, m->marioObj, MODEL_NUMBER, bhvOrangeNumber)->oPosY += 80;
                    play_sound(SOUND_MENU_COLLECT_RED_COIN + (((u8) (redCoins - 1)) << 16), gGlobalSoundArgs);
                } else if (m->prevAction == ACT_OMM_SPARKLY_STAR_DANCE) {
                    lava->oAction = 2;
                }
            }
        } break;

        // SSL
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_SSL, 1): {
            coinArrow = COIN_ARROW(-2050, 2000, -1020, 0x8000);
        } break;
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_SSL, 2): {
            omm_sparkly_context_update_timer(m, OMM_SPARKLY_SSL_TIMER, true);
            if (omm_sparkly_context_is_state_ok()) {
                s32 difY = omm_sparkly_context_dif_counter(OMM_SPARKLY_CONTEXT_COUNTER_YELLOW_COINS);
                s32 difB = omm_sparkly_context_dif_counter(OMM_SPARKLY_CONTEXT_COUNTER_BLUE_COINS);
                s32 difR = omm_sparkly_context_dif_counter(OMM_SPARKLY_CONTEXT_COUNTER_RED_COINS);
                s32 difC = omm_max_s(difY, 0) * 1 + omm_max_s(difR, 0) * 2 + omm_max_s(difB, 0) * 5;
                gHudDisplay.timer += difC * 45;
            }
        } break;

        // DDD
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_DDD, 2): {
            coinArrow = COIN_ARROW(3900, 1700, -4000, 0x8000);
        } break;
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_DDD, 2): {
            coinArrow = COIN_ARROW(3900, 2000, 6000, 0x0000);
        } break;

        // SL
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_SL, 1):
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_SL, 1): {
            if (omm_sparkly_context_is_state_ok()) {
                s32 numSnowmenAtTop = 0;
                for_each_object_with_behavior(mrBlizzard, bhvMrBlizzard) {
                    f32 dx = mrBlizzard->oPosX - OMM_SPARKLY_SL_REACH_X;
                    f32 dy = mrBlizzard->oPosY - OMM_SPARKLY_SL_REACH_Y;
                    f32 dz = mrBlizzard->oPosZ - OMM_SPARKLY_SL_REACH_Z;
                    f32 dr = sqrtf(omm_sqr_f(dx) + omm_sqr_f(dz));
                    numSnowmenAtTop += (dy > 0.f && dr < OMM_SPARKLY_SL_REACH_R);
                }
                omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, numSnowmenAtTop >= OMM_SPARKLY_SL_NUM_SNOWMEN, 1);
            }
        } break;

        // WDW
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_WDW, 1): {
            coinArrow = COIN_ARROW(-1075, 4500, 1280, 0xA000);
        } break;
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_WDW, 1):
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_WDW, 2): {
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
                    (-1024 < m->pos[0]) && (m->pos[0] < -512) &&
                    (-1800 < m->pos[1]) && (m->pos[1] < -1400) &&
                    (+3200 < m->pos[2]) && (m->pos[2] < +3712)) {

                    // If Mario's holding a Chuckya and the Crystal Star isn't spawned yet, spawn it and replace the Chuckya
                    if (omm_sparkly_context_get_flag(chuckyaFlagCurr) &&
                       !omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL)) {
                        obj_set_dormant(m->heldObj, true);
                        omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, 1, 0);
                        omm_sparkly_context_spawn_star(m);
                        sOmmSparklyContext->star->oAction = 4;
                        m->usedObj = spawn_object(m->marioObj, MODEL_NONE, omm_bhv_dummy);
                        m->heldObj = NULL;
                        mario_grab_used_object(m);
                    }

                    // If Mario released the Star, collect it
                    if (sOmmSparklyContext->star && !m->heldObj) {
                        sOmmSparklyContext->star->oAction = 2;
                    }
                }
                
                // If not, remove the Star from Mario's hands and replace it by the Chuckya
                else if (omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL)) {
                    sOmmSparklyContext->star = NULL;
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
            } else if (sOmmSparklyContext->star) {
                sOmmSparklyContext->star = NULL;
            }
        } break;

        // TTM
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_TTM, 3): {
            coinArrow = COIN_ARROW(1200, -600, -5000, 0x5000);
        } break;
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_TTM, 3): {
            coinArrow = COIN_ARROW(4200, 6300, 4000, 0x6000);
        } break;

        // THI
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_THI, 2):
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_THI, 1): {
            if (omm_sparkly_context_is_state_ok() &&
                m->action == ACT_OMM_POSSESSION &&
                gOmmData->mario->capture.obj->behavior == bhvGoomba &&
                gOmmData->object->goomba.stackCount >= 9) {
                omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, 1, 0);
            }
        } break;

        // TTC
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_TTC, 1): {
            s32 boxes = omm_sparkly_context_get_remaining_boxes();

            // Start the timer as soon as the first box is broken
            if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED)) {
                if (boxes < OMM_SPARKLY_TTC_BOX_COUNT) {
                    gHudDisplay.timer = OMM_SPARKLY_TTC_BOX_TIMER;
                    gHudDisplay.flags |= HUD_DISPLAY_FLAG_TIMER;
                    omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED, 1, 0);
                }
            }

            // Break the boxes before the timer runs out
            else {
                omm_sparkly_context_decrease_timer(m, boxes != 0);
                omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, boxes == 0, 0);
            }
        } break;
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_TTC, 1): {

            // Don't land after leaving the floor, Super Mario Maker 2 style
            bool onGround = !(m->action & ACT_FLAG_AIR);
            if (omm_sparkly_context_is_state_ok()) {
                if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED)) {
                    omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED, (m->action == ACT_OMM_POSSESSION) || (((m->action & ACT_GROUP_MASK) != ACT_GROUP_CUTSCENE) && !onGround), 0);
                } else if (onGround) {
                    omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_FAIL, 1);
                }
            }
        } break;

        // RR
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_RR, 1): {
        } break;
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_RR, 1): {
            if (omm_sparkly_context_is_state_ok() && omm_sparkly_context_dif_counter(OMM_SPARKLY_CONTEXT_COUNTER_1UP_MUSHROOMS) > 0) {
                spawn_object_relative(mushrooms, 0, 0, 0, m->marioObj, MODEL_NUMBER, bhvOrangeNumber)->oPosY += 80;
                play_sound(SOUND_GENERAL_COLLECT_1UP, gGlobalSoundArgs);
                omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, mushrooms == OMM_SPARKLY_RR_1UP_MUSHROOM_COUNT, 0);
            }
        } break;

        // BITDW
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_BITDW, 1): {
            if (omm_sparkly_context_is_state_ok()) {

                // 7 Goombas stack
                if (m->action == ACT_OMM_POSSESSION &&
                    gOmmData->mario->capture.obj->behavior == bhvGoomba &&
                    gOmmData->object->goomba.stackCount >= 6) {

                    // 74 coins
                    s32 collectedCoins = omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_YELLOW_COINS) * 1 +
                                         omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_BLUE_COINS) * 5 +
                                         omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_RED_COINS) * 2;
                    if (collectedCoins >= 74) {
                        omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, 1, 0);
                    }
                }
            }
        } // Fall through
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_BITDW, 1): {
            omm_sparkly_context_update_timer(m, OMM_SPARKLY_BITDW_TIMER, omm_sparkly_is_mode_selected(OMM_SPARKLY_MODE_HARD) && m->action == ACT_OMM_POSSESSION);

            // Verifying conditions...
            // Mario must be possessing a goomba or standing inside the safe zone without touching a coin
            if (m->action != ACT_OMM_POSSESSION || gOmmData->mario->capture.obj->behavior != bhvGoomba) {
                if ((m->pos[0] < OMM_SPARKLY_BITDW_SAFE_ZONE_X_MIN) || (m->pos[0] > OMM_SPARKLY_BITDW_SAFE_ZONE_X_MAX) ||
                    (m->pos[2] < OMM_SPARKLY_BITDW_SAFE_ZONE_Z_MIN) || (m->pos[2] > OMM_SPARKLY_BITDW_SAFE_ZONE_Z_MAX) ||
                    (m->numCoins != 0 && !omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_STAR_SPAWNED))) {
                    omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_FAIL, 1);
                }
            }

            // Spawns a dormant goomba at the center of the safe zone
            static struct Object *sBitdwGoomba = NULL;
            if (!obj_get_first_with_behavior(omm_bhv_sparkly_star_hint) && omm_sparkly_context_is_state_ok()) {
                f32 goombaX = (OMM_SPARKLY_BITDW_SAFE_ZONE_X_MIN + OMM_SPARKLY_BITDW_SAFE_ZONE_X_MAX) / 2;
                f32 goombaZ = (OMM_SPARKLY_BITDW_SAFE_ZONE_Z_MIN + OMM_SPARKLY_BITDW_SAFE_ZONE_Z_MAX) / 2;
                sBitdwGoomba = omm_sparkly_context_spawn_object_dormant(m->marioObj, 0, MODEL_GOOMBA, bhvGoomba, goombaX, +20000.f, goombaZ, 0, 0, 0);
                sBitdwGoomba->oBehParams = GOOMBA_SIZE_REGULAR << 16;
                sBitdwGoomba->oBehParams2ndByte = GOOMBA_SIZE_REGULAR;
                sBitdwGoomba->parentObj = sBitdwGoomba;
                sBitdwGoomba->prevObj = NULL;
                obj_drop_to_floor(sBitdwGoomba);
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
        } break;

        // BITFS
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_BITFS, 1): {
            if (omm_sparkly_context_is_state_ok() && omm_sparkly_context_dif_counter(OMM_SPARKLY_CONTEXT_COUNTER_1UP_MUSHROOMS) > 0) {
                if (m->action == ACT_OMM_POSSESSION) {
                    spawn_object_relative(mushrooms, 0, 0, 0, m->marioObj, MODEL_NUMBER, bhvOrangeNumber)->oPosY += 80;
                    play_sound(SOUND_GENERAL_COLLECT_1UP, gGlobalSoundArgs);
                    omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, mushrooms == OMM_SPARKLY_BITFS_1UP_MUSHROOM_COUNT, 0);
                } else {
                    omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_FAIL, 1);
                }
            }
        } break;
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_BITFS, 1): {
            static s32 sMarioBlownTimer = 0;
            static f32 sMarioLastGroundedY = 0.f;
            if (!(m->action & ACT_FLAG_AIR) && (m->action != ACT_OMM_POSSESSION || obj_is_on_ground(gOmmData->mario->capture.obj))) {
                sMarioLastGroundedY = m->pos[1];
            }

            // Init
            if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED)) {

                // Spawn secrets
                for (s32 i = 0; i != OMM_SPARKLY_BITFS_SECRET_COUNT; ++i) {
                    spawn_object_abs_with_rot(m->marioObj, 0,
                        MODEL_NONE, bhvHiddenStarTrigger,
                        OMM_SPARKLY_BITFS_SECRETS_POS[3 * i + 0],
                        OMM_SPARKLY_BITFS_SECRETS_POS[3 * i + 1],
                        OMM_SPARKLY_BITFS_SECRETS_POS[3 * i + 2],
                        0, 0, 0
                    );
                }

                // Spawn pillars and flamethrowers
                omm_spawn_bitfs_pillar(m->marioObj, -5528, -3100, -572);
                omm_spawn_bitfs_pillar(m->marioObj, -5528, -3100, +572);
                omm_spawn_bitfs_pillar(m->marioObj, +4300,  +250, -572);
                omm_spawn_bitfs_pillar(m->marioObj, +4300,  +250, +572);
                spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvFlamethrower, -5528,  -250, +600, 0x0000, 0x8000, 0x0000);
                spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvFlamethrower, -5528,  -350, -600, 0x0000, 0x0000, 0x0000);
                spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvFlamethrower, -5528, -1250, +600, 0x0000, 0x8000, 0x0000);
                spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvFlamethrower, -5528, -2150, -600, 0x0000, 0x0000, 0x0000);
                spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvFlamethrower, +4300, +3050, -600, 0x0000, 0x0000, 0x0000);
                spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvFlamethrower, +4300, +2950, +600, 0x0000, 0x8000, 0x0000);
                spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvFlamethrower, +4300, +2050, -600, 0x0000, 0x0000, 0x0000);
                spawn_object_abs_with_rot(m->marioObj, 0, MODEL_NONE, bhvFlamethrower, +4300, +1150, +600, 0x0000, 0x8000, 0x0000);
                omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED, 1, 0);
            } else {

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
                struct Object *o = obj_get_nearest_with_behavior_and_radius(omm_cappy_get_object(), bhvHiddenStarTrigger, 300.f);
                if (o != NULL && o->oPosY > -2500.f && o->oPosY < sMarioLastGroundedY) {
                    omm_cappy_unload();
                }
                if (sMarioBlownTimer == 0) {
                    struct Object *o = obj_get_nearest_with_behavior_and_radius(m->marioObj, bhvHiddenStarTrigger, 300.f);
                    if (o != NULL && o->oPosY > -2500.f && o->oPosY < sMarioLastGroundedY) {
                        sMarioBlownTimer = 30;
                        mario_stop_riding_and_holding(m);
                        m->faceAngle[1] = obj_get_object1_angle_yaw_to_object2(m->marioObj, o);
                        m->unkC4 = 0.4f;
                        m->forwardVel = -24.f;
                        m->vel[1] = 12.f;
                        play_sound(SOUND_MARIO_WAAAOOOW, m->marioObj->oCameraToObject);
                        update_mario_sound_and_camera(m);
                        omm_mario_unpossess_object(m, OMM_MARIO_UNPOSSESS_ACT_NONE, false, 0);
                        omm_mario_set_action(m, ACT_GETTING_BLOWN, 0, 0);
                    }
                } else {
                    play_sound(SOUND_ENV_WIND1, gGlobalSoundArgs);
                    for (s32 i = 0; i != 3; ++i) {
                        struct Object *wind = spawn_object(m->marioObj, MODEL_MIST, bhvWind);
                        wind->oMoveAngleYaw = 0x8000 + m->faceAngle[1];
                        wind->oMoveAnglePitch = 0;
                    }
                    sMarioBlownTimer--;
                }

                // A secret was collected
                if (omm_sparkly_context_is_state_ok() && omm_sparkly_context_dif_counter(OMM_SPARKLY_CONTEXT_COUNTER_SECRETS) < 0) {
                    s32 index = OMM_SPARKLY_BITFS_SECRET_COUNT - omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_SECRETS);
                    spawn_object_relative(index, 0, 0, 0, m->marioObj, MODEL_NUMBER, bhvOrangeNumber)->oPosY += 80;
                    play_sound(SOUND_MENU_COLLECT_SECRET + (((u8) index - 1) << 16), gGlobalSoundArgs);
                }
                omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_SECRETS) == 0, 0);
            }
        } break;

        // BITS
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_BITS, 1):
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_BITS, 1): {
        } break;

        // TOTWC
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_TOTWC, 1):
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_TOTWC, 1): {
            omm_sparkly_context_update_cap_course(m, MARIO_WING_CAP, OMM_SPARKLY_TOTWC_CAP_TIMER);
        } break;

        // VCUTM
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_VCUTM, 1):
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_VCUTM, 1): {
            omm_sparkly_context_update_cap_course(m, MARIO_VANISH_CAP, OMM_SPARKLY_VCUTM_CAP_TIMER);
            const Vec2f zones[] = OMM_SPARKLY_VCUTM_ZONES;
            bool newZone = false;
            s32 total = 0;

            // Zones
            for (s32 i = 0; i != OMM_SPARKLY_VCUTM_ZONE_COUNT; ++i) {
                if (omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_ZONES << i)) {
                    total++;
                } else {
                    f32 x = zones[i][0];
                    f32 z = zones[i][1];
                    f32 d = sqrtf(omm_sqr_f(m->pos[0] - x) + omm_sqr_f(m->pos[2] - z));
                    if (d < OMM_SPARKLY_VCUTM_ZONE_RADIUS) {
                        omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_ZONES << i, 1, 0);
                        newZone = true;
                        total++;
                    }
                }
            }
            omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, total == OMM_SPARKLY_VCUTM_ZONE_COUNT, 0);

            // Spawn secret effect if Mario enters a new zone
            if (newZone && omm_sparkly_context_is_state_ok()) {
                spawn_object_relative(total, 0, 0, 0, m->marioObj, MODEL_NUMBER, bhvOrangeNumber)->oPosY += 80;
                play_sound(SOUND_MENU_COLLECT_SECRET + (((u8) total - 1) << 16), gGlobalSoundArgs);
            }
        } break;

        // COTMC
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_COTMC, 1):
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_COTMC, 1): {
            omm_sparkly_context_update_cap_course(m, MARIO_METAL_CAP, OMM_SPARKLY_COTMC_CAP_TIMER);
            coinArrow = COIN_ARROW(0, 600, -7300, 0x8000);
            bool onGround = !(m->action & ACT_FLAG_AIR);

            // When warp-landing, trigger the dialog box...
            if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_DIALOG_STARTED)) {
                if (onGround) {
                    omm_mario_set_action(m, ACT_READING_AUTOMATIC_DIALOG, OMM_SPARKLY_COTMC_START_DIALOG, 0xFFFF);
                    omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_DIALOG_STARTED, 1, 0);
                }
            }

            // ...then, enter the starting action...
            else if (m->prevAction == ACT_READING_AUTOMATIC_DIALOG) {
                omm_mario_set_action(m, OMM_SPARKLY_COTMC_START_ACTION, 0, 0xFFFF);
                mario_set_forward_vel(m, 48.f);
            }

            // ...and never touch the floor again
            else if ((onGround && ((m->action & ACT_GROUP_MASK) != ACT_GROUP_CUTSCENE)) ||
                (omm_peach_vibe_is_joy() && m->pos[1] < find_floor_height(m->pos[0], m->pos[1], m->pos[2]) + 4.f)) {
                omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_FAIL, 1);
            }
        } break;

        // PSS
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_PSS, 1):
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_PSS, 1): {
            if ((gHudDisplay.flags & HUD_DISPLAY_FLAG_TIMER) && gHudDisplay.timer >= OMM_SPARKLY_COTMC_PSS_TIMER) {
                omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_FAIL, 1);
            }
        } break;

        // SA
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_SA, 1):
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_SA, 1): {
            if (omm_world_is_frozen()) {
                if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_DIALOG_STARTED) &&
                    !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_GAME_PAUSED) &&
                    !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_TRANSITION)) {
                    level_set_transition(-1, NULL);
                    create_dialog_box(OMM_SPARKLY_SA_START_DIALOG);
                    omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_DIALOG_STARTED, 1, 0);
                }
            } else {
                omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 0);
            }
        } break;

        // WMOTR
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_WMOTR, 1):
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_WMOTR, 1): {
        } break;

        // GROUNDS
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_GROUNDS, 1):
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_GROUNDS, 1): {
        } break;

        // INSIDE
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_CASTLE, 1): {
            omm_sparkly_context_update_shards(m, OMM_SPARKLY_INSIDE_STAR_SHARDS_POS, redCoins);
        } break;
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_CASTLE, 1): {
            if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED)) {
                obj_deactivate_all_with_behavior(bhvToadMessage);
                omm_sparkly_context_spawn_toad_at(m,  -670,    0,  -320, 0xA000, DIALOG_134);
                omm_sparkly_context_spawn_toad_at(m, -2300, -204, -2100, 0x0000, DIALOG_135);
                omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED, 1, 0);
            }
        } break;

        // BASEMENT
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_CASTLE, 3): {
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
                            omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL,
                                distToad1 < OMM_SPARKLY_BASEMENT_MOAT_PILLARS_RADIUS &&
                                distToad2 < OMM_SPARKLY_BASEMENT_MOAT_PILLARS_RADIUS, 0
                            );
                        }
                    }
                }
            }
        } break;
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_CASTLE, 3): {
            if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED)) {
                obj_deactivate_all_with_behavior(bhvMips);
                omm_spawn_mips(m->marioObj, -1509, -1177, -1564, 60.f);
                omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED, 1, 0);
            }
            if (omm_sparkly_context_is_state_ok()) {
                struct Object *mips = obj_get_first_with_behavior(omm_bhv_mips);
                if (mips) {
                    omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, mips->oMipsGrabbedCounter >= 3, 0);
                }
            }
        } break;

        // UPSTAIRS
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_CASTLE, 2): {
            omm_sparkly_context_update_shards(m, OMM_SPARKLY_UPSTAIRS_STAR_SHARDS_POS, redCoins);
        } break;
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_CASTLE, 2): {
            if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED)) {
                obj_deactivate_all_with_behavior(bhvToadMessage);
                omm_sparkly_context_spawn_toad_at(m, -2260, 2253, 4850, 0xA000, DIALOG_076);
                omm_sparkly_context_spawn_toad_at(m,  -205, 2253, 7200, 0x7000, DIALOG_083);
                omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED, 1, 0);
            }
            if (m->action == ACT_LONG_JUMP ||
                m->action == ACT_LONG_JUMP_LAND ||
                m->action == ACT_LONG_JUMP_LAND_STOP) {
                if (gOmmData->mario->state.previous.pos[2] < 4500.f && m->pos[2] >= 4500.f && m->forwardVel <= -320.f) {
                    omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED, 1, 0);
                } else if (m->forwardVel == 0.f && omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED)) {
                    mario_set_forward_vel(m, gOmmData->mario->state.previous.vel[3]);
                    omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED, 0, 1);
                } else if (m->forwardVel > -320.f || m->pos[2] < 4500.f || m->pos[2] >= 4800.f) {
                    omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED, 0, 1);
                }
            } else {
                omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED, 0, 1);
            }
        } break;

        // COURTYARD
        case LEVEL(OMM_SPARKLY_MODE_NORMAL, LEVEL_COURT, 1): {

            // Spawn secrets in order
            if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED)) {
                for (s32 i = 0; i != OMM_SPARKLY_COURT_SECRET_COUNT; ++i) {
                    spawn_object_abs_with_rot(m->marioObj, 0,
                        MODEL_NONE, bhvHiddenStarTrigger,
                        OMM_SPARKLY_COURT_SECRETS_POS[3 * i + 0],
                        OMM_SPARKLY_COURT_SECRETS_POS[3 * i + 1],
                        OMM_SPARKLY_COURT_SECRETS_POS[3 * i + 2],
                        0, 0, 0
                    )->oBehParams = (i + 1);
                }
                omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_EVENT_STARTED, 1, 0);
            } else {

                // A secret was collected
                if (omm_sparkly_context_is_state_ok() && omm_sparkly_context_dif_counter(OMM_SPARKLY_CONTEXT_COUNTER_SECRETS) < 0) {
                    s32 expectedIndex = OMM_SPARKLY_COURT_SECRET_COUNT - omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_SECRETS);
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
            }
        } break;
        case LEVEL(OMM_SPARKLY_MODE_HARD, LEVEL_COURT, 1): {
            static s32 sCappyTimer = 0;
            if (!omm_sparkly_flags_get(OMM_SPARKLY_FLAG_GAME_PAUSED) && !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_TRANSITION)) {
                struct Object *cappy = omm_cappy_get_object();
                f32 dist = (cappy ? vec3f_dist(&cappy->oPosX, OMM_SPARKLY_COURT_CAPPY_STAR_POS) : 10000.f);
                if (omm_sparkly_context_is_state_ok() && !omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL) && dist < OMM_SPARKLY_COURT_CAPPY_STAR_RADIUS) {
                    switch (sCappyTimer++) {
                        case 0:
                        case 14:
                        case 27:
                        case 39:
                        case 50:
                        case 60:
                        case 69:
                        case 77:
                        case 84:
                        case 90:
                        case 95:
                        case 99:
                        case 102:
                        case 104:
                        case 105: {
                            play_sound(SOUND_GENERAL_SHORT_STAR, gGlobalSoundArgs);
                            for (s32 i = 0; i != 5; ++i) {
                                omm_spawn_sparkly_star_sparkle(cappy, OMM_SPARKLY_MODE_HARD, 0, 10.f, 0.4f, 50.f);
                            }
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
        } break;
    }

    // Coin requirement
    if (sOmmSparklyContext->data->flags & OMM_SPARKLY_DATA_BITS_COINS) {
        s32 requiredCoins  = OMM_SPARKLY_DATA_FLAG_GET_COINS(sOmmSparklyContext->data->flags);
        s32 collectedCoins = omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_YELLOW_COINS) * 1 +
                             omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_BLUE_COINS) * 5 +
                             omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_RED_COINS) * 2;
        omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, collectedCoins >= requiredCoins, 0);
    }

    // All red coins
    if (sOmmSparklyContext->data->flags & OMM_SPARKLY_DATA_FLAG_ALL_RED_COINS) {
        omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_SUCCESSFUL, redCoins >= 8, 0);
    }

    // Coin arrow pointing up
    if (coinArrow) {
        if (!omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_COINS_SPAWNED)) {
            f32 x = (f32) ((s16) ((u16) ((coinArrow >> 48llu) & 0xFFFF)));
            f32 y = (f32) ((s16) ((u16) ((coinArrow >> 32llu) & 0xFFFF)));
            f32 z = (f32) ((s16) ((u16) ((coinArrow >> 16llu) & 0xFFFF)));
            s16 a = (f32) ((s16) ((u16) ((coinArrow >>  0llu) & 0xFFFF)));
            f32 dx = coss(a);
            f32 dz = sins(a);
            spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin, x,            y +  80, z,            0, 0, 0);
            spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin, x,            y + 160, z,            0, 0, 0);
            spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin, x,            y + 240, z,            0, 0, 0);
            spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin, x,            y + 320, z,            0, 0, 0);
            spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin, x,            y + 400, z,            0, 0, 0);
            spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin, x - dx *  50, y + 350, z - dz *  50, 0, 0, 0);
            spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin, x - dx * 100, y + 300, z - dz * 100, 0, 0, 0);
            spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin, x + dx *  50, y + 350, z + dz *  50, 0, 0, 0);
            spawn_object_abs_with_rot(m->marioObj, 0, MODEL_YELLOW_COIN, bhvYellowCoin, x + dx * 100, y + 300, z + dz * 100, 0, 0, 0);
            omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_COINS_SPAWNED, 1, 0);
        }
    }
}

#undef LEVEL
#undef COIN_ARROW

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
                omm_sparkly_context_unset_flag(OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED, 1);
                omm_sparkly_context_unset_flag(OMM_SPARKLY_CONTEXT_FLAG_DIALOG_STARTED, 1);
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
    if (sOmmSparklyContext->data) {
        omm_sparkly_context_set_counter(OMM_SPARKLY_CONTEXT_COUNTER_SECRETS, obj_get_count_with_behavior(bhvHiddenStarTrigger));
        omm_sparkly_update_context_per_level(m);
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
            omm_sparkly_flags_set(OMM_SPARKLY_FLAG_NON_STOP_MODE, OMM_STARS_NON_STOP)
            gOmmStarsMode = 0;
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
    if (!omm_sparkly_is_mode_completed(omm_sparkly_get_current_mode()) && (!omm_sparkly_is_mode_selected(OMM_SPARKLY_MODE_NORMAL) || !omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_STAR_SPAWNED))) {

        // No caps
        if (((m->flags & MARIO_WING_CAP) && (gCurrLevelNum != LEVEL_TOTWC)) ||
            ((m->flags & MARIO_METAL_CAP) && (gCurrLevelNum != LEVEL_COTMC)) ||
            ((m->flags & MARIO_VANISH_CAP) && (gCurrLevelNum != LEVEL_VCUTM))) {
            omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
        }

        // No cannon
        if (m->action == ACT_IN_CANNON && !omm_sparkly_is_mode_selected(OMM_SPARKLY_MODE_NORMAL)) {
            omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
        }

        // No Vibes outside Lunatic mode
        if (omm_peach_vibe_is_active() && !omm_sparkly_is_mode_selected(OMM_SPARKLY_MODE_LUNATIC)) {
            omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
        }
    }

    // Requirements
    if (sOmmSparklyContext->data && omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_INITED)) {
        if (!omm_sparkly_is_mode_selected(OMM_SPARKLY_MODE_NORMAL) || !omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_STAR_SPAWNED)) {

            // No yellow coin
            if ((sOmmSparklyContext->data->flags & OMM_SPARKLY_DATA_FLAG_NO_YELLOW_COIN) && (
                omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_YELLOW_COINS) != 0 ||
                omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_LOST_COINS) != 0)) {
                omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
            }

            // No blue coin
            if ((sOmmSparklyContext->data->flags & OMM_SPARKLY_DATA_FLAG_NO_BLUE_COIN) &&
                omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_BLUE_COINS) != 0) {
                omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
            }

            // No red coin
            if ((sOmmSparklyContext->data->flags & OMM_SPARKLY_DATA_FLAG_NO_RED_COIN) &&
                omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_RED_COINS) != 0) {
                omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
            }

            // No mushroom 1-up
            if ((sOmmSparklyContext->data->flags & OMM_SPARKLY_DATA_FLAG_NO_1UP_MUSHROOM) &&
                omm_sparkly_context_get_counter(OMM_SPARKLY_CONTEXT_COUNTER_1UP_MUSHROOMS) != 0) {
                omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
            }

            // No red coin
            if ((sOmmSparklyContext->data->flags & OMM_SPARKLY_DATA_FLAG_NO_CAPTURE) &&
                (m->action == ACT_OMM_POSSESSION || gOmmData->mario->capture.obj != NULL)) {
                omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
            }

            // No button press
            u16 buttons = (m->controller->buttonPressed | m->controller->buttonDown);
            u16 noButtons = OMM_SPARKLY_DATA_FLAG_GET_BUTTONS(sOmmSparklyContext->data->flags);
            if (!omm_mario_is_reading(m) && (buttons & noButtons) != 0) {
                omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 1);
            }
        }
    }

    // Star
    // Unload if data is NULL or state == 2
    // Make it intangible if already spawned and state == 1
    if (!sOmmSparklyContext->data || sOmmSparklyContext->state == OMM_SPARKLY_CONTEXT_STATE_INVALID) {
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
    if (omm_sparkly_context_is_state_ok() &&
        sOmmSparklyContext->data != NULL &&
        sOmmSparklyContext->data->signD != 0) {

        // Spawn sign
        if (!obj_get_first_with_behavior(omm_bhv_sparkly_star_hint)) {
            omm_spawn_sparkly_star_hint(m->marioObj,
                omm_sparkly_get_current_mode(),
                sOmmSparklyContext->data->signX + 10,
                sOmmSparklyContext->data->signY,
                sOmmSparklyContext->data->signZ,
                sOmmSparklyContext->data->signA,
                sOmmSparklyContext->data->signD
            );
        }

        // Make objects awake after Mario finishes reading the hint sign
        else if (m->prevAction == ACT_READING_SIGN && m->interactObj && m->interactObj->behavior == omm_bhv_sparkly_star_hint) {
            omm_sparkly_context_awaken_objects(omm_bhv_rising_lava);
            omm_sparkly_context_awaken_objects(bhvYellowCoin);
            omm_sparkly_context_awaken_objects(bhvRedCoin);
            omm_sparkly_context_awaken_objects(bhvGoomba);
        }
    }
}

//
// Utils
//

s32 omm_sparkly_context_get_remaining_flames() {
    return obj_get_count_with_behavior(bhvFlame);
}

s32 omm_sparkly_context_get_remaining_boxes() {
    return omm_sparkly_context_count_objects(bhvExclamationBox, 0x00, 0, false) -
           omm_sparkly_context_count_objects(bhvExclamationBox, 0x2F, 0, false) -
           omm_sparkly_context_count_objects(bhvExclamationBox, 0x2F, 1, false) -
           omm_sparkly_context_count_objects(bhvExclamationBox, 0x2F, 2, false);
}

#endif