#include "custom.model.inc.h"

ALIGNED8 u8 dorrie_geo__texture_06009DA0[] = {
#include "actors/dorrie/dorrie_geo_0x6009da0_custom.rgba16.inc.c"
};

Vtx VB_dorrie_geo_0x600c978[] = {
{{{ 0, 0, 0 }, 0, { -6055, 6737 }, { 0, 0, 0, 255}}},
{{{ 0, -13, -917 }, 0, { -6059, 6424 }, { 0, 0, 0, 255}}},
{{{ 300, -374, -649 }, 0, { -6183, 6516 }, { 0, 0, 0, 255}}},
{{{ -299, -374, -649 }, 0, { -6183, 6516 }, { 0, 0, 0, 255}}},
{{{ 0, -3375, -5845 }, 0, { -7207, 4742 }, { 0, 0, 0, 255}}},
{{{ 0, -787, -470 }, 0, { -6324, 6577 }, { 0, 0, 0, 255}}},
{{{ 0, -787, 470 }, 0, { -6324, 6898 }, { 0, 0, 0, 255}}},
{{{ -299, -374, 649 }, 0, { -6183, 6959 }, { 0, 0, 0, 255}}},
{{{ 0, -3375, 5845 }, 0, { -7207, 8733 }, { 0, 0, 0, 255}}},
{{{ 0, -13, 917 }, 0, { -6059, 7050 }, { 0, 0, 0, 255}}},
{{{ 300, -374, 649 }, 0, { -6183, 6959 }, { 0, 0, 0, 255}}},
{{{ 300, 750, 0 }, 0, { -5799, 6737 }, { 0, 0, 0, 255}}},
{{{ 0, 801, 447 }, 0, { -5781, 6890 }, { 0, 0, 0, 255}}},
{{{ 0, 6750, 0 }, 0, { -3751, 6737 }, { 0, 0, 0, 255}}},
{{{ -299, 750, 0 }, 0, { -5799, 6737 }, { 0, 0, 0, 255}}},
};

Vtx VB_dorrie_geo_0x600ca68[] = {
{{{ 0, 801, -447 }, 0, { -5781, 6585 }, { 0, 0, 0, 255}}},
{{{ 0, 6750, 0 }, 0, { -3751, 6737 }, { 0, 0, 0, 255}}},
{{{ 300, 750, 0 }, 0, { -5799, 6737 }, { 0, 0, 0, 255}}},
{{{ 0, 0, 0 }, 0, { -6055, 6737 }, { 0, 0, 0, 255}}},
{{{ -299, 750, 0 }, 0, { -5799, 6737 }, { 0, 0, 0, 255}}},
{{{ 0, 801, 447 }, 0, { -5781, 6890 }, { 0, 0, 0, 255}}},
{{{ 0, 0, 0 }, 0, { 0, 0 }, { 0, 0, 0, 255}}},
};

Light_t Light_dorrie_geo_0x600cfc8 = {
{ 255, 255, 255}, 0, { 255, 255, 255}, 0, { 255, 255, 255}, 0
};

Gfx DL_dorrie_geo_0x600ce28[] = {
gsDPPipeSync(),
gsDPSetCombineLERP(TEXEL0, 0, SHADE, 0, 0, 0, 0, SHADE, TEXEL0, 0, SHADE, 0, 0, 0, 0, SHADE),
gsSPTexture(65535, 65535, 0, 0, 1),
gsSPLight(&Light_dorrie_geo_0x600cfc8.col, 1),
gsSPLight(&Light_dorrie_geo_0x600cfc8.col, 2),
gsDPSetTextureImage(0, 2, 1, dorrie_geo__texture_06009DA0),
gsDPSetTile(0, 2, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0),
gsDPLoadSync(),
gsDPLoadBlock(7, 0, 0, 1023, 256),
gsDPPipeSync(),
gsDPSetTile(0, 2, 8, 0, 0, 0, 0, 5, 0, 0, 5, 0),
gsDPSetTileSize(0, 0, 0, 124, 124),
gsSPVertex(VB_dorrie_geo_0x600c978, 15, 0),
gsSP2Triangles(0, 6, 7, 0,0, 7, 9, 0),
gsSP2Triangles(0, 9, 10, 0,0, 10, 6, 0),
gsSP2Triangles(0, 11, 12, 0,1, 0, 2, 0),
gsSP2Triangles(1, 3, 0, 0,2, 0, 5, 0),
gsSP2Triangles(3, 5, 0, 0,4, 1, 2, 0),
gsSP2Triangles(4, 2, 5, 0,4, 3, 1, 0),
gsSP2Triangles(4, 5, 3, 0,6, 8, 7, 0),
gsSP2Triangles(6, 10, 8, 0,7, 8, 9, 0),
gsSP2Triangles(10, 9, 8, 0,13, 12, 11, 0),
gsSP1Triangle(14, 0, 12, 0),
gsSPVertex(VB_dorrie_geo_0x600ca68, 7, 0),
gsSP2Triangles(0, 1, 2, 0,0, 2, 3, 0),
gsSP2Triangles(0, 3, 4, 0,0, 4, 1, 0),
gsSP2Triangles(4, 5, 1, 0,6, 6, 6, 0),
gsSPTexture(65535, 65535, 0, 0, 0),
gsSPEndDisplayList(),
};

