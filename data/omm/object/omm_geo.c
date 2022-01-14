#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// Geo
//

static void omm_geo_relocate_graph_node_pointers(struct GraphNode *head, u64 offset) {
    struct GraphNode *node = head;
    do {
        if (node->prev) node->prev = (struct GraphNode *) ((u64) node->prev + offset);
        if (node->next) node->next = (struct GraphNode *) ((u64) node->next + offset);
        if (node->parent) node->parent = (struct GraphNode *) ((u64) node->parent + offset);
        if (node->children) node->children = (struct GraphNode *) ((u64) node->children + offset);
        if (node->children) omm_geo_relocate_graph_node_pointers(node->children, offset);
        node = node->next;
    } while (node != head);
}

struct GraphNode *omm_geo_get_graph_node(const GeoLayout *geoLayout, bool keepInMemory) {
    static struct { const GeoLayout *geoLayout; struct GraphNode *graphNode; } sLoadedGraphNodes[0x1000];
    static s32 sLoadedGraphNodeCount = 0;
    if (keepInMemory) {
        for (s32 i = 0; i != sLoadedGraphNodeCount; ++i) {
            if (sLoadedGraphNodes[i].geoLayout == geoLayout) {
                return sLoadedGraphNodes[i].graphNode;
            }
        }
    }

    // NULL check
    if (!geoLayout) {
        return NULL;
    }

    // Process the geo layout on a pool allocated on stack
    u8 poolBuffer[0x100000];
    struct AllocOnlyPool *pool = (struct AllocOnlyPool *) poolBuffer;
    pool->totalSpace = 0x100000 - sizeof(struct AllocOnlyPool);
    pool->usedSpace = 0;
    pool->startPtr = poolBuffer + sizeof(struct AllocOnlyPool);
    pool->freePtr = poolBuffer + sizeof(struct AllocOnlyPool);
    void *processed = (void *) process_geo_layout(pool, (void *) geoLayout);

    // Copy the graph node data to the minimum amount of memory needed allocated on heap
    if (processed && pool->usedSpace != 0) {
        struct GraphNode *node = OMM_MEMDUP(pool->startPtr, pool->usedSpace);
        omm_geo_relocate_graph_node_pointers(node, (u64) node - (u64) pool->startPtr);
        if (keepInMemory) {
            sLoadedGraphNodes[sLoadedGraphNodeCount].geoLayout = geoLayout;
            sLoadedGraphNodes[sLoadedGraphNodeCount].graphNode = node;
            sLoadedGraphNodeCount++;
            if (sLoadedGraphNodeCount >= 0x1000) {
                sys_fatal("Graph Node memory pool is full");
            }
        }
        return node;
    }
    return NULL;
}

void *omm_geo_get_geo_data(struct Object *o, s32 size, const Gfx *gfxSrc, s32 gfxSize) {
    if (o->oGeoData == NULL) {
        void *data = omm_memory_new(gOmmMemoryPoolGeoData, size, o);
        OMM_MEMCPY(data, gfxSrc, gfxSize);
        for (s32 i = 0, n = gfxSize / sizeof(const Gfx); i != n; ++i) {
            Gfx *gfx = ((Gfx *) data) + i;
            if (_SHIFTR(gfx->words.w0, 24, 8) == G_DL && gfx->words.w1 == (uintptr_t) NULL_dl) {
                gfx->words.w1 = (uintptr_t)(((u8 *) data) + gfxSize);
                break;
            }
        }
        o->oGeoData = (void *) data;
    }
    return o->oGeoData;
}

Gfx *omm_geo_link_geo_data(s32 callContext, struct GraphNode *node, UNUSED void *context) {
    if (gCurrGraphNodeObject && callContext == GEO_CONTEXT_RENDER) {
        struct GraphNodeDisplayList *displayListNode = (struct GraphNodeDisplayList *) node->next;
        displayListNode->displayList = gCurrGraphNodeObject->oGeoData;
    }
    return NULL;
}

//
// Mario
//

static Vec3f sMarioForeArmPos[2];
f32 *omm_geo_get_marios_forearm_pos(bool isLeft) {
    return sMarioForeArmPos[isLeft];
}

static Vec3f sMarioHandPos[2];
f32 *omm_geo_get_marios_hand_pos(bool isLeft) {
    return sMarioHandPos[isLeft];
}

static Vec3f sMarioHeadPos;
f32 *omm_geo_get_marios_head_pos() {
    return sMarioHeadPos;
}

static Vec3f sMarioHeadUp;
f32 *omm_geo_get_marios_head_up() {
    return sMarioHeadUp;
}

static Vec3f sMarioHeadForward;
f32 *omm_geo_get_marios_head_forward() {
    return sMarioHeadForward;
}

static Vec3f sMarioHeadRight;
f32 *omm_geo_get_marios_head_right() {
    return sMarioHeadRight;
}

static bool sMarioRootFlag;
static Vec3f sMarioRootPos;
f32 *omm_geo_get_marios_root_pos() {
    return sMarioRootPos;
}

static struct GraphNode *sMarioGraphNode = NULL;
static f32 sMarioHeight; // Vanilla Mario's height is 120
f32 omm_geo_get_marios_height() {
    return sMarioHeight;
}

