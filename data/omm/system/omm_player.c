#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

static s32 sOmmPlayer = OMM_PLAYER_MARIO;

//
// Properties
//

typedef struct { f32 ground; f32 air; f32 jump; } Phy;
typedef struct { s32 id; const GeoLayout *geo; } IdGeo;
typedef struct {
    const char *name;
    u32 color;
    Phy phy;
    IdGeo body;
    IdGeo cap;
    IdGeo wcap;
    IdGeo mcap;
    IdGeo wmcap;
    void *anims;
} OmmPlayerProperties;

static const OmmPlayerProperties sOmmPlayerProperties[] = {

    /* Mario */ {
    OMM_TEXT_MARIO_UPPER, 0xFF0000FF,
    { 1.00f, 1.00f, 1.00f },
    { MODEL_MARIO, mario_geo },
    { MODEL_MARIOS_CAP, marios_cap_geo },
    { MODEL_MARIOS_WING_CAP, marios_wing_cap_geo },
    { MODEL_MARIOS_METAL_CAP, marios_metal_cap_geo },
    { MODEL_MARIOS_WINGED_METAL_CAP, marios_winged_metal_cap_geo },
    &gMarioAnimsData },
    
    /* Peach */ {
    OMM_TEXT_PEACH_UPPER, 0xFF80FFFF,
    { 1.00f, 1.00f, 1.00f },
    { MODEL_PEACH_OMM, omm_geo_peach },
    { MODEL_PEACHS_CAP_OMM, omm_geo_peachs_cap },
    { MODEL_PEACHS_WING_CAP_OMM, omm_geo_peachs_cap },
    { MODEL_PEACHS_METAL_CAP_OMM, omm_geo_peachs_metal_cap },
    { MODEL_PEACHS_WINGED_METAL_CAP_OMM, omm_geo_peachs_metal_cap },
    &gMarioAnimsData },
    
#if defined(R96A)
    /* Luigi */ {
    OMM_TEXT_LUIGI_UPPER, 0x00C000FF,
    { 1.10f, 0.90f, 1.05f },
    { MODEL_LUIGI, luigi_geo },
    { MODEL_LUIGIS_CAP, luigis_cap_geo },
    { MODEL_LUIGIS_WING_CAP, luigis_wing_cap_geo },
    { MODEL_LUIGIS_METAL_CAP, luigis_metal_cap_geo },
    { MODEL_LUIGIS_WINGED_METAL_CAP, luigis_winged_metal_cap_geo },
    &Data_LuigiAnims },
    
    /* Wario */ {
    OMM_TEXT_WARIO_UPPER, 0xD0D000FF,
    { 0.90f, 1.10f, 0.95f },
    { MODEL_WARIO, wario_geo },
    { MODEL_WARIOS_CAP, warios_cap_geo },
    { MODEL_WARIOS_WING_CAP, warios_wing_cap_geo },
    { MODEL_WARIOS_METAL_CAP, warios_metal_cap_geo },
    { MODEL_WARIOS_WINGED_METAL_CAP, warios_winged_metal_cap_geo },
    &Data_WarioAnims },
#endif
    
    /* Peach (Joy) */ {
    OMM_TEXT_PEACH_UPPER, 0xFF80FFFF,
    { 1.00f, 1.00f, 1.00f },
    { MODEL_PEACH_OMM, omm_geo_peach },
    { MODEL_PEACHS_CAP_OMM, omm_geo_peachs_cap },
    { MODEL_PEACHS_WING_CAP_OMM, omm_geo_peachs_cap },
    { MODEL_PEACHS_METAL_CAP_OMM, omm_geo_peachs_metal_cap },
    { MODEL_PEACHS_WINGED_METAL_CAP_OMM, omm_geo_peachs_metal_cap },
    &gMarioAnimsData },
    
    /* Peach (Rage) */ {
    OMM_TEXT_PEACH_UPPER, 0xFF80FFFF,
    { 0.80f, 0.80f, 0.90f },
    { MODEL_PEACH_OMM, omm_geo_peach },
    { MODEL_PEACHS_CAP_OMM, omm_geo_peachs_cap },
    { MODEL_PEACHS_WING_CAP_OMM, omm_geo_peachs_cap },
    { MODEL_PEACHS_METAL_CAP_OMM, omm_geo_peachs_metal_cap },
    { MODEL_PEACHS_WINGED_METAL_CAP_OMM, omm_geo_peachs_metal_cap },
    &gMarioAnimsData },
    
    /* Peach (Gloom) */ {
    OMM_TEXT_PEACH_UPPER, 0xFF80FFFF,
    { 1.50f, 1.50f, 1.20f },
    { MODEL_PEACH_OMM, omm_geo_peach },
    { MODEL_PEACHS_CAP_OMM, omm_geo_peachs_cap },
    { MODEL_PEACHS_WING_CAP_OMM, omm_geo_peachs_cap },
    { MODEL_PEACHS_METAL_CAP_OMM, omm_geo_peachs_metal_cap },
    { MODEL_PEACHS_WINGED_METAL_CAP_OMM, omm_geo_peachs_metal_cap },
    &gMarioAnimsData },
    
    /* Peach (Calm) */ {
    OMM_TEXT_PEACH_UPPER, 0xFF80FFFF,
    { 1.00f, 1.00f, 1.00f },
    { MODEL_PEACH_OMM, omm_geo_peach },
    { MODEL_PEACHS_CAP_OMM, omm_geo_peachs_cap },
    { MODEL_PEACHS_WING_CAP_OMM, omm_geo_peachs_cap },
    { MODEL_PEACHS_METAL_CAP_OMM, omm_geo_peachs_metal_cap },
    { MODEL_PEACHS_WINGED_METAL_CAP_OMM, omm_geo_peachs_metal_cap },
    &gMarioAnimsData },
};

