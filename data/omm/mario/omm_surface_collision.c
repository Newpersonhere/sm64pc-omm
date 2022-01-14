#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

s32 gFindFloorForCutsceneStar = 0;
u8 gInterpolatingSurfaces;

// Enhanced surface collision system
// Enabled by setting Moveset to Odyssey
// Adds several fixes and increases accuracy:
// - Treats PUs as out of bounds
// - Fixes unreferenced wall glitch, and returns the closest wall in the direction of Mario
// - Checks floor on a square instead of a point, to prevent objects to fall inside small gaps
// - Increases number of cells checked for more precise wall collisions

//
// Walls
//

static bool check_wall_extension(struct Surface *surf, struct WallCollisionData *data) {

#if OMM_GAME_IS_SM64
    // Extended volcano walls
    // The purpose is to extend the volcano walls height from 5200 to 8000 to prevent
    // Mario from wall-jumping behind them when the rising lava object is spawned
    if (gCurrLevelNum == LEVEL_LLL && gCurrAreaIndex == 2 && surf->upperY > 5200) {
        surf->upperY = 8000;
        if (surf->vertex1[1] > 5200) surf->vertex1[1] = 8000;
        if (surf->vertex2[1] > 5200) surf->vertex2[1] = 8000;
        if (surf->vertex3[1] > 5200) surf->vertex3[1] = 8000;
        return (data->y > 5200.f);
    }

    // Extended docks walls
    // The purpose is to extend the docks walls height from 2500 to 5000
    // to prevent Mario from falling behind them
    if (gCurrLevelNum == LEVEL_DDD && gCurrAreaIndex == 2 && surf->upperY > 2500) {
        surf->upperY = 5000;
        if (surf->vertex1[1] > 2500) surf->vertex1[1] = 5000;
        if (surf->vertex2[1] > 2500) surf->vertex2[1] = 5000;
        if (surf->vertex3[1] > 2500) surf->vertex3[1] = 5000;
        return (data->y > 2500.f);
    }
#else
    OMM_UNUSED(surf);
    OMM_UNUSED(data);
#endif

    return false;
}

static s32 find_wall_collisions_from_list(struct Surface **head, s32 count, struct WallCollisionData *data) {
    s32 collisions = 0;
    for (s32 i = 0, isWallExtended = 0; i < count; head++, i++, isWallExtended = 0) {
        struct Surface *surf = *head;
        f32 x = data->x;
        f32 y = data->y + data->offsetY;
        f32 z = data->z;

        // Check wall extensions
        isWallExtended = check_wall_extension(surf, data);

        // Height check optimization
        if (y < surf->lowerY || y > surf->upperY) {
            continue;
        }

        // Offset check optimization
        f32 offset = surf->normal.x * x + surf->normal.y * y + surf->normal.z * z + surf->originOffset;
        if (offset < -data->radius || offset > data->radius) {
            continue;
        }

        // Camera collision
        if (gCheckingSurfaceCollisionsForCamera) {
            if (surf->flags & SURFACE_FLAG_NO_CAM_COLLISION) {
                continue;
            }
        }

        // Object collision
        else {
            if ((surf->type == SURFACE_CAMERA_BOUNDARY) ||
                (surf->type == SURFACE_VANISH_CAP_WALLS && gCurrentObject == gMarioObject && (gMarioState->flags & MARIO_VANISH_CAP)) ||
                (surf->type == SURFACE_VANISH_CAP_WALLS && gCurrentObject != NULL && (gCurrentObject->activeFlags & ACTIVE_FLAG_MOVE_THROUGH_GRATE))) {
                continue;
            }
        }

        // Project on X axis
        if (surf->flags & SURFACE_FLAG_X_PROJECTION) {
            f32 w1 = -surf->vertex1[2];
            f32 w2 = -surf->vertex2[2];
            f32 w3 = -surf->vertex3[2];
            f32 y1 = +surf->vertex1[1];
            f32 y2 = +surf->vertex2[1];
            f32 y3 = +surf->vertex3[1];
            if (surf->normal.x > 0.f) {
                if (((y1 - y) * (w2 - w1) - (w1 - -z) * (y2 - y1) > 0.f) ||
                    ((y2 - y) * (w3 - w2) - (w2 - -z) * (y3 - y2) > 0.f) ||
                    ((y3 - y) * (w1 - w3) - (w3 - -z) * (y1 - y3) > 0.f)) {
                    continue;
                }
            } else {
                if (((y1 - y) * (w2 - w1) - (w1 - -z) * (y2 - y1) < 0.f) ||
                    ((y2 - y) * (w3 - w2) - (w2 - -z) * (y3 - y2) < 0.f) ||
                    ((y3 - y) * (w1 - w3) - (w3 - -z) * (y1 - y3) < 0.f)) {
                    continue;
                }
            }
        }
        
        // Project on Z axis
        else {
            f32 w1 = surf->vertex1[0];
            f32 w2 = surf->vertex2[0];
            f32 w3 = surf->vertex3[0];
            f32 y1 = surf->vertex1[1];
            f32 y2 = surf->vertex2[1];
            f32 y3 = surf->vertex3[1];
            if (surf->normal.z > 0.f) {
                if (((y1 - y) * (w2 - w1) - (w1 - x) * (y2 - y1) > 0.f) ||
                    ((y2 - y) * (w3 - w2) - (w2 - x) * (y3 - y2) > 0.f) ||
                    ((y3 - y) * (w1 - w3) - (w3 - x) * (y1 - y3) > 0.f)) {
                    continue;
                }
            } else {
                if (((y1 - y) * (w2 - w1) - (w1 - x) * (y2 - y1) < 0.f) ||
                    ((y2 - y) * (w3 - w2) - (w2 - x) * (y3 - y2) < 0.f) ||
                    ((y3 - y) * (w1 - w3) - (w3 - x) * (y1 - y3) < 0.f)) {
                    continue;
                }
            }
        }

        // Apply collision
        // Register the wall only if it's not an extended wall
        data->x += surf->normal.x * (data->radius - offset);
        data->z += surf->normal.z * (data->radius - offset);
        if (!isWallExtended) {
            if (data->numWalls < 4) {
                if (OMM_COLLISION_FIX_UNREFERENCED_WALLS) {
                    s16 sAngle = atan2s(surf->normal.z, surf->normal.x);
                    for (s16 i = 0; i <= data->numWalls; ++i) {
                        if (i == data->numWalls) {
                            data->walls[data->numWalls++] = surf;
                            break;
                        } else {
                            struct Surface *w = data->walls[i];
                            s16 wAngle = atan2s(w->normal.z, w->normal.x);
                            u16 dAngle = (u16) omm_abs_s((s16) (sAngle - wAngle));
                            if (dAngle < 0x1000) {
                                break;
                            }
                        }
                    }
                } else {
                    data->walls[data->numWalls++] = surf;
                }
            }
            collisions++;
        }
    }
    return collisions;
}