static s32 sMarioRootNodeState = 0;
void omm_geo_fix_marios_anim_translation_y(struct Object *o, struct GraphNodeAnimatedPart *node, f32 *translationY) {
    if (o == gMarioObject) {
        if (sMarioRootNodeState < (1 + OMM_IS_60_FPS)) {
            if (node->translation[1] != 0) {
                f32 dty;
                if (gMarioObject->header.gfx.mAnimInfo.animID == MARIO_ANIM_DIVE) { // MARIO_ANIM_DIVE is weird: it has spikes in its y translation, so let's handle it separately
                    s32 af = (gMarioObject->header.gfx.mAnimInfo.animFrame << 16);
                    s32 afaa = (gMarioObject->header.gfx.mAnimInfo.animFrameAccelAssist);
                    f32 df = omm_clamp_0_1_f((f32) omm_max_s(af, afaa) / (f32) (19 * 0x10000));
                    dty = omm_lerp_f(df, 265.f, 85.f);
                } else {
                    dty = *translationY - node->translation[1];
                }
                s32 dta = (omm_invlerp_0_1_f(dty, 60.f, 150.f) * 0x8000) - 0x4000;
                *translationY -= ((f32) node->translation[1]) * (1.f - ((sins(dta) + 1.f) / 2.f));
            }
            
            // Hanging action offset
            if (omm_mario_is_hanging(gMarioState)) {
                *translationY += 2.f * (160.f - sMarioHeight);
            }
            
            // Jumbo star cutscene animation offset
            if (gMarioObject->header.gfx.mAnimInfo.animID == MARIO_ANIM_FINAL_BOWSER_RAISE_HAND_SPIN) {
                *translationY += 4.f * (sMarioHeight - 120.f + (60.f * (gMarioState->action != ACT_JUMBO_STAR_CUTSCENE)));
            }
            
            sMarioRootNodeState++;
            sMarioRootFlag = true;
        }
    } else {
        sMarioRootNodeState = 0;
    }
}

OMM_ROUTINE_GFX(omm_geo_fix_marios_anim_reset_flag) {
    sMarioRootNodeState = 0;
}

//
// Matrix stuff
//

typedef struct { f32 x, y, z; } v3f;
typedef struct { f32 m[4][4]; } m4f;

static m4f m4f_rotate_zxy_and_translate(v3f t, v3f r) {
    m4f m;
    f32 sx = sins(r.x);
    f32 cx = coss(r.x);
    f32 sy = sins(r.y);
    f32 cy = coss(r.y);
    f32 sz = sins(r.z);
    f32 cz = coss(r.z);
    m.m[0][0] = cy * cz + sx * sy * sz;
    m.m[0][1] = cx * sz;
    m.m[0][2] = -sy * cz + sx * cy * sz;
    m.m[0][3] = 0.f;
    m.m[1][0] = -cy * sz + sx * sy * cz;
    m.m[1][1] = cx * cz;
    m.m[1][2] = sy * sz + sx * cy * cz;
    m.m[1][3] = 0.f;
    m.m[2][0] = cx * sy;
    m.m[2][1] = -sx;
    m.m[2][2] = cx * cy;
    m.m[2][3] = 0.f;
    m.m[3][0] = t.x;
    m.m[3][1] = t.y;
    m.m[3][2] = t.z;
    m.m[3][3] = 1.f;
    return m;
}

static m4f m4f_rotate_xyz_and_translate(v3f t, v3f r) {
    m4f m;
    f32 sx = sins(r.x);
    f32 cx = coss(r.x);
    f32 sy = sins(r.y);
    f32 cy = coss(r.y);
    f32 sz = sins(r.z);
    f32 cz = coss(r.z);
    m.m[0][0] = cy * cz;
    m.m[0][1] = cy * sz;
    m.m[0][2] = -sy;
    m.m[0][3] = 0;
    m.m[1][0] = sx * sy * cz - cx * sz;
    m.m[1][1] = sx * sy * sz + cx * cz;
    m.m[1][2] = sx * cy;
    m.m[1][3] = 0;
    m.m[2][0] = cx * sy * cz + sx * sz;
    m.m[2][1] = cx * sy * sz - sx * cz;
    m.m[2][2] = cx * cy;
    m.m[2][3] = 0;
    m.m[3][0] = t.x;
    m.m[3][1] = t.y;
    m.m[3][2] = t.z;
    m.m[3][3] = 1;
    return m;
}

static m4f m4f_mul(m4f a, m4f b) {
    m4f m;
    m.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0] + a.m[0][2] * b.m[2][0];
    m.m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1] + a.m[0][2] * b.m[2][1];
    m.m[0][2] = a.m[0][0] * b.m[0][2] + a.m[0][1] * b.m[1][2] + a.m[0][2] * b.m[2][2];
    m.m[0][3] = 0.f;
    m.m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0] + a.m[1][2] * b.m[2][0];
    m.m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1] + a.m[1][2] * b.m[2][1];
    m.m[1][2] = a.m[1][0] * b.m[0][2] + a.m[1][1] * b.m[1][2] + a.m[1][2] * b.m[2][2];
    m.m[1][3] = 0.f;
    m.m[2][0] = a.m[2][0] * b.m[0][0] + a.m[2][1] * b.m[1][0] + a.m[2][2] * b.m[2][0];
    m.m[2][1] = a.m[2][0] * b.m[0][1] + a.m[2][1] * b.m[1][1] + a.m[2][2] * b.m[2][1];
    m.m[2][2] = a.m[2][0] * b.m[0][2] + a.m[2][1] * b.m[1][2] + a.m[2][2] * b.m[2][2];
    m.m[2][3] = 0.f;
    m.m[3][0] = a.m[3][0] * b.m[0][0] + a.m[3][1] * b.m[1][0] + a.m[3][2] * b.m[2][0] + b.m[3][0];
    m.m[3][1] = a.m[3][0] * b.m[0][1] + a.m[3][1] * b.m[1][1] + a.m[3][2] * b.m[2][1] + b.m[3][1];
    m.m[3][2] = a.m[3][0] * b.m[0][2] + a.m[3][1] * b.m[1][2] + a.m[3][2] * b.m[2][2] + b.m[3][2];
    m.m[3][3] = 1.f;
    return m;
}