#if defined(R96A)

//
// Render96 characters functions
// Redefined to fit the player system of OMM
//

// Ignore 'characterType', set the model according to the selected character
void setCharacterModel(UNUSED s32 characterType) {
    omm_player_select(sOmmPlayer);
}

// Useless, always return 0
s32 getCharacterType() {
    return 0;
}

// Handled by other functions, always return 1
f32 getCharacterMultiplier() {
    return 1.f;
}

// Check if Luigi is selected
s32 isLuigi() {
    return OMM_PLAYER_IS_LUIGI;
}

// Check if Wario is selected
s32 isWario() {
    return OMM_PLAYER_IS_WARIO;
}

//
// Luigi notification
//

s8 notificationStatus = FALSE;

void triggerLuigiNotification() {
    if (!notificationStatus) {
        r96_play_collect_jingle(R96_EVENT_SOLVE_PUZZLE);
        set_notification_state(TRUE);
        notificationStatus = TRUE;
    }    
}

void set_notification_status(s8 newState) {
    notificationStatus = newState;
}

s8 get_notification_status() {
    return notificationStatus;
}

//
// Is Luigi/Wario unlocked?
// In the main menu, checks all files, but in-game, checks only the current savefile
//

static bool is_unlocked(s32 (get_collectibles_count(s32)), s32 required) {
    for (s32 i = (omm_is_main_menu() ? 0 : gCurrSaveFileNum - 1); i != (omm_is_main_menu() ? NUM_SAVE_FILES : gCurrSaveFileNum); ++i) {
        if (get_collectibles_count(i) >= required) {
            return true;
        }
    }
    return false;
}

#endif

//
// Select
//

bool omm_player_is_unlocked(s32 index) {
    switch (index) {
        case OMM_PLAYER_MARIO: return true;
        case OMM_PLAYER_PEACH: return omm_sparkly_is_mode_completed(OMM_SPARKLY_MODE_NORMAL);
#if defined(R96A)
        case OMM_PLAYER_LUIGI: return is_unlocked(save_file_get_keys, NUM_KEYS);
        case OMM_PLAYER_WARIO: return is_unlocked(save_file_get_wario_coins, NUM_WARIO_COINS);
#endif
    }
    return false;
}

// If the game tries to select a locked character, select Mario instead
void omm_player_select(s32 index) {
    sOmmPlayer = (omm_player_is_unlocked(index) ? index : OMM_PLAYER_MARIO);
    gOmmCharacter = sOmmPlayer;
    gSaveFileModified = true;
}

bool omm_player_is_selected(s32 index) {
    return sOmmPlayer == index;
}

s32 omm_player_get_selected_index() {
    return sOmmPlayer;
}

//
// Gfx properties
//

const char *omm_player_get_name(s32 index) {
    return sOmmPlayerProperties[index].name;
}

u32 omm_player_get_color(s32 index) {
    return sOmmPlayerProperties[index].color;
}

s32 omm_player_get_model(s32 index) {
    return sOmmPlayerProperties[index].body.id;
}

s32 omm_player_get_normal_cap(s32 index) {
    return sOmmPlayerProperties[index].cap.id;
}

s32 omm_player_get_wing_cap(s32 index) {
    return sOmmPlayerProperties[index].wcap.id;
}

s32 omm_player_get_metal_cap(s32 index) {
    return sOmmPlayerProperties[index].mcap.id;
}

s32 omm_player_get_winged_metal_cap(s32 index) {
    return sOmmPlayerProperties[index].wmcap.id;
}

