#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// Constants
//

#define OMM_CAM_NUM_DIRS            (8 * (OMM_CAMERA_CLASSIC ? 1 : gOmmCameraMode))
#define OMM_CAM_DELTA_ANGLE         (0x10000 / OMM_CAM_NUM_DIRS)
#define OMM_CAM_DIST_NUM_MODES      5
#define OMM_CAM_DIST_MODE_MIN       0
#define OMM_CAM_DIST_MODE_MAX       (OMM_CAM_DIST_NUM_MODES - 1)
#define OMM_CAM_DIST_MODE_MEDIUM    (OMM_CAM_DIST_MODE_MAX / 2)
#define OMM_CAM_DIST_MODE_HIGH      (OMM_CAM_DIST_MODE_MEDIUM + 1)
#define OMM_CAM_DIST_MODE_LOW       (OMM_CAM_DIST_MODE_MEDIUM - 1)
static const f32 sOmmCameraDistances[OMM_CAM_DIST_NUM_MODES] = { 400.f, 800.f, 1200.f, 1600.f, 2000.f };

//
// Inlines
//

inline static s16 approach(s16 value, s16 target, s16 inc) {
    return target - approach_s32((s16) (target - value), 0, inc, inc);
}

inline static s16 get_nearest_dir_angle(s16 value) {
    s32 va = (s32) value + 0x10000;
    s32 da = OMM_CAM_DELTA_ANGLE;
    s32 a0 = (((va / da) + 0) * da);
    s32 a1 = (((va / da) + 1) * da);
    s32 d0 = omm_abs_s(va - a0);
    s32 d1 = omm_abs_s(va - a1);
    if (d0 == omm_min_s(d0, d1)) return (s16) a0;
    return (s16) a1;
}

//
// Data
//

static s32 sOmmCamFpMode      = 0;
static s16 sOmmCamPitch       = 0x0C00;
static s16 sOmmCamPitchFp     = 0x0000;
static s16 sOmmCamPitchTarget = 0x0C00;
static s16 sOmmCamYaw         = 0x0000;
static s16 sOmmCamYawTarget   = 0x0000;
static s32 sOmmCamDistMode    = OMM_CAM_DIST_MODE_MEDIUM;
static f32 sOmmCamDistance    = 0.f;
static f32 sOmmCamPos[3];
static f32 sOmmCamFocus[3];

//
// Functions
//

bool omm_camera_is_available(struct MarioState *m) {
    return
        !omm_is_ending_cutscene() &&
        (!OMM_CAMERA_CLASSIC || omm_cappy_get_object_play_as()) &&
        (m->action != ACT_IN_CANNON);
}

#if OMM_GAME_IS_SM64
// Level, Area, Pitch, Yaw, Distance
static const s16 sOmmCameraInitPresets[][5] = {
    { LEVEL_BITDW,   1, 0x1000, 0x0000, OMM_CAM_DIST_MODE_HIGH },
    { LEVEL_BITFS,   1, 0x1000, 0x0000, OMM_CAM_DIST_MODE_HIGH },
    { LEVEL_BITS,    1, 0x1000, 0x0000, OMM_CAM_DIST_MODE_HIGH },
    { LEVEL_WF,      1, 0x0C00, 0x0000, OMM_CAM_DIST_MODE_MEDIUM },
    { LEVEL_CCM,     1, 0x0C00, 0x8000, OMM_CAM_DIST_MODE_MEDIUM },
    { LEVEL_BBH,     1, 0x0C00, 0x0000, OMM_CAM_DIST_MODE_HIGH },
    { LEVEL_WDW,     1, 0x0C00, 0x2000, OMM_CAM_DIST_MODE_MEDIUM },
    { LEVEL_TTM,     1, 0x0C00, 0x0000, OMM_CAM_DIST_MODE_HIGH },
    { LEVEL_RR,      1, 0x0C00, 0xE000, OMM_CAM_DIST_MODE_HIGH },
    { LEVEL_VCUTM,   1, 0x0C00, 0x0000, OMM_CAM_DIST_MODE_HIGH },
    { LEVEL_COURT,   1, 0x0C00, 0x8000, OMM_CAM_DIST_MODE_MEDIUM },
    { LEVEL_GROUNDS, 1, 0x0C00, 0x0000, OMM_CAM_DIST_MODE_MEDIUM },
};
static const s32 sOmmCameraInitPresetCount = sizeof(sOmmCameraInitPresets) / sizeof(sOmmCameraInitPresets[0]);

// Level, Area, xMin, xMax, yMin, yMax, zMin, zMax
static const s16 sOmmCameraNoColBoxes[][8] = {
    { LEVEL_WDW,   1,  450, 1400,   100, 3600, -2350, -1100 },
    { LEVEL_BITFS, 1, 2950, 4550, -1450, -600,  -350,   950 },
    { LEVEL_BITFS, 1, 2550, 3150, -1250,  450,     0,   600 },
};
static const s32 sOmmCameraNoColBoxCount = sizeof(sOmmCameraNoColBoxes) / sizeof(sOmmCameraNoColBoxes[0]);
#endif