s32 find_wall_collisions(struct WallCollisionData *data) {
    s32 collisions = 0;
    data->numWalls = 0;

#if OMM_GAME_IS_R96A
    if (cheats_no_bounds(gMarioState)) {
        return 0;
    }
#endif

    // CCM Racing penguin
    // His radius is bigger than 200, but not capping it at 200 breaks its path
#if !OMM_GAME_IS_SMSR
    if (gCurrentObject && gCurrentObject->behavior == bhvRacingPenguin) {
        data->radius = omm_min_f(data->radius, 200.f);
    }
#endif

    // Check PUs
    if (OMM_COLLISION_CHECK_PUS) {
        if (data->x < -OMM_COLLISION_LEVEL_BOUNDARY || data->x >= +OMM_COLLISION_LEVEL_BOUNDARY ||
            data->z < -OMM_COLLISION_LEVEL_BOUNDARY || data->z >= +OMM_COLLISION_LEVEL_BOUNDARY) {
            return 0;
        }
    }

    // Check cells
    s16 cx0 = ((s16) ((data->x + OMM_COLLISION_LEVEL_BOUNDARY) / OMM_COLLISION_CELL_SIZE)) & OMM_COLLISION_CELL_BITS;
    s16 cz0 = ((s16) ((data->z + OMM_COLLISION_LEVEL_BOUNDARY) / OMM_COLLISION_CELL_SIZE)) & OMM_COLLISION_CELL_BITS;
    for (s16 dz = -(s16) OMM_COLLISION_CHECK_NEIGHBOR_CELLS; dz <= +(s16) OMM_COLLISION_CHECK_NEIGHBOR_CELLS; ++dz)
    for (s16 dx = -(s16) OMM_COLLISION_CHECK_NEIGHBOR_CELLS; dx <= +(s16) OMM_COLLISION_CHECK_NEIGHBOR_CELLS; ++dx) {
        s16 cx = cx0 + dx;
        s16 cz = cz0 + dz;
        if (cx < 0 || cx >= OMM_COLLISION_CELL_COUNT ||
            cz < 0 || cz >= OMM_COLLISION_CELL_COUNT) {
            continue;
        }
        collisions += find_wall_collisions_from_list((struct Surface **) omm_array_data(gOmmDynamicSurfaces.surfaces[cz][cx].walls), gOmmDynamicSurfaces.counts[cz][cx].walls, data);
        collisions += find_wall_collisions_from_list((struct Surface **) omm_array_data(gOmmStaticSurfaces.surfaces[cz][cx].walls), gOmmStaticSurfaces.counts[cz][cx].walls, data);
    }
    return collisions;
}

