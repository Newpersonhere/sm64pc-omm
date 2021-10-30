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

static s16 sVertexPool[0x20000]; // 0x8000 * 4
static s16 *sVertexBuffer = NULL;
static s16 sVertexCount = 0;

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

static struct Surface *omm_surface_create(s16 v1, s16 v2, s16 v3) {

    // Check vertex indices
    if (v1 < 0 || v1 >= sVertexCount) return NULL;
    if (v2 < 0 || v2 >= sVertexCount) return NULL;
    if (v3 < 0 || v3 >= sVertexCount) return NULL;

    // Compute vertices and normal
    f32 x1 = sVertexBuffer[4 * v1 + 1];
    f32 y1 = sVertexBuffer[4 * v1 + 2];
    f32 z1 = sVertexBuffer[4 * v1 + 3];
    f32 x2 = sVertexBuffer[4 * v2 + 1];
    f32 y2 = sVertexBuffer[4 * v2 + 2];
    f32 z2 = sVertexBuffer[4 * v2 + 3];
    f32 x3 = sVertexBuffer[4 * v3 + 1];
    f32 y3 = sVertexBuffer[4 * v3 + 2];
    f32 z3 = sVertexBuffer[4 * v3 + 3];
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
    static const s32 sCmdColSize[] = { 1, 2, 4, 3, 4, 5, 1, 1, 0, 2 };
    struct Surface properties;
    for (bool transformed = false;;) {
        s16 cmd = *data;
        switch (cmd) {

            // Initialize collision data
            case CMD_COL_INIT: {
                sVertexBuffer = NULL;
                sVertexCount = 0;
                properties.type = SURFACE_DEFAULT;
                properties.flags = 0;
                properties.force = false;
                transformed = false;
            } break;

            // Initialize vertex buffer
            case CMD_COL_VERTEX_INIT: {
                sVertexBuffer = &data[2];
                sVertexCount = 0;
                transformed = false;
            } break;

            // Increment vertex counter
            case CMD_COL_VERTEX: {
                sVertexCount++;
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
                        omm_array_add_inplace(sOmmCollisionBuffers, OmmCollisionBuffer, { data, OMM_MEMDUP(sVertexBuffer, sVertexCount * 4 * sizeof(s16)), sVertexCount });
                    }

                    // Compute the transform matrix
                    if (o->header.gfx.throwMatrix == NULL) {
                        o->header.gfx.throwMatrix = &o->transform;
                        obj_build_transform_from_pos_and_angle(o, O_POS_INDEX, O_FACE_ANGLE_INDEX);
                    }
                    Mat4 m; obj_apply_scale_to_matrix(o, m, o->transform);

                    // Transform and store in another buffer
                    for (s32 i = 0; i != sVertexCount; ++i) {
                        f32 vx = (f32) sVertexBuffer[4 * i + 1];
                        f32 vy = (f32) sVertexBuffer[4 * i + 2];
                        f32 vz = (f32) sVertexBuffer[4 * i + 3];
                        sVertexPool[4 * i + 1] = (s16) (vx * m[0][0] + vy * m[1][0] + vz * m[2][0] + m[3][0]);
                        sVertexPool[4 * i + 2] = (s16) (vx * m[0][1] + vy * m[1][1] + vz * m[2][1] + m[3][1]);
                        sVertexPool[4 * i + 3] = (s16) (vx * m[0][2] + vy * m[1][2] + vz * m[2][2] + m[3][2]);
                    }
                    sVertexBuffer = sVertexPool;
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
                struct Surface *surface = omm_surface_create(v1, v2, v3);
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
    if (sOmmCollisionBuffers) {
        for (s32 i = 0; i != omm_array_count(sOmmCollisionBuffers); ++i) {
            OmmCollisionBuffer *colBuffer = omm_array_getp(sOmmCollisionBuffers, OmmCollisionBuffer, i);
            OMM_MEMDEL(colBuffer->vertices);
        }
    }
    omm_array_delete(sOmmCollisionBuffers);
}

void load_object_collision_model() {
    struct Object *o = gCurrentObject;

    // Load and transform object collision
    if (!(gTimeStopState & TIME_STOP_ACTIVE) && !(o->activeFlags & ACTIVE_FLAG_IN_DIFFERENT_ROOM)) {
        s32 i = omm_array_find(sOmmCollisionPointers, o->collisionData);
        if (i != -1) {
            OmmCollisionBuffer *colBuffer = omm_array_getp(sOmmCollisionBuffers, OmmCollisionBuffer, i);
            sVertexBuffer = colBuffer->vertices;
            sVertexCount = colBuffer->count;
            omm_surface_process_data(gCurrAreaIndex, colBuffer->start, o, colBuffer);
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