void omm_camera_init() {
    if (omm_camera_is_available(gMarioState)) {
    
        // Default
        sOmmCamPitch = 0x0C00;
        sOmmCamYaw = gMarioState->faceAngle[1] + 0x8000;
        sOmmCamDistMode = OMM_CAM_DIST_MODE_MEDIUM;

#if OMM_GAME_IS_SM64
        // Per level
        for (s32 i = 0; i != sOmmCameraInitPresetCount; ++i) {
            if (sOmmCameraInitPresets[i][0] == gCurrLevelNum &&
                sOmmCameraInitPresets[i][1] == gCurrAreaIndex) {
                sOmmCamPitch = sOmmCameraInitPresets[i][2];
                sOmmCamYaw = sOmmCameraInitPresets[i][3];
                sOmmCamDistMode = sOmmCameraInitPresets[i][4];
                break;
            }
        }
#endif

        // Init
        sOmmCamPitchTarget = sOmmCamPitch;
        sOmmCamYawTarget = sOmmCamYaw;
        sOmmCamDistance = sOmmCameraDistances[sOmmCamDistMode];
        sOmmCamFpMode = 0;
        sOmmCamPitchFp = 0;
    }
}

OMM_ROUTINE_LEVEL_ENTRY(omm_camera_init_from_level_entry) {
    omm_camera_init();
    sOmmCamFpMode = 0;
}

void omm_camera_set_yaw(s16 yaw) {
    sOmmCamYaw = yaw;
}

s16 omm_camera_get_intended_yaw(struct MarioState *m) {

    // OMM cam
    if (omm_camera_is_available(m)) {
        return atan2s(-m->controller->stickY, m->controller->stickX) + sOmmCamYaw;
    }

    // Better cam (not Puppy cam)
    if (BETTER_CAM_IS_ENABLED && !BETTER_CAM_IS_PUPPY_CAM && gLakituState.mode == BETTER_CAM_MODE) {
        return atan2s(-m->controller->stickY, m->controller->stickX) - BETTER_CAM_YAW + 0x4000;
    }

    // Lakitu/Puppy cam
    return atan2s(-m->controller->stickY, m->controller->stickX) + m->area->camera->yaw;
}

s32 omm_camera_get_relative_dist_mode() {
    return sOmmCamDistMode - OMM_CAM_DIST_MODE_MEDIUM;
}

//
// Collision
//

typedef struct { f32 x, y, z; } vec3;
typedef struct { vec3 pos; f32 dist; f32 ratio; struct Surface *surf; } RayHit;
typedef struct { RayHit hit[16]; s32 count; } RayHits;
typedef struct { void *buffer[4096]; s32 count; } Surfaces;

inline static vec3 vec3_set(f32 x, f32 y, f32 z) { vec3 v = { x, y, z }; return v; }
inline static vec3 vec3_add(vec3 u, vec3 v) { return vec3_set(u.x + v.x, u.y + v.y, u.z + v.z); }
inline static vec3 vec3_sub(vec3 u, vec3 v) { return vec3_set(u.x - v.x, u.y - v.y, u.z - v.z); }
inline static vec3 vec3_mult(vec3 v, f32 f) { return vec3_set(v.x * f, v.y * f, v.z * f); }
inline static vec3 vec3_cross(vec3 u, vec3 v) { return vec3_set(u.y * v.z - v.y * u.z, u.z * v.x - v.z * u.x, u.x * v.y - v.x * u.y); }
inline static f32  vec3_length(vec3 v) { return sqrtf(omm_sqr_f(v.x) + omm_sqr_f(v.y) + omm_sqr_f(v.z)); }
inline static f32  vec3_dist(vec3 u, vec3 v) { return vec3_length(vec3_sub(u, v)); }
inline static f32  vec3_dot(vec3 u, vec3 v) { return u.x * v.x + u.y * v.y + u.z * v.z; }
inline static bool find_surface(const Surfaces *surfaces, void *ptr) { for (s32 i = 0; i != surfaces->count; ++i) { if (surfaces->buffer[i] == ptr) { return true; } } return false; }

