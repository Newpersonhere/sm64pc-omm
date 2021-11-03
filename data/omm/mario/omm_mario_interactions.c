#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

static bool is_object_destructible(struct Object *o) {
    return
        (o->behavior == bhvBowlingBall) ||
        (o->behavior == bhvPitBowlingBall) ||
        (o->behavior == bhvFreeBowlingBall) ||
        (o->behavior == bhvMrBlizzard) ||
        (o->behavior == bhvSnowmansBottom) ||
        (o->behavior == bhvBulletBill) ||
        (o->behavior == bhvHauntedChair) ||
        (o->behavior == bhvJrbSlidingBox) ||
        (o->behavior == bhvMrI) ||
        (o->behavior == bhvSpiny) ||
        (o->behavior == bhvBigBoulder) ||
        (o->behavior == bhvMrIParticle) ||
        (o->behavior == bhvMadPiano) ||
        (o->behavior == bhvMontyMoleRock) ||
        (o->behavior == bhvMrBlizzardSnowball) ||
        (o->behavior == bhvWaterBomb);
}

static void omm_mario_destroy_or_push_away_object(struct MarioState *m, struct Object *o) {
    
    // Destroy
    if (is_object_destructible(o)) {
        obj_destroy(o);
    }
    
    // Push away
    else {
        f32 x = m->pos[0];
        f32 z = m->pos[2];
        push_mario_out_of_object(m, o, 0);
        o->oPosX -= (m->pos[0] - x);
        o->oPosZ -= (m->pos[2] - z);
        o->oInteractStatus = INT_STATUS_INTERACTED;
        m->pos[0] = x;
        m->pos[2] = z;
    }
}

//
// Preprocess interactions
// Interactions types that are preprocessed here are not processed in mario_process_interactions
//

static bool sMarioIsPunching = false;
static bool sMarioIsKicking  = false;
static bool sMarioIsTripping = false;
u32 determine_interaction(struct MarioState *m, struct Object *o) {
    u32 interaction = 0;

    // Bounces
    if (m->action & ACT_FLAG_AIR) {

        // From above
        if (m->vel[1] < 0.f) {
            if (m->pos[1] > o->oPosY) {
                interaction = INT_HIT_FROM_ABOVE;
            }
        }
        
        // From below
        else {
            if (m->pos[1] < o->oPosY) {
                interaction = INT_HIT_FROM_BELOW;
            }
        }
    }

    // Attacks
    if (m->action & ACT_FLAG_ATTACKING) {

        // Punching / Kicking
        if (omm_mario_is_punching(m) || omm_mario_is_kicking(m)) {
            s16 dYawToObject = mario_obj_angle_to_object(m, o) - m->faceAngle[1];
            if (sMarioIsPunching && omm_abs_s(dYawToObject) <= 0x2AAA) {
                interaction = INT_PUNCH;
            }
            if (sMarioIsKicking && omm_abs_s(dYawToObject) <= 0x2AAA) {
                interaction = INT_KICK;
            }
            if (sMarioIsTripping && omm_abs_s(dYawToObject) <= 0x4000) {
                interaction = INT_TRIP;
            }
        }
        
        // Ground pounding
        else if (omm_mario_is_ground_pounding(m) || m->action == ACT_TWIRLING) {
            if (m->vel[1] < 0.f) {
                interaction = INT_GROUND_POUND_OR_TWIRL;
            }
        }
        
        // Landing
        else if (omm_mario_is_ground_pound_landing(m) || m->action == ACT_TWIRL_LAND) {
            if (m->vel[1] < 0.f && m->actionState == 0) {
                interaction = INT_GROUND_POUND_OR_TWIRL;
            }
        }
        
        // Slide kicking
        else if (m->action == ACT_SLIDE_KICK || m->action == ACT_SLIDE_KICK_SLIDE) {
            interaction = INT_SLIDE_KICK;
        }
        
        // Riding shell
        else if (m->action & ACT_FLAG_RIDING_SHELL) {
            interaction = INT_FAST_ATTACK_OR_SHELL;
        }
        
        // Going fast?
        else if (omm_abs_f(m->forwardVel) >= 26.f) {
            interaction = INT_FAST_ATTACK_OR_SHELL;
        }
    }

    return interaction;
}

bool omm_mario_interact_coin(struct MarioState *m, struct Object *o) {
#if OMM_CODE_SPARKLY
    if (o->respawnInfoType == OMM_OBJECT_LOST_COIN) omm_sparkly_context_update(OMM_SPARKLY_CONTEXT_UPDATE_LOST_COIN);
    else if (o->oDamageOrCoinValue == 1) omm_sparkly_context_update(OMM_SPARKLY_CONTEXT_UPDATE_YELLOW_COIN);
    else if (o->oDamageOrCoinValue == 5) omm_sparkly_context_update(OMM_SPARKLY_CONTEXT_UPDATE_BLUE_COIN);
    else if (o->oDamageOrCoinValue == 2) omm_sparkly_context_update(OMM_SPARKLY_CONTEXT_UPDATE_RED_COIN);
#endif
    interact_coin(m, INTERACT_COIN, o);
    return true;
}