struct Surface *resolve_and_return_wall_collisions(Vec3f pos, f32 offsetY, f32 radius) {
    struct WallCollisionData data;
    data.x = pos[0];
    data.y = pos[1];
    data.z = pos[2];
    data.radius = radius;
    data.offsetY = offsetY;

    struct Surface *wRef = NULL;
    if (find_wall_collisions(&data)) {
        if (OMM_COLLISION_FIX_UNREFERENCED_WALLS) {
            s16 yaw = gMarioState->faceAngle[1];
            for (u16 i = 0, mAngle = 0xFFFF; i < (u16) data.numWalls; i++) {
                struct Surface *w = data.walls[i];
                s16 wAngle = 0x8000 + atan2s(w->normal.z, w->normal.x);
                u16 dAngle = (u16) omm_abs_s((s16) (yaw - wAngle));
                if ((i == 0) || (dAngle < mAngle)) {
                    wRef = w;
                    mAngle = dAngle;
                }
            }
        } else {
            wRef = data.walls[0];
        }
    }

    pos[0] = data.x;
    pos[1] = data.y;
    pos[2] = data.z;
    return wRef;
}

s32 f32_find_wall_collision(f32 *x, f32 *y, f32 *z, f32 offsetY, f32 radius) {
    struct WallCollisionData collision;
    collision.x = *x;
    collision.y = *y;
    collision.z = *z;
    collision.radius = radius;
    collision.offsetY = offsetY;
    s32 numCollisions = find_wall_collisions(&collision);
    *x = collision.x;
    *y = collision.y;
    *z = collision.z;
    return numCollisions;
}

//
// Ceilings
//

static struct Surface *find_ceil_from_list(struct Surface **head, s32 count, f32 x, f32 y, f32 z, f32 *pHeight) {
    struct Surface *ceil = NULL;
    for (s32 i = 0; i < count; head++, i++) {
        struct Surface *surf = *head;

        // Ignore no-cam if checking for the camera, or camera surfaces if checking for an object
        if ((gCheckingSurfaceCollisionsForCamera && (surf->flags & SURFACE_FLAG_NO_CAM_COLLISION)) ||
           (!gCheckingSurfaceCollisionsForCamera && (surf->type == SURFACE_CAMERA_BOUNDARY))) {
            continue;
        }

        // Skip if too low or above the previous ceiling
        f32 height = -(x * surf->normal.x + z * surf->normal.z + surf->originOffset) / surf->normal.y;
        if (height < y - 78.f || height >= *pHeight) {
            continue;
        }

        // Checking if point is in bounds of the triangle
        f32 x1 = surf->vertex1[0];
        f32 x2 = surf->vertex2[0];
        f32 x3 = surf->vertex3[0];
        f32 z1 = surf->vertex1[2];
        f32 z2 = surf->vertex2[2];
        f32 z3 = surf->vertex3[2];
        if (((z1 - z) * (x2 - x1) - (x1 - x) * (z2 - z1) > 0.f) ||
            ((z2 - z) * (x3 - x2) - (x2 - x) * (z3 - z2) > 0.f) ||
            ((z3 - z) * (x1 - x3) - (x3 - x) * (z1 - z3) > 0.f)) {
            continue;
        }

        // Valid ceiling
        *pHeight = height;
        ceil = surf;
    }
    return ceil;
}

f32 find_ceil(f32 x, f32 y, f32 z, struct Surface **pCeil) {
    static struct Surface *sDummyCeil = NULL;
    if (!pCeil) { pCeil = &sDummyCeil; }
    f32 sHeight = +20000.f;
    f32 dHeight = +20000.f;
    *pCeil = NULL;

#if OMM_GAME_IS_R96A
    if (cheats_no_bounds(gMarioState)) {
        return sHeight;
    }
#endif

    // Check PUs
    if (OMM_COLLISION_CHECK_PUS) {
        if (x < -OMM_COLLISION_LEVEL_BOUNDARY || x >= +OMM_COLLISION_LEVEL_BOUNDARY ||
            z < -OMM_COLLISION_LEVEL_BOUNDARY || z >= +OMM_COLLISION_LEVEL_BOUNDARY) {
            return 0;
        }
    }

    // Check the center cell only
    s16 cx = ((s16) ((x + OMM_COLLISION_LEVEL_BOUNDARY) / OMM_COLLISION_CELL_SIZE)) & OMM_COLLISION_CELL_BITS;
    s16 cz = ((s16) ((z + OMM_COLLISION_LEVEL_BOUNDARY) / OMM_COLLISION_CELL_SIZE)) & OMM_COLLISION_CELL_BITS;
    struct Surface *dCeil = find_ceil_from_list((struct Surface **) omm_array_data(gOmmDynamicSurfaces.surfaces[cz][cx].ceils), gOmmDynamicSurfaces.counts[cz][cx].ceils, x, y, z, &dHeight);
    struct Surface *sCeil = find_ceil_from_list((struct Surface **) omm_array_data(gOmmStaticSurfaces.surfaces[cz][cx].ceils), gOmmStaticSurfaces.counts[cz][cx].ceils, x, y, z, &sHeight);

    // Return the lowest of the two ceilings
    if (dCeil != NULL && dHeight < sHeight) {
        *pCeil = dCeil;
        return dHeight;
    }
    *pCeil = sCeil;
    return sHeight;
}