static bool omm_camera_intersect_ray_surface(vec3 origin, vec3 dirNormalized, f32 dirLength, struct Surface *surface, RayHit *hit) {
    vec3 p0 = vec3_set(surface->vertex1[0], surface->vertex1[1], surface->vertex1[2]);
    vec3 p1 = vec3_set(surface->vertex2[0], surface->vertex2[1], surface->vertex2[2]);
    vec3 p2 = vec3_set(surface->vertex3[0], surface->vertex3[1], surface->vertex3[2]);
    vec3 vc = vec3_mult(vec3_add(vec3_add(p0, p1), p2), 1.f / 3.f);
    vec3 v0 = vec3_add(vc, vec3_mult(vec3_sub(p0, vc), 1.1f));
    vec3 v1 = vec3_add(vc, vec3_mult(vec3_sub(p1, vc), 1.1f));
    vec3 v2 = vec3_add(vc, vec3_mult(vec3_sub(p2, vc), 1.1f));
    vec3 e1 = vec3_sub(v1, v0);
    vec3 e2 = vec3_sub(v2, v0);
    vec3 vh = vec3_cross(dirNormalized, e2);

    // Perpendicular?
    f32 dot = vec3_dot(e1, vh);
    if (dot > -0.01f && dot < 0.01f) {
        return false;
    }

    // Contact?
    vec3 vo = vec3_sub(origin, v0);
    f32 u = vec3_dot(vo, vh) / dot;
    if (u < 0.f || u > 1.f) {
        return false;
    }

    vec3 voeCross = vec3_cross(vo, e1);
    f32 v = vec3_dot(dirNormalized, voeCross) / dot;
    if (v < 0.f || (u + v) > 1.f) {
        return false;
    }

    // Compute hitLength
    hit->dist = vec3_dot(e2, voeCross) / dot;
    if (hit->dist < 0.01f || hit->dist > dirLength) {
        return false;
    }

    // Fill hit
    hit->pos = vec3_add(origin, vec3_mult(dirNormalized, hit->dist));
    hit->surf = surface;
    hit->ratio = hit->dist / dirLength;
    return true;
}

static bool omm_camera_check_point_in_no_col_box(vec3 pos) {
#if OMM_GAME_IS_SM64
    for (s32 i = 0; i != sOmmCameraNoColBoxCount; ++i) {
        if (sOmmCameraNoColBoxes[i][0] == gCurrLevelNum &&
            sOmmCameraNoColBoxes[i][1] == gCurrAreaIndex &&
            sOmmCameraNoColBoxes[i][2] < pos.x && pos.x < sOmmCameraNoColBoxes[i][3] &&
            sOmmCameraNoColBoxes[i][4] < pos.y && pos.y < sOmmCameraNoColBoxes[i][5] &&
            sOmmCameraNoColBoxes[i][6] < pos.z && pos.z < sOmmCameraNoColBoxes[i][7]) {
            return true;
        }
    }
#else
    OMM_UNUSED(pos);
#endif
    return false;
}

static void omm_camera_find_hits_on_list(struct Surface **head, s32 count, vec3 origin, vec3 dirNormalized, f32 dirLength, RayHits *hits, Surfaces *surfaces) {
    f32 lowerY = omm_min_f(origin.y, vec3_add(origin, vec3_mult(dirNormalized, dirLength)).y);
    f32 upperY = omm_max_f(origin.y, vec3_add(origin, vec3_mult(dirNormalized, dirLength)).y);
    for (s32 i = 0; i < count && hits->count < 16; head++, i++) {
        struct Surface *surf = *head;

        // Reject surface out of vertical bounds
        if (surf->lowerY > upperY ||
            surf->upperY < lowerY) {
            continue;
        }

        // Reject no-cam collision and vanish cap walls surfaces
        if ((surf->flags & SURFACE_FLAG_NO_CAM_COLLISION) ||
            (surf->type == SURFACE_NO_CAM_COLLISION) ||
            (surf->type == SURFACE_NO_CAM_COLLISION_77) ||
            (surf->type == SURFACE_NO_CAM_COL_VERY_SLIPPERY) ||
            (surf->type == SURFACE_NO_CAM_COL_SLIPPERY) ||
            (surf->type == SURFACE_SWITCH) ||
            (surf->type == SURFACE_VANISH_CAP_WALLS)) {
            continue;
        }

        // Exclude already checked surfaces
        if (find_surface(surfaces, surf)) {
            continue;
        }
        surfaces->buffer[surfaces->count++] = surf;

        // Check intersection
        // Reject hit if inside a no-collision box
        // Reject hit if not opposed to the camera direction
        RayHit hit;
        if (omm_camera_intersect_ray_surface(origin, dirNormalized, dirLength, surf, &hit)) {
            if (!omm_camera_check_point_in_no_col_box(hit.pos)) {
                if (vec3_dot(dirNormalized, vec3_set(hit.surf->normal.x, hit.surf->normal.y, hit.surf->normal.z)) < 0.f) {
                    hits->hit[hits->count++] = hit;
                }
            }
        }
    }
}

