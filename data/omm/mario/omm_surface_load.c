#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS
#include "game/macro_special_objects.h"

OmmSurfaceHashMap gOmmStaticSurfaces;
OmmSurfaceHashMap gOmmDynamicSurfaces;
static OmmArray sOmmSurfaces = NULL;

typedef struct { s16 *start; s16 *vertices; s16 count; } OmmCollisionBuffer;
static OmmArray sOmmCollisionBuffers = NULL;
static OmmArray sOmmCollisionPointers = NULL;
static OmmArray sOmmCollisionJumps = NULL;

//
// Add
//

static void omm_surface_add_to_cell(struct Surface *surface, bool dynamic, s16 cx, s16 cz) {
    OmmArray *surfaces = NULL;
    s32 *count = NULL;

    // Floors
    if (surface->normal.y > +0.01f) {
        surfaces = (dynamic ? &gOmmDynamicSurfaces.surfaces[cz][cx].floors : &gOmmStaticSurfaces.surfaces[cz][cx].floors);
        count = (dynamic ? &gOmmDynamicSurfaces.counts[cz][cx].floors : &gOmmStaticSurfaces.counts[cz][cx].floors);
    }
    
    // Ceilings
    else if (surface->normal.y < -0.01f) {
        surfaces = (dynamic ? &gOmmDynamicSurfaces.surfaces[cz][cx].ceils : &gOmmStaticSurfaces.surfaces[cz][cx].ceils);
        count = (dynamic ? &gOmmDynamicSurfaces.counts[cz][cx].ceils : &gOmmStaticSurfaces.counts[cz][cx].ceils);
    }
    
    // Walls
    else {
        surfaces = (dynamic ? &gOmmDynamicSurfaces.surfaces[cz][cx].walls : &gOmmStaticSurfaces.surfaces[cz][cx].walls);
        count = (dynamic ? &gOmmDynamicSurfaces.counts[cz][cx].walls : &gOmmStaticSurfaces.counts[cz][cx].walls);
        surface->flags |= SURFACE_FLAG_X_PROJECTION * (omm_abs_f(surface->normal.x) > 0.707f);
    }

    // Insert surface
    omm_array_grow(*surfaces, *count + 1, struct Surface *, NULL);
    omm_array_set(*surfaces, surface, *count);
    (*count)++;
}

static void omm_surface_add(struct Surface *surface, bool dynamic) {
    s16 x0  = omm_min_3_s(surface->vertex1[0], surface->vertex2[0], surface->vertex3[0]);
    s16 z0  = omm_min_3_s(surface->vertex1[2], surface->vertex2[2], surface->vertex3[2]);
    s16 x1  = omm_max_3_s(surface->vertex1[0], surface->vertex2[0], surface->vertex3[0]);
    s16 z1  = omm_max_3_s(surface->vertex1[2], surface->vertex2[2], surface->vertex3[2]);
    s16 cx0 = omm_clamp_s(((x0 + OMM_COLLISION_LEVEL_BOUNDARY - (OMM_COLLISION_CELL_SIZE / 2)) / OMM_COLLISION_CELL_SIZE), 0, OMM_COLLISION_CELL_COUNT - 1);
    s16 cz0 = omm_clamp_s(((z0 + OMM_COLLISION_LEVEL_BOUNDARY - (OMM_COLLISION_CELL_SIZE / 2)) / OMM_COLLISION_CELL_SIZE), 0, OMM_COLLISION_CELL_COUNT - 1);
    s16 cx1 = omm_clamp_s(((x1 + OMM_COLLISION_LEVEL_BOUNDARY + (OMM_COLLISION_CELL_SIZE / 2)) / OMM_COLLISION_CELL_SIZE), 0, OMM_COLLISION_CELL_COUNT - 1);
    s16 cz1 = omm_clamp_s(((z1 + OMM_COLLISION_LEVEL_BOUNDARY + (OMM_COLLISION_CELL_SIZE / 2)) / OMM_COLLISION_CELL_SIZE), 0, OMM_COLLISION_CELL_COUNT - 1);
    for (s16 cz = cz0; cz <= cz1; cz++) {
        for (s16 cx = cx0; cx <= cx1; cx++) {
            omm_surface_add_to_cell(surface, dynamic, cx, cz);
        }
    }
}

//
// Read
//

void omm_surface_register_collision_jump(s16 index, s16 *ptr) {
    omm_array_grow(sOmmCollisionJumps, index + 1, s16 *, NULL);
    omm_array_set(sOmmCollisionJumps, ptr, index);
}

