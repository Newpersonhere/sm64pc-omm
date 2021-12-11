#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

#define OMM_PERRY_TRAIL_NUM_SEGMENTS_MAX        16
#define OMM_PERRY_TRAIL_NUM_POINTS_PER_FRAME    4
#define OMM_PERRY_TRAIL_NUM_POINTS_MAX          (OMM_PERRY_TRAIL_NUM_SEGMENTS_MAX * OMM_PERRY_TRAIL_NUM_POINTS_PER_FRAME)

//
// Gfx data
//

static const Gfx omm_perry_trail_gfx[] = {
    gsSPClearGeometryMode(G_CULL_BOTH),
    gsSPSetGeometryMode(G_LIGHTING),
    gsDPSetCombineLERP(0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE),
    gsSPDisplayList(NULL_dl),
    gsSPSetGeometryMode(G_CULL_BACK),
    gsSPEndDisplayList(),
};

//
// Geo data
//

typedef struct {
    Gfx gfx[OMM_ARRAY_SIZE(omm_perry_trail_gfx)];
    Gfx tri[OMM_PERRY_TRAIL_NUM_POINTS_MAX * 7 + 1];
    Vtx vtx[OMM_PERRY_TRAIL_NUM_POINTS_MAX][4];
    Lights1 lightsFront;
    Lights1 lightsBack;
} OmmPerryTrailGeoData;

//
// Geo layout
//

const GeoLayout omm_geo_perry_trail[] = {
	GEO_NODE_START(),
	GEO_OPEN_NODE(),
        GEO_ASM(0, omm_geo_link_geo_data),
		GEO_DISPLAY_LIST(LAYER_TRANSPARENT, NULL),
	GEO_CLOSE_NODE(),
	GEO_END(),
};

//
// Behavior
//