static void omm_camera_find_hits_on_cell(s16 cx, s16 cz, vec3 origin, vec3 dirNormalized, f32 dirLength, RayHits *hits, Surfaces *surfaces) {
    if (cx < 0 || cz < 0 || cx > OMM_COLLISION_CELL_BITS || cz > OMM_COLLISION_CELL_BITS) {
        return;
    }

    // Walls
    omm_camera_find_hits_on_list((struct Surface **) omm_array_data(gOmmStaticSurfaces.surfaces[cz][cx].walls), gOmmStaticSurfaces.counts[cz][cx].walls, origin, dirNormalized, dirLength, hits, surfaces);
    omm_camera_find_hits_on_list((struct Surface **) omm_array_data(gOmmDynamicSurfaces.surfaces[cz][cx].walls), gOmmDynamicSurfaces.counts[cz][cx].walls, origin, dirNormalized, dirLength, hits, surfaces);

    // Floors
    if (dirNormalized.y < +0.95f) {
        omm_camera_find_hits_on_list((struct Surface **) omm_array_data(gOmmStaticSurfaces.surfaces[cz][cx].floors), gOmmStaticSurfaces.counts[cz][cx].floors, origin, dirNormalized, dirLength, hits, surfaces);
        omm_camera_find_hits_on_list((struct Surface **) omm_array_data(gOmmDynamicSurfaces.surfaces[cz][cx].floors), gOmmDynamicSurfaces.counts[cz][cx].floors, origin, dirNormalized, dirLength, hits, surfaces);
    }

    // Ceilings
    if (dirNormalized.y > -0.95f) {
        omm_camera_find_hits_on_list((struct Surface **) omm_array_data(gOmmStaticSurfaces.surfaces[cz][cx].ceils), gOmmStaticSurfaces.counts[cz][cx].ceils, origin, dirNormalized, dirLength, hits, surfaces);
        omm_camera_find_hits_on_list((struct Surface **) omm_array_data(gOmmDynamicSurfaces.surfaces[cz][cx].ceils), gOmmDynamicSurfaces.counts[cz][cx].ceils, origin, dirNormalized, dirLength, hits, surfaces);
    }
}

static RayHits omm_camera_find_hits(vec3 origin, vec3 dir) {
    f32 dirLength = vec3_length(dir);
    vec3 dirNormalized = vec3_mult(dir, 1.f / dirLength);
    f32 steps = 4.f * omm_max_f(omm_abs_f(dir.x), omm_abs_f(dir.z)) / OMM_COLLISION_CELL_SIZE;
    f32 dx = dir.x / (steps * OMM_COLLISION_CELL_SIZE);
    f32 dz = dir.z / (steps * OMM_COLLISION_CELL_SIZE);
    f32 cx = (origin.x + OMM_COLLISION_LEVEL_BOUNDARY) / OMM_COLLISION_CELL_SIZE;
    f32 cz = (origin.z + OMM_COLLISION_LEVEL_BOUNDARY) / OMM_COLLISION_CELL_SIZE;
    Surfaces surfaces = { 0 };
    RayHits hits = { 0 };
    for (s32 i = 0; i <= (s32) steps; i++, cx += dx, cz += dz) {
        omm_camera_find_hits_on_cell((s16) cx, (s16) cz, origin, dirNormalized, dirLength, &hits, &surfaces);
    }
    return hits;
}

static RayHits omm_camera_find_hits_focus_to_camera() {
    vec3 origin = {
        sOmmCamFocus[0],
        sOmmCamFocus[1],
        sOmmCamFocus[2]
    };
    vec3 dir = {
        sOmmCamPos[0] - origin.x,
        sOmmCamPos[1] - origin.y,
        sOmmCamPos[2] - origin.z,
    };

    // Find hits, then sort them by distance
    // A simple bubblesort easily do the trick
    RayHits hits = omm_camera_find_hits(origin, dir);
    for (s32 n = hits.count; n > 1;) {
        s32 m = 1;
        for (s32 i = 1; i != n; ++i) {
            if (hits.hit[i - 1].dist > hits.hit[i].dist) {
                RayHit temp = hits.hit[i];
                hits.hit[i] = hits.hit[i - 1];
                hits.hit[i - 1] = temp;
                m = i;
            }
        }
        n = m;
    }
    return hits;
}

static bool omm_camera_check_collision(const RayHit *hit) {
    struct Surface *ceil;
    f32 lowestCeil = find_ceil(sOmmCamFocus[0], sOmmCamFocus[1], sOmmCamFocus[2], &ceil) - 10.f;
    vec3 origin = {
        sOmmCamFocus[0],
        omm_min_f(lowestCeil, (sOmmCamPos[1] + sOmmCamFocus[1]) / 2),
        sOmmCamFocus[2]
    };
    vec3 dir = {
        sOmmCamPos[0] - origin.x,
        sOmmCamPos[1] - origin.y,
        sOmmCamPos[2] - origin.z,
    };

    // Before applying the collision, try to find another hit from above the focus,
    // to know if the first hit was against a fence or a small object
    RayHits hits = omm_camera_find_hits(origin, dir);
    for (s32 i = 0; i != hits.count; ++i) {
        if (hits.hit[i].ratio - hit->ratio < 0.15f) {
            return true;
        }
    }
    return false;
}

static void omm_camera_process_collisions() {
    RayHits hits = omm_camera_find_hits_focus_to_camera();
    for (s32 i = 0; i != hits.count; ++i) {
        const RayHit *hit = &hits.hit[i];
        if (sOmmCamPos[1] <= sOmmCamFocus[1] || omm_camera_check_collision(hit)) {
            sOmmCamPos[0] = hit->pos.x + 8.f * hit->surf->normal.x;
            sOmmCamPos[1] = hit->pos.y + 8.f * hit->surf->normal.y;
            sOmmCamPos[2] = hit->pos.z + 8.f * hit->surf->normal.z;
            break;
        }
    }
}

