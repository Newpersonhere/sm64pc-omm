#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS
#if OMM_CODE_SPARKLY

OMM_ROUTINE_UPDATE(omm_sparkly_update) {
    sAcousticReachPerLevel[LEVEL_GROUNDS] = 25000 + 35000 * (gCurrAreaIndex == 2);
    struct MarioState *m = gMarioState;
    omm_sparkly_ending_update_cutscene();

    // Check Sparkly Stars mode option
    static u32 sOmmSparklyStarsMode = 0;
    if (gOmmSparklyStarsMode != OMM_SPARKLY_MODE_DISABLED && !omm_sparkly_is_mode_unlocked(gOmmSparklyStarsMode)) {
        gOmmSparklyStarsMode = OMM_SPARKLY_MODE_DISABLED;
    } else if (gOmmSparklyStarsMode != sOmmSparklyStarsMode) {
        s32 dir = omm_sign_s(gOmmSparklyStarsMode - sOmmSparklyStarsMode) * (omm_abs_s(gOmmSparklyStarsMode - sOmmSparklyStarsMode) > 1 ? -1 : +1);
        while (gOmmSparklyStarsMode != OMM_SPARKLY_MODE_DISABLED && !omm_sparkly_is_mode_unlocked(gOmmSparklyStarsMode)) {
            gOmmSparklyStarsMode = (gOmmSparklyStarsMode + dir + OMM_SPARKLY_MODE_COUNT) % OMM_SPARKLY_MODE_COUNT;
        }
        sOmmSparklyStarsMode = gOmmSparklyStarsMode;
    }

    // Reset the Sparkly Stars mode in the main menu, and return
    if (omm_is_main_menu()) {
        omm_sparkly_disable();
        omm_sparkly_ending = OMM_SPARKLY_ENDING_REGULAR;
        omm_sparkly_flags_set(OMM_SPARKLY_FLAG_STAR_BLOCK, 0);
        omm_sparkly_flags_set(OMM_SPARKLY_FLAG_GRAND_STAR, 0);
        omm_sparkly_flags_set(OMM_SPARKLY_FLAG_CHEAT_DETECTED, 0);
        return;
    }

    // If Bowser 3 was never defeated, disable the Sparkly Stars mode, sets the option to 0 and return
    if (!omm_sparkly_is_mode_unlocked(OMM_SPARKLY_MODE_NORMAL)) {
        omm_sparkly_set_opt_mode(OMM_SPARKLY_MODE_DISABLED);
        omm_sparkly_disable();
        return;
    }

    // If a save file with less than 120 stars is selected, disable the Sparkly Stars mode and return
    if (m->numStars < 120) {
        omm_sparkly_disable();
        return;
    }

    // Disable the Sparkly Stars mode if the option is set to 0 (this case doesn't need a level change)
    if (gOmmSparklyStarsMode == OMM_SPARKLY_MODE_DISABLED) {
        if (omm_sparkly_is_enabled() && omm_sparkly_context_get_flag(OMM_SPARKLY_CONTEXT_FLAG_TIMER_STARTED)) {
            gHudDisplay.flags &= ~HUD_DISPLAY_FLAG_TIMER;
        }
        omm_sparkly_disable();
    }

    // Disable the Sparkly Stars mode during the ending cutscenes and return
    if (m->action == ACT_END_WAVING_CUTSCENE ||
        m->action == ACT_END_PEACH_CUTSCENE ||
        m->action == ACT_CREDITS_CUTSCENE) {
        omm_sparkly_disable();
        return;
    }

    // Stop the update here during the jumbo star cutscene
    if (m->action == ACT_JUMBO_STAR_CUTSCENE) {
        return;
    }

    // Stop if the object lists or Mario's object are not loaded yet
    if (OMM_UNLIKELY(!gObjectLists || !gMarioObject || !m->marioObj)) {
        return;
    }

    // Immediately return during regular Bowser fights
    if (gCurrLevelNum == LEVEL_BOWSER_1 ||
        gCurrLevelNum == LEVEL_BOWSER_2 ||
        gCurrLevelNum == LEVEL_BOWSER_3) {
        return;
    }

    // Sparkly Stars blocks
    // Appears on Castle Grounds if >= 120 stars and at least 1 Sparkly Star mode is available
    // Break it to enable the corresponding Sparkly Stars mode
    if (m->numStars >= 120 && gCurrLevelNum == LEVEL_GROUNDS && gCurrAreaIndex == 1) {
        for (s32 mode = OMM_SPARKLY_MODE_NORMAL; mode != OMM_SPARKLY_MODE_COUNT; ++mode) {
            if (omm_sparkly_is_mode_unlocked(mode) && !omm_sparkly_is_mode_selected(mode)) {
                struct Object *block = obj_get_first_with_behavior(OMM_SPARKLY_STAR_BLOCK_BHV[mode]);
                if (!block) {
                    omm_spawn_sparkly_star_block(m->marioObj, mode,
                        OMM_SPARKLY_STAR_BLOCK_POS[mode][0], 
                        OMM_SPARKLY_STAR_BLOCK_POS[mode][1], 
                        OMM_SPARKLY_STAR_BLOCK_POS[mode][2]
                    );
                } else if (block->oAction == 2) {
                    omm_sparkly_flags_set(OMM_SPARKLY_FLAG_STAR_BLOCK, 1);
                    obj_deactivate_all_with_behavior(omm_bhv_sparkly_star);
                    obj_deactivate_all_with_behavior(omm_bhv_sparkly_star_hint);
                    omm_sparkly_set_opt_mode(mode);
                    omm_sparkly_context_restart();
                    omm_sparkly_turn_off_cheats();
                    block->oAction = 3;
                    block->oTimer = 0;
                }
            }
        }
    }

    // Unload everything if not Sparkly Stars mode, then return
    // Also ejects Mario from the Bowser 4 fight
    if (!omm_sparkly_is_enabled()) {
        if (omm_sparkly_is_bowser_4()) {
            omm_return_to_castle(true, false);
        }
        obj_deactivate_all_with_behavior(omm_bhv_sparkly_star);
        obj_deactivate_all_with_behavior(omm_bhv_sparkly_star_hint);
        obj_deactivate_all_with_behavior(omm_bhv_sparkly_star_sparkle);
        omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 0);
        omm_sparkly_context_reset_data();
        omm_sparkly_flags_set(OMM_SPARKLY_FLAG_STAR_BLOCK, 0);
        omm_sparkly_flags_set(OMM_SPARKLY_FLAG_CHEAT_DETECTED, 0);
        omm_sparkly_cheats[0] = 0;
        omm_sparkly_cheats[1] = 0;
        omm_sparkly_cheats[2] = 0;
        omm_sparkly_cheats[3] = 0;
        return;
    }

    // Update game state
    static s32 sPrevMarioTimer = -1;
    omm_sparkly_flags_set(OMM_SPARKLY_FLAG_GAME_PAUSED, omm_is_game_paused());
    omm_sparkly_flags_set(OMM_SPARKLY_FLAG_TIME_STOP, (gTimeStopState & TIME_STOP_ENABLED) != 0);
    omm_sparkly_flags_set(OMM_SPARKLY_FLAG_TRANSITION, omm_is_transition_active());
    omm_sparkly_flags_set(OMM_SPARKLY_FLAG_MARIO_UPDATED, (m->marioObj->oTimer != sPrevMarioTimer) && (m->floor != NULL) && (m->action != 0)); // Mario does not update if he's out of bounds
    sPrevMarioTimer = m->marioObj->oTimer;
    gPlayer1Controller->buttonPressed &= ~(omm_sparkly_flags_get(OMM_SPARKLY_FLAG_CHEAT_DETECTED) * START_BUTTON);
    gPlayer2Controller->buttonPressed &= ~(omm_sparkly_flags_get(OMM_SPARKLY_FLAG_CHEAT_DETECTED) * START_BUTTON);
    gPlayer3Controller->buttonPressed &= ~(omm_sparkly_flags_get(OMM_SPARKLY_FLAG_CHEAT_DETECTED) * START_BUTTON);

    // Broken Sparkly Stars block
    // Display the Castle grounds hint message if the player has not started the timer yet
    if (!omm_sparkly_is_timer_started(omm_sparkly_get_current_mode()) &&
         omm_sparkly_flags_get(OMM_SPARKLY_FLAG_STAR_BLOCK) &&
        !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_GAME_PAUSED) &&
        !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_TRANSITION) &&
         omm_sparkly_flags_get(OMM_SPARKLY_FLAG_MARIO_UPDATED) &&
        (m->action == ACT_IDLE || m->action == ACT_PANTING || m->action == ACT_WALKING)) {
        struct Object *sign = obj_get_first_with_behavior(omm_bhv_sparkly_star_hint);
        if (sign) {
            m->marioObj->oMarioReadingSignDYaw  = (s16) (0x8000 - m->faceAngle[1]);
            m->marioObj->oMarioReadingSignDPosX = 0;
            m->marioObj->oMarioReadingSignDPosZ = 0;
            m->interactObj = sign;
            m->usedObj = sign;
            omm_mario_set_action(m, ACT_READING_SIGN, 0, 0);
            omm_sparkly_flags_set(OMM_SPARKLY_FLAG_STAR_BLOCK, 0);
            omm_sparkly_start_timer(omm_sparkly_get_current_mode());
            return;
        }
    }

    // Anti-cheat message
    // Read the messages and return
    if (omm_sparkly_cheats[0] >= 1) {
        if (m->action != ACT_READING_AUTOMATIC_DIALOG) {
            omm_mario_set_action(m, ACT_READING_AUTOMATIC_DIALOG, omm_sparkly_cheats[omm_sparkly_cheats[0]], 0);
            omm_sparkly_cheats[0] = ((omm_sparkly_cheats[0] + 1) % 4);
        }
        return;
    }

    // Anti-cheat trigger
    // Kill Mario, disable the Pause menu, eject him from the current level
    // (bypassing the Non-Stop mode) and display a special dialog...
    static bool sDeathState = false;
    if (omm_sparkly_flags_get(OMM_SPARKLY_FLAG_CHEAT_DETECTED)) {

        // Turn off cheats, turn off Non-Stop mode,
        // disable caps and reduce Mario's health to 0...
        if (!sDeathState) {
            omm_sparkly_context_unset_flag(OMM_SPARKLY_CONTEXT_FLAG_CHEAT_DETECTED, 1);
            omm_sparkly_turn_off_cheats();
            gOmmStarsMode = 0;
            m->capTimer = 0;
            m->flags &= ~(MARIO_WING_CAP | MARIO_METAL_CAP | MARIO_VANISH_CAP);
            if (OMM_MOVESET_ODYSSEY) {
                m->health = OMM_HEALTH_DEAD;
            } else {
                m->healCounter = 0;
                m->hurtCounter = 30;
            }
        }

        // ...until Mario is in a death state
        if (m->action == ACT_STANDING_DEATH ||
            m->action == ACT_QUICKSAND_DEATH ||
            m->action == ACT_ELECTROCUTION ||
            m->action == ACT_SUFFOCATION ||
            m->action == ACT_DEATH_ON_STOMACH ||
            m->action == ACT_DEATH_ON_BACK ||
            m->action == ACT_EATEN_BY_BUBBA ||
            m->action == ACT_DROWNING ||
            m->action == ACT_WATER_DEATH) {
            sDeathState = true;
        }

        // When warped back to Castle, trigger a random Bowser anti-cheat dialog
        if (m->health > OMM_HEALTH_DEAD &&
            gCurrCourseNum == COURSE_NONE &&
           !omm_sparkly_is_bowser_4() &&
           !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_GAME_PAUSED) &&
           !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_TRANSITION) &&
            omm_sparkly_flags_get(OMM_SPARKLY_FLAG_MARIO_UPDATED) &&
            (m->action == ACT_IDLE || m->action == ACT_PANTING || m->action == ACT_WALKING)) {
            play_sound(SOUND_OBJ_BOWSER_INTRO_LAUGH, gGlobalSoundArgs);
            play_sequence(SEQ_PLAYER_ENV, SEQ_EVENT_KOOPA_MESSAGE, 0);
            omm_sparkly_cheats[0] = 1;
            omm_sparkly_cheats[1] = OMM_DIALOG_SPARKLY_ANTI_CHEAT_INTRO;
            omm_sparkly_cheats[2] = OMM_DIALOG_SPARKLY_ANTI_CHEAT_0 + (random_u16() % 8);
            omm_sparkly_cheats[3] = OMM_DIALOG_SPARKLY_ANTI_CHEAT_END_0 + (random_u16() % 3);
            omm_sparkly_flags_set(OMM_SPARKLY_FLAG_CHEAT_DETECTED, 0);
            gOmmStarsMode = omm_sparkly_flags_get(OMM_SPARKLY_FLAG_NON_STOP_MODE);
            sDeathState = false;
        }
        return;
    }
    sDeathState = false;

    // Unlock Bowser 4
    // Display the "unlocked Bowser 4" message if the player has met the requirements to unlock the final fight
    if (!omm_sparkly_is_bowser_4_unlocked(omm_sparkly_get_current_mode()) &&
         omm_sparkly_get_star_count(omm_sparkly_get_current_mode()) == 29 &&
        !omm_sparkly_is_grand_star_collected(omm_sparkly_get_current_mode()) &&
         OMM_SPARKLY_BOWSER_4_SHOULD_UNLOCK[omm_sparkly_get_current_mode()] &&
        (m->action == ACT_IDLE || m->action == ACT_PANTING || m->action == ACT_WALKING)) {
        omm_mario_set_action(m, ACT_READING_AUTOMATIC_DIALOG, OMM_SPARKLY_BOWSER_4_DIALOG_UNLOCKED[omm_sparkly_get_current_mode()], 0);
        omm_sparkly_unlock_bowser_4(omm_sparkly_get_current_mode());
        audio_play_puzzle_jingle();
        return;
    }

    // Handle the Bowser 4 fight separately
    static s32 sBowser4WarpTimer = 0;
    sBowser4WarpTimer--;
    if (omm_sparkly_is_bowser_4()) {

        // Change the grand star to a Sparkly Grand Star
        struct Object *grandStar = obj_get_first_with_behavior(bhvGrandStar);
        if (grandStar) {
            grandStar->oGraphNode = omm_geo_get_graph_node(OMM_SPARKLY_STAR_OPAQUE_GEO[omm_sparkly_get_current_mode()], true);
            omm_sparkly_flags_set(OMM_SPARKLY_FLAG_GRAND_STAR, 1);
            return;
        }
    } else {

        // Transition to Bowser 4
        if (sBowser4WarpTimer > 0) {
            return;
        }
    
        // Warping to Bowser 4
        if (sBowser4WarpTimer == 0) {
            clear_time_stop_flags(TIME_STOP_ENABLED | TIME_STOP_MARIO_AND_DOORS);
            if (omm_sparkly_is_enabled()) {
                initiate_warp(LEVEL_GROUNDS, 2, 0x0A, 0);
            } else {
                omm_return_to_castle(false, true);
            }
            return;
        }
    
        // Initiating warp to Bowser 4
        if (omm_sparkly_is_bowser_4_unlocked(omm_sparkly_get_current_mode()) &&
            !omm_sparkly_flags_get(OMM_SPARKLY_FLAG_GAME_PAUSED) &&
            (gCurrLevelNum == LEVEL_GROUNDS) &&
            (gCurrAreaIndex == 1)) {

            // Spawn sparkles from the window
            struct Object *sparkle = obj_spawn_from_geo(m->marioObj, OMM_SPARKLY_STAR_SPARKLE_GEO[omm_sparkly_get_current_mode()], omm_bhv_sparkly_star_sparkle);
            sparkle->activeFlags |= ACTIVE_FLAG_INITIATED_TIME_STOP;
            sparkle->oPosX = OMM_SPARKLY_BOWSER_4_WARP_CENTER_X + OMM_SPARKLY_BOWSER_4_WARP_HITBOX_X * ((2.f * random_float()) - 1.f);
            sparkle->oPosY = OMM_SPARKLY_BOWSER_4_WARP_CENTER_Y + OMM_SPARKLY_BOWSER_4_WARP_HITBOX_Y * ((2.f * random_float()) - 1.f);
            sparkle->oPosZ = OMM_SPARKLY_BOWSER_4_WARP_CENTER_Z;
            sparkle->oVelX = 0;
            sparkle->oVelY = 0;
            sparkle->oVelZ = 20.f * random_float();
            sparkle->oAnimState = (random_u16() & 1);
            sparkle->oSparklyStarMode = omm_sparkly_get_current_mode();
            obj_scale_random(sparkle, 1.f, 0.5f);

            // Trigger the warp to Bowser 4 if Mario touches the window
            if (omm_abs_f(m->pos[0] - OMM_SPARKLY_BOWSER_4_WARP_CENTER_X) < OMM_SPARKLY_BOWSER_4_WARP_HITBOX_X &&
                omm_abs_f(m->pos[1] - OMM_SPARKLY_BOWSER_4_WARP_CENTER_Y) < OMM_SPARKLY_BOWSER_4_WARP_HITBOX_Y &&
                omm_abs_f(m->pos[2] - OMM_SPARKLY_BOWSER_4_WARP_CENTER_Z) < OMM_SPARKLY_BOWSER_4_WARP_HITBOX_Z && m->vel[2] < 0.f) {
                sBowser4WarpTimer = 40;
                omm_cappy_unload();
                omm_mario_set_action(m, ACT_OMM_WARPING, 0, 0xFFFF);
                play_sound(SOUND_MENU_ENTER_HOLE, gGlobalSoundArgs);
                play_transition(WARP_TRANSITION_FADE_INTO_CIRCLE, 20, 0, 0, 0);
                set_time_stop_flags(TIME_STOP_ENABLED | TIME_STOP_MARIO_AND_DOORS);
                m->marioObj->oNodeFlags |= GRAPH_RENDER_INVISIBLE;
                return;
            }
        }
    }

    // Context update
    omm_sparkly_context_update_state(m);
    omm_sparkly_context_spawn_star(m);
    omm_sparkly_context_spawn_sign(m);
    omm_sparkly_context_set_flag(OMM_SPARKLY_CONTEXT_FLAG_INITED, 1, 0);
}

#endif