f32 vec3f_find_ceil(Vec3f pos, f32 height, struct Surface **ceil) {
    return find_ceil(pos[0], height + 80.f, pos[2], ceil);
}

//
// Floors
//

static bool check_is_point_inside_floor_triangle(struct Surface *surf, f32 x, f32 z) {
    f32 x1 = surf->vertex1[0];
    f32 x2 = surf->vertex2[0];
    f32 x3 = surf->vertex3[0];
    f32 z1 = surf->vertex1[2];
    f32 z2 = surf->vertex2[2];
    f32 z3 = surf->vertex3[2];
    if (((z1 - z) * (x2 - x1) - (x1 - x) * (z2 - z1) < 0.f) ||
        ((z2 - z) * (x3 - x2) - (x2 - x) * (z3 - z2) < 0.f) ||
        ((z3 - z) * (x1 - x3) - (x3 - x) * (z1 - z3) < 0.f)) {
        return false;
    }
    return true;
}

static struct Surface *find_floor_from_list(struct Surface **head, s32 count, f32 x, f32 y, f32 z, f32 *pHeight) {
    struct Surface *floor = NULL;
    for (s32 i = 0; i < count; head++, i++) {
        struct Surface *surf = *head;

        // Ignore no-cam if checking for the camera, or camera surfaces if checking for an object
        if ((gCheckingSurfaceCollisionsForCamera && (surf->flags & SURFACE_FLAG_NO_CAM_COLLISION)) ||
           (!gCheckingSurfaceCollisionsForCamera && (surf->type == SURFACE_CAMERA_BOUNDARY))) {
            continue;
        }

        // Skip if too high or below the previous floor
        f32 height = -(x * surf->normal.x + z * surf->normal.z + surf->originOffset) / surf->normal.y;
        if (height > y + 78.f || height <= *pHeight) {
            continue;
        }

        // Checking if point is in bounds of the triangle
        if (!check_is_point_inside_floor_triangle(surf, x, z)) {

            // If not, checking if one of 4 surrounding points is in bounds
            // Prevent the object from going through gaps smaller than 20 units
            // Does not apply to steep floors, to prevent Mario from walking mid-air
            if (!OMM_COLLISION_CHECK_FLOOR_SQUARE || surf->normal.y < 0.7f || (
                !check_is_point_inside_floor_triangle(surf, x - 10.f, z) &&
                !check_is_point_inside_floor_triangle(surf, x + 10.f, z) &&
                !check_is_point_inside_floor_triangle(surf, x, z - 10.f) &&
                !check_is_point_inside_floor_triangle(surf, x, z + 10.f))) {
                continue;
            }
        }

        // Valid floor
        *pHeight = height;
        floor = surf;
    }
    return floor;
}

struct Surface *get_pseudo_floor_at_pos(f32 x, f32 y, f32 z) {
    static struct Surface sPseudoFloor[1];
    sPseudoFloor->type = SURFACE_DEFAULT;
    sPseudoFloor->force = 0;
    sPseudoFloor->flags = 0;
    sPseudoFloor->room = -1;
    sPseudoFloor->lowerY = y;
    sPseudoFloor->upperY = y;
    sPseudoFloor->vertex1[0] = x;
    sPseudoFloor->vertex1[1] = y;
    sPseudoFloor->vertex1[2] = z;
    sPseudoFloor->vertex2[0] = x;
    sPseudoFloor->vertex2[1] = y;
    sPseudoFloor->vertex2[2] = z;
    sPseudoFloor->vertex3[0] = x;
    sPseudoFloor->vertex3[1] = y;
    sPseudoFloor->vertex3[2] = z;
    sPseudoFloor->normal.x = 0.f;
    sPseudoFloor->normal.y = 1.f;
    sPseudoFloor->normal.z = 0.f;
    sPseudoFloor->originOffset = -y;
    sPseudoFloor->object = NULL;
    return sPseudoFloor;
}