static m4f m4f_scale(m4f a, v3f s) {
    m4f m;
    m.m[0][0] = a.m[0][0] * s.x;
    m.m[0][1] = a.m[0][1] * s.x;
    m.m[0][2] = a.m[0][2] * s.x;
    m.m[0][3] = a.m[0][3] * s.x;
    m.m[1][0] = a.m[1][0] * s.y;
    m.m[1][1] = a.m[1][1] * s.y;
    m.m[1][2] = a.m[1][2] * s.y;
    m.m[1][3] = a.m[1][3] * s.y;
    m.m[2][0] = a.m[2][0] * s.z;
    m.m[2][1] = a.m[2][1] * s.z;
    m.m[2][2] = a.m[2][2] * s.z;
    m.m[2][3] = a.m[2][3] * s.z;
    m.m[3][0] = a.m[3][0];
    m.m[3][1] = a.m[3][1];
    m.m[3][2] = a.m[3][2];
    m.m[3][3] = a.m[3][3];
    return m;
}

//
// Preprocessing
//

static void omm_geo_preprocess_graph_node_and_siblings(struct GraphNode *node);
static s16  sMatStackIndex;
static m4f  sMatStack[32];
static s16  sCurAnimFrame;
static u8   sCurAnimType;
static f32  sCurAnimMult;
static u16 *sCurAnimAttr;
static s16 *sCurAnimData;
static s32  sMarioHandFlag;
static v3f  sMarioHeadRot;

static void omm_geo_preprocess_graph_node_level_of_detail(struct GraphNodeLevelOfDetail *node) {
    if (node->minDistance <= 0 && 0 < node->maxDistance) {
        omm_geo_preprocess_graph_node_and_siblings(node->node.children);
    }
}

static void omm_geo_preprocess_graph_node_switch_case(struct GraphNodeSwitchCase *node) {
    if (node->fnNode.func != NULL) {
        node->fnNode.func(GEO_CONTEXT_RENDER, &node->fnNode.node, (void *) &sMatStack[sMatStackIndex]);
        if (node->fnNode.func == (GraphNodeFunc) geo_switch_mario_hand) {
            static bool sIsLeftHand = true;
            sMarioHandFlag = 1 + sIsLeftHand;
            sMarioForeArmPos[sIsLeftHand][0] = sMatStack[sMatStackIndex].m[3][0];
            sMarioForeArmPos[sIsLeftHand][1] = sMatStack[sMatStackIndex].m[3][1];
            sMarioForeArmPos[sIsLeftHand][2] = sMatStack[sMatStackIndex].m[3][2];
            sIsLeftHand = !sIsLeftHand;
        }
        if (node->fnNode.func == (GraphNodeFunc) geo_switch_mario_eyes) {
            sMarioHeadPos[0] = sMatStack[sMatStackIndex].m[3][0];
            sMarioHeadPos[1] = sMatStack[sMatStackIndex].m[3][1];
            sMarioHeadPos[2] = sMatStack[sMatStackIndex].m[3][2];

            m4f rotMatrix = sMatStack[sMatStackIndex];
            rotMatrix.m[3][0] = 0;
            rotMatrix.m[3][1] = 0;
            rotMatrix.m[3][2] = 0;

            m4f upMatrix = m4f_mul((m4f) { { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 1, 0, 0, 1 }, } }, rotMatrix);
            sMarioHeadUp[0] = upMatrix.m[3][0];
            sMarioHeadUp[1] = upMatrix.m[3][1];
            sMarioHeadUp[2] = upMatrix.m[3][2];
            vec3f_norm(sMarioHeadUp);

            m4f forwardMatrix = m4f_mul((m4f) { { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 1, 0, 1 }, } }, rotMatrix);
            sMarioHeadForward[0] = forwardMatrix.m[3][0];
            sMarioHeadForward[1] = forwardMatrix.m[3][1];
            sMarioHeadForward[2] = forwardMatrix.m[3][2];
            vec3f_norm(sMarioHeadForward);

            m4f rightMatrix = m4f_mul((m4f) { { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, -1, 1 }, } }, rotMatrix);
            sMarioHeadRight[0] = rightMatrix.m[3][0];
            sMarioHeadRight[1] = rightMatrix.m[3][1];
            sMarioHeadRight[2] = rightMatrix.m[3][2];
            vec3f_norm(sMarioHeadRight);
        }
    }

    struct GraphNode *selectedChild = node->fnNode.node.children;
    for (s32 i = 0; selectedChild != NULL && node->selectedCase > i; i++) {
        selectedChild = selectedChild->next;
    }
    omm_geo_preprocess_graph_node_and_siblings(selectedChild);
}

static void omm_geo_preprocess_graph_node_translation_rotation(struct GraphNodeTranslationRotation *node) {
    v3f t = { node->translation[0], node->translation[1], node->translation[2] };
    v3f r = { node->rotation[0], node->rotation[1], node->rotation[2] };
    sMatStack[sMatStackIndex + 1] = m4f_mul(m4f_rotate_zxy_and_translate(t, r), sMatStack[sMatStackIndex]);

    sMatStackIndex++;
    omm_geo_preprocess_graph_node_and_siblings(node->node.children);
    sMatStackIndex--;
}

static void omm_geo_preprocess_graph_node_translation(struct GraphNodeTranslation *node) {
    v3f t = { node->translation[0], node->translation[1], node->translation[2] };
    v3f r = { 0, 0, 0 };
    sMatStack[sMatStackIndex + 1] = m4f_mul(m4f_rotate_zxy_and_translate(t, r), sMatStack[sMatStackIndex]);

    sMatStackIndex++;
    omm_geo_preprocess_graph_node_and_siblings(node->node.children);
    sMatStackIndex--;
}