bool omm_mario_interact_star_or_key(struct MarioState *m, struct Object *o) {
    if (m->health > OMM_HEALTH_DEAD) {

        // Collect star or key
        omm_sparkly_interact_grand_star(m, o);
        save_file_collect_star_or_key(m->numCoins, (o->oBehParams >> 24) & 0x1F);
        m->numStars = save_file_get_total_star_count(gCurrSaveFileNum - 1, COURSE_MIN - 1, COURSE_MAX - 1);

        // Update Mario
        mario_stop_riding_and_holding(m);
        update_mario_sound_and_camera(m);
        play_sound(SOUND_MENU_STAR_SOUND, m->marioObj->oCameraToObject);
        spawn_object(o, MODEL_NONE, bhvStarKeyCollectionPuffSpawner);
        o->oInteractStatus = INT_STATUS_INTERACTED;
        m->interactObj = o;
        m->usedObj = o;

        // Grand star?
        // Some rom-hacks replace the jumbo star cutscene by a warp to a 'we saved the world' cutscene level
        if (o->behavior == bhvGrandStar || (o->oInteractionSubtype & INT_SUBTYPE_GRAND_STAR)) {
            if (o->oInteractType & INTERACT_WARP) {
                interact_warp(m, INTERACT_WARP, o);
            } else {
                omm_mario_set_action(m, ACT_JUMBO_STAR_CUTSCENE, 0, 0xFFFF); 
            }
            return true;
        }

        // Mario exits the course if:
        // - All stars have been collected, or
        // - It's not a noExit star and Non-Stop mode is disabled
        bool noExit = !OMM_ALL_STARS && (OMM_STARS_NON_STOP || (o->oInteractionSubtype & INT_SUBTYPE_NO_EXIT));
        if (!noExit) {
            m->hurtCounter = 0;
            m->healCounter = 0;
            m->capTimer = omm_min_s(m->capTimer, 1);
            drop_queued_background_music();
            fadeout_level_music(126);
        } else if (m->action == ACT_OMM_POSSESSION) {
            omm_mario_lock_star_grab(m);
            return true;
        }

        // Action
        u32 starGrabAction = (noExit ? ACT_STAR_DANCE_NO_EXIT : ACT_STAR_DANCE_EXIT);
        if (m->action & ACT_FLAG_SWIMMING) starGrabAction = ACT_STAR_DANCE_WATER;
        if (m->action & ACT_FLAG_METAL_WATER) starGrabAction = ACT_STAR_DANCE_WATER;
        if (m->action & ACT_FLAG_AIR) starGrabAction = ACT_FALL_AFTER_STAR_GRAB;
        omm_mario_unpossess_object(m, OMM_MARIO_UNPOSSESS_ACT_NONE, false, 0);
        omm_mario_set_action(m, starGrabAction, noExit, 0);
        return true;
    }
    return false;
}

static bool omm_mario_interact_whirlpool(struct MarioState *m, UNUSED struct Object *o) {
    return omm_mario_has_metal_cap(m);
}

static bool omm_mario_interact_strong_wind(struct MarioState *m, UNUSED struct Object *o) {
    return omm_mario_has_metal_cap(m);
}

static bool omm_mario_interact_flame(struct MarioState *m, UNUSED struct Object *o) {
    if (omm_peach_vibe_is_gloom() && omm_obj_is_flame(o)) {
        obj_spawn_white_puff(o, SOUND_GENERAL_FLAME_OUT);
        obj_mark_for_deletion(o);
        return true;
    }
    return false;
}

static bool omm_mario_interact_clam_or_bubba(struct MarioState *m, UNUSED struct Object *o) {
    return omm_mario_has_metal_cap(m);
}

static bool omm_mario_interact_shock(struct MarioState *m, UNUSED struct Object *o) {
    return omm_mario_has_metal_cap(m);
}

static bool omm_mario_interact_mr_blizzard(struct MarioState *m, struct Object *o) {
    if (omm_mario_has_metal_cap(m)) {
        omm_mario_destroy_or_push_away_object(m, o);
        return true;
    }
    return false;
}

static bool omm_mario_interact_unknown_08(struct MarioState *m, struct Object *o) {
    if (omm_mario_has_metal_cap(m)) {
        omm_mario_destroy_or_push_away_object(m, o);
        return true;
    }
    return false;
}

static bool omm_mario_interact_damage(struct MarioState *m, struct Object *o) {
    if (omm_mario_has_metal_cap(m)) {
        omm_mario_destroy_or_push_away_object(m, o);
        return true;
    }
    return false;
}

// flag, duration, music
static const u32 sOmmCapValues[][3] = {
    { MARIO_NORMAL_CAP, 0, 0 },
    { MARIO_VANISH_CAP, OMM_IMPROVED_VANISH_CAP_DURATION, SEQUENCE_ARGS(4, SEQ_EVENT_POWERUP) },
    { MARIO_METAL_CAP, OMM_IMPROVED_METAL_CAP_DURATION, SEQUENCE_ARGS(4, SEQ_EVENT_METAL_CAP) },
    { MARIO_WING_CAP, OMM_IMPROVED_WING_CAP_DURATION, SEQUENCE_ARGS(4, SEQ_EVENT_POWERUP) },
};

