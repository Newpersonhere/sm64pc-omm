#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// Init
//

bool cappy_bully_init(struct Object *o) {
    if (o->oAction == BULLY_ACT_LAVA_DEATH ||
        o->oAction == BULLY_ACT_DEATH_PLANE_DEATH) {
        return false;
    }
    gOmmData->object->state.actionFlag = false;
    gOmmData->object->state.actionTimer = 0;
    return true;
}

void cappy_bully_end(struct Object *o) {
    obj_set_animation_with_accel(o, 1, 1.f);
    obj_set_vel(o, 0.f, 0.f, 0.f);
    o->oAction = BULLY_ACT_PATROL;
}

f32 cappy_bully_get_top(struct Object *o) {
    return 115.f * o->oScaleY;
}

//
// Update
//

s32 cappy_bully_update(struct Object *o) {

    // Hitbox
    o->hitboxRadius = omm_capture_get_hitbox_radius(o);
    o->hitboxHeight = omm_capture_get_hitbox_height(o);
    o->hitboxDownOffset = omm_capture_get_hitbox_down_offset(o);
    o->oWallHitboxRadius = omm_capture_get_wall_hitbox_radius(o);

    // Properties
    POBJ_SET_ABOVE_WATER;
    POBJ_SET_UNDER_WATER;
    POBJ_SET_IMMUNE_TO_LAVA;
    POBJ_SET_IMMUNE_TO_SAND;
    POBJ_SET_IMMUNE_TO_WIND;

    // Inputs
    if (!obj_update_door(o) && !omm_mario_is_locked(gMarioState)) {

        // Dash attack
        if (gOmmData->object->state.actionFlag) {
            if (gOmmData->mario->capture.stickMag < 0.1f) {
                gOmmData->mario->capture.stickYaw = o->oFaceAngleYaw;
            }
            gOmmData->mario->capture.stickMag = 1.f;
            pobj_move(o, false, true, false);
            if (gOmmData->object->state.actionTimer-- > 0) {
                POBJ_SET_IMMUNE_TO_FIRE;
                POBJ_SET_ABLE_TO_MOVE_ON_SLOPES;
                POBJ_SET_ATTACKING;
            } else {
                gOmmData->object->state.actionFlag = false;
                gOmmData->object->state.actionTimer = 15;
            }
        }
        
        // Regular movement
        else {
            pobj_move(o, false, false, false);
            if (gOmmData->object->state.actionTimer-- <= 0 && POBJ_B_BUTTON_PRESSED) {
                gOmmData->object->state.actionFlag = true;
                gOmmData->object->state.actionTimer = 60;
            } else if (pobj_jump(o, 0, 1) == POBJ_RESULT_JUMP_START) {
                obj_play_sound(o, SOUND_OBJ_GOOMBA_ALERT);
            }
        }
    }

    // Movement
    obj_update_pos_and_vel(o, true, POBJ_IS_ABLE_TO_MOVE_THROUGH_WALLS, POBJ_IS_ABLE_TO_MOVE_ON_SLOPES, obj_is_on_ground(o), &gOmmData->object->state.squishTimer);
    pobj_decelerate(o, 0.80f, 0.95f);
    pobj_apply_gravity(o, 1.f);
    pobj_handle_special_floors(o);
    POBJ_STOP_IF_UNPOSSESSED;

    // Lava must be handled separately
    if (obj_is_on_ground(o) && o->oFloor->type == SURFACE_BURNING) {
        omm_mario_unpossess_object(gMarioState, OMM_MARIO_UNPOSSESS_ACT_BURNT, false, 0);
        omm_mario_set_action(gMarioState, ACT_LAVA_BOOST, 0, 0);
        o->oAction = BULLY_ACT_LAVA_DEATH;
        POBJ_STOP_IF_UNPOSSESSED;
    }

    // Interactions
    POBJ_INTERACTIONS(

    // Doors
    obj_open_door(o, obj);

    );
    POBJ_STOP_IF_UNPOSSESSED;

    // Gfx
    obj_update_gfx(o);
    if (gOmmData->object->state.actionFlag) {
        o->header.gfx.angle[0] = 0x800;
        obj_set_animation_with_accel(o, 1, 2.f);
        if (obj_is_on_ground(o)) {
            obj_make_step_sound_and_particle(o, &gOmmData->object->state.walkDistance, omm_capture_get_walk_speed(o) * 8.f, o->oForwardVel, SOUND_OBJ_BULLY_WALKING, OBJ_STEP_PARTICLE_NONE);
            obj_make_step_sound_and_particle(o, &gOmmData->object->state.walkDistance, 0, 0, -1, OBJ_STEP_PARTICLE_MIST); o->oPosY += 60.f * o->oScaleY;
            obj_make_step_sound_and_particle(o, &gOmmData->object->state.walkDistance, 0, 0, -1, OBJ_STEP_PARTICLE_SMOKE); o->oPosY -= 60.f * o->oScaleY;
        }
    } else {
        obj_set_animation_with_accel(o, 0, omm_relerp_0_1_f(o->oForwardVel, 0.f, omm_capture_get_walk_speed(o), 0.75f, 1.5f));
        if (obj_is_on_ground(o)) {
            obj_make_step_sound_and_particle(o, &gOmmData->object->state.walkDistance, omm_capture_get_walk_speed(o) * 8.f, o->oForwardVel, SOUND_OBJ_BULLY_WALK, OBJ_STEP_PARTICLE_NONE);
        }
    }

    // Cappy values
    gOmmData->object->cappy.copyGfx   = true;
    gOmmData->object->cappy.offset[1] = 115.f;
    gOmmData->object->cappy.scale     = 1.f;

    // OK
    POBJ_RETURN_OK;
}