static void omm_bhv_perry_trail_update() {
    struct Object *o = gCurrentObject;

    // Update previous points
    OmmPerryTrailGeoData *data = o->oGeoData;
    OMM_MEMMOV(data->vtx + OMM_PERRY_TRAIL_NUM_POINTS_PER_FRAME, data->vtx, sizeof(data->vtx[0]) * (OMM_PERRY_TRAIL_NUM_POINTS_MAX - OMM_PERRY_TRAIL_NUM_POINTS_PER_FRAME));
    for (s32 i = 0; i != OMM_PERRY_TRAIL_NUM_POINTS_MAX; ++i) {
        for (s32 j = 0; j != 4; ++j) {
            u8 da = (data->vtx[i][j].n.tc[0] + OMM_PERRY_TRAIL_NUM_SEGMENTS_MAX - 1) / OMM_PERRY_TRAIL_NUM_SEGMENTS_MAX;
            if (data->vtx[i][j].n.a < da) {
                data->vtx[i][j].n.a = 0;
            } else {
                data->vtx[i][j].n.a -= da;
            }
        }
    }

    // Add new point...
    struct Object *perry = obj_get_first_with_behavior(omm_bhv_perry);
    if (o->oAction == 0 && perry && (perry->oPerryFlags & OBJ_INT_PERRY_TRAIL)) {
        Vec3f perryDir = { 0, 110, 0 };
        vec3f_rotate_zxy(perryDir, perryDir, perry->oFaceAnglePitch, perry->oFaceAngleYaw, perry->oFaceAngleRoll);

        // Real points, not interpolated
        Vtx *vtx[4][3] = {
            { &data->vtx[0][0], &data->vtx[OMM_PERRY_TRAIL_NUM_POINTS_PER_FRAME][0], &data->vtx[OMM_PERRY_TRAIL_NUM_POINTS_PER_FRAME * 2][0] }, // Bottom-right
            { &data->vtx[0][1], &data->vtx[OMM_PERRY_TRAIL_NUM_POINTS_PER_FRAME][1], &data->vtx[OMM_PERRY_TRAIL_NUM_POINTS_PER_FRAME * 2][1] }, // Bottom-left
            { &data->vtx[0][2], &data->vtx[OMM_PERRY_TRAIL_NUM_POINTS_PER_FRAME][2], &data->vtx[OMM_PERRY_TRAIL_NUM_POINTS_PER_FRAME * 2][2] }, // Top-right
            { &data->vtx[0][3], &data->vtx[OMM_PERRY_TRAIL_NUM_POINTS_PER_FRAME][3], &data->vtx[OMM_PERRY_TRAIL_NUM_POINTS_PER_FRAME * 2][3] }, // Top-left
        };

        // Current point, left side
        vtx[1][0]->n.ob[0] = perry->oPosX;
        vtx[1][0]->n.ob[1] = perry->oPosY;
        vtx[1][0]->n.ob[2] = perry->oPosZ;
        vtx[1][0]->n.tc[0] = 0xF0 * (perry->oOpacity / 255.f);
        vtx[1][0]->n.a     = 0xF0 * (perry->oOpacity / 255.f);
        vtx[3][0]->n.ob[0] = perry->oPosX + perryDir[0] * perry->oScaleX;
        vtx[3][0]->n.ob[1] = perry->oPosY + perryDir[1] * perry->oScaleY;
        vtx[3][0]->n.ob[2] = perry->oPosZ + perryDir[2] * perry->oScaleZ;
        vtx[3][0]->n.tc[0] = 0xF0 * (perry->oOpacity / 255.f);
        vtx[3][0]->n.a     = 0xF0 * (perry->oOpacity / 255.f);

        // Current point, right side
        if (o->oTimer == 0) {
            *vtx[0][0] = *vtx[1][0];
            *vtx[2][0] = *vtx[3][0];
            *vtx[1][1] = *vtx[1][0];
            *vtx[3][1] = *vtx[3][0];
            *vtx[0][1] = *vtx[0][0];
            *vtx[2][1] = *vtx[2][0];
            *vtx[1][2] = *vtx[1][0];
            *vtx[3][2] = *vtx[3][0];
            *vtx[0][2] = *vtx[0][0];
            *vtx[2][2] = *vtx[2][0];
        } else {
            *vtx[0][0] = *vtx[1][1];
            *vtx[2][0] = *vtx[3][1];
        }

        // Interpolated points
        for (s32 i = 0; i != OMM_PERRY_TRAIL_NUM_POINTS_PER_FRAME; ++i) {
            f32 t = (f32) (i + 1) / (f32) OMM_PERRY_TRAIL_NUM_POINTS_PER_FRAME;
            Vec3f p[2];
            vec3f_interpolate(p[0], t, vtx[1][0]->n.ob, 0.f, vtx[1][1]->n.ob, 1.f, vtx[1][2]->n.ob, 2.f);
            vec3f_interpolate(p[1], t, vtx[3][0]->n.ob, 0.f, vtx[3][1]->n.ob, 1.f, vtx[3][2]->n.ob, 2.f);
            u8 alpha = omm_lerp_s(t, vtx[0][0]->n.a, vtx[0][1]->n.a);
            data->vtx[i][0].n.ob[0] = p[0][0];
            data->vtx[i][0].n.ob[1] = p[0][1];
            data->vtx[i][0].n.ob[2] = p[0][2];
            data->vtx[i][0].n.tc[0] = data->vtx[0][0].n.tc[0];
            data->vtx[i][0].n.a     = alpha;
            data->vtx[i][2].n.ob[0] = p[1][0];
            data->vtx[i][2].n.ob[1] = p[1][1];
            data->vtx[i][2].n.ob[2] = p[1][2];
            data->vtx[i][2].n.tc[0] = data->vtx[0][2].n.tc[0];
            data->vtx[i][2].n.a     = alpha;
            data->vtx[i + 1][1]     = data->vtx[i][0];
            data->vtx[i + 1][3]     = data->vtx[i][2];
        }
    }
    
    // ...or progressively unload the trail
    else {
        OMM_MEMSET(data->vtx, 0, sizeof(data->vtx[0]) * OMM_PERRY_TRAIL_NUM_POINTS_PER_FRAME);
        o->oAction = 1;
        if (o->oSubAction++ > OMM_PERRY_TRAIL_NUM_SEGMENTS_MAX) {
            obj_mark_for_deletion(o);
        }
    }

    // Triangles
    Gfx *tri = data->tri;
    for (s32 i = 0; i != OMM_PERRY_TRAIL_NUM_POINTS_MAX; ++i) {
        gSPLight(tri++, &data->lightsFront.l, 1);
        gSPLight(tri++, &data->lightsFront.a, 2);
        gSPVertex(tri++, data->vtx[i], 4, 0);
        gSPLight(tri++, &data->lightsBack.l, 1);
        gSPLight(tri++, &data->lightsBack.a, 2);
        gSPVertex(tri++, data->vtx[i], 2, 0);
        gSP2Triangles(tri++, 0, 2, 1, 0, 1, 2, 3, 0);
    }
    gSPEndDisplayList(tri);

    // Update properties
    obj_scale(o, 1.f);
    obj_set_pos(o, 0, 0, 0);
    obj_set_angle(o, 0, 0, 0);
    obj_set_always_rendered(o, true);
}