static void omm_geo_preprocess_graph_node_rotation(struct GraphNodeRotation *node) {
    v3f t = { 0, 0, 0 };
    v3f r = { node->rotation[0], node->rotation[1], node->rotation[2] };
    sMatStack[sMatStackIndex + 1] = m4f_mul(m4f_rotate_zxy_and_translate(t, r), sMatStack[sMatStackIndex]);

    sMatStackIndex++;
    omm_geo_preprocess_graph_node_and_siblings(node->node.children);
    sMatStackIndex--;
}

static void omm_geo_preprocess_graph_node_scale(struct GraphNodeScale *node) {
    v3f s = { node->scale, node->scale, node->scale };
    sMatStack[sMatStackIndex + 1] = m4f_scale(sMatStack[sMatStackIndex], s);

    sMatStackIndex++;
    omm_geo_preprocess_graph_node_and_siblings(node->node.children);
    sMatStackIndex--;
}

static void omm_geo_preprocess_graph_node_animated_part(struct GraphNodeAnimatedPart *node) {
    if (OMM_LIKELY(sCurAnimAttr && sCurAnimData)) {
        v3f t = { node->translation[0], node->translation[1], node->translation[2] };
        v3f r = { 0, 0, 0 };

        if (sCurAnimType == ANIM_TYPE_TRANSLATION) {
            t.x += sCurAnimData[retrieve_animation_index(sCurAnimFrame, &sCurAnimAttr)] * sCurAnimMult;
            t.y += sCurAnimData[retrieve_animation_index(sCurAnimFrame, &sCurAnimAttr)] * sCurAnimMult;
            t.z += sCurAnimData[retrieve_animation_index(sCurAnimFrame, &sCurAnimAttr)] * sCurAnimMult;
            sCurAnimType = ANIM_TYPE_ROTATION;
        } else {
            if (sCurAnimType == ANIM_TYPE_LATERAL_TRANSLATION) {
                t.x += sCurAnimData[retrieve_animation_index(sCurAnimFrame, &sCurAnimAttr)] * sCurAnimMult;
                sCurAnimAttr += 2;
                t.z += sCurAnimData[retrieve_animation_index(sCurAnimFrame, &sCurAnimAttr)] * sCurAnimMult;
                sCurAnimType = ANIM_TYPE_ROTATION;
            } else {
                if (sCurAnimType == ANIM_TYPE_VERTICAL_TRANSLATION) {
                    sCurAnimAttr += 2;
                    t.y += sCurAnimData[retrieve_animation_index(sCurAnimFrame, &sCurAnimAttr)] * sCurAnimMult;
                    sCurAnimAttr += 2;
                    sCurAnimType = ANIM_TYPE_ROTATION;
                } else if (sCurAnimType == ANIM_TYPE_NO_TRANSLATION) {
                    sCurAnimAttr += 6;
                    sCurAnimType = ANIM_TYPE_ROTATION;
                }
            }
        }
        if (sCurAnimType == ANIM_TYPE_ROTATION) {
            r.x = sCurAnimData[retrieve_animation_index(sCurAnimFrame, &sCurAnimAttr)];
            r.y = sCurAnimData[retrieve_animation_index(sCurAnimFrame, &sCurAnimAttr)];
            r.z = sCurAnimData[retrieve_animation_index(sCurAnimFrame, &sCurAnimAttr)];
        }

        omm_geo_fix_marios_anim_translation_y(gCurrGraphNodeObject, node, &t.y);
        if (OMM_IS_60_FPS) {
            f32 dummy = 0.f; // Needs to be called twice for 60 FPS interpolation
            omm_geo_fix_marios_anim_translation_y(gCurrGraphNodeObject, node, &dummy);
        }
        sMatStack[sMatStackIndex + 1] = m4f_mul(m4f_rotate_xyz_and_translate(t, r), sMatStack[sMatStackIndex]);
        if (sMarioRootFlag) {
            sMarioRootPos[0] = sMatStack[sMatStackIndex + 1].m[3][0];
            sMarioRootPos[1] = sMatStack[sMatStackIndex + 1].m[3][1];
            sMarioRootPos[2] = sMatStack[sMatStackIndex + 1].m[3][2];
            sMarioRootFlag = 0;
        } else if (sMarioHandFlag) {
            sMarioHandPos[sMarioHandFlag - 1][0] = sMatStack[sMatStackIndex + 1].m[3][0];
            sMarioHandPos[sMarioHandFlag - 1][1] = sMatStack[sMatStackIndex + 1].m[3][1];
            sMarioHandPos[sMarioHandFlag - 1][2] = sMatStack[sMatStackIndex + 1].m[3][2];
            sMarioHandFlag = 0;
        }

        sMatStackIndex++;
        omm_geo_preprocess_graph_node_and_siblings(node->node.children);
        sMatStackIndex--;
    }
}

static void omm_geo_preprocess_graph_node_generated_list(struct GraphNodeGenerated *node) {
    if (node->fnNode.func != NULL) {
        node->fnNode.func(GEO_CONTEXT_RENDER, &node->fnNode.node, (void *) &sMatStack[sMatStackIndex]);
        if (node->fnNode.func == (GraphNodeFunc) geo_mario_head_rotation) {
            struct GraphNodeRotation *rotNode = (struct GraphNodeRotation *) node->fnNode.node.next;
            sMarioHeadRot.x += rotNode->rotation[0];
            sMarioHeadRot.y += rotNode->rotation[1];
            sMarioHeadRot.z += rotNode->rotation[2];
        }
    }
    omm_geo_preprocess_graph_node_and_siblings(node->fnNode.node.children);
}