static struct Surface *omm_surface_create(s16 *vBuffer, s16 vCount, s16 *vTranslation, s16 v1, s16 v2, s16 v3) {

    // Check vertex indices
    if (v1 < 0 || v1 >= vCount) return NULL;
    if (v2 < 0 || v2 >= vCount) return NULL;
    if (v3 < 0 || v3 >= vCount) return NULL;

    // Compute vertices and normal
    f32 x1 = vBuffer[4 * v1 + 1] + vTranslation[0];
    f32 y1 = vBuffer[4 * v1 + 2] + vTranslation[1];
    f32 z1 = vBuffer[4 * v1 + 3] + vTranslation[2];
    f32 x2 = vBuffer[4 * v2 + 1] + vTranslation[0];
    f32 y2 = vBuffer[4 * v2 + 2] + vTranslation[1];
    f32 z2 = vBuffer[4 * v2 + 3] + vTranslation[2];
    f32 x3 = vBuffer[4 * v3 + 1] + vTranslation[0];
    f32 y3 = vBuffer[4 * v3 + 2] + vTranslation[1];
    f32 z3 = vBuffer[4 * v3 + 3] + vTranslation[2];
    f32 nx = (y2 - y1) * (z3 - z2) - (z2 - z1) * (y3 - y2);
    f32 ny = (z2 - z1) * (x3 - x2) - (x2 - x1) * (z3 - z2);
    f32 nz = (x2 - x1) * (y3 - y2) - (y2 - y1) * (x3 - x2);
    f32 mag = sqrtf(omm_sqr_f(nx) + omm_sqr_f(ny) + omm_sqr_f(nz));
    if (mag < 0.1f) return NULL;

    // Create surface
    omm_array_grow(sOmmSurfaces, gSurfacesAllocated + 1, struct Surface *, OMM_MEMNEW(struct Surface, 1));
    struct Surface *surface = omm_array_get(sOmmSurfaces, struct Surface *, gSurfacesAllocated++);
    surface->type           = 0;
    surface->force          = 0;
    surface->flags          = 0;
    surface->room           = 0;
    surface->lowerY         = omm_min_3_s(y1, y2, y3) - 5;
    surface->upperY         = omm_max_3_s(y1, y2, y3) + 5;
    surface->vertex1[0]     = x1;
    surface->vertex1[1]     = y1;
    surface->vertex1[2]     = z1;
    surface->vertex2[0]     = x2;
    surface->vertex2[1]     = y2;
    surface->vertex2[2]     = z2;
    surface->vertex3[0]     = x3;
    surface->vertex3[1]     = y3;
    surface->vertex3[2]     = z3;
    surface->normal.x       = nx / mag;
    surface->normal.y       = ny / mag;
    surface->normal.z       = nz / mag;
    surface->originOffset   = -(nx * x1 + ny * y1 + nz * z1) / mag;
    surface->object         = NULL;
    return surface;
}

static bool omm_surface_has_force(s16 surfaceType) {
    return surfaceType == SURFACE_0004 ||
           surfaceType == SURFACE_FLOWING_WATER ||
           surfaceType == SURFACE_HORIZONTAL_WIND ||
           surfaceType == SURFACE_MOVING_QUICKSAND ||
           surfaceType == SURFACE_DEEP_MOVING_QUICKSAND ||
           surfaceType == SURFACE_SHALLOW_MOVING_QUICKSAND ||
           surfaceType == SURFACE_INSTANT_MOVING_QUICKSAND;
}

static bool omm_surface_has_no_cam_collision(s16 surfaceType) {
    return surfaceType == SURFACE_SWITCH ||
           surfaceType == SURFACE_NO_CAM_COLLISION ||
           surfaceType == SURFACE_NO_CAM_COLLISION_77 ||
           surfaceType == SURFACE_NO_CAM_COL_VERY_SLIPPERY;
}