const BehaviorScript omm_bhv_perry_trail[] = {
    OBJ_TYPE_DEFAULT, // This object must be updated after omm_bhv_perry
	0x08000000,
	0x0C000000, (uintptr_t) omm_bhv_perry_trail_update,
	0x09000000,
};

//
// Spawner (auto)
//

OMM_ROUTINE_UPDATE(omm_spawn_perry_trail) {
    if (!gMarioObject || !OMM_PLAYER_IS_PEACH) {
        return;
    }

    // If no active trail exists, try to spawn one
    for_each_object_with_behavior(trail, omm_bhv_perry_trail) {
        if (trail->oAction == 0) {
            return;
        }
    }
    struct Object *perry = obj_get_first_with_behavior(omm_bhv_perry);
    if (!perry || !(perry->oPerryFlags & OBJ_INT_PERRY_TRAIL)) {
        return;
    }

    // Colors
    static const u8 sOmmPerryColors[][2][3] = {
        { { OMM_PEACH_COLOR_PERRY_FRONT      }, { OMM_PEACH_COLOR_PERRY_BACK      } },
        { { OMM_PEACH_COLOR_VIBE_JOY_FRONT   }, { OMM_PEACH_COLOR_VIBE_JOY_BACK   } },
        { { OMM_PEACH_COLOR_VIBE_RAGE_FRONT  }, { OMM_PEACH_COLOR_VIBE_RAGE_BACK  } },
        { { OMM_PEACH_COLOR_VIBE_GLOOM_FRONT }, { OMM_PEACH_COLOR_VIBE_GLOOM_BACK } },
        { { OMM_PEACH_COLOR_VIBE_CALM_FRONT  }, { OMM_PEACH_COLOR_VIBE_CALM_BACK  } },
    };
    const u8 *cf = sOmmPerryColors[gOmmData->mario->peach.vibeType][0];
    const u8 *cb = sOmmPerryColors[gOmmData->mario->peach.vibeType][1];

    // Spawn new trail
    struct Object *trail = obj_spawn_from_geo(gMarioObject, omm_geo_perry_trail, omm_bhv_perry_trail);
    OmmPerryTrailGeoData *data = omm_geo_get_geo_data(trail, sizeof(OmmPerryTrailGeoData), omm_perry_trail_gfx, sizeof(omm_perry_trail_gfx));
    data->lightsFront = (Lights1) gdSPDefLights1(cf[0], cf[1], cf[2], cf[0], cf[1], cf[2], 0x28, 0x28, 0x28);
    data->lightsBack = (Lights1) gdSPDefLights1(cb[0], cb[1], cb[2], cb[0], cb[1], cb[2], 0x28, 0x28, 0x28);
    trail->oGeoData = (void *) data;
    trail->oAction = 0;
    trail->oSubAction = 0;
}