static void omm_geo_preprocess_graph_node_culling_radius(struct GraphNodeCullingRadius *node) {
    static Vec3f sUpVector;
    static Vec3f sFwdVector;
    static Vec3f sLatVector;
    static Vec3f sTorsoPos;
    static Vec3f sLeftLegPos;
    static Vec3f sLeftFootPos;
    static Vec3f sRightLegPos;
    static Vec3f sRightFootPos;
    static Vec3s sPeachDressRot;

    // Use this node as a special GEO_ASM that works only during preprocessing
    // Each index is mapped to a specific function
    switch (node->cullingRadius) {

        // Compute the 'up' vector
        case 1: {
            sUpVector[0] = sMatStack[sMatStackIndex].m[3][0] - gCurrGraphNodeObject->header.gfx.pos[0];
            sUpVector[1] = sMatStack[sMatStackIndex].m[3][1] - gCurrGraphNodeObject->header.gfx.pos[1];
            sUpVector[2] = sMatStack[sMatStackIndex].m[3][2] - gCurrGraphNodeObject->header.gfx.pos[2];
        } break;

        // Compute the 'forward' vector
        case 2: {
            sFwdVector[0] = sMatStack[sMatStackIndex].m[3][0] - gCurrGraphNodeObject->header.gfx.pos[0];
            sFwdVector[1] = sMatStack[sMatStackIndex].m[3][1] - gCurrGraphNodeObject->header.gfx.pos[1];
            sFwdVector[2] = sMatStack[sMatStackIndex].m[3][2] - gCurrGraphNodeObject->header.gfx.pos[2];
        } break;

        // Compute the 'lateral' vector
        case 3: {
            sLatVector[0] = sMatStack[sMatStackIndex].m[3][0] - gCurrGraphNodeObject->header.gfx.pos[0];
            sLatVector[1] = sMatStack[sMatStackIndex].m[3][1] - gCurrGraphNodeObject->header.gfx.pos[1];
            sLatVector[2] = sMatStack[sMatStackIndex].m[3][2] - gCurrGraphNodeObject->header.gfx.pos[2];
        } break;

        // Compute torso position
        case 4: {
            sTorsoPos[0] = sMatStack[sMatStackIndex].m[3][0] - gCurrGraphNodeObject->header.gfx.pos[0];
            sTorsoPos[1] = sMatStack[sMatStackIndex].m[3][1] - gCurrGraphNodeObject->header.gfx.pos[1];
            sTorsoPos[2] = sMatStack[sMatStackIndex].m[3][2] - gCurrGraphNodeObject->header.gfx.pos[2];
        } break;

        // Compute left leg position
        case 5: {
            sLeftLegPos[0] = sMatStack[sMatStackIndex].m[3][0] - gCurrGraphNodeObject->header.gfx.pos[0];
            sLeftLegPos[1] = sMatStack[sMatStackIndex].m[3][1] - gCurrGraphNodeObject->header.gfx.pos[1];
            sLeftLegPos[2] = sMatStack[sMatStackIndex].m[3][2] - gCurrGraphNodeObject->header.gfx.pos[2];
        } break;

        // Compute left foot position
        case 6: {
            sLeftFootPos[0] = sMatStack[sMatStackIndex].m[3][0] - gCurrGraphNodeObject->header.gfx.pos[0];
            sLeftFootPos[1] = sMatStack[sMatStackIndex].m[3][1] - gCurrGraphNodeObject->header.gfx.pos[1];
            sLeftFootPos[2] = sMatStack[sMatStackIndex].m[3][2] - gCurrGraphNodeObject->header.gfx.pos[2];
        } break;

        // Compute right leg position
        case 7: {
            sRightLegPos[0] = sMatStack[sMatStackIndex].m[3][0] - gCurrGraphNodeObject->header.gfx.pos[0];
            sRightLegPos[1] = sMatStack[sMatStackIndex].m[3][1] - gCurrGraphNodeObject->header.gfx.pos[1];
            sRightLegPos[2] = sMatStack[sMatStackIndex].m[3][2] - gCurrGraphNodeObject->header.gfx.pos[2];
        } break;

        // Compute right foot position
        case 8: {
            sRightFootPos[0] = sMatStack[sMatStackIndex].m[3][0] - gCurrGraphNodeObject->header.gfx.pos[0];
            sRightFootPos[1] = sMatStack[sMatStackIndex].m[3][1] - gCurrGraphNodeObject->header.gfx.pos[1];
            sRightFootPos[2] = sMatStack[sMatStackIndex].m[3][2] - gCurrGraphNodeObject->header.gfx.pos[2];
        } break;

        // Compute Peach's dress rotation, set hips rotation, set legs visibility
        case 9: {

            // Origin (torso)
            Vec3f o = {
                sTorsoPos[0],
                sTorsoPos[1],
                sTorsoPos[2]
            };

            // Up
            Vec3f u = {
                sUpVector[0] - o[0],
                sUpVector[1] - o[1],
                sUpVector[2] - o[2],
            };
            vec3f_normalize(u);

            // Down
            Vec3f d = {
                o[0] - sUpVector[0],
                o[1] - sUpVector[1],
                o[2] - sUpVector[2],
            };
            vec3f_normalize(d);

            // Forward
            Vec3f v = {
                sFwdVector[0] - o[0],
                sFwdVector[1] - o[1],
                sFwdVector[2] - o[2],
            };
            vec3f_normalize(v);

            // Lateral
            Vec3f n = {
                sLatVector[0] - o[0],
                sLatVector[1] - o[1],
                sLatVector[2] - o[2],
            };
            vec3f_normalize(n);

            // Left leg
            Vec3f ll = {
                sLeftLegPos[0],
                sLeftLegPos[1],
                sLeftLegPos[2],
            };
            vec3f_project_point(ll, ll, o, n);

            // Left foot
            Vec3f lf = {
                sLeftFootPos[0],
                sLeftFootPos[1],
                sLeftFootPos[2],
            };
            vec3f_project_point(lf, lf, o, n);

            // Right leg
            Vec3f rl = {
                sRightLegPos[0],
                sRightLegPos[1],
                sRightLegPos[2],
            };
            vec3f_project_point(rl, rl, o, n);

            // Right foot
            Vec3f rf = {
                sRightFootPos[0],
                sRightFootPos[1],
                sRightFootPos[2],
            };
            vec3f_project_point(rf, rf, o, n);

            // Left leg vector
            Vec3f l = {
                lf[0] - ll[0],
                lf[1] - ll[1],
                lf[2] - ll[2]
            };
            vec3f_normalize(l);

            // Right leg vector
            Vec3f r = {
                rf[0] - rl[0],
                rf[1] - rl[1],
                rf[2] - rl[2]
            };
            vec3f_normalize(r);

            // Angle left
            f32 ldot = l[0] * d[0] + l[1] * d[1] + l[2] * d[2];
            s16 la = (s16) ((acosf(ldot) / M_PI) * 0x8000);
            Vec3f ldcross; vec3f_cross(ldcross, l, d);
            if (vec3f_dot(n, ldcross) < 0) {
                la = -la;
            }

            // Angle right
            f32 rdot = r[0] * d[0] + r[1] * d[1] + r[2] * d[2];
            s16 ra = (s16) ((acosf(rdot) / M_PI) * 0x8000);
            Vec3f rdcross; vec3f_cross(rdcross, r, d);
            if (vec3f_dot(n, rdcross) < 0) {
                ra = -ra;
            }

            // Dress pitch
            s16 ua = omm_min_s(la, ra);
            s16 va = omm_max_s(la, ra);
            f32 da = (f32) ((u16) (va - ua));
            f32 ta = (0.50f * (1.f - omm_clamp_0_1_f((da - 0x2000) / 0x4000)));
            f32 pa = ((ua + da * ta) / 3.f);
            sPeachDressRot[0] = 0;
            sPeachDressRot[1] = 0;
            sPeachDressRot[2] = ((s16) pa - 0x0800);

            // Legs visibility
            u16 dlp = (u16) omm_abs_s(la - (sPeachDressRot[2] * 2));
            u16 drp = (u16) omm_abs_s(ra - (sPeachDressRot[2] * 2));
            s32 animId = gCurrGraphNodeObject->header.gfx.mAnimInfo.animID;
            if (animId == MARIO_ANIM_WALKING || animId == MARIO_ANIM_RUNNING) {
                gCurrGraphNodeObject->oAnimState = ((dlp > 0x2800) << 1) | ((drp > 0x2800) << 0);
            } else {
                gCurrGraphNodeObject->oAnimState = ((dlp > 0x3000) << 1) | ((drp > 0x3000) << 0);
            }

            // Set hips pitch
            struct GraphNodeRotation *rotNode = (struct GraphNodeRotation *) node->node.next;
            rotNode->rotation[0] = -(sPeachDressRot[0] * 4) / 3;
            rotNode->rotation[1] = -(sPeachDressRot[1] * 4) / 3;
            rotNode->rotation[2] = -(sPeachDressRot[2] * 4) / 3;
        } break;

        // Set Peach's dress rotation
        case 10: {
            struct GraphNodeRotation *rotNode = (struct GraphNodeRotation *) node->node.next;
            rotNode->rotation[0] = -(sPeachDressRot[0] * 2) / 3;
            rotNode->rotation[1] = -(sPeachDressRot[1] * 2) / 3;
            rotNode->rotation[2] = -(sPeachDressRot[2] * 2) / 3;
        } break;
        
        // Compute head rotation
        case 11: {
            if (OMM_LIKELY(sCurAnimAttr && sCurAnimData) && sCurAnimType == ANIM_TYPE_ROTATION) {
                u16 *animAttributes = sCurAnimAttr;
                sMarioHeadRot.x = sCurAnimData[retrieve_animation_index(sCurAnimFrame, &animAttributes)];
                sMarioHeadRot.y = sCurAnimData[retrieve_animation_index(sCurAnimFrame, &animAttributes)];
                sMarioHeadRot.z = sCurAnimData[retrieve_animation_index(sCurAnimFrame, &animAttributes)];
            } else {
                sMarioHeadRot.x = 0;
                sMarioHeadRot.y = 0;
                sMarioHeadRot.z = 0;
            }
        } break;

        // Set Peach's hair rotation
        case 12: {
            struct GraphNodeRotation *rotNode = (struct GraphNodeRotation *) node->node.next;
            rotNode->rotation[0] = 0;
            rotNode->rotation[1] = -sMarioHeadRot.y / 2;
            rotNode->rotation[2] = -sMarioHeadRot.z / (1 + (sMarioHeadRot.z > 0));
        } break;

        // Switch between Peach's crown and Tiara
        case 13: {
            struct GraphNode *crownNode = node->node.next;
            struct GraphNode *tiaraNode = node->node.next->next;
            if (OMM_EXTRAS_CAPPY_AND_TIARA) {
                crownNode->flags &= ~GRAPH_RENDER_ACTIVE;
                tiaraNode->flags |= GRAPH_RENDER_ACTIVE;
            } else {
                crownNode->flags |= GRAPH_RENDER_ACTIVE;
                tiaraNode->flags &= ~GRAPH_RENDER_ACTIVE;
            }
        } break;

        // Switch between Perry's closed and open states
        case 14: {
            struct GraphNode *closeNode = node->node.next;
            struct GraphNode *openNode = node->node.next->next;
            if (gCurrGraphNodeObject->oAction != 0) {
                closeNode->flags &= ~GRAPH_RENDER_ACTIVE;
                openNode->flags |= GRAPH_RENDER_ACTIVE;
            } else {
                closeNode->flags |= GRAPH_RENDER_ACTIVE;
                openNode->flags &= ~GRAPH_RENDER_ACTIVE;
            }
        } break;
    }
}