static void omm_surface_process_data(s16 areaIndex, s16 *data, struct Object *o, void *params) {
    static const s32 sCmdColSize[] = { 1, 2, 4, 3, 4, 5, 1, 1, 0, 2, 4, 2 };

    s16 vPool[0x20000]; // 0x8000 * 4
    s16 *vBuffer = NULL;
    s16 vCount = 0;
    Vec3s vTranslation = { 0, 0, 0 };
    struct Surface properties;

    // Optimization: for objects (dynamic surfaces), don't process the vertex data again, start directly with triangles
    if (o && params) {
        OmmCollisionBuffer *colBuffer = (OmmCollisionBuffer *) params;
        vBuffer = colBuffer->vertices;
        vCount = colBuffer->count;
        data = colBuffer->start;
    }

    // Process every command, until a CMD_COL_END is found
    for (bool transformed = false;;) {
        s16 cmd = *data;
        switch (cmd) {

            // Initialize collision data
            case CMD_COL_INIT: {
                vBuffer = NULL;
                vCount = 0;
                properties.type = SURFACE_DEFAULT;
                properties.flags = 0;
                properties.force = false;
                transformed = false;
            } break;

            // Initialize vertex buffer
            case CMD_COL_VERTEX_INIT: {
                vBuffer = &data[2];
                vCount = 0;
                transformed = false;
            } break;

            // Increment vertex counter
            case CMD_COL_VERTEX: {
                vCount++;
            } break;

            // Initialize surface data
            case CMD_COL_TRI_INIT: {
                properties.type = data[1];
                properties.flags = (SURFACE_FLAG_DYNAMIC * (o != NULL)) | (SURFACE_FLAG_NO_CAM_COLLISION * omm_surface_has_no_cam_collision(properties.type));
                properties.force = omm_surface_has_force(properties.type);
                if (o && !transformed) {

                    // Store the vertex buffer in the look-up table
                    if (!params) {
                        omm_array_init(sOmmCollisionPointers, void *);
                        omm_array_init(sOmmCollisionBuffers, OmmCollisionBuffer);
                        omm_array_add_inplace(sOmmCollisionPointers, void *, o->collisionData);
                        omm_array_add_inplace(sOmmCollisionBuffers, OmmCollisionBuffer, { data, OMM_MEMDUP(vBuffer, vCount * 4 * sizeof(s16)), vCount });
                    }

                    // Compute the transform matrix
                    if (o->header.gfx.throwMatrix == NULL) {
                        o->header.gfx.throwMatrix = &o->transform;
                        obj_build_transform_from_pos_and_angle(o, O_POS_INDEX, O_FACE_ANGLE_INDEX);
                    }
                    Mat4 m; obj_apply_scale_to_matrix(o, m, o->transform);

                    // Transform and store in another buffer
                    for (s32 i = 0; i != vCount; ++i) {
                        f32 vx = (f32) vBuffer[4 * i + 1];
                        f32 vy = (f32) vBuffer[4 * i + 2];
                        f32 vz = (f32) vBuffer[4 * i + 3];
                        vPool[4 * i + 1] = (s16) (vx * m[0][0] + vy * m[1][0] + vz * m[2][0] + m[3][0]);
                        vPool[4 * i + 2] = (s16) (vx * m[0][1] + vy * m[1][1] + vz * m[2][1] + m[3][1]);
                        vPool[4 * i + 3] = (s16) (vx * m[0][2] + vy * m[1][2] + vz * m[2][2] + m[3][2]);
                    }
                    vBuffer = vPool;
                    transformed = true;
                }
            } break;

            // Create surface
            case CMD_COL_TRI:
            case CMD_COL_TRI_SPECIAL: {

                // Room
                s8 room = 0;
                if (o) {
                    room = 5 * (o->behavior == bhvDddWarp);
                } else if (params) {
                    s8 **surfaceRooms = (s8 **) params;
                    room = *(*surfaceRooms);
                    *surfaceRooms += 1;
                }

                // Surface
                s16 v1 = data[1];
                s16 v2 = data[2];
                s16 v3 = data[3];
                struct Surface *surface = omm_surface_create(vBuffer, vCount, vTranslation, v1, v2, v3);
                if (surface != NULL) {
                    surface->object = o;
                    surface->room = room;
                    surface->type = properties.type;
                    surface->flags = properties.flags;
                    surface->force = (properties.force ? data[4] : 0);
                    omm_surface_add(surface, o != NULL);
                }
            } break;

            // Stop creating surfaces
            case CMD_COL_TRI_STOP: {
                properties.type = SURFACE_DEFAULT;
                properties.flags = 0;
                properties.force = false;
            } break;

            // End collision data
            case CMD_COL_END: {
                vec3s_set(vTranslation, 0, 0, 0);
                return;
            } break;

            // Spawn special objects
            case CMD_COL_SPECIAL_INIT: {
                data += 1;
                spawn_special_objects(areaIndex, &data);
            } break;

            // Initialize environment regions
            case CMD_COL_WATER_BOX_INIT: {
                gEnvironmentRegions = &data[1];
                s32 numRegions = data[1];
                for (s32 i = 0; i < numRegions; ++i, data += 6) {
                    if (OMM_LIKELY(i < 20)) {
                        gEnvironmentLevels[i] = data[7];
                    }
                }
            } break;

            // Translate all vertices by a fixed amount (only works with static surfaces)
            case CMD_COL_TRANSLATE: {
                if (!o) {
                    vTranslation[0] = data[1];
                    vTranslation[1] = data[2];
                    vTranslation[2] = data[3];
                }
            } break;

            // Process another collision script (only works with static surfaces)
            case CMD_COL_JUMP: {
                if (!o) {
                    s16 index = data[1];
                    omm_surface_process_data(areaIndex, omm_array_get(sOmmCollisionJumps, s16 *, index), NULL, params);
                }
            } break;
        }
        data += sCmdColSize[cmd]; 
    }
}

