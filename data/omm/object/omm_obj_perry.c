#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// Behavior
//

static void omm_bhv_perry_update() {
    struct MarioState *m = gMarioState;
    struct Object *o = gCurrentObject;

    // Compute Peach's body values
    Vec3f peachArm, peachWrist, peachHand, peachDelta; Vec3s peachAngle;
    vec3f_copy(peachArm, omm_geo_get_marios_forearm_pos(false));
    vec3f_copy(peachWrist, omm_geo_get_marios_hand_pos(false));
    vec3f_dif(peachDelta, peachWrist, peachArm);
    vec3s_set(peachAngle, atan2s(sqrtf(omm_sqr_f(peachDelta[0]) + omm_sqr_f(peachDelta[2])), peachDelta[1]) - 0x4000, atan2s(peachDelta[2], peachDelta[0]) + 0x8000, 0);
    vec3f_copy(peachHand, peachDelta);
    vec3f_mul(peachHand, 8.f / vec3f_length(peachDelta));
    vec3f_mult(peachHand, peachHand, &m->marioObj->oScaleX);
    vec3f_add(peachHand, peachWrist);

    // Determine attack type
    u32 attackFlags = 0;
    switch (gOmmData->mario->peach.vibeType) {
        case OMM_PEACH_VIBE_TYPE_RAGE:  attackFlags = OBJ_INT_PRESET_PERRY_ATTACK_RAGE; break;
        case OMM_PEACH_VIBE_TYPE_GLOOM: attackFlags = OBJ_INT_PRESET_PERRY_ATTACK_GLOOM; break;
        default:                        attackFlags = OBJ_INT_PRESET_PERRY_ATTACK; break;
    }

    // Compute Perry's attributes based on Peach's current action
    f32 perryScale = 0.f;
    u32 perryFlags = 0;
    switch (m->action) {

        // Glide
        case ACT_OMM_PEACH_GLIDE: {
            vec3s_set(peachAngle, 0, m->faceAngle[1] + 0x4000, 0);
            perryScale = omm_min_f(m->actionTimer / 6.f, 1.f);
            perryFlags = OBJ_INT_PRESET_PERRY_COLLECT | OBJ_INT_PERRY_OPEN;
        } break;

        // Attack (ground)
        case ACT_OMM_PEACH_ATTACK_GROUND: {
            perryScale = omm_clamp_0_1_f((m->actionTimer - 2) / 4.f);
            perryFlags = attackFlags | (OBJ_INT_GRAB_OBJECTS * (m->actionArg < 4)) | (OBJ_INT_PERRY_TRAIL * (m->actionState == 2));
        } break;

        // Attack (fast)
        case ACT_OMM_PEACH_ATTACK_FAST: {
            perryScale = omm_clamp_0_1_f((m->actionTimer - 2) / 4.f);
            perryFlags = attackFlags | OBJ_INT_PERRY_TRAIL;
        } break;

        // Attack (air)
        case ACT_OMM_PEACH_ATTACK_AIR: {
            if (m->flags & MARIO_KICKING) {
                perryScale = omm_clamp_0_1_f((m->actionTimer - 2) / 4.f);
                perryFlags = attackFlags | OBJ_INT_PERRY_TRAIL;
            }
        } break;
    }

    // Update Perry's position, angle, scale, opacity and more
    obj_set_pos(o, peachHand[0], peachHand[1], peachHand[2]);
    obj_set_angle(o, peachAngle[0], peachAngle[1], peachAngle[2]);
    obj_scale_xyz(o, perryScale * m->marioObj->oScaleX, perryScale * m->marioObj->oScaleY, perryScale * m->marioObj->oScaleZ);
    obj_set_params(o, 0, 0, 0, 0, 0);
    obj_reset_hitbox(o, 0, 0, 0, 0, 0, 0);
    o->oAction = ((perryFlags & OBJ_INT_PERRY_OPEN) != 0);
    o->oOpacity = (perryScale != 0.f) * ((m->marioBodyState->modelState & 0x100) ? (m->marioBodyState->modelState & 0xFF) : 0xFF) * !(m->marioObj->oNodeFlags & GRAPH_RENDER_INVISIBLE);
    o->oPerryFlags = perryFlags;

    // Hitbox and interactions
    if (perryScale > 0.f) {

        // Update Perry's hitbox
        Vec3f d = { 0, OMM_PEACH_PERRY_HITBOX, 0 };
        vec3f_rotate_zxy(d, d, o->oFaceAnglePitch, o->oFaceAngleYaw, o->oFaceAngleRoll);
        obj_set_pos(o, o->oPosX + d[0] * o->oScaleX / 2.f, o->oPosY + d[1] * o->oScaleY / 2.f, o->oPosZ + d[2] * o->oScaleZ / 2.f);
        obj_set_params(o, 0, 0, 0, 0, true);
        obj_reset_hitbox(o, OMM_PEACH_PERRY_HITBOX / 2.f, OMM_PEACH_PERRY_HITBOX, 0, 0, 0, OMM_PEACH_PERRY_HITBOX / 2.f);

        // Process interactions
        struct Object *interacted = omm_obj_process_interactions(o, o->oPerryFlags);
        if (interacted && !omm_obj_is_collectible(interacted)) {
            play_sound(SOUND_ACTION_BOUNCE_OFF_OBJECT, interacted->oCameraToObject);
        }

        // Reset values
        obj_set_pos(o, peachHand[0], peachHand[1], peachHand[2]);
        obj_set_params(o, 0, 0, 0, 0, false);
        obj_reset_hitbox(o, 0, 0, 0, 0, 0, 0);
    }

    // Preprocess
    omm_geo_preprocess_object_graph_node(o);
}

const BehaviorScript omm_bhv_perry[] = {
    OBJ_TYPE_ONE_WAY_INTERACTION, // This object must be updated before omm_bhv_perry_trail
    0x11010001,
    0x08000000,
    0x0C000000, (uintptr_t) omm_bhv_perry_update,
    0x09000000,
};

//
// Spawner (auto)
//

OMM_ROUTINE_UPDATE(omm_spawn_perry) {
    if (gMarioObject) {
        if (OMM_PLAYER_IS_PEACH) {
            struct Object *perry = obj_get_first_with_behavior(omm_bhv_perry);
            if (!perry) {
                obj_spawn_from_geo(gMarioObject, omm_geo_perry, omm_bhv_perry);
            }
        } else {
            obj_deactivate_all_with_behavior(omm_bhv_perry);
            obj_deactivate_all_with_behavior(omm_bhv_perry_trail);
            obj_deactivate_all_with_behavior(omm_bhv_perry_shockwave);
        }
    }
}