#if !OMM_GAME_IS_SM64
struct Surface *get_eyerock_boss_floor(f32 x, f32 z, f32 x0, f32 y0, f32 z0, f32 radius) {
    static struct Surface sEyerockBossFloor[2][1];

    sEyerockBossFloor[0]->type = SURFACE_DEFAULT;
    sEyerockBossFloor[0]->force = 0;
    sEyerockBossFloor[0]->flags = 0;
    sEyerockBossFloor[0]->room = -1;
    sEyerockBossFloor[0]->lowerY = y0;
    sEyerockBossFloor[0]->upperY = y0;
    sEyerockBossFloor[0]->vertex1[0] = x0 - radius;
    sEyerockBossFloor[0]->vertex1[1] = y0;
    sEyerockBossFloor[0]->vertex1[2] = z0 - radius;
    sEyerockBossFloor[0]->vertex2[0] = x0 + radius;
    sEyerockBossFloor[0]->vertex2[1] = y0;
    sEyerockBossFloor[0]->vertex2[2] = z0 + radius;
    sEyerockBossFloor[0]->vertex3[0] = x0 + radius;
    sEyerockBossFloor[0]->vertex3[1] = y0;
    sEyerockBossFloor[0]->vertex3[2] = z0 - radius;
    sEyerockBossFloor[0]->normal.x = 0.f;
    sEyerockBossFloor[0]->normal.y = 1.f;
    sEyerockBossFloor[0]->normal.z = 0.f;
    sEyerockBossFloor[0]->originOffset = -y0;
    sEyerockBossFloor[0]->object = NULL;
    if (check_is_point_inside_floor_triangle(sEyerockBossFloor[0], x, z)) {
        return sEyerockBossFloor[0];
    }

    sEyerockBossFloor[1]->type = SURFACE_DEFAULT;
    sEyerockBossFloor[1]->force = 0;
    sEyerockBossFloor[1]->flags = 0;
    sEyerockBossFloor[1]->room = -1;
    sEyerockBossFloor[1]->lowerY = y0;
    sEyerockBossFloor[1]->upperY = y0;
    sEyerockBossFloor[1]->vertex1[0] = x0 - radius;
    sEyerockBossFloor[1]->vertex1[1] = y0;
    sEyerockBossFloor[1]->vertex1[2] = z0 - radius;
    sEyerockBossFloor[1]->vertex2[0] = x0 - radius;
    sEyerockBossFloor[1]->vertex2[1] = y0;
    sEyerockBossFloor[1]->vertex2[2] = z0 + radius;
    sEyerockBossFloor[1]->vertex3[0] = x0 + radius;
    sEyerockBossFloor[1]->vertex3[1] = y0;
    sEyerockBossFloor[1]->vertex3[2] = z0 + radius;
    sEyerockBossFloor[1]->normal.x = 0.f;
    sEyerockBossFloor[1]->normal.y = 1.f;
    sEyerockBossFloor[1]->normal.z = 0.f;
    sEyerockBossFloor[1]->originOffset = -y0;
    sEyerockBossFloor[1]->object = NULL;
    if (check_is_point_inside_floor_triangle(sEyerockBossFloor[1], x, z)) {
        return sEyerockBossFloor[1];
    }

    return NULL;
}
#endif

#if !OMM_GAME_IS_SMSR
// floorGeo is always filled, even when there is no floor, to avoid NULL pointer deref crash
// (most of the code calling it assumes that (*floorGeo) is never NULL...)
f32 find_floor_height_and_data(f32 x, f32 y, f32 z, struct FloorGeometry **floorGeo) {
    static struct FloorGeometry sFloorGeo;
    struct Surface *floor;
    f32 floorHeight = find_floor(x, y, z, &floor);
    if (floor != NULL) {
        sFloorGeo.normalX = floor->normal.x;
        sFloorGeo.normalY = floor->normal.y;
        sFloorGeo.normalZ = floor->normal.z;
        sFloorGeo.originOffset = floor->originOffset;
    } else {
        sFloorGeo.normalX = 0.f;
        sFloorGeo.normalY = 1.f;
        sFloorGeo.normalZ = 0.f;
        sFloorGeo.originOffset = +11000.f;    
    }
    *floorGeo = &sFloorGeo;
    return floorHeight;
}
#endif

f32 find_floor_height(f32 x, f32 y, f32 z) {
    return find_floor(x, y, z, NULL);
}

f32 find_room_floor(f32 x, f32 y, f32 z, struct Surface **pFloor) {
    gFindFloorIncludeSurfaceIntangible = TRUE;
    return find_floor(x, y, z, pFloor);
}

