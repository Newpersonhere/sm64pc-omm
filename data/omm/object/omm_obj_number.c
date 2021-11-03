#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// Gfx data
//

static const Vtx omm_number_vertices[] = {
    { { { -32, -32, 0 }, 0, {    0, 1024 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
    { { { +32, -32, 0 }, 0, { 1024, 1024 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
    { { { +32, +32, 0 }, 0, { 1024,    0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
    { { { -32, +32, 0 }, 0, {    0,    0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
    { { { -56, -32, 0 }, 0, {    0, 1024 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
    { { {  +8, -32, 0 }, 0, { 1024, 1024 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
    { { {  +8, +32, 0 }, 0, { 1024,    0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
    { { { -56, +32, 0 }, 0, {    0,    0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
    { { {  -8, -32, 0 }, 0, {    0, 1024 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
    { { { +56, -32, 0 }, 0, { 1024, 1024 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
    { { { +56, +32, 0 }, 0, { 1024,    0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
    { { {  -8, +32, 0 }, 0, {    0,    0 }, { 0xFF, 0xFF, 0xFF, 0xFF } } },
};

#define omm_number_define_gfx(type, digit, offset) \
const Gfx omm_number_##type##_gfx[] = { \
    gsSPClearGeometryMode(G_LIGHTING), \
    gsDPSetCombineLERP(0, 0, 0, TEXEL0, 0, 0, 0, TEXEL0, 0, 0, 0, TEXEL0, 0, 0, 0, TEXEL0), \
    gsDPLoadTextureBlock(OMM_TEXTURE_HUD_##digit, G_IM_FMT_RGBA, G_IM_SIZ_32b, 16, 16, 0, G_TX_CLAMP, G_TX_CLAMP, 0, 0, 0, 0), \
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON), \
    gsSPVertex(omm_number_vertices + offset, 4, 0), \
    gsSP2Triangles(0, 1, 2, 0, 0, 2, 3, 0), \
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_OFF), \
    gsDPSetCombineLERP(0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE, 0, 0, 0, SHADE), \
    gsSPSetGeometryMode(G_LIGHTING), \
    gsSPEndDisplayList(), \
};

omm_number_define_gfx(1digit_0, 0, 0);
omm_number_define_gfx(1digit_1, 1, 0);
omm_number_define_gfx(1digit_2, 2, 0);
omm_number_define_gfx(1digit_3, 3, 0);
omm_number_define_gfx(1digit_4, 4, 0);
omm_number_define_gfx(1digit_5, 5, 0);
omm_number_define_gfx(1digit_6, 6, 0);
omm_number_define_gfx(1digit_7, 7, 0);
omm_number_define_gfx(1digit_8, 8, 0);
omm_number_define_gfx(1digit_9, 9, 0);

omm_number_define_gfx(2digits_0x, 0, 4);
omm_number_define_gfx(2digits_1x, 1, 4);
omm_number_define_gfx(2digits_2x, 2, 4);
omm_number_define_gfx(2digits_3x, 3, 4);
omm_number_define_gfx(2digits_4x, 4, 4);
omm_number_define_gfx(2digits_5x, 5, 4);
omm_number_define_gfx(2digits_6x, 6, 4);
omm_number_define_gfx(2digits_7x, 7, 4);
omm_number_define_gfx(2digits_8x, 8, 4);
omm_number_define_gfx(2digits_9x, 9, 4);

omm_number_define_gfx(2digits_x0, 0, 8);
omm_number_define_gfx(2digits_x1, 1, 8);
omm_number_define_gfx(2digits_x2, 2, 8);
omm_number_define_gfx(2digits_x3, 3, 8);
omm_number_define_gfx(2digits_x4, 4, 8);
omm_number_define_gfx(2digits_x5, 5, 8);
omm_number_define_gfx(2digits_x6, 6, 8);
omm_number_define_gfx(2digits_x7, 7, 8);
omm_number_define_gfx(2digits_x8, 8, 8);
omm_number_define_gfx(2digits_x9, 9, 8);

//
// Geo functions
//

Gfx *omm_geo_number_switch_1digit(s32 callContext, struct GraphNode *node, UNUSED void *context) {
    if (gCurrGraphNodeObject && callContext == GEO_CONTEXT_RENDER) {
        s32 number = gCurrGraphNodeObject->oBehParams2ndByte;
        struct GraphNodeSwitchCase *switchCase = (struct GraphNodeSwitchCase *) node;
        if (number >= 0 && number < 10) {
            switchCase->selectedCase = number;
        } else {
            switchCase->selectedCase = 10;
        }
    }
    return NULL;
}

Gfx *omm_geo_number_switch_2digits_left(s32 callContext, struct GraphNode *node, UNUSED void *context) {
    if (gCurrGraphNodeObject && callContext == GEO_CONTEXT_RENDER) {
        s32 number = gCurrGraphNodeObject->oBehParams2ndByte;
        struct GraphNodeSwitchCase *switchCase = (struct GraphNodeSwitchCase *) node;
        if (number >= 10) {
            switchCase->selectedCase = number / 10;
        } else {
            switchCase->selectedCase = 10;
        }
    }
    return NULL;
}

Gfx *omm_geo_number_switch_2digits_right(s32 callContext, struct GraphNode *node, UNUSED void *context) {
    if (gCurrGraphNodeObject && callContext == GEO_CONTEXT_RENDER) {
        s32 number = gCurrGraphNodeObject->oBehParams2ndByte;
        struct GraphNodeSwitchCase *switchCase = (struct GraphNodeSwitchCase *) node;
        if (number >= 10) {
            switchCase->selectedCase = number % 10;
        } else {
            switchCase->selectedCase = 10;
        }
    }
    return NULL;
}

//
// Geo layout
//

const GeoLayout omm_geo_number[] = {
    GEO_NODE_START(),
    GEO_OPEN_NODE(),
        GEO_BILLBOARD(),
        GEO_OPEN_NODE(),
            GEO_SWITCH_CASE(11, omm_geo_number_switch_1digit),
            GEO_OPEN_NODE(),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_1digit_0_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_1digit_1_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_1digit_2_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_1digit_3_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_1digit_4_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_1digit_5_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_1digit_6_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_1digit_7_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_1digit_8_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_1digit_9_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, NULL),
            GEO_CLOSE_NODE(),
            GEO_SWITCH_CASE(11, omm_geo_number_switch_2digits_left),
            GEO_OPEN_NODE(),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_0x_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_1x_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_2x_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_3x_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_4x_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_5x_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_6x_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_7x_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_8x_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_9x_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, NULL),
            GEO_CLOSE_NODE(),
            GEO_SWITCH_CASE(11, omm_geo_number_switch_2digits_right),
            GEO_OPEN_NODE(),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_x0_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_x1_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_x2_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_x3_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_x4_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_x5_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_x6_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_x7_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_x8_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, omm_number_2digits_x9_gfx),
                GEO_DISPLAY_LIST(LAYER_ALPHA, NULL),
            GEO_CLOSE_NODE(),
        GEO_CLOSE_NODE(),
    GEO_CLOSE_NODE(),
    GEO_END(),
};

//
// Spawner
//

struct Object *omm_spawn_number(struct Object *o, s32 number) {
    struct Object *orangeNumber = spawn_object_relative(number, 0, 0, 0, o, MODEL_NONE, bhvOrangeNumber);
    orangeNumber->oGraphNode = omm_geo_get_graph_node(omm_geo_number, true);
    orangeNumber->oPosY += 25.f;
    return orangeNumber;
}