static s32 get_cap_index(struct Object *o) {
    if (o->behavior == bhvNormalCap) return 0;
    if (o->behavior == bhvVanishCap) return 1;
    if (o->behavior == bhvMetalCap)  return 2;
    if (o->behavior == bhvWingCap)   return 3;
    return -1;
}

bool omm_mario_interact_cap(struct MarioState *m, struct Object *o) {
    if (omm_peach_vibe_is_active()) {
        return true;
    }
    s32 cap = get_cap_index(o);
    if ((cap != -1) && OMM_POWER_UPS_IMPROVED && (m->action != ACT_OMM_POSSESSION) && (m->action != ACT_GETTING_BLOWN)) {
        m->interactObj = o;
        m->flags &= ~MARIO_CAP_ON_HEAD & ~MARIO_CAP_IN_HAND;
        m->flags |= sOmmCapValues[cap][0];
        m->capTimer = omm_max_s(m->capTimer, sOmmCapValues[cap][1]);
        if (sOmmCapValues[cap][2]) play_cap_music(sOmmCapValues[cap][2]);
        play_sound(SOUND_MENU_STAR_SOUND, m->marioObj->oCameraToObject);
        play_sound(SOUND_MARIO_HERE_WE_GO, m->marioObj->oCameraToObject);
        o->oInteractStatus = INT_STATUS_INTERACTED;

        if ((m->action & ACT_FLAG_IDLE) || (m->action == ACT_WALKING)) {
            m->flags |= MARIO_CAP_IN_HAND;
            omm_mario_set_action(m, ACT_PUTTING_ON_CAP, 0, 0);
        } else {
            m->flags |= MARIO_CAP_ON_HEAD;
        }
        return true;
    }
    return false;
}

static bool omm_mario_interact_pole(UNUSED struct MarioState *m, UNUSED struct Object *o) {
    return (gOmmData->mario->state.poleTimer-- > 0);
}

static bool omm_mario_interact_grabbable(struct MarioState *m, struct Object *o) {
    if ((o->oInteractionSubtype & INT_SUBTYPE_GRABS_MARIO) && obj_is_object1_facing_object2(o, m->marioObj, 0x3000)) {
        return omm_mario_has_metal_cap(m);
    }
    return false;
}

struct OmmInteractionHandler { u32 interactType; bool (*handler)(struct MarioState *, struct Object *); };
static const struct OmmInteractionHandler sOmmInteractionHandlers[] = {
    { INTERACT_COIN,            omm_mario_interact_coin },
    { INTERACT_STAR_OR_KEY,     omm_mario_interact_star_or_key },
    { INTERACT_WHIRLPOOL,       omm_mario_interact_whirlpool },
    { INTERACT_STRONG_WIND,     omm_mario_interact_strong_wind },
    { INTERACT_FLAME,           omm_mario_interact_flame },
    { INTERACT_CLAM_OR_BUBBA,   omm_mario_interact_clam_or_bubba },
    { INTERACT_SHOCK,           omm_mario_interact_shock },
    { INTERACT_MR_BLIZZARD,     omm_mario_interact_mr_blizzard },
    { INTERACT_UNKNOWN_08,      omm_mario_interact_unknown_08 },
    { INTERACT_DAMAGE,          omm_mario_interact_damage },
    { INTERACT_CAP,             omm_mario_interact_cap },
    { INTERACT_POLE,            omm_mario_interact_pole },
    { INTERACT_GRABBABLE,       omm_mario_interact_grabbable },
    { INTERACT_UNKNOWN_31,      omm_sparkly_interact_star }
};
static const s32 sOmmInteractionHandlerCount = sizeof(sOmmInteractionHandlers) / sizeof(sOmmInteractionHandlers[0]);

void omm_mario_preprocess_interactions(struct MarioState *m) {
    if (!(m->action & ACT_FLAG_INTANGIBLE)) {
        for (s32 i = 0; i < sOmmInteractionHandlerCount; i++) {
            u32 interactType = sOmmInteractionHandlers[i].interactType;
            if (m->collidedObjInteractTypes & interactType) {
                struct Object *o = mario_get_collided_object(m, interactType);
                if (!(o->oInteractStatus & INT_STATUS_INTERACTED) && sOmmInteractionHandlers[i].handler(m, o)) {
                    m->collidedObjInteractTypes &= ~interactType;
                }
            }
        }
    }

    // Save the attack flags, and remove them from Mario when jump-kicking if Odyssey Moveset is enabled
    sMarioIsPunching = (m->flags & MARIO_PUNCHING);
    sMarioIsKicking  = (m->flags & MARIO_KICKING);
    sMarioIsTripping = (m->flags & MARIO_TRIPPING);
    if (OMM_MOVESET_ODYSSEY && omm_mario_is_kicking(m)) {
        m->flags &= ~(MARIO_PUNCHING | MARIO_KICKING | MARIO_TRIPPING);
    }
}