f32 find_floor(f32 x, f32 y, f32 z, struct Surface **pFloor) {
    static struct Surface *sDummyFloor = NULL;
    if (!pFloor) { pFloor = &sDummyFloor; }
    f32 sHeight = -11000.f;
    f32 dHeight = -11000.f;
    *pFloor = NULL;

    // Call from geo_switch_area
    // Tells the game to find a floor for the star cutscene object
    if (gFindFloorForCutsceneStar) {
        gFindFloorForCutsceneStar = FALSE;
        if (gCamera && gCutsceneFocus && (
            gCamera->cutscene == CUTSCENE_STAR_SPAWN ||
            gCamera->cutscene == CUTSCENE_RED_COIN_STAR_SPAWN)) {
            f32 floorY = find_floor(
                gCutsceneFocus->oPosX,
                gCutsceneFocus->oPosY + 100.f,
                gCutsceneFocus->oPosZ, 
                pFloor
            );
            if (*pFloor) {
                return floorY;
            }
        }
    }

    // Check PUs
    if (OMM_COLLISION_CHECK_PUS) {
        if (x < -OMM_COLLISION_LEVEL_BOUNDARY || x >= +OMM_COLLISION_LEVEL_BOUNDARY ||
            z < -OMM_COLLISION_LEVEL_BOUNDARY || z >= +OMM_COLLISION_LEVEL_BOUNDARY) {
            return 0;
        }
    }

    // Check the center cell only
    s16 cx = ((s16) ((x + OMM_COLLISION_LEVEL_BOUNDARY) / OMM_COLLISION_CELL_SIZE)) & OMM_COLLISION_CELL_BITS;
    s16 cz = ((s16) ((z + OMM_COLLISION_LEVEL_BOUNDARY) / OMM_COLLISION_CELL_SIZE)) & OMM_COLLISION_CELL_BITS;
    struct Surface *dFloor = find_floor_from_list((struct Surface **) omm_array_data(gOmmDynamicSurfaces.surfaces[cz][cx].floors), gOmmDynamicSurfaces.counts[cz][cx].floors, x, y, z, &dHeight);
    struct Surface *sFloor = find_floor_from_list((struct Surface **) omm_array_data(gOmmStaticSurfaces.surfaces[cz][cx].floors), gOmmStaticSurfaces.counts[cz][cx].floors, x, y, z, &sHeight);

    // BBH Merry-Go-Round
    // Handle SURFACE_INTANGIBLE, used to prevent the wrong room from loading
    // when passing above the Merry-Go-Round, but allows Mario to pass through.
    if (!gFindFloorIncludeSurfaceIntangible && sFloor && sFloor->type == SURFACE_INTANGIBLE) {
        f32 y0 = sHeight - 200.f; sHeight = -11000.f;
        sFloor = find_floor_from_list((struct Surface **) omm_array_data(gOmmStaticSurfaces.surfaces[cz][cx].floors), gOmmStaticSurfaces.counts[cz][cx].floors, x, y0, z, &sHeight);
    } else {
        gFindFloorIncludeSurfaceIntangible = FALSE;
    }

    // Possession fake floor
    // Because the Possession action doesn't check for floors during the animation (first 20 frames),
    // it can easily result in an out of bounds step, triggering the instant death sequence
    if (!sFloor && !dFloor) {
        if (gMarioState->action == ACT_OMM_POSSESSION && gOmmData->mario->capture.timer < 20) {
            sFloor = get_pseudo_floor_at_pos(x, y, z);
            sHeight = y;
        }
    }

#if !OMM_GAME_IS_SM64
    // Eyerock boss fake floor
    // Fix Eyerock boss fight in various rom-hacks
    // The vanilla behavior heavily depends on hard-coded coordinates, so it kinda breaks when used elsewhere...
    // ...but it works pretty good if we define a plane square floor under the hands
    if (gCurrentObject && gCurrentObject->behavior == bhvEyerokHand) {
        struct Object *eyerokBoss = gCurrentObject->parentObj;
        if (eyerokBoss && eyerokBoss->behavior == bhvEyerokBoss) {
            struct Surface *floor = get_eyerock_boss_floor(x, z, eyerokBoss->oPosX, eyerokBoss->oPosY, eyerokBoss->oPosZ, 2000.f);
            if (floor && floor->upperY > sHeight && floor->upperY > dHeight) {
                *pFloor = floor;
                return floor->upperY;
            }
        }
    }
#endif

#if OMM_GAME_IS_R96A
    // If out-of-bounds but NoBounds is enabled, place a fake death plane below Mario
    if (!sFloor && !dFloor && cheats_no_bounds(gMarioState)) {
        sFloor = get_pseudo_floor_at_pos(x, -11000.f, z);
        sFloor->type = SURFACE_DEATH_PLANE;
        sHeight = -11000.f;
    }
#endif

    // Return the highest of the two floors
    if (dFloor != NULL && dHeight > sHeight) {
        *pFloor = dFloor;
        return dHeight;
    }
    *pFloor = sFloor;
    return sHeight;
}