//
// Terrain
//

void alloc_surface_pools() {
    gCCMEnteredSlide = 0;
    reset_red_coins_collected();
}

u32 get_area_terrain_size(s16 *data) {
    static const s16 sCmdColEndByteSequence[] = { CMD_COL_END_BYTE_SEQUENCE };
    for (s16 *head = data;; head++) {
        if (*head == CMD_COL_END) {
            if (OMM_MEMCMP(head + 1, sCmdColEndByteSequence, OMM_ARRAY_SIZE(sCmdColEndByteSequence))) {
                return (u32) ((head - data) + 1 + OMM_ARRAY_SIZE(sCmdColEndByteSequence));
            }
        }
    }
    return 0;
}

void load_area_terrain(s16 index, s16 *data, s8 *surfaceRooms, s16 *macroObjects) {
    OMM_MEMSET(gOmmStaticSurfaces.counts, 0, sizeof(gOmmStaticSurfaces.counts));
    gEnvironmentRegions = NULL;
    gSurfacesAllocated = 0;

    // Load terrain
    omm_surface_process_data(index, data, NULL, surfaceRooms ? &surfaceRooms : NULL);
    gNumStaticSurfaces = gSurfacesAllocated;

    // Spawn macro objects
    if (macroObjects != NULL && *macroObjects != -1) {
        if (0 <= *macroObjects && *macroObjects < 30) {
            spawn_macro_objects_hardcoded(index, macroObjects);
        } else {
            spawn_macro_objects(index, macroObjects);
        }
    }
}

void clear_dynamic_surfaces() {
    if (!(gTimeStopState & TIME_STOP_ACTIVE)) {
        OMM_MEMSET(gOmmDynamicSurfaces.counts, 0, sizeof(gOmmDynamicSurfaces.counts));
        gSurfacesAllocated = gNumStaticSurfaces;
    }
}

//
// Object collision model
//

// Small optimization to not overload the collision buffers (and the memory)
OMM_ROUTINE_LEVEL_ENTRY(omm_clear_collision_buffers) {
    omm_array_delete(sOmmCollisionPointers);
    omm_array_for_each(sOmmCollisionBuffers, OmmCollisionBuffer, colBuffer) { OMM_MEMDEL(colBuffer->vertices); }
    omm_array_delete(sOmmCollisionBuffers);
}

void load_object_collision_model() {
    struct Object *o = gCurrentObject;

    // Load and transform object collision
    if (!(gTimeStopState & TIME_STOP_ACTIVE) && !(o->activeFlags & ACTIVE_FLAG_IN_DIFFERENT_ROOM)) {
        s32 i = omm_array_find(sOmmCollisionPointers, o->collisionData);
        if (i != -1) {
            OmmCollisionBuffer *colBuffer = omm_array_getp(sOmmCollisionBuffers, OmmCollisionBuffer, i);
            omm_surface_process_data(gCurrAreaIndex, (s16 *) o->collisionData, o, colBuffer);
        } else {
            omm_surface_process_data(gCurrAreaIndex, (s16 *) o->collisionData, o, NULL);
        }
    }
    
    // Update distances
    o->oCollisionDistance = OMM_COLLISION_LEVEL_BOUNDARY;
    o->oDrawingDistance = omm_max_f(o->oDrawingDistance, o->oCollisionDistance);
    o->oDistanceToMario = dist_between_objects(o, gMarioObject);
    o->oNodeFlags |= GRAPH_RENDER_ACTIVE;
}