s32 omm_player_get_cap(s32 index, bool wing, bool metal) {
    if (wing && metal) return omm_player_get_winged_metal_cap(index);
    if (        metal) return omm_player_get_metal_cap(index);
    if (wing         ) return omm_player_get_wing_cap(index);
    else               return omm_player_get_normal_cap(index);
}

//
// Physics properties
//

f32 omm_player_get_ground_speed_multiplier(s32 index) {
    f32 mult = sOmmPlayerProperties[index].phy.ground;
    return mult * (gMarioState->action == ACT_WALKING ? sqrtf(mult) : 1.f);
}

f32 omm_player_get_air_speed_multiplier(s32 index) {
    return sOmmPlayerProperties[index].phy.air;
}

f32 omm_player_get_jump_multiplier(s32 index) {
    return sOmmPlayerProperties[index].phy.jump;
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

OMM_ROUTINE_GFX(omm_player_update_gfx) {
    if (gLoadedGraphNodes && gMarioObject) {
        const OmmPlayerProperties *pp   = &sOmmPlayerProperties[sOmmPlayer];
        gLoadedGraphNodes[pp->body.id]  = omm_geo_get_graph_node(pp->body.geo, true);
        gLoadedGraphNodes[pp->cap.id]   = omm_geo_get_graph_node(pp->cap.geo, true);
        gLoadedGraphNodes[pp->wcap.id]  = omm_geo_get_graph_node(pp->wcap.geo, true);
        gLoadedGraphNodes[pp->mcap.id]  = omm_geo_get_graph_node(pp->mcap.geo, true);
        gLoadedGraphNodes[pp->wmcap.id] = omm_geo_get_graph_node(pp->wmcap.geo, true);
        gMarioObject->oGraphNode        = omm_geo_get_graph_node(pp->body.geo, true);
        gMarioState->mMarioAnimations    = (MarioAnimationsStruct *) pp->anims;
#if defined(R96A)
        Cheats.ChaosPlayAs = 0;
        Cheats.PlayAs = 0;
#endif
    }
}

//
// One of the sickiest combo with Peach
// With frame-perfect inputs, she can reach 10000+ units with a full Vibe gauge
// (for comparison, it's "only" 7000 units with only Joy + some air moves)
//
// Here's how it's done:
// - Start with a Gloom spin jump
// - At the top, deactivate Gloom, up-throw, kick, gp, dive
// - When the gauge is back at 100, start the Gloom/Joy combo
// - Activate Gloom, gp, and at the peak, dive
// - Activate Joy as soon as possible, and spam A instead of holding it to maintain your height without draining the gauge too much
// - Then reactivate Gloom, and repeat
// - When the gauge is < 15, activate Joy one last time, and hold A
// - At 0, Joy deactivates itself, end the combo with a spin throw, a kick and a gp dive
//
/*OMM_ROUTINE_UPDATE(__joy_gloom_combo) {
    static f32 dir = +1.f;
    struct MarioState *m = gMarioState;
    
    // Hold L to perform the combo
    if (gPlayer1Controller->buttonDown & L_TRIG) {
        gPlayer1Controller->buttonDown &= ~L_TRIG;
        gPlayer1Controller->stickX = 64.f * dir;
        gPlayer1Controller->stickY = 0.f;
        gPlayer1Controller->stickMag = 64.f;
        u16 buttons = 0;

        // Gloom part
        if (omm_peach_vibe_is_gloom()) {

            // If not gping or diving, trigger a gp
            if (m->action != ACT_GROUND_POUND && m->action != ACT_DIVE) {
                buttons |= Z_TRIG;
            }
            
            // At the peak of the gp, dive
            else if (m->action == ACT_GROUND_POUND && m->actionTimer >= 10) {
                buttons |= B_BUTTON;
            }
            
            // When diving, try to activate Joy
            else if (m->action == ACT_DIVE) {
                buttons |= (Y_BUTTON | U_JPAD);
            }
        }
        
        // Joy part
        else if (omm_peach_vibe_is_joy()) {
            if (gOmmData->mario->peach.vibeTimer == 0) {
                dir *= -1.f;
            }

            // Every 2 frames, press A to not lose height without draining the Vibe gauge too much
            buttons |= (A_BUTTON * ((gGlobalTimer % 2) == 0));

            // Try to activate Gloom as soon as possible
            buttons |= (Y_BUTTON | L_JPAD);
        }
        
        // Start the combo with Gloom
        else {
            buttons |= (Y_BUTTON | L_JPAD);
        }

        gPlayer1Controller->buttonPressed = buttons;
        gPlayer1Controller->buttonDown |= buttons;
    }
}*/
