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

static s32 sMarioRootNodeState = 0;
void omm_geo_fix_marios_anim_translation_y(struct Object *o, struct GraphNodeAnimatedPart *node, f32 *translationY) {
    if (o == gMarioObject) {
        if (sMarioRootNodeState != (1 + OMM_IS_60_FPS)) {
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
            
            // Jumbo star cutscene animation offset
            if (gMarioObject->header.gfx.mAnimInfo.animID == MARIO_ANIM_FINAL_BOWSER_RAISE_HAND_SPIN) {
                *translationY += (
                    (240.f * (gMarioState->action != ACT_JUMBO_STAR_CUTSCENE)) + 
                    (120.f * OMM_PLAYER_IS_PEACH)
                );
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
static void (*sPreprocessFunc)(void *, struct Object *, struct GraphNode *, f32, f32, f32);
static struct Object *sPreprocessObject;

static s16  sMatStackIndex;
static m4f  sMatStack[32];
static s16  sCurAnimFrame;
static u8   sCurAnimType;
static f32  sCurAnimMult;
static u16 *sCurAnimAttr;
static s16 *sCurAnimData;
static s32  sMarioHandFlag;

static void omm_geo_preprocess_graph_node_level_of_detail(struct GraphNodeLevelOfDetail *node) {
    if (node->minDistance <= 0 && 0 < node->maxDistance) {
        omm_geo_preprocess_graph_node_and_siblings(node->node.children);
    }
}

static void omm_geo_preprocess_graph_node_switch_case(struct GraphNodeSwitchCase *node) {
    if (node->fnNode.func != NULL) {
        node->fnNode.func(GEO_CONTEXT_RENDER, &node->fnNode.node, NULL);
        if (node->fnNode.func == (GraphNodeFunc) geo_switch_mario_hand) {
            static bool sIsLeftHand = true;
            sMarioHandFlag = 1 + sIsLeftHand;
            sMarioForeArmPos[sIsLeftHand][0] = sMatStack[sMatStackIndex].m[3][0];
            sMarioForeArmPos[sIsLeftHand][1] = sMatStack[sMatStackIndex].m[3][1];
            sMarioForeArmPos[sIsLeftHand][2] = sMatStack[sMatStackIndex].m[3][2];
            sIsLeftHand = !sIsLeftHand;
        }
        if (node->fnNode.func == (GraphNodeFunc) geo_switch_mario_cap_on_off) {
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

    omm_geo_fix_marios_anim_translation_y(sPreprocessObject, node, &t.y);
    if (OMM_IS_60_FPS) {
        f32 dummy = 0.f; // Needs to be called twice for 60 FPS interpolation
        omm_geo_fix_marios_anim_translation_y(sPreprocessObject, node, &dummy);
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

static void omm_geo_preprocess_graph_node_and_siblings(struct GraphNode *node) {
    if (node) {
        struct GraphNode *current = node;
        do {
            if (sPreprocessFunc && (current->type & GRAPH_NODE_TYPE_FUNCTIONAL)) {
                sPreprocessFunc(
                    (void *) ((struct FnGraphNode *) current)->func,
                    sPreprocessObject,
                    current,
                    sMatStack[sMatStackIndex].m[3][0],
                    sMatStack[sMatStackIndex].m[3][1],
                    sMatStack[sMatStackIndex].m[3][2]
                );
            }
            switch (current->type) {
                case GRAPH_NODE_TYPE_LEVEL_OF_DETAIL:       omm_geo_preprocess_graph_node_level_of_detail((struct GraphNodeLevelOfDetail *) current); break;
                case GRAPH_NODE_TYPE_SWITCH_CASE:           omm_geo_preprocess_graph_node_switch_case((struct GraphNodeSwitchCase *) current); break;
                case GRAPH_NODE_TYPE_TRANSLATION_ROTATION:  omm_geo_preprocess_graph_node_translation_rotation((struct GraphNodeTranslationRotation *) current); break;
                case GRAPH_NODE_TYPE_TRANSLATION:           omm_geo_preprocess_graph_node_translation((struct GraphNodeTranslation *) current); break;
                case GRAPH_NODE_TYPE_ROTATION:              omm_geo_preprocess_graph_node_rotation((struct GraphNodeRotation *) current); break;
                case GRAPH_NODE_TYPE_ANIMATED_PART:         omm_geo_preprocess_graph_node_animated_part((struct GraphNodeAnimatedPart *) current); break;
                case GRAPH_NODE_TYPE_SCALE:                 omm_geo_preprocess_graph_node_scale((struct GraphNodeScale *) current); break;
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

    sCurAnimAttr = segmented_to_virtual((void *) anim->index);
    sCurAnimData = segmented_to_virtual((void *) anim->values);

    if (anim->mAnimYTransDivisor == 0) {
        sCurAnimMult = 1.f;
    } else {
        sCurAnimMult = (f32) node->animYTrans / (f32) anim->mAnimYTransDivisor;
    }
}

//
// Public
//

void omm_geo_preprocess_object_graph_node(struct Object *o, void (*func)(void *, struct Object *, struct GraphNode *, f32, f32, f32)) {
    if (o && o->oGraphNode) {
        sPreprocessObject = o;
        sPreprocessFunc = func;
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

        // Preprocess
        omm_geo_preprocess_graph_node_and_siblings(o->oGraphNode);
        sMarioRootNodeState = 0;
    }
}