// What's the purpose of the original function?
// If it's about optimization, it's badly done.
// The game retrieves the floor at (x, z) ONLY if there is an intangible floor in that cell.
// Otherwise, it uses Mario's floor... even if the roomFocusObj is not Mario.
// Remember kids: premature optimization is the root of all evil.
s32 floor_type_exists_in_current_cell(UNUSED f32 x, UNUSED f32 z, UNUSED s16 type, UNUSED u32 dynamic) {
    return TRUE;
}

//
// Environment
//

typedef struct {
    s16 type;
    s16 x0;
    s16 z0;
    s16 x1;
    s16 z1;
    s16 value;
} EnvironmentRegion;

f32 find_water_level_and_floor(f32 x, f32 z, struct Surface **pFloor) {
    if (pFloor) *pFloor = NULL;
    return find_water_level(x, z);
}

f32 find_water_level(f32 x, f32 z) {
    if (!omm_world_is_flooded()) {
        if (gEnvironmentRegions != NULL) {
            s16 numRegions = gEnvironmentRegions[0];
            const EnvironmentRegion *regions = (const EnvironmentRegion *) &gEnvironmentRegions[1];
            for (s16 i = 0; i < numRegions; ++i) {
                if (regions[i].type < 50 &&
                    regions[i].x0 <= x && x <= regions[i].x1 &&
                    regions[i].z0 <= z && z <= regions[i].z1) {
                    return regions[i].value;
                }
            }
        }
        return -11000.f;
    }
    return +20000.f;
}

f32 find_poison_gas_level(f32 x, f32 z) {
    if (gEnvironmentRegions != NULL) {
        s16 numRegions = gEnvironmentRegions[0];
        const EnvironmentRegion *regions = (const EnvironmentRegion *) &gEnvironmentRegions[1];
        for (s16 i = 0; i < numRegions; ++i) {
            if (regions[i].type >= 50 && (regions[i].type % 10) == 0 &&
                regions[i].x0 <= x && x <= regions[i].x1 &&
                regions[i].z0 <= z && z <= regions[i].z1) {
                return regions[i].value;
            }
        }
    }
    return -11000.f;
}

void debug_surface_list_info(UNUSED f32 x, UNUSED f32 z) {
}

//
// Ray-casting
//

static s32 ray_surface_intersect(Vec3f orig, Vec3f dir, f32 dirLength, struct Surface *surface, Vec3f hitPos, f32 *length) {
    Vec3f v0; vec3s_to_vec3f(v0, surface->vertex1);
    Vec3f v1; vec3s_to_vec3f(v1, surface->vertex2);
    Vec3f v2; vec3s_to_vec3f(v2, surface->vertex3);
    Vec3f e1; vec3f_dif(e1, v1, v0);
    Vec3f e2; vec3f_dif(e2, v2, v0);
    Vec3f h; vec3f_cross(h, dir, e2);

    // Check if we're perpendicular from the surface
    f32 a = vec3f_dot(e1, h);
    if (a > -0.00001f && a < +0.00001f) {
        return FALSE;
    }

    // Check if we're making contact with the surface
    Vec3f surf; vec3f_dif(surf, orig, v0);
    f32 f = 1.f / a;
    f32 u = f * vec3f_dot(surf, h);
    if (u < 0.f || u > 1.f) {
        return FALSE;
    }

    Vec3f q; vec3f_cross(q, surf, e1);
    f32 v = f * vec3f_dot(dir, q);
    if (v < 0.f || u + v > 1.f) {
        return FALSE;
    }

    // Get the length between our origin and the surface contact point
    *length = f * vec3f_dot(e2, q);
    if (*length <= 0.00001f || *length > dirLength) {
        return FALSE;
    }

    // Successful contact
    vec3f_copy(hitPos, dir);
    vec3f_mul(hitPos, *length);
    vec3f_sum(hitPos, orig, hitPos);
    return TRUE;
}

static void find_surface_on_ray_list(struct Surface **head, s32 count, Vec3f orig, Vec3f dir, f32 dirLength, struct Surface **hitSurface, Vec3f hitPos, f32 *maxLength) {
    f32 upper = omm_max_f(orig[1], orig[1] + dir[1] * dirLength);
    f32 lower = omm_min_f(orig[1], orig[1] + dir[1] * dirLength);

    // Iterate through every surface of the list
    for (s32 i = 0; i < count; head++, i++) {
        struct Surface *surf = *head;

        // Reject surface if out of vertical bounds
        if (surf->lowerY > upper || surf->upperY < lower) {
            continue;
        }

        // Reject no-cam collision surfaces
        if (gCheckingSurfaceCollisionsForCamera && (surf->flags & SURFACE_FLAG_NO_CAM_COLLISION)) {
            continue;
        }

        // Check intersection between the ray and this surface
        f32 length;
        Vec3f chkHitPos;
        s32 hit = ray_surface_intersect(orig, dir, dirLength, surf, chkHitPos, &length);
        if (hit != 0 && length <= *maxLength) {
            *hitSurface = surf;
            vec3f_copy(hitPos, chkHitPos);
            *maxLength = length;
        }
    }
}

