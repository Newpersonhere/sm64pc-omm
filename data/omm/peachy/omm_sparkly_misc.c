#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS
#if OMM_CODE_SPARKLY

bool omm_sparkly_check_cheats(struct MarioState *m) {

    // No more problem after getting all 30 Sparkly Stars
    if (omm_sparkly_is_mode_completed(omm_sparkly_get_current_mode())) {
        return false;
    }

    // Regular cheats
    // The option "Enable Cheats" is not counted as a cheat
    static const u8 sNoCheat[sizeof(Cheats) - sizeof(Cheats.EnableCheats)] = { 0 };
    const u8 *cheats = ((const u8 *) &Cheats) + sizeof(Cheats.EnableCheats);
    if (!OMM_MEMCMP(cheats, sNoCheat, sizeof(sNoCheat))) {
        return true;
    }

    // Debug move
    if (m->action == ACT_DEBUG_FREE_MOVE) {
        return true;
    }

    // OMM cheats
    if (OMM_CHEAT_CAPPY_STAYS_FOREVER ||
        OMM_CHEAT_UNLIMITED_CAPPY_BOUNCES ||
        OMM_CHEAT_MARIO_TELEPORTS_TO_CAPPY ||
        OMM_CHEAT_HOMING_ATTACK_GLOBAL_RANGE ||
        OMM_CHEAT_PEACH_ENDLESS_VIBE_GAUGE) {
        return true;
    }

    // Castle grounds cheese (thanks, Petch)
    if (gCurrLevelNum == LEVEL_GROUNDS && gCurrAreaIndex == 1 && m->pos[1] > 6000.f) {
        if (m->action == ACT_OMM_POSSESSION && gOmmData->mario->capture.timer >= 20) {
            omm_mario_unpossess_object(m, OMM_MARIO_UNPOSSESS_ACT_NONE, false, 0);
            return true;
        }
        struct Object *o = NULL;
        o = obj_get_nearest_with_behavior_and_radius(m->marioObj, bhvRecoveryHeart, 100.f);
        o = (((m->floorHeight > (m->pos[1] -  40.f)) && (m->floor != NULL) && (m->floor->object != NULL)) ? m->floor->object : o);
        o = ((                                          (m->wall  != NULL) && (m->wall->object  != NULL)) ? m->wall->object  : o);
        o = (((m->ceilHeight  < (m->pos[1] + 200.f)) && (m->ceil  != NULL) && (m->ceil->object  != NULL)) ? m->ceil->object  : o);
        if (o) {
            obj_spawn_white_puff(o, SOUND_GENERAL_BREAK_BOX);
            obj_spawn_triangle_break_particles(o, OBJ_SPAWN_TRI_BREAK_PRESET_TRIANGLES_20);
            obj_mark_for_deletion(o);
            return true;
        }
    }

    // Nothing to report
    return false;
}

void omm_sparkly_turn_off_cheats() {
    if (!omm_sparkly_is_mode_completed(omm_sparkly_get_current_mode())) {
        OMM_MEMSET(&Cheats, 0, sizeof(Cheats));
        gOmmCheatUnlimitedCappyBounces = false;
        gOmmCheatCappyStaysForever = false;
        gOmmCheatHomingAttackGlobalRange = false;
        gOmmCheatMarioTeleportsToCappy = false;
        gOmmCheatPeachEndlessVibeGauge = false;
    }
}

void omm_sparkly_set_opt_mode(s32 mode) {
    if (gOmmSparklyStarsMode != (u32) mode) {
        gOmmSparklyStarsMode = (u32) mode;
        save_file_do_save(gCurrSaveFileNum - 1);
    }
}

void omm_sparkly_disable() {
    gOmmData->mario->sparkly.mode = OMM_SPARKLY_MODE_DISABLED;
}

#endif