static void omm_geo_preprocess_graph_node_and_siblings(struct GraphNode *node) {
    if (node) {
        struct GraphNode *current = node;
        do {
            switch (current->type) {
                case GRAPH_NODE_TYPE_LEVEL_OF_DETAIL:       omm_geo_preprocess_graph_node_level_of_detail((struct GraphNodeLevelOfDetail *) current); break;
                case GRAPH_NODE_TYPE_SWITCH_CASE:           omm_geo_preprocess_graph_node_switch_case((struct GraphNodeSwitchCase *) current); break;
                case GRAPH_NODE_TYPE_TRANSLATION_ROTATION:  omm_geo_preprocess_graph_node_translation_rotation((struct GraphNodeTranslationRotation *) current); break;
                case GRAPH_NODE_TYPE_TRANSLATION:           omm_geo_preprocess_graph_node_translation((struct GraphNodeTranslation *) current); break;
                case GRAPH_NODE_TYPE_ROTATION:              omm_geo_preprocess_graph_node_rotation((struct GraphNodeRotation *) current); break;
                case GRAPH_NODE_TYPE_ANIMATED_PART:         omm_geo_preprocess_graph_node_animated_part((struct GraphNodeAnimatedPart *) current); break;
                case GRAPH_NODE_TYPE_SCALE:                 omm_geo_preprocess_graph_node_scale((struct GraphNodeScale *) current); break;
                case GRAPH_NODE_TYPE_GENERATED_LIST:        omm_geo_preprocess_graph_node_generated_list((struct GraphNodeGenerated *) current); break;
                case GRAPH_NODE_TYPE_CULLING_RADIUS:        omm_geo_preprocess_graph_node_culling_radius((struct GraphNodeCullingRadius *) current); break;
                default:                                    omm_geo_preprocess_graph_node_and_siblings(current->children); break;        
            }
            if (current->parent && current->parent->type == GRAPH_NODE_TYPE_SWITCH_CASE) {
                break;
            }
            current = current->next;
        } while (current && current != node);
    }
}

