#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

typedef struct {
    f32 x, y, z;
} v3f;

typedef struct {
    v3f data[0x100];
    s32 count;
} v3fa;

inline static f32 v3f_mag(v3f v) {
    return sqrtf(omm_sqr_f(v.x) + omm_sqr_f(v.y) + omm_sqr_f(v.z));
}

//
// Cappy eyes
//

#define CAPPY_EYES_COUNT_DEFAULT    32
#define CAPPY_EYES_RADIUS_DEFAULT   44.f
#define CAPPY_EYES_FORWARD_DEFAULT  8.f
#define CAPPY_EYES_GAP              0.90f
#define CAPPY_EYES_TEXCOORDS_MULT   1.05f
#define CAPPY_EYES_WH_RATIO         0.75f
#define CAPPY_EYES_DEPTH            16
#define CAPPY_EYES_CURVE            12

static FILE *omm_cappy_gfx_find_file(u32 id) {
    OMM_STRING(folder, 256, "%s/%s", OMM_EXE_FOLDER, OMM_CAPPY_FOLDER);
    DIR *dir = opendir(folder);
    if (dir) {
        struct dirent *dirent = NULL;
        while ((dirent = readdir(dir)) != NULL) {
            if (strcmp(dirent->d_name, ".") == 0) continue;
            if (strcmp(dirent->d_name, "..") == 0) continue;
            OMM_STRING(dirname, 256, "%s/%s/%s", OMM_EXE_FOLDER, OMM_CAPPY_FOLDER, dirent->d_name);
            if (fs_sys_dir_exists(dirname)) {
                OMM_STRING(filename, 256, "%s/%08X.txt", dirname, id);
                FILE *f = fopen(filename, "r");
                if (f) {
                    closedir(dir);
                    return f;
                }
            }
        }
        closedir(dir);
    }
    return NULL;
}

static bool omm_cappy_gfx_read_data_from_file(u32 id, v3fa *points, v3fa *normals, v3fa *tops, s32 *count, f32 *radius, f32 *offset) {
    FILE *f = omm_cappy_gfx_find_file(id);
    if (!f) return false;

    // Init
    points->count = 0;
    normals->count = 0;
    tops->count = 0;
    *count = CAPPY_EYES_COUNT_DEFAULT;
    *radius = CAPPY_EYES_RADIUS_DEFAULT;
    *offset = CAPPY_EYES_FORWARD_DEFAULT;

    // Read
    char buffer[256];
    while (fgets(buffer, 256, f)) {
        switch (buffer[0]) {
            case 'p': {
                v3f point = { 0, 0, 0 };
                sscanf(buffer + 1, "%f %f %f", &point.x, &point.y, &point.z);
                points->data[points->count++] = point;
            } break;

            case 'n': {
                v3f normal = { 0, 0, 0 };
                sscanf(buffer + 1, "%f %f %f", &normal.x, &normal.y, &normal.z);
                normals->data[normals->count++] = normal;
            } break;

            case 'v': {
                v3f top = { 0, 0, 0 };
                sscanf(buffer + 1, "%f %f %f", &top.x, &top.y, &top.z);
                tops->data[tops->count++] = top;
            } break;

            case 'c': {
                sscanf(buffer + 1, "%d", count);
                *count = omm_clamp_s(*count, 4, 32);
            } break;

            case 'r': {
                sscanf(buffer + 1, "%f", radius);
            } break;

            case 'f': {
                sscanf(buffer + 1, "%f", offset);
            } break;
        }
    }
    fclose(f);
    return (points->count != 0) && (normals->count != 0) && (tops->count != 0);
}

static v3f omm_cappy_gfx_get_origin(const v3fa *points) {
    v3f ori = { 0, 0, 0 };
    for (s32 i = 0; i != points->count; ++i) {
        ori.x += points->data[i].x;
        ori.y += points->data[i].y;
        ori.z += points->data[i].z;
    }
    ori.x /= points->count;
    ori.y /= points->count;
    ori.z /= points->count;
    return ori;
}

