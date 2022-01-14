#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

static inline s32 obj_get_list_index_from_behavior(const BehaviorScript *behavior) {
    if (behavior && !(behavior[0] & 0xFF000000)) {
        return (s32) ((u32) ((behavior[0] >> 16) & 0xFF));
    }
    return OBJ_LIST_DEFAULT;
}

struct Object *obj_spawn_from_geo(struct Object *parent, const GeoLayout *geoLayout, const BehaviorScript *behavior) {
    if (OMM_LIKELY(gObjectLists)) {
        struct Object *obj = spawn_object(parent, 0, behavior);
        obj->oGraphNode = omm_geo_get_graph_node(geoLayout, true);
        return obj;
    }
    return NULL;
}

struct Object *obj_get_first(s32 list) {
    if (OMM_LIKELY(gObjectLists && list >= 0 && list < NUM_OBJ_LISTS)) {
        struct Object *head = (struct Object *) &gObjectLists[list];
        struct Object *obj = (struct Object *) head->header.next;
        if (obj != head) {
            return obj;
        }
    }
    return NULL;
}

struct Object *obj_get_next(struct Object *o, s32 list) {
    if (OMM_LIKELY(gObjectLists && o)) {
        struct Object *head = (struct Object *) &gObjectLists[list];
        struct Object *next = (struct Object *) o->header.next;
        if (next != head) {
            return next;
        }
    }
    return NULL;
}

struct Object *obj_get_first_with_behavior(const BehaviorScript *behavior) {
    if (OMM_LIKELY(behavior)) {
        for_each_object_in_list(obj, obj_get_list_index_from_behavior(behavior)) {
            if (obj->behavior == behavior && obj->activeFlags != ACTIVE_FLAG_DEACTIVATED) {
                return obj;
            }
        }
    }
    return NULL;
}

struct Object *obj_get_next_with_behavior(struct Object *o, const BehaviorScript *behavior) {
    if (OMM_LIKELY(o && behavior)) {
        s32 list = obj_get_list_index_from_behavior(behavior);
        for (struct Object *obj = obj_get_next(o, list); obj != NULL; obj = obj_get_next(obj, list)) {
            if (obj->behavior == behavior && obj->activeFlags != ACTIVE_FLAG_DEACTIVATED) {
                return obj;
            }
        }
    }
    return NULL;
}

struct Object *obj_get_first_with_behavior_and_field_s32(const BehaviorScript *behavior, s32 fieldIndex, s32 value) {
    if (OMM_LIKELY(behavior)) {
        for_each_object_in_list(obj, obj_get_list_index_from_behavior(behavior)) {
            if (obj->behavior == behavior && obj->activeFlags != ACTIVE_FLAG_DEACTIVATED && obj->OBJECT_FIELD_S32(fieldIndex) == value) {
                return obj;
            }
        }
    }
    return NULL;
}

struct Object *obj_get_first_with_behavior_and_field_f32(const BehaviorScript *behavior, s32 fieldIndex, f32 value) {
    if (OMM_LIKELY(behavior)) {
        for_each_object_in_list(obj, obj_get_list_index_from_behavior(behavior)) {
            if (obj->behavior == behavior && obj->activeFlags != ACTIVE_FLAG_DEACTIVATED && obj->OBJECT_FIELD_F32(fieldIndex) == value) {
                return obj;
            }
        }
    }
    return NULL;
}

struct Object *obj_get_nearest_with_behavior(struct Object *o, const BehaviorScript *behavior) {
    if (OMM_LIKELY(o && behavior)) {
        f32 distMin = 1e10f;
        struct Object *nearest = NULL;
        for_each_object_with_behavior(obj, behavior) {
            if (obj != o) {
                f32 dist = obj_get_distance(o, obj);
                if (dist < distMin) {
                    nearest = obj;
                    distMin = dist;
                }
            }
        }
        return nearest;
    }
    return NULL;
}

struct Object *obj_get_nearest_with_behavior_and_radius(struct Object *o, const BehaviorScript *behavior, f32 distMax) {
    if (OMM_LIKELY(o && behavior)) {
        f32 distMin = distMax;
        struct Object *nearest = NULL;
        for_each_object_with_behavior(obj, behavior) {
            if (obj != o) {
                f32 dist = obj_get_distance(o, obj);
                if (dist < distMin) {
                    nearest = obj;
                    distMin = dist;
                }
            }
        }
        return nearest;
    }
    return NULL;
}

s32 obj_get_list_index(struct Object *o) {
    return (OMM_LIKELY(o) ? obj_get_list_index_from_behavior(o->behavior) : -1);
}

s32 obj_get_count_with_behavior(const BehaviorScript *behavior) {
    if (OMM_LIKELY(behavior)) {
        s32 count = 0;
        for_each_object_with_behavior(obj, behavior) {
            count++;
        }
        return count;
    }
    return 0;
}