//
// Animation
//

static s16 omm_geo_preprocess_animation_get_next_frame(struct AnimInfoStruct *obj) {
    struct Animation *anim = obj->curAnim;
    if ((obj->animTimer == gAreaUpdateCounter) || (anim->flags & ANIM_FLAG_2)) {
        return obj->animFrame;
    }

    s32 nextFrameAccelAssist;
    if (anim->flags & ANIM_FLAG_FORWARD) {
        if (obj->animAccel) {
            nextFrameAccelAssist = (obj->animFrameAccelAssist - obj->animAccel);
        } else {
            nextFrameAccelAssist = (obj->animFrame - 1) << 16;
        }
        if (nextFrameAccelAssist < (anim->mLoopStart << 16)) {
            if (anim->flags & ANIM_FLAG_NOLOOP) {
                nextFrameAccelAssist = (anim->mLoopStart << 16);
            } else {
                nextFrameAccelAssist = ((anim->mLoopEnd - 1) << 16);
            }
        }
    } else {
        if (obj->animAccel) {
            nextFrameAccelAssist = (obj->animFrameAccelAssist + obj->animAccel);
        } else {
            nextFrameAccelAssist = (obj->animFrame + 1) << 16;
        }
        if (nextFrameAccelAssist >= (anim->mLoopEnd << 16)) {
            if (anim->flags & ANIM_FLAG_NOLOOP) {
                nextFrameAccelAssist = ((anim->mLoopEnd - 1) << 16);
            } else {
                nextFrameAccelAssist = (anim->mLoopStart << 16);
            }
        }
    }
    return (nextFrameAccelAssist >> 16);
}

static void omm_geo_preprocess_animation_init_data(struct AnimInfoStruct *node, bool hasAnimation) {
    struct Animation *anim = node->curAnim;

    if (anim->flags & ANIM_FLAG_HOR_TRANS) {
        sCurAnimType = ANIM_TYPE_VERTICAL_TRANSLATION;
    } else if (anim->flags & ANIM_FLAG_VERT_TRANS) {
        sCurAnimType = ANIM_TYPE_LATERAL_TRANSLATION;
    } else if (anim->flags & ANIM_FLAG_6) {
        sCurAnimType = ANIM_TYPE_NO_TRANSLATION;
    } else {
        sCurAnimType = ANIM_TYPE_TRANSLATION;
    }

    if (hasAnimation) {
        sCurAnimFrame = omm_geo_preprocess_animation_get_next_frame(node);
    } else {
        sCurAnimFrame = node->animFrame;
    }

    sCurAnimAttr = (u16 *) anim->index;
    sCurAnimData = (s16 *) anim->values;

    if (anim->mAnimYTransDivisor == 0) {
        sCurAnimMult = 1.f;
    } else {
        sCurAnimMult = (f32) node->animYTrans / (f32) anim->mAnimYTransDivisor;
    }
}