static v3f omm_cappy_gfx_get_forward_axis(const v3fa *normals) {
    v3f fwd = { 0, 0, 0 };
    for (s32 i = 0; i != normals->count; ++i) {
        fwd.x += normals->data[i].x;
        fwd.y += normals->data[i].y;
        fwd.z += normals->data[i].z;
    }
    f32 mag = v3f_mag(fwd);
    fwd.x /= mag;
    fwd.y /= mag;
    fwd.z /= mag;
    return fwd;
}

static v3f omm_cappy_gfx_get_vertical_axis(v3f ori, v3f fwd, const v3fa *tops) {

    // Top
    v3f top = { 0, 0, 0 };
    for (s32 i = 0; i != tops->count; ++i) {
        top.x += tops->data[i].x;
        top.y += tops->data[i].y;
        top.z += tops->data[i].z;
    }
    top.x /= tops->count;
    top.y /= tops->count;
    top.z /= tops->count;

    // Pseudo-vertical axis
    v3f vrt0 = {
        top.x - ori.x,
        top.y - ori.y,
        top.z - ori.z
    };
    f32 mag0 = v3f_mag(vrt0);
    vrt0.x /= mag0;
    vrt0.y /= mag0;
    vrt0.z /= mag0;

    // True vertical axis
    // V = V0 - (V0 . F) * F
    f32 dot = (vrt0.x * fwd.x) + (vrt0.y * fwd.y) + (vrt0.z * fwd.z);
    v3f vrt = {
        vrt0.x - dot * fwd.x,
        vrt0.y - dot * fwd.y,
        vrt0.z - dot * fwd.z
    };
    f32 mag = v3f_mag(vrt);
    vrt.x /= mag;
    vrt.y /= mag;
    vrt.z /= mag;
    return vrt;
}

static v3f omm_cappy_gfx_get_horizontal_axis(v3f fwd, v3f vrt) {

    // Rodrigues' rotation formula with theta = -90 degrees
    // H = (V x F) + F * (V . F)
    f32 dot = (vrt.x * fwd.x) + (vrt.y * fwd.y) + (vrt.z * fwd.z);
    v3f hrz = {
        (fwd.z * vrt.y) - (fwd.y * vrt.z) + fwd.x * dot,
        (fwd.x * vrt.z) - (fwd.z * vrt.x) + fwd.y * dot,
        (fwd.y * vrt.x) - (fwd.x * vrt.y) + fwd.z * dot
    };
    f32 mag = v3f_mag(hrz);
    hrz.x /= mag;
    hrz.y /= mag;
    hrz.z /= mag;
    return hrz;
}

//
// Cappy eyes display list
//

static Vtx omm_cappy_gfx_get_vertex(v3f ori, v3f hrz, v3f vrt, v3f fwd, f32 hrzv, f32 vrtv, f32 fwdv, f32 u, f32 v) {
    if (u < 0) u = ((u + 0.5f) * CAPPY_EYES_TEXCOORDS_MULT) - 0.5f;
    else       u = ((u - 0.5f) * CAPPY_EYES_TEXCOORDS_MULT) + 0.5f;
    if (v < 0) v = ((v + 0.5f) * CAPPY_EYES_TEXCOORDS_MULT) - 0.5f;
    else       v = ((v - 0.5f) * CAPPY_EYES_TEXCOORDS_MULT) + 0.5f;
    Vtx vtx = { { { 0, 0, 0 }, 0, { (s16) (u * 0x1000), (s16) (v * 0x1000) }, { 0xFF, 0xFF, 0xFF, 0xFF } } };
    vtx.v.ob[0] = hrzv * hrz.x + vrtv * vrt.x + fwdv * fwd.x + ori.x;
    vtx.v.ob[1] = hrzv * hrz.y + vrtv * vrt.y + fwdv * fwd.y + ori.y;
    vtx.v.ob[2] = hrzv * hrz.z + vrtv * vrt.z + fwdv * fwd.z + ori.z;
    return vtx;
}

