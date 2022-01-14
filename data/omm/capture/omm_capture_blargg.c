#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// Init
//

bool cappy_blargg_init(struct Object *o) {
    gOmmData->object->state.actionState = 0;
    return true;
}

void cappy_blargg_end(struct Object *o) {
    o->oAction = 0;
    o->oInteractStatus = 0;
}

f32 cappy_blargg_get_top(struct Object *o) {
    return 100.f * o->oScaleY;
}

//
// Update
//

s32 cappy_blargg_update(struct Object *o) {

    // Hitbox
    o->hitboxRadius = omm_capture_get_hitbox_radius(o);
    o->hitboxHeight = omm_capture_get_hitbox_height(o);
    o->hitboxDownOffset = omm_capture_get_hitbox_down_offset(o);
    o->oWallHitboxRadius = omm_capture_get_wall_hitbox_radius(o);

    // Properties
    POBJ_SET_ABOVE_WATER;
    POBJ_SET_INVULNERABLE;
    POBJ_SET_IMMUNE_TO_FIRE;
    POBJ_SET_IMMUNE_TO_LAVA;
    POBJ_SET_IMMUNE_TO_SAND;
    POBJ_SET_IMMUNE_TO_WIND;
    POBJ_SET_ATTACKING;

    // Inputs
    if (!omm_mario_is_locked(gMarioState)) {
        gOmmData->object->state.actionState = 0;
        pobj_move(o, false, false, false);
        if (POBJ_B_BUTTON_PRESSED) {
            gOmmData->object->state.actionState = 1;
            omm_mario_lock(gMarioState, -1);
        }
    }

    // Movement
    Vec3f previousPos = { o->oPosX, o->oPosY, o->oPosZ };
    obj_update_pos_and_vel(o, true, POBJ_IS_ABLE_TO_MOVE_THROUGH_WALLS, POBJ_IS_ABLE_TO_MOVE_ON_SLOPES, obj_is_on_ground(o), &gOmmData->object->state.squishTimer);
    if (!obj_is_on_ground(o) || o->oFloor->type != SURFACE_BURNING) {
        vec3f_copy(&o->oPosX, previousPos);
        obj_drop_to_floor(o);
    }
    pobj_decelerate(o, 0.80f, 0.95f);
    pobj_handle_special_floors(o);
    POBJ_STOP_IF_UNPOSSESSED;

    // Interactions
    POBJ_INTERACTIONS();
    POBJ_STOP_IF_UNPOSSESSED;

    // Gfx
    obj_update_gfx(o);
    obj_make_step_sound_and_particle(o,
        &gOmmData->object->state.walkDistance, 0.f, 0.f,
        -1, OBJ_STEP_PARTICLE_FLAME
    );

    // Animation
    switch (gOmmData->object->state.actionState) {

        // Idle/Moving
        case 0: {
            obj_set_animation_with_accel(o, 0, 1.f);
        } break;

        // Throw fire ball
        case 1: {
            obj_set_animation_with_accel(o, 1, 1.f);
            if (o->header.gfx.mAnimInfo.animFrame == 12) {
                obj_play_sound(o, SOUND_OBJ_FLAME_BLOWN | 0xFF00);
                omm_spawn_blargg_fire_ball(o);
            } else if (obj_is_anim_at_end(o)) {
                gOmmData->object->state.actionState = 0;
                omm_mario_unlock(gMarioState);
            }
        } break;
    }
    
    // Cappy values
    gOmmData->object->cappy.offset[0] = -6.f;
    gOmmData->object->cappy.offset[1] = 96.f + 10.f * sins(((o->header.gfx.mAnimInfo.animFrame - 6) * 0x10000) / 30);
    gOmmData->object->cappy.offset[2] = 88.f;
    gOmmData->object->cappy.angle[0]  = -0x1400;
    gOmmData->object->cappy.scale     = 1.f * (gOmmData->object->state.actionState != 1);

    // OK
    POBJ_RETURN_OK;
}