//
// Update
//

static void omm_camera_exit_first_person(struct Camera *c, struct MarioState *m, bool ignoreFpState, bool resetCamera) {
    if (sOmmCamFpMode || ignoreFpState) {
        sOmmCamFpMode = 0;
        update_mario_sound_and_camera(m);
        if (resetCamera) {
            reset_camera(c);
            init_camera(c);
        }
        gCameraMovementFlags &= ~(CAM_MOVING_INTO_MODE | CAM_MOVE_STARTED_EXITING_C_UP | CAM_MOVE_C_UP_MODE);
        c->mode = c->defMode;
        m->input &= ~INPUT_FIRST_PERSON;
        omm_mario_set_action(m, ACT_IDLE, 0, 0xFFFF);
    }
}

static void omm_camera_process_inputs(struct Camera *c, struct MarioState *m) {
    if (!sOmmCamFpMode) {

        // Camera rotation - C-left/right
        if (m->controller->buttonPressed & R_CBUTTONS) {
            play_sound(SOUND_MENU_CAMERA_TURN + 0x0100, gGlobalSoundArgs);
            sOmmCamYawTarget += OMM_CAM_DELTA_ANGLE;
        } else if (m->controller->buttonPressed & L_CBUTTONS) {
            play_sound(SOUND_MENU_CAMERA_TURN + 0x0100, gGlobalSoundArgs);
            sOmmCamYawTarget -= OMM_CAM_DELTA_ANGLE;
        }

        // Zooms-in - C-up
        if (m->controller->buttonPressed & U_CBUTTONS) {
            if (sOmmCamDistMode == OMM_CAM_DIST_MODE_MIN) {
                play_sound(SOUND_MENU_CAMERA_BUZZ + 0x0300, gGlobalSoundArgs);
            } else {
                play_sound(SOUND_MENU_CAMERA_ZOOM_IN + 0x0200, gGlobalSoundArgs);
                sOmmCamDistMode--;
            }
        }
    
        // Zooms-out - C-down
        else if (m->controller->buttonPressed & D_CBUTTONS) {
            if (sOmmCamDistMode == OMM_CAM_DIST_MODE_MAX) {
                play_sound(SOUND_MENU_CAMERA_BUZZ + 0x0300, gGlobalSoundArgs);
            } else {
                play_sound(SOUND_MENU_CAMERA_ZOOM_OUT + 0x0200, gGlobalSoundArgs);
                sOmmCamDistMode++;
            }
        }
    }

    // Modes - R button
    static s32 sButtonRHeld = 0;
    bool allowFp = ((m->action & ACT_FLAG_ALLOW_FIRST_PERSON) != 0);
    if (m->controller->buttonDown & R_TRIG) {
        sButtonRHeld = (sButtonRHeld * allowFp) + 1;
    } else {
        if (sButtonRHeld > 0 && !OMM_CAMERA_CLASSIC) {
            play_sound(SOUND_MENU_CLICK_CHANGE_VIEW, gGlobalSoundArgs);
            gOmmCameraMode = 3 - gOmmCameraMode;
        }
        omm_camera_exit_first_person(c, m, false, false);
        sButtonRHeld = 0;
    }

    // If held for more than 6 frames while Mario is in idle action, trigger the first person mode
    if (sButtonRHeld >= 6) {
        sOmmCamFpMode = 1;
        sOmmCamPitchFp = 0;
    }

    // First person mode
    if (sOmmCamFpMode) {
        sOmmCamFpMode &= allowFp;
        sOmmCamPitchFp *= allowFp;
        sButtonRHeld = 0;
    } else {
        sMarioCamState->headRotation[0] = approach(sMarioCamState->headRotation[0], 0, 0x400);
        sMarioCamState->headRotation[1] = approach(sMarioCamState->headRotation[1], 0, 0x400);
        sOmmCamPitchFp = 0;
    }
}