static Gfx *omm_cappy_gfx_get_display_list(u32 id, bool metal, bool mirror) {

    // Load data from file
    v3fa points;
    v3fa normals;
    v3fa tops;
    s32 count;
    f32 radius;
    f32 offset;
    if (!omm_cappy_gfx_read_data_from_file(id, &points, &normals, &tops, &count, &radius, &offset)) {
        return NULL;
    }

    // Axes
    v3f ori = omm_cappy_gfx_get_origin(&points);
    v3f fwd = omm_cappy_gfx_get_forward_axis(&normals);
    v3f vrt = omm_cappy_gfx_get_vertical_axis(ori, fwd, &tops);
    v3f hrz = omm_cappy_gfx_get_horizontal_axis(fwd, vrt);

    // Offset
    ori.x += fwd.x * offset;
    ori.y += fwd.y * offset;
    ori.z += fwd.z * offset;

    // Vertices
    Vtx *vtxData = OMM_MEMNEW(Vtx, ((count * 4) + 2) * 2);
    Vtx *vtxHead = vtxData;

    // Triangles
    Gfx *triData = OMM_MEMNEW(Gfx, (((count * 4) + 3) * 2) + 1);
    Gfx *triHead = triData;

    // +1 for Left eye, -1 for Right eye
    for (s32 left = 0; left != 2; ++left) {
        f32 sign = (left ? +1 : -1);

        // Eye triangles
        gSPVertex(triHead++, vtxHead, count + 1, 0);
        for (s32 i = 0; i != count; ++i) {
            s32 v0 = 0;
            s32 v1 = (i + 1);
            s32 v2 = ((((i + 2) - 1) % count) + 1);
            gSP1Triangle(triHead++, v0, v1, v2, 0);
        }

        // Eye vertices
        *(vtxHead++) = omm_cappy_gfx_get_vertex(ori, hrz, vrt, fwd, sign * CAPPY_EYES_GAP * (radius * CAPPY_EYES_WH_RATIO), 0, -(1.f - coss(0x2000)) * CAPPY_EYES_CURVE, 0.5f, 0.5f);
        for (s32 i = 0; i != count; ++i) {
            s16 a = (s16) ((65536.f * i) / count);
            f32 u = (1.f - ((1.f + sins(a)) / 2.f)) * sign - ((sign - 1.f) / 2.f);
            f32 v = (1.f - ((1.f + coss(a)) / 2.f));
            f32 f = (1.f - (coss((sins(-sign * a) + 1.f) * 0x2000))) * CAPPY_EYES_CURVE;
            *(vtxHead++) = omm_cappy_gfx_get_vertex(ori, hrz, vrt, fwd, (sign * CAPPY_EYES_GAP - sins(a)) * (radius * CAPPY_EYES_WH_RATIO), coss(a) * radius, -f, u, v);
        }

        // Border triangles
        gSPVertex(triHead++, vtxHead, count * 2, 0);
        for (s32 i = 0; i != count; ++i) {
            s32 v0 = (2 * i + 0);
            s32 v1 = (2 * i + 1);
            s32 v2 = (2 * i + 2) % (count * 2);
            s32 v3 = (2 * i + 3) % (count * 2);
            gSP1Triangle(triHead++, v0, v1, v2, 0);
            gSP1Triangle(triHead++, v2, v1, v3, 0);
        }

        // Border vertices
        for (s32 i = 0; i != count; ++i) {
            s16 a = (s16) ((65536.f * i) / count);
            f32 u = (1.f - ((1.f + sins(a)) / 2.f)) * sign - ((sign - 1.f) / 2.f);
            f32 v = (1.f - ((1.f + coss(a)) / 2.f));
            f32 f = (1.f - (coss((sins(-sign * a) + 1.f) * 0x2000))) * CAPPY_EYES_CURVE;
            *(vtxHead++) = omm_cappy_gfx_get_vertex(ori, hrz, vrt, fwd, (sign * CAPPY_EYES_GAP - sins(a)) * (radius * CAPPY_EYES_WH_RATIO), coss(a) * radius, -f,                    u, v);
            *(vtxHead++) = omm_cappy_gfx_get_vertex(ori, hrz, vrt, fwd, (sign * CAPPY_EYES_GAP - sins(a)) * (radius * CAPPY_EYES_WH_RATIO), coss(a) * radius, -f - CAPPY_EYES_DEPTH, u, v);
        }

        // Back triangles
        gSPVertex(triHead++, vtxHead, count + 1, 0);
        for (s32 i = 0; i != count; ++i) {
            s32 v0 = 0;
            s32 v1 = (i + 1);
            s32 v2 = (((i + 2) - 1) % count) + 1;
            gSP1Triangle(triHead++, v0, v2, v1, 0);
        }

        // Back vertices
        *(vtxHead++) = omm_cappy_gfx_get_vertex(ori, hrz, vrt, fwd, sign * CAPPY_EYES_GAP * (radius * CAPPY_EYES_WH_RATIO), 0, -(1.f - coss(0x2000)) * CAPPY_EYES_CURVE - CAPPY_EYES_DEPTH, 0, 0);
        for (s32 i = 0; i != count; ++i) {
            s16 a = (s16) ((65536.f * i) / count);
            f32 f = (1.f - (coss((sins(-sign * a) + 1.f) * 0x2000))) * CAPPY_EYES_CURVE;
            *(vtxHead++) = omm_cappy_gfx_get_vertex(ori, hrz, vrt, fwd, (sign * CAPPY_EYES_GAP - sins(a)) * (radius * CAPPY_EYES_WH_RATIO), coss(a) * radius, -f - CAPPY_EYES_DEPTH, 0, 0);
        }
    }
    gSPEndDisplayList(triHead++);

    // Textures
    const char *texCappy = (count < 12 ? OMM_TEXTURE_CAPPY_EYE_32   : (count < 20 ? OMM_TEXTURE_CAPPY_EYE_64   : OMM_TEXTURE_CAPPY_EYE_128  ));
    const char *texMetal = (count < 12 ? OMM_TEXTURE_CAPPY_METAL_32 : (count < 20 ? OMM_TEXTURE_CAPPY_METAL_64 : OMM_TEXTURE_CAPPY_METAL_128));

    // Display list
    Gfx *gfxData = OMM_MEMNEW(Gfx, 0x1000);
    Gfx *gfxHead = gfxData + 1;
    gDPPipeSync(gfxHead++);
    gSPClearGeometryMode(gfxHead++, G_TEXTURE_GEN | G_CULL_BOTH);
    gSPSetGeometryMode(gfxHead++, mirror ? G_CULL_FRONT : G_CULL_BACK);
    gDPSetCombineLERP(gfxHead++, TEXEL0, SHADE, TEXEL0_ALPHA, SHADE, 0, 0, 0, ENVIRONMENT, TEXEL0, SHADE, TEXEL0_ALPHA, SHADE, 0, 0, 0, ENVIRONMENT); // G_CC_BLENDRGBFADEA
    gSPTexture(gfxHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPLoadTextureBlock(gfxHead++, texCappy, G_IM_FMT_RGBA, G_IM_SIZ_32b, 128, 128, 0, 0, 0, 0, 0, 0, 0);
    gSPDisplayList(gfxHead++, triData);
    gSPTexture(gfxHead++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_OFF);
    gDPPipeSync(gfxHead++);
    if (metal) {
        gSPSetGeometryMode(gfxHead++, G_TEXTURE_GEN);
        gDPSetCombineLERP(gfxHead++, 0, 0, 0, TEXEL0, 0, 0, 0, ENVIRONMENT, 0, 0, 0, TEXEL0, 0, 0, 0, ENVIRONMENT); // G_CC_DECALFADE
        gDPLoadTextureBlock(gfxHead++, texMetal, G_IM_FMT_RGBA, G_IM_SIZ_32b, 128, 128, 0, 0, 0, 0, 0, 0, 0);
        gSPTexture(gfxHead++, 0x1F00, 0x1F00, 0, G_TX_RENDERTILE, G_ON);
    } else {
        gDPSetCombineLERP(gfxHead++, 0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT, 0, 0, 0, SHADE, 0, 0, 0, ENVIRONMENT); // G_CC_SHADEFADEA
    }
    gSPEndDisplayList(gfxHead++);

    return gfxData;
}

static bool omm_cappy_gfx_draw(u32 id, u8 alpha, bool metal, bool mirror, void (*append)(void *, s16)) {
    static OmmArray sCappyEyesDisplayLists[2][2] = { NULL };
    omm_array_init(sCappyEyesDisplayLists[mirror][metal], uintptr_t);
    Gfx *displayList = NULL;

    // Find existing display list
    for (s32 i = 0; i != omm_array_count(sCappyEyesDisplayLists[mirror][metal]); i += 2) {
        if (id == (u32) omm_array_get(sCappyEyesDisplayLists[mirror][metal], uintptr_t, i + 0)) {
            displayList = (Gfx *) omm_array_get(sCappyEyesDisplayLists[mirror][metal], uintptr_t, i + 1);
            break;
        }
    }

    // Create new display list if not found
    if (!displayList) {
        displayList = omm_cappy_gfx_get_display_list(id, metal, mirror);
        if (!displayList) return false;
        omm_array_add_inplace(sCappyEyesDisplayLists[mirror][metal], uintptr_t, (uintptr_t) id);
        omm_array_add_inplace(sCappyEyesDisplayLists[mirror][metal], uintptr_t, (uintptr_t) displayList);
    }

    // Draw display list
    gDPSetEnvColor(displayList, 0xFF, 0xFF, 0xFF, alpha);
    append(displayList, (alpha != 0xFF) ? LAYER_TRANSPARENT : LAYER_OPAQUE);
    return true;
}

bool omm_cappy_gfx_draw_eyes(struct GraphNodeSwitchCase *node, void (*append)(void *, s16)) {
    if (OMM_EXTRAS_CAPPY_AND_TIARA && gCurGraphNodeObject) {

        // Mario's cap
        if (node->fnNode.func == (GraphNodeFunc) geo_switch_mario_cap_on_off) {

            // Cap must be on Mario's head
            struct MarioBodyState *bodyState = gMarioState->marioBodyState;
            if (bodyState->capState & 1) {
                return false;
            }

            // States
            u32 id = omm_cappy_gfx_get_graph_node_identifier(gCurGraphNodeObject->sharedChild);
            u8 alpha = ((bodyState->modelState & 0x100) ? (bodyState->modelState & 0xFF) : 0xFF);
            bool metal = (bodyState->modelState & MODEL_STATE_METAL) != 0;
            bool mirror = (gCurGraphNodeObject == &gMirrorMario);
            return omm_cappy_gfx_draw(id, alpha, metal, mirror, append);
        }

        // Cap object
        if (node->fnNode.func == (GraphNodeFunc) geo_switch_anim_state) {

            // Object's model must be a cap model
            struct Object *o = (struct Object *) gCurGraphNodeObject;
            for (s32 i = 0; i != OMM_NUM_PLAYABLE_CHARACTERS; ++i) {
                if (obj_check_model(o, omm_player_get_normal_cap(i)) ||
                    obj_check_model(o, omm_player_get_wing_cap(i)) ||
                    obj_check_model(o, omm_player_get_metal_cap(i)) ||
                    obj_check_model(o, omm_player_get_winged_metal_cap(i))) {

                    // States
                    u32 id = omm_cappy_gfx_get_graph_node_identifier(gCurGraphNodeObject->sharedChild);
                    return omm_cappy_gfx_draw(id, o->oOpacity, false, false, append);
                }
            }
        }
    }
    return false;
}

//
// Graph node identifier
// Compute a unique identification number for a given graph node,
// based on some values that constitute the graph node
//

static u16 sVtxCount = 0;
static u16 sTriCount = 0;

#define GFXCMD(gfx) ((gfx->words.w0 >> 24) & 0xFF)
static void omm_cappy_gfx_process_display_list(const Gfx *displayList) {
    static const Gfx sSPEndDisplayList[] = { gsSPEndDisplayList() };
    static const Gfx sSPDisplayList[] = { gsSPDisplayList(0) };
    static const Gfx sSPBranchList[] = { gsSPBranchList(0) };
    static const Gfx sSPVertex[] = { gsSPVertex(0, 0, 0) };
    static const Gfx sSP1Triangle[] = { gsSP1Triangle(0, 0, 0, 0) };
    static const Gfx sSP2Triangles[] = { gsSP2Triangles(0, 0, 0, 0, 0, 0, 0, 0) };
    if (displayList) {
        for (const Gfx *head = displayList;; ++head) {
        
            // End
            if (OMM_MEMCMP(head, sSPEndDisplayList, sizeof(sSPEndDisplayList))) {
                return;
            }

            // Display list
            if (OMM_MEMCMP(&head->words.w0, &sSPDisplayList->words.w0, sizeof(sSPDisplayList->words.w0))) {
                omm_cappy_gfx_process_display_list((const Gfx *) head->words.w1);
                continue;
            }

            // Branch list
            if (OMM_MEMCMP(&head->words.w0, &sSPBranchList->words.w0, sizeof(sSPBranchList->words.w0))) {
                omm_cappy_gfx_process_display_list((const Gfx *) head->words.w1);
                return;
            }

            // Vertex array
            if (GFXCMD(head) == GFXCMD(sSPVertex)) {
                sVtxCount += (u16) ((head->words.w0 >> 12) & 0xFF);
                continue;
            }

            // 1 Triangle
            if (GFXCMD(head) == GFXCMD(sSP1Triangle)) {
                sTriCount += 1;
                continue;
            }

            // 2 Triangles
            if (GFXCMD(head) == GFXCMD(sSP2Triangles)) {
                sTriCount += 2;
                continue;
            }
        }
    }
}

static void omm_cappy_gfx_process_graph_node(struct GraphNode *node) {
    if (node) {
        struct GraphNode *curr = node;
        do {
            if (curr->type == GRAPH_NODE_TYPE_TRANSLATION_ROTATION ||
                curr->type == GRAPH_NODE_TYPE_TRANSLATION ||
                curr->type == GRAPH_NODE_TYPE_ROTATION ||
                curr->type == GRAPH_NODE_TYPE_ANIMATED_PART ||
                curr->type == GRAPH_NODE_TYPE_BILLBOARD ||
                curr->type == GRAPH_NODE_TYPE_DISPLAY_LIST ||
                curr->type == GRAPH_NODE_TYPE_SCALE) {
                omm_cappy_gfx_process_display_list(((struct GraphNodeDisplayList *) curr)->displayList);
            }
            omm_cappy_gfx_process_graph_node(curr->children);
        } while ((curr = curr->next) != node);
    }
}

u32 omm_cappy_gfx_get_graph_node_identifier(struct GraphNode *node) {
    static OmmArray sGraphNodes = NULL;
    static OmmArray sIdentifiers = NULL;

    // Find the graph node and return its identifier
    s32 index = omm_array_find(sGraphNodes, node);
    if (index != -1) {
        return omm_array_get(sIdentifiers, u32, index);
    }
    
    // Compute the graph node identifier and add it to the look-up table
    sVtxCount = 0;
    sTriCount = 0;
    omm_cappy_gfx_process_graph_node(node);
    u32 identifier = (u32) ((sVtxCount << 16u) | (sTriCount << 0u));
    omm_array_init(sGraphNodes, struct GraphNode *);
    omm_array_add(sGraphNodes, node);
    omm_array_init(sIdentifiers, u32);
    omm_array_add(sIdentifiers, identifier);
    return identifier;
}