#if OMM_CODE_DEBUG

//
// Surface display
//

static Gfx *omm_debug_render_surfaces() {
    static Lights1  sOmmDebugRenderSurfacesLightFloor1 = gdSPDefLights1(0x00, 0x80, 0xFF, 0x00, 0x40, 0x80, 0x28, 0x28, 0x28);
    static Lights1  sOmmDebugRenderSurfacesLightFloor2 = gdSPDefLights1(0x00, 0x40, 0x80, 0x00, 0x20, 0x40, 0x28, 0x28, 0x28);
    static Lights1  sOmmDebugRenderSurfacesLightCeil1  = gdSPDefLights1(0xFF, 0x00, 0x00, 0x80, 0x00, 0x00, 0x28, 0x28, 0x28);
    static Lights1  sOmmDebugRenderSurfacesLightCeil2  = gdSPDefLights1(0x80, 0x00, 0x00, 0x40, 0x00, 0x00, 0x28, 0x28, 0x28);
    static Lights1  sOmmDebugRenderSurfacesLightWall1  = gdSPDefLights1(0x00, 0xA0, 0x00, 0x00, 0x50, 0x00, 0x28, 0x28, 0x28);
    static Lights1  sOmmDebugRenderSurfacesLightWall2  = gdSPDefLights1(0x00, 0x50, 0x00, 0x00, 0x28, 0x00, 0x28, 0x28, 0x28);
    static OmmArray sOmmDebugRenderSurfacesDisplayList = NULL;
    static OmmArray sOmmDebugRenderSurfacesVertices    = NULL;

    // Init
    omm_array_init(sOmmDebugRenderSurfacesDisplayList, Gfx);
    omm_array_grow(sOmmDebugRenderSurfacesDisplayList, 4 + gSurfacesAllocated * 10, Gfx, { 0 });
    Gfx *gfx = omm_array_getp(sOmmDebugRenderSurfacesDisplayList, Gfx, 0);
    Gfx *head = gfx;

    // Surfaces
    gSPClearGeometryMode(head++, G_CULL_BOTH);
    gSPSetGeometryMode(head++, G_CULL_BACK | G_LIGHTING);
    gDPSetCombineLERP(head++, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE);
    for (s32 i = 0; i != gSurfacesAllocated; ++i) {
        omm_array_grow(sOmmDebugRenderSurfacesVertices, i + 1, Vtx *, OMM_MEMNEW(Vtx, 6));
        Vtx *vertices = omm_array_get(sOmmDebugRenderSurfacesVertices, Vtx *, i);
        struct Surface *surface = omm_array_get(sOmmSurfaces, struct Surface *, i);

        // Vertices
        vertices[0] = vertices[3] = (Vtx) { { { surface->vertex1[0], surface->vertex1[1], surface->vertex1[2] }, 0, { 0, 0 }, { surface->normal.x * 127.f, surface->normal.y * 127.f, surface->normal.z * 127.f, 0xFF } } };
        vertices[1] = vertices[4] = (Vtx) { { { surface->vertex2[0], surface->vertex2[1], surface->vertex2[2] }, 0, { 0, 0 }, { surface->normal.x * 127.f, surface->normal.y * 127.f, surface->normal.z * 127.f, 0xFF } } };
        vertices[2] = vertices[5] = (Vtx) { { { surface->vertex3[0], surface->vertex3[1], surface->vertex3[2] }, 0, { 0, 0 }, { surface->normal.x * 127.f, surface->normal.y * 127.f, surface->normal.z * 127.f, 0xFF } } };
        Vec3f center = {
            (vertices[0].v.ob[0] + vertices[1].v.ob[0] + vertices[2].v.ob[0]) / 3,
            (vertices[0].v.ob[1] + vertices[1].v.ob[1] + vertices[2].v.ob[1]) / 3,
            (vertices[0].v.ob[2] + vertices[1].v.ob[2] + vertices[2].v.ob[2]) / 3,
        };
        for (s32 k = 0; k != 3; ++k) {
            Vec3f dv = {
                vertices[k].v.ob[0] - center[0],
                vertices[k].v.ob[1] - center[1],
                vertices[k].v.ob[2] - center[2],
            };
            f32 border = 5.f;
            f32 length = vec3f_length(dv);
            f32 mult = omm_max_f(0.5f, 1.f - (border / length));
            vertices[k].v.ob[0] = center[0] + mult * dv[0];
            vertices[k].v.ob[1] = center[1] + mult * dv[1];
            vertices[k].v.ob[2] = center[2] + mult * dv[2];
        }
        
        // Floor
        if (surface->normal.y > +0.01f) {
            gSPLight(head++, &sOmmDebugRenderSurfacesLightFloor1.l, 1);
            gSPLight(head++, &sOmmDebugRenderSurfacesLightFloor1.a, 2);
            gSPVertex(head++, vertices, 3, 0);
            gSP1Triangle(head++, 0, 1, 2, 0);
            gSPLight(head++, &sOmmDebugRenderSurfacesLightFloor2.l, 1);
            gSPLight(head++, &sOmmDebugRenderSurfacesLightFloor2.a, 2);
            gSPVertex(head++, vertices, 6, 0);
            gSP2Triangles(head++, 3, 4, 0, 0, 0, 4, 1, 0);
            gSP2Triangles(head++, 4, 5, 1, 0, 1, 5, 2, 0);
            gSP2Triangles(head++, 5, 3, 2, 0, 2, 3, 0, 0);
        }
        
        // Ceiling
        else if (surface->normal.y < -0.01f) {
            gSPLight(head++, &sOmmDebugRenderSurfacesLightCeil1.l, 1);
            gSPLight(head++, &sOmmDebugRenderSurfacesLightCeil1.a, 2);
            gSPVertex(head++, vertices, 3, 0);
            gSP1Triangle(head++, 0, 1, 2, 0);
            gSPLight(head++, &sOmmDebugRenderSurfacesLightCeil2.l, 1);
            gSPLight(head++, &sOmmDebugRenderSurfacesLightCeil2.a, 2);
            gSPVertex(head++, vertices, 6, 0);
            gSP2Triangles(head++, 3, 4, 0, 0, 0, 4, 1, 0);
            gSP2Triangles(head++, 4, 5, 1, 0, 1, 5, 2, 0);
            gSP2Triangles(head++, 5, 3, 2, 0, 2, 3, 0, 0);
        }
        
        // Wall
        else {
            gSPLight(head++, &sOmmDebugRenderSurfacesLightWall1.l, 1);
            gSPLight(head++, &sOmmDebugRenderSurfacesLightWall1.a, 2);
            gSPVertex(head++, vertices, 3, 0);
            gSP1Triangle(head++, 0, 1, 2, 0);
            gSPLight(head++, &sOmmDebugRenderSurfacesLightWall2.l, 1);
            gSPLight(head++, &sOmmDebugRenderSurfacesLightWall2.a, 2);
            gSPVertex(head++, vertices, 6, 0);
            gSP2Triangles(head++, 3, 4, 0, 0, 0, 4, 1, 0);
            gSP2Triangles(head++, 4, 5, 1, 0, 1, 5, 2, 0);
            gSP2Triangles(head++, 5, 3, 2, 0, 2, 3, 0, 0);
        }
    }
    gSPEndDisplayList(head);
    return gfx;
}

