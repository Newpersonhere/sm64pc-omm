#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

#define MODEL_LUIGI                     0xFF
#define MODEL_LUIGIS_CAP                0xFF
#define MODEL_LUIGIS_WING_CAP           0xFF
#define MODEL_LUIGIS_METAL_CAP          0xFF
#define MODEL_LUIGIS_WINGED_METAL_CAP   0xFF

#define MODEL_WARIO                     0xFF
#define MODEL_WARIOS_CAP                0xFF
#define MODEL_WARIOS_WING_CAP           0xFF
#define MODEL_WARIOS_METAL_CAP          0xFF
#define MODEL_WARIOS_WINGED_METAL_CAP   0xFF

#define omm_luigi_is_unlocked()         false
#define omm_wario_is_unlocked()         false
#define omm_luigi_is_selected()         false
#define omm_wario_is_selected()         false
#define omm_select_player(...)

//
// Select
//

bool omm_player_is_unlocked(s32 index) {
    switch (index) {
        case OMM_PLAYER_MARIO: return true;
        case OMM_PLAYER_PEACH: return omm_peach_is_unlocked();
        case OMM_PLAYER_LUIGI: return omm_luigi_is_unlocked();
        case OMM_PLAYER_WARIO: return omm_wario_is_unlocked();
    }
    return false;
}

bool omm_player_is_selected(s32 index) {
    switch (index) {
        case OMM_PLAYER_MARIO: return !omm_peach_is_selected() && !omm_luigi_is_selected() && !omm_wario_is_selected();
        case OMM_PLAYER_PEACH: return omm_peach_is_selected();
        case OMM_PLAYER_LUIGI: return omm_luigi_is_selected();
        case OMM_PLAYER_WARIO: return omm_wario_is_selected();
    }
    return false;
}

void omm_player_select(s32 index) {
    if (omm_player_is_unlocked(index)) {
        switch (index) {
            case OMM_PLAYER_MARIO: omm_peach_select(0); omm_select_player(MARIO_DEFAULT); break;
            case OMM_PLAYER_PEACH: omm_peach_select(1); omm_select_player(MARIO_DEFAULT); break;
            case OMM_PLAYER_LUIGI: omm_peach_select(0); omm_select_player(LUIGI); break;
            case OMM_PLAYER_WARIO: omm_peach_select(0); omm_select_player(WARIO); break;
        }
    }
}

s32 omm_player_get_selected_index() {
    for (s32 i = 0; i != OMM_NUM_PLAYABLE_CHARACTERS; ++i) {
        if (omm_player_is_selected(i)) {
            return i;
        }
    }
    return -1;
}

//
// Gfx properties
//

static const uintptr_t sOmmPlayerGfxProperties[][7] = {
    { (uintptr_t) OMM_TEXT_MARIO_UPPER, 0xFF0000FF, MODEL_MARIO,     MODEL_MARIOS_CAP,     MODEL_MARIOS_WING_CAP,     MODEL_MARIOS_METAL_CAP,     MODEL_MARIOS_WINGED_METAL_CAP     },
    { (uintptr_t) OMM_TEXT_PEACH_UPPER, 0xFF80FFFF, MODEL_PEACH_OMM, MODEL_PEACHS_CAP_OMM, MODEL_PEACHS_WING_CAP_OMM, MODEL_PEACHS_METAL_CAP_OMM, MODEL_PEACHS_WINGED_METAL_CAP_OMM },
    { (uintptr_t) OMM_TEXT_LUIGI_UPPER, 0x00C000FF, MODEL_LUIGI,     MODEL_LUIGIS_CAP,     MODEL_LUIGIS_WING_CAP,     MODEL_LUIGIS_METAL_CAP,     MODEL_LUIGIS_WINGED_METAL_CAP     },
    { (uintptr_t) OMM_TEXT_WARIO_UPPER, 0xD0D000FF, MODEL_WARIO,     MODEL_WARIOS_CAP,     MODEL_WARIOS_WING_CAP,     MODEL_WARIOS_METAL_CAP,     MODEL_WARIOS_WINGED_METAL_CAP     },
};

const char *omm_player_get_name(s32 index) {
    return (const char *) sOmmPlayerGfxProperties[index][0];
}

u32 omm_player_get_color(s32 index) {
    return (u32) sOmmPlayerGfxProperties[index][1];
}

s32 omm_player_get_model(s32 index) {
    return (s32) sOmmPlayerGfxProperties[index][2];
}

s32 omm_player_get_normal_cap(s32 index) {
    return (s32) sOmmPlayerGfxProperties[index][3];
}

s32 omm_player_get_wing_cap(s32 index) {
    return (s32) sOmmPlayerGfxProperties[index][4];
}

s32 omm_player_get_metal_cap(s32 index) {
    return (s32) sOmmPlayerGfxProperties[index][5];
}

s32 omm_player_get_winged_metal_cap(s32 index) {
    return (s32) sOmmPlayerGfxProperties[index][6];
}

s32 omm_player_get_cap(s32 index, bool wing, bool metal) {
    return (u32) sOmmPlayerGfxProperties[index][3 + wing + 2 * metal];
}

//
// Physics properties
//

static const f32 sOmmPlayerPhyProperties[][3] = {
    { 1.00f, 1.00f, 1.00f }, // Mario
    { 1.00f, 1.00f, 1.00f }, // Peach
    { 1.10f, 0.90f, 1.05f }, // Luigi
    { 0.90f, 1.10f, 0.95f }, // Wario
    { 1.00f, 1.00f, 1.00f }, // Peach (Joy)
    { 0.80f, 0.80f, 0.90f }, // Peach (Rage)
    { 1.50f, 1.50f, 1.20f }, // Peach (Gloom)
    { 1.00f, 1.00f, 1.00f }, // Peach (Calm)
};

f32 omm_player_get_ground_speed_multiplier(s32 index) {
    f32 mult = sOmmPlayerPhyProperties[index][0];
    return mult * (gMarioState->action == ACT_WALKING ? sqrtf(mult) : 1.f);
}

f32 omm_player_get_air_speed_multiplier(s32 index) {
    return sOmmPlayerPhyProperties[index][1];
}

f32 omm_player_get_jump_multiplier(s32 index) {
    return sOmmPlayerPhyProperties[index][2];
}

//
// Update
//

OMM_ROUTINE_LEVEL_ENTRY(omm_player_init) {
    omm_player_select(gOmmCharacter);
}

OMM_ROUTINE_UPDATE(omm_player_update) {
    static u32 sOmmCharacter = OMM_PLAYER_MARIO;
    if (gOmmCharacter != sOmmCharacter) {
        s32 dir = omm_sign_s(gOmmCharacter - sOmmCharacter) * (omm_abs_s(gOmmCharacter - sOmmCharacter) > 1 ? -1 : +1);
        while (!omm_player_is_unlocked(gOmmCharacter)) {
            gOmmCharacter = (gOmmCharacter + dir + OMM_NUM_PLAYABLE_CHARACTERS) % OMM_NUM_PLAYABLE_CHARACTERS;
        }
        sOmmCharacter = gOmmCharacter;
    }
}