static void find_surface_on_ray_cell(s16 cx, s16 cz, Vec3f orig, Vec3f normalizedDir, f32 dirLength, struct Surface **hitSurface, Vec3f hitPos, f32 *maxLength) {
    if (cx >= 0 && cx < OMM_COLLISION_CELL_COUNT && cz >= 0 && cz < OMM_COLLISION_CELL_COUNT) {

        // Ceilings
        if (normalizedDir[1] > -0.99f) {
            find_surface_on_ray_list((struct Surface **) omm_array_data(gOmmStaticSurfaces.surfaces[cz][cx].walls), gOmmStaticSurfaces.counts[cz][cx].walls, orig, normalizedDir, dirLength, hitSurface, hitPos, maxLength);
            find_surface_on_ray_list((struct Surface **) omm_array_data(gOmmDynamicSurfaces.surfaces[cz][cx].walls), gOmmDynamicSurfaces.counts[cz][cx].walls, orig, normalizedDir, dirLength, hitSurface, hitPos, maxLength);
        }

        // Floors
        if (normalizedDir[1] < +0.99f) {
            find_surface_on_ray_list((struct Surface **) omm_array_data(gOmmStaticSurfaces.surfaces[cz][cx].floors), gOmmStaticSurfaces.counts[cz][cx].floors, orig, normalizedDir, dirLength, hitSurface, hitPos, maxLength);
            find_surface_on_ray_list((struct Surface **) omm_array_data(gOmmDynamicSurfaces.surfaces[cz][cx].floors), gOmmDynamicSurfaces.counts[cz][cx].floors, orig, normalizedDir, dirLength, hitSurface, hitPos, maxLength);
        }

        // Walls
        find_surface_on_ray_list((struct Surface **) omm_array_data(gOmmStaticSurfaces.surfaces[cz][cx].ceils), gOmmStaticSurfaces.counts[cz][cx].ceils, orig, normalizedDir, dirLength, hitSurface, hitPos, maxLength);
        find_surface_on_ray_list((struct Surface **) omm_array_data(gOmmDynamicSurfaces.surfaces[cz][cx].ceils), gOmmDynamicSurfaces.counts[cz][cx].ceils, orig, normalizedDir, dirLength, hitSurface, hitPos, maxLength);
    }
}

void find_surface_on_ray(Vec3f orig, Vec3f dir, struct Surface **hitSurface, Vec3f hitPos BETTER_CAM_RAYCAST_ARGS) {
    *hitSurface = NULL;
    vec3f_sum(hitPos, orig, dir);

    // Get normalized direction
    f32 dirLength = vec3f_length(dir);
    f32 maxLength = dirLength;
    Vec3f normalizedDir; vec3f_copy(normalizedDir, dir);
    vec3f_normalize(normalizedDir);

    // Get our cell coordinate
    f32 fcx = ((orig[0] + OMM_COLLISION_LEVEL_BOUNDARY) / OMM_COLLISION_CELL_SIZE);
    f32 fcz = ((orig[2] + OMM_COLLISION_LEVEL_BOUNDARY) / OMM_COLLISION_CELL_SIZE);
    s16 cx = (s16) fcx;
    s16 cz = (s16) fcz;

    // Don't do DDA if straight down
    if (normalizedDir[1] >= +0.99f || normalizedDir[1] <= -0.99f) {
        find_surface_on_ray_cell(cx, cz, orig, normalizedDir, dirLength, hitSurface, hitPos, &maxLength);
    } else {
        f32 step = (((gCheckingSurfaceCollisionsForCamera ? 3.f : 1.f) * omm_max_f(omm_abs_f(dir[0]), omm_abs_f(dir[2]))) / OMM_COLLISION_CELL_SIZE);
        f32 dx = ((dir[0] / step) / OMM_COLLISION_CELL_SIZE);
        f32 dz = ((dir[2] / step) / OMM_COLLISION_CELL_SIZE);
        for (s32 i = 0; i < step && *hitSurface == NULL; i++) {
            find_surface_on_ray_cell(cx, cz, orig, normalizedDir, dirLength, hitSurface, hitPos, &maxLength);
            fcx += dx;
            fcz += dz;
            cx = (s16) fcx;
            cz = (s16) fcz;
        }
    }
}