OMM_ROUTINE_UPDATE(omm_debug_render_surfaces_update) {
    if (OMM_LIKELY(gObjectLists && gMarioObject)) {
        static Gfx sOmmDebugRenderSurfacesGfx[] = { gsSPDisplayList(NULL), gsSPEndDisplayList() };
        static const GeoLayout sOmmDebugRenderSurfacesGeo[] = { GEO_NODE_START(), GEO_OPEN_NODE(), GEO_DISPLAY_LIST(LAYER_OPAQUE, sOmmDebugRenderSurfacesGfx), GEO_CLOSE_NODE(), GEO_END() };
        static const BehaviorScript sOmmDebugRenderSurfacesBhv[] = { OBJ_TYPE_UNIMPORTANT, 0x11010001, 0x08000000, 0x09000000 };
        struct Object *o = obj_get_first_with_behavior(sOmmDebugRenderSurfacesBhv);
        if (!o && gOmmDebugSurface) {
            o = obj_spawn_from_geo(gMarioObject, sOmmDebugRenderSurfacesGeo, sOmmDebugRenderSurfacesBhv);
            obj_set_pos(o, 0, 0, 0);
            obj_set_angle(o, 0, 0, 0);
            obj_scale(o, 1.f);
            obj_set_always_rendered(o, true);
        } else if (o && !gOmmDebugSurface) {
            obj_mark_for_deletion(o);
        }
        sOmmDebugRenderSurfacesGfx->words.w1 = (uintptr_t) omm_debug_render_surfaces();
    }
}

#endif