void obj_deactivate_all_with_behavior(const BehaviorScript *behavior) {
    struct Object *obj;
    while ((obj = obj_get_first_with_behavior(behavior)) != NULL) {
        obj_mark_for_deletion(obj);
    }
}

void obj_unload_all_with_behavior(const BehaviorScript *behavior) {
    struct Object *obj;
    while ((obj = obj_get_first_with_behavior(behavior)) != NULL) {
        unload_object(obj);
    }
}

//
// Specific object behaviors
//

// Prevent the object from falling off a ledge if the floor below is too low
void obj_safe_step(struct Object *o, s32 update) {
    o->oFloorHeight = find_floor(o->oPosX, o->oPosY, o->oPosZ, &o->oFloor);
    if (update) {
        if (o->oFloorHeight - o->oParentRelativePosY < -100.f) {
            o->oPosX = o->oParentRelativePosX;
            o->oPosZ = o->oParentRelativePosZ;
            o->oFloorHeight = find_floor(o->oPosX, o->oPosY, o->oPosZ, &o->oFloor);
            if (o->oPosY < o->oFloorHeight) {
                o->oPosY = o->oFloorHeight;
                o->oVelY *= o->oBounciness;
                if (o->oVelY > 5.f) {
                    o->oMoveFlags |= OBJ_MOVE_BOUNCE;
                } else {
                    o->oVelY = 0;
                    o->oMoveFlags |= OBJ_MOVE_ON_GROUND;
                }
            }
        }
    } else {
        o->oParentRelativePosX = o->oPosX;
        o->oParentRelativePosY = o->oFloorHeight;
        o->oParentRelativePosZ = o->oPosZ;
    }
    if (o->oVelY <= 0.f) {
        o->oIntangibleTimer = 0;
    }
}

//
// Global objects updates
//