static void omm_camera_update_first_person_mode(struct MarioState *m) {
    omm_mario_set_action(m, ACT_FIRST_PERSON, 0, 0);

    // Pitch (from -89 deg to +89 deg)
    sOmmCamPitchFp += (s16) (m->controller->stickY * 10.f);
    sOmmCamPitchFp = omm_clamp_s(sOmmCamPitchFp, -0x3C00, +0x3C00);

    // Yaw
    m->faceAngle[1] -= (s16) (m->controller->stickX * 10.f);

    // Mario's neck
    sMarioCamState->headRotation[0] = (sOmmCamPitchFp * 3) / 4;
    sMarioCamState->headRotation[1] = 0;

    // Pos
    Vec3f marioPos = { sMarioCamState->pos[0], sMarioCamState->pos[1] + 125, sMarioCamState->pos[2] };
    vec3f_set_dist_and_angle(marioPos, sOmmCamPos, 50, sOmmCamPitchFp, m->faceAngle[1] + 0x8000);

    // Focus
    struct Object *playAsCappy = omm_cappy_get_object_play_as();
    if (playAsCappy) {
        sOmmCamFocus[0] = playAsCappy->oPosX;
        sOmmCamFocus[1] = playAsCappy->oPosY + 125.f;
        sOmmCamFocus[2] = playAsCappy->oPosZ;
    } else {
        sOmmCamFocus[0] = sMarioCamState->pos[0];
        sOmmCamFocus[1] = sMarioCamState->pos[1] + 125;
        sOmmCamFocus[2] = sMarioCamState->pos[2];
    }
    
    // TOTWC entrance
    if (m->floor && m->floor->type == SURFACE_LOOK_UP_WARP) {
        if (save_file_get_total_star_count(gCurrSaveFileNum - 1, COURSE_MIN - 1, COURSE_MAX - 1) >= OMM_CAMERA_LOOK_UP_WARP_STARS) {
            if (sMarioCamState->headRotation[0] <= -0x1800 && (m->faceAngle[1] < -0x6FFF || m->faceAngle[1] >= 0x7000)) {
                level_trigger_warp(m, WARP_OP_UNKNOWN_01);
            }
        }
    }
}

static void omm_camera_calc_y_offsets(f32 *camPosOffsetY, f32 *camFocOffsetY) {
    f32 curGeometryFloorHeight = sMarioGeometry.currFloorHeight;
    if (!(sMarioCamState->action & ACT_FLAG_METAL_WATER)) {
        curGeometryFloorHeight = omm_max_f(curGeometryFloorHeight, sMarioGeometry.waterHeight);
    }

    // Camera vertical position offset
    *camPosOffsetY = (curGeometryFloorHeight - sMarioCamState->pos[1]);
    struct Object *pole = gMarioState->usedObj;
    if ((pole != NULL) &&
        (sMarioCamState->action & ACT_FLAG_ON_POLE) &&
        (curGeometryFloorHeight >= pole->oPosY) &&
        (sMarioCamState->pos[1] <= pole->oPosY + pole->hitboxHeight * 0.7f)) {
        *camPosOffsetY = omm_clamp_f(*camPosOffsetY, -1200.f, 1200.f);
    } else {
        *camPosOffsetY = omm_clamp_f(*camPosOffsetY, -200.f, 200.f);
    }

    // Camera vertical focus offset
    *camFocOffsetY = (curGeometryFloorHeight - sMarioCamState->pos[1]) * 0.9f;
    *camFocOffsetY = omm_clamp_f(*camFocOffsetY, -200.f, 200.f);
}

static bool omm_camera_is_bowser_fight() {
    for_each(const BehaviorScript *, bhv, 2, OMM_ARRAY_OF(const BehaviorScript *) { bhvBowser, omm_bhv_bowser }) {
        for_each_object_with_behavior(obj, *bhv) {
            if (!obj_is_dormant(obj)) {
#if OMM_GAME_IS_R96A
                // Spamba Bowser
                if (*bhv == bhvBowser && obj->oInteractType == INTERACT_DAMAGE) {
                    continue;
                }
#endif
                return true;
            }
        }
    }
    return false;
}

static bool omm_camera_is_slide(struct MarioState *m) {

    // Slide level
    if (gCurrLevelNum == OMM_LEVEL_SLIDE) {
        return true;
    }

    // Slide terrain
    if (m->area && (m->area->terrainType & TERRAIN_MASK) == TERRAIN_SLIDE) {
        return true;
    }

    // Racing penguin
    for_each_object_with_behavior(obj, bhvRacingPenguin) {
        if (!obj_is_dormant(obj) && obj->oAction == RACING_PENGUIN_ACT_RACE) {
            return true;
        }
    }

    // Not a slide
    return false;
}