static void __omm_geo_preprocess_object_graph_node(struct Object *o) {
    sMatStackIndex = 0;
    sCurAnimFrame = 0;
    sCurAnimType = 0;
    sCurAnimMult = 0.f;
    sCurAnimAttr = NULL;
    sCurAnimData = NULL;
    sMarioHandFlag = 0;
    sMarioRootFlag = false;
    sMarioRootNodeState = 0;

    // Init the base matrix
    if (o->header.gfx.throwMatrix) {
        OMM_MEMCPY(&sMatStack[sMatStackIndex].m, *o->header.gfx.throwMatrix, sizeof(f32) * 4 * 4);
    } else {
        v3f t = { o->header.gfx.pos[0], o->header.gfx.pos[1], o->header.gfx.pos[2] };
        v3f r = { o->header.gfx.angle[0], o->header.gfx.angle[1], o->header.gfx.angle[2] };
        sMatStack[sMatStackIndex] = m4f_rotate_zxy_and_translate(t, r);
    }
    v3f s = { o->header.gfx.scale[0], o->header.gfx.scale[1], o->header.gfx.scale[2] };
    sMatStack[sMatStackIndex] = m4f_scale(sMatStack[sMatStackIndex], s);

    // Init the animation data
    if (o->header.gfx.mAnimInfo.curAnim) {
        omm_geo_preprocess_animation_init_data(&o->header.gfx.mAnimInfo, (o->oNodeFlags & GRAPH_RENDER_HAS_ANIMATION) != 0);
    }

    // Init some globals
    Mat4 curGraphNodeCameraMatrix = { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } };
    struct GraphNodeCamera curGraphNodeCamera = { .config.camera = gCamera, .matrixPtr = &curGraphNodeCameraMatrix };
    gCurGraphNodeCamera = &curGraphNodeCamera;
    gCurGraphNodeObject = (struct GraphNodeObject *) o;

    // Preprocess
    omm_geo_preprocess_graph_node_and_siblings(o->oGraphNode);
    sMarioRootNodeState = 0;
}

//
// Public
//

void omm_geo_preprocess_object_graph_node(struct Object *o) {
    if (o && o->oGraphNode) {

        // Compute Mario's height
        if (o == gMarioObject && o->oGraphNode != sMarioGraphNode) {

            // Back-up animation data
            struct AnimInfoStruct marioAnimStruct;
            struct Animation marioAnim;
            OMM_MEMCPY(&marioAnimStruct, &o->header.gfx.mAnimInfo, sizeof(struct AnimInfoStruct));
            OMM_MEMCPY(&marioAnim, gMarioState->mMarioAnimations->mMarioTargetAnim, sizeof(struct Animation));

            // Preprocess with A-pose animation
            omm_mario_set_animation(gMarioState, MARIO_ANIM_A_POSE, 1.f, -1);
            __omm_geo_preprocess_object_graph_node(o);

            // Restore animation data and compute height
            OMM_MEMCPY(&o->header.gfx.mAnimInfo, &marioAnimStruct, sizeof(struct AnimInfoStruct));
            OMM_MEMCPY(gMarioState->mMarioAnimations->mMarioTargetAnim, &marioAnim, sizeof(struct Animation));
            sMarioHeight = 35.f + (sMarioHeadPos[1] - gMarioState->pos[1]);
            sMarioGraphNode = o->oGraphNode;
        }

        // Preprocess
        __omm_geo_preprocess_object_graph_node(o);
    }
}

#if OMM_CODE_DEBUG

static void omm_debug_render_surfaces_update_node(struct GraphNode *node, bool active) {
    if (node) {
        struct GraphNode *current = node;
        do {
            switch (current->type) {

                // Start to disable rendering
                case GRAPH_NODE_TYPE_CAMERA: {
                    active = !gOmmDebugSurface;
                } break;

                // Stop to disable rendering
                case GRAPH_NODE_TYPE_OBJECT_PARENT: {
                    active = true;
                    omm_debug_render_surfaces_update_node(((struct GraphNodeObjectParent *) current)->sharedChild, active);
                } break;
                
                // Disable rendering
                case GRAPH_NODE_TYPE_TRANSLATION_ROTATION:
                case GRAPH_NODE_TYPE_TRANSLATION:
                case GRAPH_NODE_TYPE_ROTATION:
                case GRAPH_NODE_TYPE_SCALE:
                case GRAPH_NODE_TYPE_BILLBOARD:
                case GRAPH_NODE_TYPE_DISPLAY_LIST:
                case GRAPH_NODE_TYPE_ANIMATED_PART: {
                    if (active) {
                        current->flags |= GRAPH_RENDER_ACTIVE;
                    } else {
                        current->flags &= ~GRAPH_RENDER_ACTIVE;
                    }
                } break;

                // Make invisible only surface objects
                case GRAPH_NODE_TYPE_OBJECT: {
                    struct Object *object = (struct Object *) current;
                    if (object->activeFlags) {
                        static s32 sSurfaceModelIds[] = { 0x03, 0x16, 0x1C, 0x2F, 0x35, 0x53 };
                        for (s32 modelId = sSurfaceModelIds[0], j = 1; j < (s32) OMM_ARRAY_SIZE(sSurfaceModelIds); ++modelId) {
                            if (modelId > sSurfaceModelIds[j]) {
                                modelId = sSurfaceModelIds[j + 1];
                                j += 2;
                            }
                            if (obj_check_model(object, modelId) || object->collisionData) {
                                if (gOmmDebugSurface) {
                                    current->flags |= GRAPH_RENDER_INVISIBLE;
                                } else if (
                                    (object->behavior != bhvExclamationBox || object->oAction != 5) &&
                                    (object->behavior != bhvBlueCoinSwitch || object->oAction != 2) &&
                                    (object->behavior != bhvWhompKingBoss  || object->oAction != 9)
                                ) {
                                    current->flags &= ~GRAPH_RENDER_INVISIBLE;
                                }
                                break;
                            }
                        }
                    } else {
                        current->flags = 0;
                    }
                } break;
            }
            omm_debug_render_surfaces_update_node(current->children, active);
            current = current->next;
        } while (current && current != node);
    }
}

OMM_ROUTINE_GFX(omm_debug_render_surfaces_update_area) {
    if (gMarioObject && gCurrentArea && gCurrentArea->unk04 && !omm_is_main_menu() && !omm_is_ending_cutscene()) {
        omm_debug_render_surfaces_update_node(&gCurrentArea->unk04->node, true);
    }
}

#endif
