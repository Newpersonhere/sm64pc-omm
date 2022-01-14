#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

#define OMM_PEACH_PERRY_SHOCKWAVE_WAVE_NUM_POINTS   16
#define OMM_PEACH_PERRY_SHOCKWAVE_WAVE_RADIUS       100.f
#define OMM_PEACH_PERRY_SHOCKWAVE_WAVE_SEGMENTS     (OMM_ARRAY_OF(f32) { 0.f, 0.9f, 1.f })
#define OMM_PEACH_PERRY_SHOCKWAVE_GLOW_NUM_POINTS   16
#define OMM_PEACH_PERRY_SHOCKWAVE_GLOW_RADIUS       128.f
#define OMM_PEACH_PERRY_SHOCKWAVE_GLOW_SEGMENTS     (OMM_ARRAY_OF(f32) { 0.f, 0.25f, 1.f })

//
// Gfx data
//

static const Gfx omm_perry_shockwave_gfx[] = {
    gsSPClearGeometryMode(G_CULL_BOTH | G_LIGHTING),
    gsDPSetCombineLERP(TEXEL0, 0, SHADE, 0, TEXEL0, 0, SHADE, 0, TEXEL0, 0, SHADE, 0, TEXEL0, 0, SHADE, 0),
    gsDPLoadTextureBlock(OMM_TEXTURE_EFFECT_PERRY_SHOCKWAVE, G_IM_FMT_RGBA, G_IM_SIZ_32b, 128, 128, 0, 0, 0, 0, 0, 0, 0),
    gsSPTexture(0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON),
    gsSPDisplayList(NULL_dl),
    gsSPSetGeometryMode(G_CULL_BACK | G_LIGHTING),
    gsSPEndDisplayList(),
};

//
// Geo data
//

typedef struct {
    Gfx gfx[OMM_ARRAY_SIZE(omm_perry_shockwave_gfx)];
    Gfx tri[4 * (OMM_PEACH_PERRY_SHOCKWAVE_WAVE_NUM_POINTS + OMM_PEACH_PERRY_SHOCKWAVE_GLOW_NUM_POINTS) + 3];
    Vtx vtx[8 * (OMM_PEACH_PERRY_SHOCKWAVE_WAVE_NUM_POINTS + OMM_PEACH_PERRY_SHOCKWAVE_GLOW_NUM_POINTS)];
} OmmPeachPerryShockwaveGeoData;

//
// Geo layout
//

const GeoLayout omm_geo_perry_shockwave[] = {
    GEO_NODE_START(),
    GEO_OPEN_NODE(),
        GEO_OPEN_NODE(),
            GEO_ASM(0, omm_geo_link_geo_data),
            GEO_DISPLAY_LIST(LAYER_TRANSPARENT, NULL),
        GEO_CLOSE_NODE(),
    GEO_CLOSE_NODE(),
    GEO_END(),
};

//
// Behavior
//

static void omm_bhv_perry_shockwave_explode(struct Object *o, f32 x, f32 y, f32 z) {
    o->oPosX = x;
    o->oPosY = y;
    o->oPosZ = z;
    o->oTimer = 0;
    o->oAction = 2;
}