static void omm_camera_update_angles_from_state(struct MarioState *m, s16 camPitchTarget) {
    if (!omm_cappy_get_object_play_as()) {
        
        // Flying
        if ((m->action == ACT_SHOT_FROM_CANNON) ||
            (m->action == ACT_FLYING) || (
            (m->action == ACT_OMM_POSSESSION) &&
            (gOmmData->object->state.camBehindMario))) {
            sOmmCamPitch = approach(sOmmCamPitch, (-m->faceAngle[0] * 0.75f) + 0xC00, omm_max_f(m->forwardVel, 4.f) * 0x20);
            sOmmCamYaw = approach(sOmmCamYaw, m->faceAngle[1] + 0x8000, omm_max_f(m->forwardVel, 4.f) * 0x80);
            sOmmCamYawTarget = sOmmCamYaw;
            return;
        }

        // Underwater
        if (m->action & ACT_FLAG_SWIMMING) {
            sOmmCamPitch = approach(sOmmCamPitch, (-m->faceAngle[0] * 0.75f) + 0xC00, omm_max_f(m->forwardVel, 2.f) * 0x20);
            sOmmCamYaw = approach(sOmmCamYaw, m->faceAngle[1] + 0x8000, omm_max_f(m->forwardVel, 2.f) * 0x80);
            sOmmCamYawTarget = sOmmCamYaw;
            return;
        }

        // Sliding states
        if (omm_camera_is_slide(m)) {
        
            // Ground states
            if ((m->action & ACT_FLAG_BUTT_OR_STOMACH_SLIDE) ||
                (m->action == ACT_DIVE_SLIDE) ||
                (m->action == ACT_CROUCH_SLIDE) ||
                (m->action == ACT_SLIDE_KICK_SLIDE) ||
                (m->action == ACT_BUTT_SLIDE) ||
                (m->action == ACT_STOMACH_SLIDE) ||
                (m->action == ACT_HOLD_BUTT_SLIDE) ||
                (m->action == ACT_HOLD_STOMACH_SLIDE) ||
                (m->action == ACT_OMM_ROLL)) {
                sOmmCamPitch = approach(sOmmCamPitch, camPitchTarget, 0x400);
                sOmmCamYaw = approach(sOmmCamYaw, m->faceAngle[1] + 0x8000, omm_max_f(m->forwardVel, 4.f) * 0x80);
                sOmmCamYawTarget = sOmmCamYaw;
                return;
            }
        
            // Air states
            if ((m->action == ACT_DIVE) ||
                (m->action == ACT_SLIDE_KICK) ||
                (m->action == ACT_BUTT_SLIDE_AIR) ||
                (m->action == ACT_HOLD_BUTT_SLIDE_AIR) ||
                (m->action == ACT_OMM_ROLL_AIR)) {
                sOmmCamPitch = approach(sOmmCamPitch, camPitchTarget, 0x400);
                sOmmCamYaw = approach(sOmmCamYaw, sOmmCamYawTarget, 0x800);
                return;
            }
        }
    }
    
    // Default
    sOmmCamYawTarget = get_nearest_dir_angle(sOmmCamYawTarget);
    sOmmCamPitch = approach(sOmmCamPitch, camPitchTarget, 0x400);
    sOmmCamYaw = approach(sOmmCamYaw, sOmmCamYawTarget, 0x800);
}

static void omm_camera_update_transform(struct MarioState *m) {
    f32 camFocHeight = 120.f;
    f32 camPosOffsetY = 0.f;
    f32 camFocOffsetY = 0.f;
    f32 camDistTarget = sOmmCameraDistances[sOmmCamDistMode];
    s16 camPitchTarget = sOmmCamPitchTarget;

    // Compute Y offsets on ground and airborne
    if (!(m->action & ACT_FLAG_SWIMMING)) {
        omm_camera_calc_y_offsets(&camPosOffsetY, &camFocOffsetY);
    }

    // Bowser fight camera
    // The distance is increased and the pitch is set
    // to be high enough to show the whole arena
    if (OMM_CAMERA_IS_BOWSER_FIGHT) {
        camPitchTarget = 0x1800;
        camDistTarget *= 2.5f;
    }

    // Possessed Goomba tower
    // The camera focus and distance are computed to always show
    // the entire stack if the distance setting is at its max value
    if (sOmmCamDistMode == OMM_CAM_DIST_MODE_MAX && m->action == ACT_OMM_POSSESSION &&
        omm_obj_is_goomba(gOmmData->mario->capture.obj)) {
        f32 goombaHeight = omm_capture_get_top(gOmmData->mario->capture.obj);
        camFocHeight = omm_max_f(camFocHeight, goombaHeight / 1.8f);
        camDistTarget = omm_max_f(camDistTarget, goombaHeight * 2.5f);
    }

    // Angles
    omm_camera_update_angles_from_state(m, camPitchTarget);

    // Focus
    struct Object *playAsCappy = omm_cappy_get_object_play_as();
    if (playAsCappy) {
        sOmmCamFocus[0] = playAsCappy->oPosX;
        sOmmCamFocus[1] = playAsCappy->oPosY + camFocHeight;
        sOmmCamFocus[2] = playAsCappy->oPosZ;
    } else {
        sOmmCamFocus[0] = m->pos[0];
        sOmmCamFocus[1] = m->pos[1] + camFocHeight;
        sOmmCamFocus[2] = m->pos[2];
    }
    
    // Position
    sOmmCamDistance = approach(sOmmCamDistance, camDistTarget, 0x80);
    Vec3f v = { 0, 0, sOmmCamDistance };
    vec3f_rotate_zxy(v, v, -(sOmmCamPitch + gLakituState.shakeMagnitude[0]), +(sOmmCamYaw + gLakituState.shakeMagnitude[1]), 0);
    sOmmCamPos[0] = sOmmCamFocus[0] + v[0];
    sOmmCamPos[1] = sOmmCamFocus[1] + v[1] + camPosOffsetY;
    sOmmCamPos[2] = sOmmCamFocus[2] + v[2];

    // Collisions
    omm_camera_process_collisions();

    // Offset
    sOmmCamFocus[1] += camFocOffsetY;
}