bool cur_obj_update_behavior_func(void (*func)(void)) {
    struct Object *o = gCurrentObject;
    struct Object *mo = gMarioObject;
    struct MarioState *m = gMarioState;

// -------------------------------------------------------------------------------------------------------------------------------
// Room and collisions
// -------------------------------------------------------------------------------------------------------------------------------

    if (func == bhv_init_room ||
        func == load_object_collision_model) {
        return false;
    }

// -------------------------------------------------------------------------------------------------------------------------------
// Knockback
// -------------------------------------------------------------------------------------------------------------------------------

    if (obj_is_knocked_back(o)) {

        // Check the timestamp to not apply knockback multiple times per frame
        u16 timestamp = (u16) (o->oSubAction >> 16);
        u16 globtimer = (gGlobalTimer & 0xFFFF);
        if (timestamp != globtimer) {
            o->oTimer--;
            o->oSubAction &= 0xFFFF;
            o->oSubAction |= (globtimer << 16);
            switch (o->oAction >> 16) {

                // Bouncy knockback (mostly ground enemies)
                case 1: {
                    f32 velY = o->oVelY;
                    obj_update_pos_and_vel(o, false, false, false, false, NULL);
                    o->oVelY -= 4.f;
                    if (obj_is_on_ground(o)) {
                        o->oVelX /= 2.f;
                        o->oVelZ /= 2.f;
                        o->oVelY = omm_abs_f(velY) / 2.f;
                        o->oForwardVel = sqrtf(omm_sqr_f(o->oVelX) + omm_sqr_f(o->oVelZ));
                        if (o->oForwardVel < 4.f || o->oVelY < 4.f) {
                            o->oAction &= 0xFFFF;
                        }
                    }
                } break;

                // 1-dir knockback (mostly flying enemies)
                case 2: {
                    obj_update_pos_and_vel(o, false, false, false, false, NULL);
                    if (o->oWall) {
                        Vec3f d = { o->oVelX, 0.f, o->oVelZ };
                        Vec3f n = { o->oWall->normal.x, 0.f, o->oWall->normal.z };
                        f32 dot = vec3f_dot(d, n);
                        o->oVelX = 0.75f * (d[0] - 2.f * dot * n[0]);
                        o->oVelY = 0.75f * o->oVelY;
                        o->oVelZ = 0.75f * (d[2] - 2.f * dot * n[2]);
                        o->oWall = NULL;
                    } else {
                        o->oVelX *= 0.95f;
                        o->oVelY *= 0.95f;
                        o->oVelZ *= 0.95f;
                    }
                    f32 dv = vec3f_length(&o->oVelX);
                    if (dv < 4.f) {
                        o->oAction &= 0xFFFF;
                    }
                } break;

                // End of knockback
                default: {
                    o->oSubAction &= 0xFFFF;
                    obj_set_vel(o, 0.f, 0.f, 0.f);
                    obj_set_home(o, o->oPosX, o->oPosY, o->oPosZ);
                    obj_set_knockback(o, NULL, -1, 0, 0);
                } break;
            }
        }
        return true;
    }

// -------------------------------------------------------------------------------------------------------------------------------
// Behaviors
// -------------------------------------------------------------------------------------------------------------------------------

    // bhvFlamethrowerFlame
    // Fix flamethrower's flame hitbox
    if (o->behavior == bhvFlamethrowerFlame) {
        o->hitboxRadius = 60.f;
        o->hitboxHeight = 120.f;
        o->hitboxDownOffset = 60.f;
        return false;
    }

    // bhvExplosion
    // Fix bob-omb's explosion hitbox
    if (o->behavior == bhvExplosion) {
        o->hitboxRadius = 150.f;
        o->hitboxHeight = 200.f;
        o->hitboxDownOffset = 50.f;
        return false;
    }

    // bhvCheckerboardElevatorGroup
    // Fix VCUTM crash
    if (o->behavior == bhvCheckerboardElevatorGroup) {
        o->oGraphNode = NULL;
        return false;
    }

#if OMM_GAME_IS_SMSR
    // Kill the Big Chill Bully if he falls too low below his starting platform
    if (o->behavior == bhvBigChillBully) {
        if (o->oAction < BULLY_ACT_LAVA_DEATH && o->oPosY < o->oHomeY - 400.f) {
            o->oAction = BULLY_ACT_LAVA_DEATH;
            o->oTimer = 0;
        }
        return false;
    }

    // Give the domino its proper model
    if (o->behavior == bhvCustomSMSRFallingDomino) {
        cur_obj_set_model(MODEL_TTM_SLIDE_EXIT_PODIUM);
        return false;
    }

    // Slightly fix the racing penguin path to prevent her from going out of bounds
    if (o->behavior == bhvRacingPenguin) {
        switch (o->oAction) {
            case RACING_PENGUIN_ACT_FINISH_RACE: {
                if (o->oPosX < -5100.f && o->oPosY < -3500.f) {
                    o->oPosX = -5100.f;
                    obj_set_forward_vel(o, 0, 0.f, 0.f);
                    play_sound(SOUND_OBJ_POUNDING_LOUD, o->oCameraToObject);
                    set_camera_shake_from_point(SHAKE_POS_SMALL, o->oPosX, o->oPosY, o->oPosZ);
                }
            } break;
        }
        return false;
    }

    // Unload the bowling balls in Starlight Runway after reaching some point
    if (o->behavior == bhvBowlingBall) {
        if (gCurrLevelNum == LEVEL_RR) {
            if (o->oPosZ < -1050.f) {
                obj_destroy(o);
                return true;
            }
        }
        return false;
    }
#endif

// -------------------------------------------------------------------------------------------------------------------------------
// Update functions
// -------------------------------------------------------------------------------------------------------------------------------

    // bhv_hidden_red_coin_star_init
    // bhv_hidden_red_coin_star_loop
    // bhv_bowser_course_red_coin_star_loop
    // Flag the spawned star as a 'red coin star'
    if (func == bhv_hidden_red_coin_star_init ||
        func == bhv_hidden_red_coin_star_loop ||
        func == bhv_bowser_course_red_coin_star_loop) {
        if (!omm_stars_is_collected(o->oBehParams >> 24)) {
            func();
            if (o->activeFlags == ACTIVE_FLAG_DEACTIVATED) {
                u8 starIndex = (o->oBehParams >> 24) & 0xFF;
                for_each_until_null(const BehaviorScript *, bhv, OMM_ARRAY_OF(const BehaviorScript *) { bhvStar, bhvStarSpawnCoordinates, NULL }) {
                    for_each_object_with_behavior(star, *bhv) {
                        if (((star->oBehParams >> 24) & 0xFF) == starIndex) {
                            star->activeFlags |= ACTIVE_FLAG_RED_COIN_STAR;
                        }
                    }
                }
            }
            return true;
        }
    }

    // Stars
    // Don't spawn a star if already collected
    if (func == bhv_star_spawn_init ||
        func == bhv_star_spawn_loop ||
        func == bhv_hidden_star_init ||
        func == bhv_hidden_star_loop ||
        func == bhv_spawned_star_loop ||
        func == bhv_collect_star_init ||
        func == bhv_collect_star_loop ||
        func == bhv_hidden_red_coin_star_init ||
        func == bhv_hidden_red_coin_star_loop ||
        func == bhv_ccm_touched_star_spawn_loop ||
        func == bhv_bowser_course_red_coin_star_loop) {
        if (omm_stars_is_collected(o->oBehParams >> 24)) {
            obj_mark_for_deletion(o);
            return true;
        }
        return false;
    }

    // Mushrooms 1-up
    // Make the mushrooms act as Life-up when collected
    if (func == bhv_1up_loop ||
        func == bhv_1up_walking_loop ||
        func == bhv_1up_sliding_loop ||
        func == bhv_1up_running_away_loop ||
        func == bhv_1up_jump_on_approach_loop ||
        func == bhv_1up_hidden_loop ||
        func == bhv_1up_hidden_in_pole_loop) {
        if ((o->oNodeFlags & GRAPH_RENDER_ACTIVE) && !(o->oNodeFlags & GRAPH_RENDER_INVISIBLE) && (o->oIntangibleTimer == 0)) {
            if (obj_check_if_collided_with_object(o, gMarioObject)) {
#if OMM_GAME_IS_R96A
                // Green Demon effect
                if (Cheats.EnableCheats && Cheats.ChaosGreenDemon) {
                    play_sound(SOUND_MENU_CAMERA_BUZZ, gDefaultSoundArgs);
                    gMarioState->health = 0xFF;
                    gMarioState->healCounter = 0;
                    gMarioState->hurtCounter = 0;
                    obj_mark_for_deletion(o);
                    return true;
                }
#endif
                omm_health_life_up(gMarioState);
                obj_mark_for_deletion(o);
                return true;
            }
        }
        return false;
    }

    // bhv_flamethrower_loop
    // Activate the flamethrower as soon as it loads
    if (func == bhv_flamethrower_loop) {
        o->oDistanceToMario = 0.f;
        return false;
    }

    // bhv_red_coin_loop
    // Remove the parenting between the red coins and the star if already collected
    // Spawn the correct orange number and play the correct sound effect when collected
    if (func == bhv_red_coin_loop) {
        if (o->parentObj == NULL ||
            o->parentObj->activeFlags == ACTIVE_FLAG_DEACTIVATED || (
            o->parentObj->behavior != bhvHiddenRedCoinStar &&
            o->parentObj->behavior != bhvBowserCourseRedCoinStar)) {
            o->parentObj = NULL;
        }
        if (o->oInteractStatus & INT_STATUS_INTERACTED) {
            if (o->parentObj) {
                o->parentObj->oHiddenStarTriggerCounter++;
                omm_spawn_number(o, 1 + omm_level_get_num_red_coins(gCurrLevelNum, gCurrAreaIndex) - obj_get_count_with_behavior(bhvRedCoin));
                play_sound(SOUND_MENU_COLLECT_RED_COIN + (omm_max_s(0, o->parentObj->oHiddenStarTriggerCounter - 1) << 16), gGlobalSoundArgs);
            }
            coin_collected();
            o->oInteractStatus = 0;
            return true;
        }
        return false;
    }

    // bhv_recovery_heart_loop
    // Every full spin restores 1 Odyssey HP
    if (func == bhv_recovery_heart_loop) {
        if (o->oSpinningHeartTotalSpin + o->oAngleVelYaw >= 0x10000) {
            omm_health_heal_mario(m);
        }
        return false;
    }

    // bhv_hidden_blue_coin_loop
    // Prevents the hidden blue coin from disappearing if not collected
    if (func == bhv_hidden_blue_coin_loop) {
        switch (o->oAction) {
            case HIDDEN_BLUE_COIN_ACT_ACTIVE: {
                if (o->oTimer >= 239) {
                    o->oAction = HIDDEN_BLUE_COIN_ACT_INACTIVE;
                    o->oNodeFlags &= ~GRAPH_RENDER_INVISIBLE;
                    return true;
                }
            } break;
        }
        return false;
    }

    // bhv_blue_coin_switch_loop
    if (func == bhv_blue_coin_switch_loop) {
        switch (o->oAction) {

            // Any type of ground pound triggers blue coin switches
            case BLUE_COIN_SWITCH_ACT_IDLE: {
                o->oHomeY = o->oPosY;
                if (mo->platform == o) {
                    if (omm_mario_is_ground_pound_landing(m)) {
                        play_sound(SOUND_GENERAL_SWITCH_DOOR_OPEN, o->oCameraToObject);
                        o->oAction = BLUE_COIN_SWITCH_ACT_RECEDING;
                        o->oVelY = -20.f;
                        o->oGravity = 0.f;
                        obj_load_collision_model(o);
                        return true;
                    }
                }
            } break;

            // Change action instead of unloading after timer ends
            case BLUE_COIN_SWITCH_ACT_TICKING: {
                if (o->oTimer >= 239) {
                    play_sound(SOUND_GENERAL_SWITCH_DOOR_OPEN, o->oCameraToObject);
                    o->oAction = 3;
                    o->oPosY = o->oHomeY - 100.f;
                    o->oNodeFlags &= ~GRAPH_RENDER_INVISIBLE;
                    obj_load_collision_model(o);
                    return true;
                }
            } break;

            // Rise up again and return to idle action
            case 3: {
                o->oPosY = omm_min_f(o->oPosY + 20.f, o->oHomeY);
                obj_load_collision_model(o);
                if (o->oPosY == o->oHomeY) {
                    o->oAction = BLUE_COIN_SWITCH_ACT_IDLE;
                }
            } return true;
        }
        return false;
    }

    // bhv_purple_switch_loop
    // Any type of ground pound triggers purple switches, even underwater
    if (func == bhv_purple_switch_loop) {
        switch (o->oAction) {
            case PURPLE_SWITCH_IDLE: {
                if (mo->platform == o && omm_mario_is_ground_pound_landing(m)
                    && obj_get_horizontal_distance(o, mo) < 85.f * o->oScaleX) {
                    o->oAction = PURPLE_SWITCH_PRESSED;
                    return true;
                }
            } break;
        }
        return false;
    }

    // bhv_hidden_star_trigger_loop
    // Reveal secrets if the option is enabled
    if (func == bhv_hidden_star_trigger_loop) {
        if (OMM_EXTRAS_REVEAL_SECRETS) {
            o->oGraphNode = gLoadedGraphNodes[MODEL_PURPLE_MARBLE];
            o->oNodeFlags |= GRAPH_RENDER_BILLBOARD;
        } else {
            o->oGraphNode = NULL;
            o->oNodeFlags &= ~GRAPH_RENDER_BILLBOARD;
        }
        return false;
    }

    // bhv_exclamation_box_loop
    // Fix exclamation boxes hitbox
    if (func == bhv_exclamation_box_loop) {
        bhv_exclamation_box_loop();
        switch (o->oAction) {
            case 2: {
                o->hitboxHeight *= 2.f;
            } break;
        }
        return true;
    }

    // bhv_wooden_post_update
    // Spawn 5 coins when fully pounded
    if (func == bhv_wooden_post_update) {
        if (!(o->oBehParams & WOODEN_POST_BP_NO_COINS_MASK) && o->oWoodenPostOffsetY <= -190.f) {
            obj_spawn_loot_yellow_coins(o, 5, 20.f);
            set_object_respawn_info_bits(o, 1);
        }
        return false;
    }

    // bhv_boo_loop
    // Make the boo slowly disappear and reappear near the ground after unpossessing it
    if (func == bhv_boo_loop) {
        switch (o->oAction) {
            case 6: {
                o->oIntangibleTimer = -1;
                o->oOpacity = omm_abs_s(omm_relerp_s(o->oTimer, 0, 60, 255, -255));
                obj_scale(o, ((o->oOpacity / 255.f) * 0.4f + 0.6f) * o->oBooBaseScale);
                if (o->oTimer < 30) {
                    o->oPosY = omm_max_f(o->oPosY, find_floor(o->oPosX, o->oPosY, o->oPosZ, NULL) + 40.f);
                } else if (o->oTimer == 30) {
                    obj_drop_to_floor(o);
                    o->oPosY += 40;
                } else if (o->oTimer >= 60) {
                    o->oAction = 1;
                    o->oIntangibleTimer = 0;
                }
            } return true;
        }
        return false;
    }

    // bhv_coin_inside_boo_loop
    // Boos always carry a Blue coin, except in Castle Courtyard
    if (func == bhv_coin_inside_boo_loop) {
#if OMM_GAME_IS_SM64
        if (gCurrLevelNum == LEVEL_COURT) {
            o->oGraphNode = gLoadedGraphNodes[MODEL_YELLOW_COIN];
            obj_scale(o, 1.f);
            return false;
        }
#endif
        o->oGraphNode = gLoadedGraphNodes[MODEL_BLUE_COIN];
        obj_scale(o, 0.7f);
        return false;
    }

    // bhv_mr_i_loop
    // Mr. I instantly targets Mario if he's near him
    if (func == bhv_mr_i_loop) {
        switch (o->oAction) {
            case 0:
            case 1: {
                if (obj_get_distance(o, mo) < 300.f) {
                    s16 angleToMario = obj_get_object1_angle_yaw_to_object2(o, mo);
                    o->oFaceAngleYaw = angleToMario;
                    o->oMoveAngleYaw = angleToMario;
                    o->oAction = 2;
                    o->oMrIUnkFC = 0;
                    o->oMrIUnk100 = 0;
                    o->oMrIUnk104 = 0;
                    o->oMrIUnk108 = (s32)(random_float() * 50.f + 50.f);
                }
            } break;
        }
        return false;
    }

    // bhv_mad_piano_update
    // Make Mad Piano always tangible (for Cappy)
    if (func == bhv_mad_piano_update) {
        static struct ObjectHitbox sMadPianoHitbox = {
            /* interactType:      */ INTERACT_MR_BLIZZARD,
            /* downOffset:        */ 0,
            /* damageOrCoinValue: */ 3,
            /* health:            */ 99,
            /* numLootCoins:      */ 0,
            /* radius:            */ 200,
            /* height:            */ 150,
            /* hurtboxRadius:     */ 200,
            /* hurtboxHeight:     */ 150,
        };
        obj_set_hitbox(o, &sMadPianoHitbox);
        obj_set_home(o, o->oPosX, o->oPosY, o->oPosZ);
        return false;
    }

#if OMM_GAME_IS_R96A
    // bhv_motos_loop
    // Gets mad if weak attacked, or knocked back if strong attacked
    if (func == bhv_motos_loop) {
        s32 attackFlags = (o->oInteractStatus >> 24) & (OBJ_INT_ATTACK_STRONG | OBJ_INT_ATTACK_WEAK);
        if (attackFlags) {
            if (attackFlags & OBJ_INT_ATTACK_STRONG) {
                o->oHeldState = HELD_THROWN;
            } else {
                o->oMoveAngleYaw += 0x8000;
                o->oAction = 1;
            }
            if (m->action == ACT_GRABBED && m->usedObj == o) {
                o->oChuckyaUnk88 = 2;
            }
            o->oInteractStatus = 0;
            play_sound(SOUND_OBJ2_LARGE_BULLY_ATTACKED, o->oCameraToObject);
        }
        return false;
    }
#endif

    // bhv_ukiki_loop
    // Cage Ukiki starts its path from the nearest point instead of the first
    if (func == bhv_ukiki_loop) {
        switch (o->oBehParams2ndByte) {
            case UKIKI_CAGE: {
                obj_set_home(o, o->oPosX, o->oPosY, o->oPosZ);
                switch (o->oAction) {
                    case UKIKI_ACT_GO_TO_CAGE: {
                        switch (o->oSubAction) {
                            case UKIKI_SUB_ACT_CAGE_RUN_TO_CAGE: {
                                if (o->oPathedStartWaypoint && o->oTimer == 1) {
                                    struct Waypoint *nearest = obj_path_get_nearest_waypoint(o, o->oPathedStartWaypoint, 200.f);
                                    if (nearest) {
                                        o->oPathedPrevWaypoint = (struct Waypoint *) max((uintptr_t) o->oPathedStartWaypoint, (uintptr_t) (nearest - 1));
                                        o->oPathedPrevWaypointFlags = WAYPOINT_FLAGS_INITIALIZED;
                                    }
                                    //o->oBehParams |= 0x100;
                                }
                            } break;
                        }
                    } break;
                }
            } break;
        }
        return false;
    }

#if OMM_GAME_IS_SM64
    // bhv_yoshi_loop
    // Replaced the 100 lives by something more... surprising!
    if (func == bhv_yoshi_loop) {
        switch (o->oAction) {
            case YOSHI_ACT_GIVE_PRESENT: {
                o->oAction = YOSHI_ACT_IDLE;
                omm_spawn_problem(m->marioObj);
                omm_mario_set_action(m, (random_u16() & 1) ? ACT_HARD_FORWARD_GROUND_KB : ACT_HARD_BACKWARD_GROUND_KB, 1, 0);
                if (OMM_MOVESET_ODYSSEY) {
                    m->health = OMM_HEALTH_DEAD;
                } else {
                    m->healCounter = 0;
                    m->hurtCounter = 64;
                }
            } break;
        }
        return false;
    }
#endif

    // bhv_menu_button_loop
    // Replace the sound menu button by the character select button
    if (func == bhv_menu_button_loop) {
        if (obj_check_model(o, MODEL_MAIN_MENU_PURPLE_SOUND_BUTTON)) {
            struct Object *characterSelectButton = obj_get_first_with_behavior(omm_bhv_menu_character_select_button);
            if (characterSelectButton) {
                characterSelectButton->oPosX = o->oParentRelativePosX;
                characterSelectButton->oPosY = o->oParentRelativePosY;
                o->oPosX = 30000.f;
                o->oPosY = 30000.f;
                o->oParentRelativePosX = 30000.f;
                o->oParentRelativePosY = 30000.f;
                o->oMenuButtonOrigPosX = 30000.f;
                o->oMenuButtonOrigPosY = 30000.f;
                o->oMenuButtonScale = 0.f;
                obj_set_dormant(o, true);
                return true;
            }
        }
        return false;
    }

    // bhv_tuxies_mother_loop
    // Make Tuxie's mother chase Mario if he's possessing her baby
    if (func == bhv_tuxies_mother_loop) {
        switch (o->oAction) {
            case 2: {
                if (m->action == ACT_OMM_POSSESSION && gOmmData->mario->capture.obj->behavior == bhvUnused20E0) {
                    gOmmData->mario->capture.obj->oHeldState = HELD_HELD;
                }
            } break;
        }
        return false;
    }

    // bhv_snowmans_bottom_loop
    // Snowman's body fixes
    if (func == bhv_snowmans_bottom_loop) {
        switch (o->oAction) {

            // Fix a softlock that can happen if Mario is not
            // in the 400 units radius after starting the dialog
            case 0: {
                if (m->action == ACT_READING_NPC_DIALOG &&
                    m->usedObj == o && vec3f_dist(m->pos, &o->oPosX) > 400.f) {
                    Vec3f dv = {
                        m->pos[0] - o->oPosX,
                        m->pos[1] - o->oPosY,
                        m->pos[2] - o->oPosZ,
                    };
                    vec3f_set_mag(dv, 399.f);
                    m->pos[0] = o->oPosX + dv[0];
                    m->pos[1] = o->oPosY + dv[1];
                    m->pos[2] = o->oPosZ + dv[2];
                }
            } break;

            // Fix snowman's body yaw after reaching the head
            case 3: {
                struct Object *snowmansHead = obj_get_nearest_with_behavior(o, bhvSnowmansHead);
                if (snowmansHead) {
                    obj_set_angle(o, 0, obj_get_object1_angle_yaw_to_object2(o, snowmansHead), 0);
                }
            } break;
        }
        return false;
    }

    // bhv_sl_snowman_wind_loop
    // The big snowman always blows wind in Sparkly Stars mode
    if (func == bhv_sl_snowman_wind_loop) {
        if (omm_sparkly_is_enabled()) {
            o->oSubAction = SL_SNOWMAN_WIND_ACT_BLOWING;
        }
        return false;
    }

    // bhv_thi_tiny_island_top_loop
    // Any type of ground pound works on THI top
    if (func == bhv_thi_tiny_island_top_loop) {
        switch (o->oAction) {
            case 0: {
                if (!gTHIWaterDrained && omm_mario_is_ground_pound_landing(m) && obj_get_distance(o, mo) < 500.0f) {
                    obj_spawn_particles(o, 30, MODEL_WHITE_PARTICLE_SMALL, 0, 40, 0, 20, 40, -4, 2, 0);
                    obj_spawn_triangle_break_particles(o, OBJ_SPAWN_TRI_BREAK_PRESET_DIRT_SMALL);
                    play_sound(SOUND_GENERAL_ACTIVATE_CAP_SWITCH, o->oCameraToObject);
                    o->oNodeFlags |= GRAPH_RENDER_INVISIBLE;
                    o->oAction++;
                    return true;
                }
            } break;
        }
        return false;
    }

    // bhv_bowser_tail_anchor_loop
    // Fix Bowser's tail anchor (used by the omm_grab object)
    if (func == bhv_bowser_tail_anchor_loop) {
        if (OMM_MOVESET_ODYSSEY) {
            struct Object *bowser = o->parentObj;
            o->oPosX = bowser->oPosX - 200.f * bowser->oScaleX * sins(bowser->oFaceAngleYaw);
            o->oPosY = bowser->oPosY;
            o->oPosZ = bowser->oPosZ - 200.f * bowser->oScaleZ * coss(bowser->oFaceAngleYaw);
            o->oParentRelativePosX = 0.f;
            o->oParentRelativePosY = 0.f;
            o->oParentRelativePosZ = 0.f;
            o->oIntangibleTimer = 0;
            o->oInteractStatus = 0;
            obj_set_scale(o, bowser->oScaleX, bowser->oScaleY, bowser->oScaleZ);
            obj_reset_hitbox(o, 80, 100, 0, 0, 0, 0);
            if (o->oAction == 1) {
                o->oAction = (o->oTimer <= 30);
            } else if (bowser->oAction == 19) {
                bowser->oIntangibleTimer = -1;
                o->oAction = 0;
            }
            return true;
        }
        return false;
    }

    // bhv_bowser_shock_wave_loop
    // Fix Bowser's shockwave interaction
    if (func == bhv_bowser_shock_wave_loop) {
        o->oDistanceToMario = 0.f;
        func();
        if (m->invincTimer == 0 && m->action != ACT_SHOCKED && o->oBowserShockwaveScale > 0.f && o->oTimer < 70) {

            // Check vertical distance
            if ((m->pos[1] - o->oPosY) < 40.f &&
                (o->oPosY - m->pos[1]) < (m->marioObj->hitboxHeight + 40.f)) {

                // Check horizontal distance
                f32 hDistToMario = obj_get_horizontal_distance(o, m->marioObj) / o->oBowserShockwaveScale;
                if ((1.9f < hDistToMario && hDistToMario < 2.4f) ||
                    (4.0f < hDistToMario && hDistToMario < 4.8f)) {
                    omm_spawn_damage_mario(m->marioObj, INTERACT_SHOCK, 1);
                }
            }
        }
        return true;
    }

    // bhv_bowsers_sub_loop
    // Always spawn DDD sub and door in Non-Stop mode
    if (func == bhv_bowsers_sub_loop) {
        if (OMM_STARS_NON_STOP_NOT_ENDING_CUTSCENE) {
            obj_load_collision_model(o);
            return true;
        }
        return false;
    }

    // bhv_intro_lakitu_loop
    // Fix Lakitu's model during cutscenes
    if (func == bhv_intro_lakitu_loop) {
        o->oGraphNode = omm_geo_get_graph_node(lakitu_geo, true);
        return false;
    }

// -------------------------------------------------------------------------------------------------------------------------------
// Init functions
// -------------------------------------------------------------------------------------------------------------------------------

    // bhv_moving_yellow_coin_init
    // Halves the initial speed to reduce randomness
    if (func == bhv_moving_yellow_coin_init) {
        o->oForwardVel /= 2.f;
        return false;
    }

    // bhv_1up_init
    // Always spawn 1-up mushrooms
    if (func == bhv_1up_init) {
        bhv_1up_common_init();
        return true;
    }

    // bhv_manta_ray_water_ring_init
    // Fix manta ray's water ring collision to match its orientation
    if (func == bhv_manta_ray_water_ring_init) {
        cur_obj_init_animation(0);
        o->oWaterRingScalePhaseX = (s32) (random_float() * 0x1000) + 0x1000;
        o->oWaterRingScalePhaseY = (s32) (random_float() * 0x1000) + 0x1000;
        o->oWaterRingScalePhaseZ = (s32) (random_float() * 0x1000) + 0x1000;
        o->oWaterRingNormalX = 0.f;
        o->oWaterRingNormalY = 1.f;
        o->oWaterRingNormalZ = 0.f;
        vec3f_rotate_zxy(
            &o->oWaterRingNormalX,
            &o->oWaterRingNormalX,
            o->oFaceAnglePitch,
            o->oFaceAngleYaw,
            o->oFaceAngleRoll
        );
        o->oWaterRingMarioDistInFront = water_ring_calc_mario_dist();
        o->oOpacity = 150;
        return true;
    }

#if !OMM_GAME_IS_SMSR
    // bhv_mips_init
    // Spawns a faster Mips at 120 stars
    if (func == bhv_mips_init) {
        if (m->numStars >= 120) {
            o->oBehParams2ndByte = 1;
            o->oMipsForwardVelocity = 50.f;
            o->oInteractionSubtype = INT_SUBTYPE_HOLDABLE_NPC;
            o->oGravity = 15.f;
            o->oFriction = 0.89f;
            o->oBuoyancy = 1.2f;
            cur_obj_init_animation(0);
            return true;
        }
        return false;
    }
#endif

    // bhv_snowmans_head_init
    // Never spawn the snowman as a whole in Non-Stop mode
    if (func == bhv_snowmans_head_init) {
        if (OMM_STARS_NON_STOP_NOT_ENDING_CUTSCENE) {
            cur_obj_scale(0.7f);
            o->oGravity = 5.f;
            o->oFriction = 0.999f;
            o->oBuoyancy = 2.f;
            return true;
        }
        return false;
    }

    // bhv_ddd_pole_init
    // Always spawn DDD poles in Non-Stop mode
    if (func == bhv_ddd_pole_init) {
        if (OMM_STARS_NON_STOP_NOT_ENDING_CUTSCENE) {
            o->hitboxDownOffset = 100.f;
            o->oDDDPoleMaxOffset = 100.f * o->oBehParams2ndByte;
            return true;
        }
        return false;
    }

    // bhv_menu_button_manager_init
    // Spawn the character select button
    if (func == bhv_menu_button_manager_init) {
        struct Object *charSelButton = spawn_object_rel_with_rot(o, MODEL_NONE, omm_bhv_menu_character_select_button, 6400.f, -3500.f, 0, 0, 0, 0);
        charSelButton->oMenuButtonScale = 1.0f;
        charSelButton->oAnimState = 0;
        switch (omm_player_get_selected_index()) {
            case OMM_PLAYER_MARIO: charSelButton->oGraphNode = omm_geo_get_graph_node(omm_geo_menu_button_mario, true); break;
            case OMM_PLAYER_PEACH: charSelButton->oGraphNode = omm_geo_get_graph_node(omm_geo_menu_button_peach, true); break;
            case OMM_PLAYER_LUIGI: charSelButton->oGraphNode = omm_geo_get_graph_node(omm_geo_menu_button_luigi, true); break;
            case OMM_PLAYER_WARIO: charSelButton->oGraphNode = omm_geo_get_graph_node(omm_geo_menu_button_wario, true); break;
            default:               charSelButton->oGraphNode = omm_geo_get_graph_node(omm_geo_menu_button_mario, true); break;
        }
    }

// -------------------------------------------------------------------------------------------------------------------------------
// Other stuff
// -------------------------------------------------------------------------------------------------------------------------------

    // Thwomp collision
    // Fix the weird Thwomp collision
    if (OMM_MOVESET_ODYSSEY) {
        if (o->collisionData == thwomp_seg5_collision_0500B7D0 ||
            o->collisionData == thwomp_seg5_collision_0500B92C) {
            o->collisionData = (void *) omm_thwomp_collision;
        }
    } else {
        if (o->collisionData == omm_thwomp_collision) {
            o->collisionData = (void *) thwomp_seg5_collision_0500B7D0;
        }
    }

    // Nothing to do
    return false;
}