static void omm_bhv_perry_shockwave_perform_step(struct Object *o) {
    f32 halfRadius = OMM_PEACH_PERRY_SHOCKWAVE_HITBOX_RADIUS / 2.f;
    f32 halfHeight = OMM_PEACH_PERRY_SHOCKWAVE_HITBOX_HEIGHT / 2.f;
    for (s32 i = 0; i != 8; ++i) {
        o->oFloor = NULL;
        o->oPosX += o->oVelX / 8.f;
        o->oPosZ += o->oVelZ / 8.f;

        // Wall collision
        struct WallCollisionData colData = { .x = o->oPosX, .y = o->oPosY, .z = o->oPosZ, .offsetY = 0, .radius = halfRadius };
        if (find_wall_collisions(&colData)) {
            omm_bhv_perry_shockwave_explode(o, colData.x, colData.y, colData.z);
            return;
        }

        // Floor collision
        struct Surface *floor;
        f32 floorY = find_floor(o->oPosX, o->oPosY, o->oPosZ, &floor);
        if (o->oPosY < floorY + halfHeight) {
            if (floor && floor->normal.y >= 0.95f) {
                o->oPosY = floorY + halfHeight;
                o->oFloor = floor;
            } else if (o->oPosY < floorY) {
                omm_bhv_perry_shockwave_explode(o, o->oPosX, floorY + halfHeight, o->oPosZ);
                return;
            }
        }

        // Ceil collision
        struct Surface *ceil;
        f32 ceilY = find_ceil(o->oPosX, o->oPosY, o->oPosZ, &ceil);
        if (ceilY > floorY) {
            if (o->oPosY > ceilY - halfHeight) {
                if (ceil && ceil->normal.y <= -0.95f) {
                    o->oPosY = ceilY - halfHeight;
                    o->oFloor = ceil;
                } else if (o->oPosY > ceilY) {
                    omm_bhv_perry_shockwave_explode(o, o->oPosX, ceilY - halfHeight, o->oPosZ);
                    return;
                }
            }
        }
    }
}

static void omm_bhv_perry_shockwave_process_interactions(struct Object *o) {

    // Update hitbox
    f32 maxScale = OMM_PEACH_PERRY_SHOCKWAVE_HITBOX_RADIUS / 100.f;
    obj_reset_hitbox(o,
        OMM_PEACH_PERRY_SHOCKWAVE_HITBOX_RADIUS / maxScale,
        OMM_PEACH_PERRY_SHOCKWAVE_HITBOX_HEIGHT / maxScale, 0, 0, 0,
        OMM_PEACH_PERRY_SHOCKWAVE_HITBOX_HEIGHT / (maxScale * 2.f)
    );

    // Handle interactions
    struct Object *interacted = NULL;
    switch (gOmmData->mario->peach.vibeType) {
        case OMM_PEACH_VIBE_TYPE_RAGE:  interacted = omm_obj_process_interactions(o, OBJ_INT_PRESET_PERRY_SHOCKWAVE_RAGE);  break;
        case OMM_PEACH_VIBE_TYPE_GLOOM:              omm_obj_process_interactions(o, OBJ_INT_PRESET_PERRY_SHOCKWAVE_GLOOM); break; // Gloom shockwaves go through objects
        default:                        interacted = omm_obj_process_interactions(o, OBJ_INT_PRESET_PERRY_SHOCKWAVE);       break;
    }
    if (interacted && !omm_obj_is_collectible(interacted)) {
        if (omm_peach_vibe_is_calm() && omm_obj_is_weak(interacted)) {
            omm_health_heal_mario(gMarioState);
            gMarioState->healCounter += 4;
        }
        omm_bhv_perry_shockwave_explode(o, o->oPosX, o->oPosY, o->oPosZ);
    }
}