static void omm_camera_apply(struct Camera *c) {

    // Position
    c->pos[0] = gLakituState.pos[0] = sOmmCamPos[0];
    c->pos[1] = gLakituState.pos[1] = sOmmCamPos[1];
    c->pos[2] = gLakituState.pos[2] = sOmmCamPos[2];

    // Focus
    c->focus[0] = gLakituState.focus[0] = sOmmCamFocus[0];
    c->focus[1] = gLakituState.focus[1] = sOmmCamFocus[1];
    c->focus[2] = gLakituState.focus[2] = sOmmCamFocus[2];

    // Angle
    c->yaw = gLakituState.yaw = sOmmCamYaw;

    // Mario transparency
    f32 dist = sqrtf(
        omm_sqr_f(gLakituState.curFocus[0] - gLakituState.curPos[0]) +
        omm_sqr_f(gLakituState.curFocus[1] - gLakituState.curPos[1]) +
        omm_sqr_f(gLakituState.curFocus[2] - gLakituState.curPos[2])
    );
    if (dist < 300.f) {
        u8 alpha = (u8) ((omm_max_f(dist - 100.f, 0) * 255.f) / 200.f);
        gMarioState->marioBodyState->modelState &= ~0xFF;
        gMarioState->marioBodyState->modelState |= (0x100 | alpha);
    }

    // Remove the cannon reticle after shooting
    if (c->mode == CAMERA_MODE_INSIDE_CANNON) {
        c->mode = c->defMode;
    }
}

bool omm_camera_update(struct Camera *c, struct MarioState *m) {
    if (!omm_camera_is_available(m)) {
        omm_camera_exit_first_person(c, m, false, true);
        return false;
    }

    // If Mario is in ACT_FIRST_PERSON when he shouldn't, 
    // disable the first person mode and reset the camera
    if (!sOmmCamFpMode && m->action == ACT_FIRST_PERSON) {
        omm_camera_exit_first_person(c, m, true, true);
    }

    // Init the camera
    gCamera = c;
    sStatusFlags &= ~CAM_FLAG_FRAME_AFTER_CAM_INIT;
    if (gCameraMovementFlags & CAM_MOVE_INIT_CAMERA) {
        init_camera(c);
        gCameraMovementFlags &= ~CAM_MOVE_INIT_CAMERA;
        sStatusFlags |= CAM_FLAG_FRAME_AFTER_CAM_INIT;
    }

    // Pre-update some camera values
    sMarioGeometry.prevFloorHeight = sMarioGeometry.currFloorHeight;
    sMarioGeometry.prevCeilHeight = sMarioGeometry.currCeilHeight;
    sMarioGeometry.prevFloor = sMarioGeometry.currFloor;
    sMarioGeometry.prevCeil = sMarioGeometry.currCeil;
    sMarioGeometry.prevFloorType = sMarioGeometry.currFloorType;
    sMarioGeometry.prevCeilType = sMarioGeometry.currCeilType;
    find_mario_floor_and_ceil(&sMarioGeometry);
    gCheckingSurfaceCollisionsForCamera = TRUE;
    vec3f_copy(c->pos, gLakituState.goalPos);
    vec3f_copy(c->focus, gLakituState.goalFocus);
    c->yaw = gLakituState.yaw;
    c->nextYaw = gLakituState.nextYaw;
    c->mode = gLakituState.mode;
    c->defMode = gLakituState.defMode;

    // Init yaws for "play as Cappy"
    struct Object *playAsCappy = omm_cappy_get_object_play_as();
    if (OMM_CAMERA_CLASSIC && !(playAsCappy->oCappyFlags & CAPPY_FLAG_CAMERA)) {
        sOmmCamYaw = c->yaw;
        sOmmCamYawTarget = c->yaw;
        playAsCappy->oCappyFlags |= CAPPY_FLAG_CAMERA;
    }

    // Play the current cutscene...
    if (c->cutscene != 0) {
        sYawSpeed = 0;
        play_cutscene(c);
        sFramesSinceCutsceneEnded = 0;
    }

    // ...or update the camera
    else {
        sFramesSinceCutsceneEnded = (sFramesSinceCutsceneEnded + 1) * (gRecentCutscene != 0);
        gRecentCutscene *= (sFramesSinceCutsceneEnded < 8);
        omm_camera_process_inputs(c, m);
        if (sOmmCamFpMode) omm_camera_update_first_person_mode(m);
        else omm_camera_update_transform(m);
        omm_camera_apply(c);
        sYawSpeed = 0x400;
    }

    // Start any Mario-related cutscenes
    // Don't trigger door camera events as long as OMM cam is enabled
    sMarioCamState->cameraEvent *= (sMarioCamState->cameraEvent != CAM_EVENT_DOOR);
    start_cutscene(c, get_cutscene_from_mario_status(c));

    // Update Lakitu
    update_lakitu(c);
    gLakituState.lastFrameAction = sMarioCamState->action;
    gCheckingSurfaceCollisionsForCamera = FALSE;
    return true;
}
