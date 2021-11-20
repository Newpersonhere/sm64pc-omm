#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// Init
//

bool cappy_koopa_shell_init(struct Object *o) {
    o->behavior = omm_bhv_possessed_koopa_shell;
    gOmmData->object->state.actionFlag = false;
    return true;
}

void cappy_koopa_shell_end(struct Object *o) {
    o->behavior = bhvKoopaShell;
    stop_shell_music();
}

f32 cappy_koopa_shell_get_top(struct Object *o) {
    return 100.f * o->oScaleY;
}

//
// Update
//

s32 cappy_koopa_shell_update(struct Object *o) {

    // Init
    if (!gOmmData->object->state.actionFlag) {
        play_shell_music();
        gOmmData->object->state.actionFlag = true;
    }

    // Inputs
    if (!omm_mario_is_locked(gMarioState)) {
        pobj_move(o, false, POBJ_B_BUTTON_DOWN, false);
        if (pobj_jump(o, 0, 1) == POBJ_RESULT_JUMP_START) {
            obj_play_sound(o, SOUND_OBJ_GOOMBA_ALERT);
        }
    }

    // Hitbox
    o->hitboxRadius = omm_capture_get_hitbox_radius(o);
    o->hitboxHeight = omm_capture_get_hitbox_height(o);
    o->hitboxDownOffset = omm_capture_get_hitbox_down_offset(o);
    o->oWallHitboxRadius = omm_capture_get_wall_hitbox_radius(o);

    // Properties
    POBJ_SET_ABOVE_WATER;
    POBJ_SET_IMMUNE_TO_LAVA;
    POBJ_SET_IMMUNE_TO_SAND;
    POBJ_SET_ABLE_TO_MOVE_ON_WATER;
    POBJ_SET_ABLE_TO_MOVE_ON_SLOPES;
    POBJ_SET_ATTACKING;

    // Movement
    obj_update_pos_and_vel(o, true, POBJ_IS_ABLE_TO_MOVE_THROUGH_WALLS, POBJ_IS_ABLE_TO_MOVE_ON_SLOPES, obj_is_on_ground(o), &gOmmData->object->state.squishTimer);
    pobj_decelerate(o, 0.80f, 0.95f);
    pobj_apply_gravity(o, 1.f);
    pobj_handle_special_floors(o);
    POBJ_STOP_IF_UNPOSSESSED;

    // Interactions
    POBJ_INTERACTIONS();
    POBJ_STOP_IF_UNPOSSESSED;

    // Gfx
    obj_update_gfx(o);
    o->header.gfx.angle[0] = 0;
    o->header.gfx.angle[1] = o->oTimer * 0x2000;
    o->header.gfx.angle[2] = 0;
    spawn_object(o, MODEL_NONE, bhvSparkleSpawn);

    // Sound effect
    switch (o->oFloorType) {
        case OBJ_FLOOR_TYPE_GROUND:
            obj_make_step_sound_and_particle(o,
                &gOmmData->object->state.walkDistance, 0.f, 0.f,
                SOUND_MOVING_TERRAIN_RIDING_SHELL + gMarioState->terrainSoundAddend,
                OBJ_STEP_PARTICLE_MIST);
            break;

        case OBJ_FLOOR_TYPE_WATER:
            obj_make_step_sound_and_particle(o,
                &gOmmData->object->state.walkDistance, 0.f, 0.f,
                SOUND_MOVING_TERRAIN_RIDING_SHELL + SOUND_TERRAIN_WATER,
                OBJ_STEP_PARTICLE_WATER_TRAIL | OBJ_STEP_PARTICLE_WATER_DROPLET);
            break;

        case OBJ_FLOOR_TYPE_LAVA:
            obj_make_step_sound_and_particle(o,
                &gOmmData->object->state.walkDistance, 0.f, 0.f,
                SOUND_MOVING_RIDING_SHELL_LAVA,
                OBJ_STEP_PARTICLE_FLAME);
            break;
    }

    // Cappy values
    gOmmData->object->cappy.scale = 0.f;

    // OK
    POBJ_RETURN_OK;
}