static void omm_bhv_perry_shockwave_update() {
    struct Object *o = gCurrentObject;
    s32 cwMult = (o->oPerryShockwaveClockwise ? -1 : +1);

    // Actions
    switch (o->oAction) {

        // Grows during delay frames
        case 0: {
            f32 t = omm_invlerp_0_1_s(o->oTimer, 0, o->oPerryShockwaveDelay);
            obj_scale(o, t * OMM_PEACH_PERRY_SHOCKWAVE_HITBOX_RADIUS / 100.f);
            o->oOpacity = t * 0xFF;
            o->oPosX = gMarioState->pos[0];
            o->oPosY = gMarioState->pos[1] + 80.f * gMarioObject->oScaleY;
            o->oPosZ = gMarioState->pos[2];
            if (t == 1.f) {
                o->oAction = 1;
                o->oTimer = 0;
                o->oVelX = OMM_PEACH_PERRY_SHOCKWAVE_FORWARD_VEL * sins(gMarioState->faceAngle[1]);
                o->oVelZ = OMM_PEACH_PERRY_SHOCKWAVE_FORWARD_VEL * coss(gMarioState->faceAngle[1]);
            }
        } break;

        // Moves and interacts
        case 1: {
            obj_scale(o, OMM_PEACH_PERRY_SHOCKWAVE_HITBOX_RADIUS / 100.f);
            o->oOpacity = 0xFF;
            omm_bhv_perry_shockwave_perform_step(o);
            omm_bhv_perry_shockwave_process_interactions(o);
            if (o->oTimer >= OMM_PEACH_PERRY_SHOCKWAVE_DURATION) {
                omm_bhv_perry_shockwave_explode(o, o->oPosX, o->oPosY, o->oPosZ);
            }
        } break;

        // Vanishes and unloads
        case 2: {
            obj_scale(o, omm_relerp_0_1_f(o->oTimer, 0, 15, 1.f, 3.f) * OMM_PEACH_PERRY_SHOCKWAVE_HITBOX_RADIUS / 100.f);
            o->oOpacity = omm_clamp_s(omm_invlerp_0_1_s(o->oTimer, 15, 0) * 0x140, 0x00, 0xFF);
            o->oIntangibleTimer = -1;
            if (o->oTimer >= 15) {
                obj_mark_for_deletion(o);
            }
        } break;
    }

    // Update gfx
    obj_set_angle(o, 0, 0, 0);
    o->oPerryShockwaveAngleYaw += 0x1800 * cwMult;
    Vec3f up = { 0.f, 1.f, 0.f };
    if (o->oFloor) {
        f32 d = (o->oFloor->normal.y < 0.f ? -1.f : +1.f);
        up[0] = (o->oFloor->normal.x * d);
        up[1] = (o->oFloor->normal.y * d);
        up[2] = (o->oFloor->normal.z * d);
    }
    Vec3f n, e1, e2;
    vec3f_get_nullspace(n, e1, e2, up);

    // Data
    static u8 sShockwaveColorByType[][3][3] = {
        { { 0xFF, 0xFF, 0xFF }, { OMM_PEACH_COLOR_PERRY_FRONT      }, { OMM_PEACH_COLOR_PERRY_BACK      } },
        { { 0xFF, 0xFF, 0xFF }, { OMM_PEACH_COLOR_VIBE_JOY_FRONT   }, { OMM_PEACH_COLOR_VIBE_JOY_BACK   } },
        { { 0xFF, 0xFF, 0xFF }, { OMM_PEACH_COLOR_VIBE_RAGE_FRONT  }, { OMM_PEACH_COLOR_VIBE_RAGE_BACK  } },
        { { 0xFF, 0xFF, 0xFF }, { OMM_PEACH_COLOR_VIBE_GLOOM_FRONT }, { OMM_PEACH_COLOR_VIBE_GLOOM_BACK } },
        { { 0xFF, 0xFF, 0xFF }, { OMM_PEACH_COLOR_VIBE_CALM_FRONT  }, { OMM_PEACH_COLOR_VIBE_CALM_BACK  } },
    };
    OmmPeachPerryShockwaveGeoData *data = omm_geo_get_geo_data(o, sizeof(OmmPeachPerryShockwaveGeoData), omm_perry_shockwave_gfx, sizeof(omm_perry_shockwave_gfx));
    Vtx *vtx = data->vtx;
    Gfx *tri = data->tri;

    // Wave
    for (s32 j = 0; j != 2; ++j) {
        f32 t0 = OMM_PEACH_PERRY_SHOCKWAVE_WAVE_SEGMENTS[j + 0];
        f32 t1 = OMM_PEACH_PERRY_SHOCKWAVE_WAVE_SEGMENTS[j + 1];

        // Radius
        f32 r0 = OMM_PEACH_PERRY_SHOCKWAVE_WAVE_RADIUS * t0;
        f32 r1 = OMM_PEACH_PERRY_SHOCKWAVE_WAVE_RADIUS * t1;

        // Color
        u8 *c0 = sShockwaveColorByType[o->oPerryShockwaveType][2 - (j + 0)];
        u8 *c1 = sShockwaveColorByType[o->oPerryShockwaveType][2 - (j + 1)];

        // Points
        for (s32 i = 0; i != OMM_PEACH_PERRY_SHOCKWAVE_WAVE_NUM_POINTS; vtx += 4, ++i) {
            s16 a0 = ((i + 0) * 0x10000) / OMM_PEACH_PERRY_SHOCKWAVE_WAVE_NUM_POINTS;
            s16 a1 = ((i + 1) * 0x10000) / OMM_PEACH_PERRY_SHOCKWAVE_WAVE_NUM_POINTS;

            // Vertex coords
            Vec3f p0, p1;
            vec3f_rotate_around_n(p0, e1, up, a0 + o->oPerryShockwaveAngleYaw);
            vec3f_rotate_around_n(p1, e1, up, a1 + o->oPerryShockwaveAngleYaw);

            // Tex coords
            s32 u00 = (sins(a0) * t0 * 64 + 64) * 32;
            s32 v00 = (coss(a0) * t0 * 64 + 64) * 32;
            s32 u01 = (sins(a1) * t0 * 64 + 64) * 32;
            s32 v01 = (coss(a1) * t0 * 64 + 64) * 32;
            s32 u10 = (sins(a0) * t1 * 64 + 64) * 32;
            s32 v10 = (coss(a0) * t1 * 64 + 64) * 32;
            s32 u11 = (sins(a1) * t1 * 64 + 64) * 32;
            s32 v11 = (coss(a1) * t1 * 64 + 64) * 32;

            // Triangles
            gSPVertex(tri++, vtx, 4, 0);
            gSP2Triangles(tri++, 0, 1, 2, 0, 2, 1, 3, 0);

            // Vertices
            vtx[0] = (Vtx) { { { r0 * p0[0], r0 * p0[1], r0 * p0[2] }, 0, { u00, v00 * cwMult }, { c0[0], c0[1], c0[2], o->oOpacity * (o->oAction < 2) } } };
            vtx[1] = (Vtx) { { { r0 * p1[0], r0 * p1[1], r0 * p1[2] }, 0, { u01, v01 * cwMult }, { c0[0], c0[1], c0[2], o->oOpacity * (o->oAction < 2) } } };
            vtx[2] = (Vtx) { { { r1 * p0[0], r1 * p0[1], r1 * p0[2] }, 0, { u10, v10 * cwMult }, { c1[0], c1[1], c1[2], o->oOpacity * (o->oAction < 2) } } };
            vtx[3] = (Vtx) { { { r1 * p1[0], r1 * p1[1], r1 * p1[2] }, 0, { u11, v11 * cwMult }, { c1[0], c1[1], c1[2], o->oOpacity * (o->oAction < 2) } } };
        }
    }

    // Billboard plane
    Vec3f camN, camE1, camE2;
    vec3f_dif(camN, gCamera->pos, &o->oPosX);
    vec3f_get_nullspace(camN, camE1, camE2, camN);
    Vec3f xyzScale = { 1.f, 1.f / (o->oAction < 2 ? 3.f : 1.f), 1.f };

    // Glow
    gSPTexture(tri++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
    gDPSetCombineLERP(tri++, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE);
    for (s32 j = 0; j != 2; ++j) {
        f32 t0 = OMM_PEACH_PERRY_SHOCKWAVE_GLOW_SEGMENTS[j + 0];
        f32 t1 = OMM_PEACH_PERRY_SHOCKWAVE_GLOW_SEGMENTS[j + 1];

        // Radius
        f32 r0 = OMM_PEACH_PERRY_SHOCKWAVE_GLOW_RADIUS * t0;
        f32 r1 = OMM_PEACH_PERRY_SHOCKWAVE_GLOW_RADIUS * t1;

        // Color
        u8 *c0 = sShockwaveColorByType[o->oPerryShockwaveType][j + 0];
        u8 *c1 = sShockwaveColorByType[o->oPerryShockwaveType][j + 1];

        // Points
        for (s32 i = 0; i != OMM_PEACH_PERRY_SHOCKWAVE_GLOW_NUM_POINTS; vtx += 4, ++i) {
            s16 a0 = ((i + 0) * 0x10000) / OMM_PEACH_PERRY_SHOCKWAVE_GLOW_NUM_POINTS;
            s16 a1 = ((i + 1) * 0x10000) / OMM_PEACH_PERRY_SHOCKWAVE_GLOW_NUM_POINTS;

            // Vertex coords
            f32 x0 = sins(a0);
            f32 y0 = coss(a0);
            f32 x1 = sins(a1);
            f32 y1 = coss(a1);

            // Triangles
            gSPVertex(tri++, vtx, 4, 0);
            gSP2Triangles(tri++, 0, 1, 2, 0, 2, 1, 3, 0);

            // Vertices
            vtx[0] = (Vtx) { { { 0, 0, 0 }, 0, { 0, 0 }, { c0[0], c0[1], c0[2], o->oOpacity * (1.f - t0) } } };
            vtx[1] = (Vtx) { { { 0, 0, 0 }, 0, { 0, 0 }, { c0[0], c0[1], c0[2], o->oOpacity * (1.f - t0) } } };
            vtx[2] = (Vtx) { { { 0, 0, 0 }, 0, { 0, 0 }, { c1[0], c1[1], c1[2], o->oOpacity * (1.f - t1) } } };
            vtx[3] = (Vtx) { { { 0, 0, 0 }, 0, { 0, 0 }, { c1[0], c1[1], c1[2], o->oOpacity * (1.f - t1) } } };
            vec2f_to_3d_plane(vtx[0].v.ob, (Vec2f) { r0 * x0, r0 * y0 }, gVec3fZero, camE1, xyzScale, camE2, xyzScale);
            vec2f_to_3d_plane(vtx[1].v.ob, (Vec2f) { r0 * x1, r0 * y1 }, gVec3fZero, camE1, xyzScale, camE2, xyzScale);
            vec2f_to_3d_plane(vtx[2].v.ob, (Vec2f) { r1 * x0, r1 * y0 }, gVec3fZero, camE1, xyzScale, camE2, xyzScale);
            vec2f_to_3d_plane(vtx[3].v.ob, (Vec2f) { r1 * x1, r1 * y1 }, gVec3fZero, camE1, xyzScale, camE2, xyzScale);
        }
    }
    gSPEndDisplayList(tri);
}

const BehaviorScript omm_bhv_perry_shockwave[] = {
    OBJ_TYPE_SPECIAL,
    0x11010001,
    0x08000000,
    0x0C000000, (uintptr_t) omm_bhv_perry_shockwave_update,
    0x09000000,
};

//
// Spawner
//

struct Object *omm_spawn_perry_shockwave(struct Object *o, s32 delay, s32 type, bool clockwise) {
    struct Object *wave = obj_spawn_from_geo(o, omm_geo_perry_shockwave, omm_bhv_perry_shockwave);
    obj_set_always_rendered(wave, true);
    obj_set_angle(wave, 0, 0, 0);
    obj_set_params(wave, 0, 0, 0, 0, true);
    obj_reset_hitbox(wave, 0, 0, 0, 0, 0, 0);
    wave->oPerryShockwaveType = type;
    wave->oPerryShockwaveDelay = omm_max_s(1, delay);
    wave->oPerryShockwaveAngleYaw = gMarioState->faceAngle[1] + 0x2000 * (delay - 7 + 2 * clockwise);
    wave->oPerryShockwaveClockwise = clockwise;
    return wave;
}
