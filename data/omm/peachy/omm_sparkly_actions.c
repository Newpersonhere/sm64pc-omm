#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// Sparkly Star interaction
//

bool omm_sparkly_interact_star(struct MarioState *m, struct Object *o) {
#if OMM_CODE_SPARKLY
    if (o->behavior == omm_bhv_sparkly_star && m->action != ACT_OMM_POSSESSION) {
        omm_sparkly_collect_star(omm_sparkly_get_current_mode(), omm_sparkly_get_star_index(omm_sparkly_get_current_mode(), gCurrLevelNum, gCurrAreaIndex));
        mario_stop_riding_and_holding(m);
        update_mario_sound_and_camera(m);
        play_sound(SOUND_MENU_STAR_SOUND, m->marioObj->oCameraToObject);
        spawn_object(o, MODEL_NONE, bhvStarKeyCollectionPuffSpawner);
        omm_sparkly_context_set_state(OMM_SPARKLY_CONTEXT_STATE_INVALID, 0);
        omm_sparkly_context_reset_data();
        omm_mario_set_action(m, ACT_OMM_SPARKLY_STAR_DANCE, (m->prevAction & ACT_FLAG_METAL_WATER) || ((m->prevAction & ACT_GROUP_MASK) == ACT_GROUP_SUBMERGED), 0);
        return true;
    }
#else
    OMM_UNUSED(m);
    OMM_UNUSED(o);
#endif
    return false;
}

void omm_sparkly_interact_grand_star(struct MarioState *m, struct Object *o) {
#if OMM_CODE_SPARKLY
    if (o->behavior == bhvGrandStar && (o->oInteractionSubtype & INT_SUBTYPE_GRAND_STAR)) {

        // Sparkly Grand Star
        // Collecting it unlocks the next mode
        // Triggers the secret Peach ending if Bowser 4 is defeated for the first time
        if (omm_sparkly_flags_get(OMM_SPARKLY_FLAG_GRAND_STAR)) {
            omm_sparkly_ending = (!omm_player_is_unlocked(OMM_PLAYER_PEACH) ? OMM_SPARKLY_ENDING_PEACH : OMM_SPARKLY_ENDING_REGULAR);
            omm_sparkly_collect_grand_star(omm_sparkly_get_current_mode());
            return;
        }

        // Regular Grand Star
        // Triggers the bad ending if Bowser 3 is defeated for the first time after completing
        // a save file (120 stars + regular Bowser 3 flag), or if the Sparkly Stars mode
        // is enabled and Peach is not unlocked yet
        if (m->numStars >= 120 && (!omm_sparkly_is_mode_unlocked(OMM_SPARKLY_MODE_NORMAL) || (omm_sparkly_is_enabled() && !omm_player_is_unlocked(OMM_PLAYER_PEACH)))) {
            omm_sparkly_ending = OMM_SPARKLY_ENDING_BAD;
            omm_sparkly_unlock_mode(OMM_SPARKLY_MODE_NORMAL);
            return;
        }
    }
#else
    OMM_UNUSED(m);
    OMM_UNUSED(o);
#endif
}

//
// Sparkly Star dance
//

#if OMM_CODE_SPARKLY
static s32 sOmmSparklyDanceAnimData[][5] = {
    // Animation, Hands, Yaw, PosY, Speed
    { MARIO_ANIM_TRIPLE_JUMP_LAND, MARIO_HAND_FISTS, 0x0000, 0, 1 }, // 00
    { MARIO_ANIM_TRIPLE_JUMP_LAND, MARIO_HAND_FISTS, 0x0000, 0, 1 }, // 01
    { MARIO_ANIM_TRIPLE_JUMP_LAND, MARIO_HAND_FISTS, 0x0000, 0, 1 }, // 02
    { MARIO_ANIM_TRIPLE_JUMP_LAND, MARIO_HAND_FISTS, 0x0080, 0, 1 }, // 03
    { MARIO_ANIM_TRIPLE_JUMP_LAND, MARIO_HAND_FISTS, 0x0100, 0, 1 }, // 04
    { MARIO_ANIM_TRIPLE_JUMP_LAND, MARIO_HAND_FISTS, 0x0200, 0, 1 }, // 05
    { MARIO_ANIM_TRIPLE_JUMP_LAND, MARIO_HAND_FISTS, 0x0400, 0, 1 }, // 06
    { MARIO_ANIM_TRIPLE_JUMP_LAND, MARIO_HAND_FISTS, 0x0800, 0, 1 }, // 07
    { MARIO_ANIM_TWIRL, MARIO_HAND_FISTS, 0x1000, 0, 1 }, // 08
    { MARIO_ANIM_TWIRL, MARIO_HAND_FISTS, 0x2000, 0, 1 }, // 09
    { MARIO_ANIM_TWIRL, MARIO_HAND_FISTS, 0x4000, 0, 1 }, // 10
    { MARIO_ANIM_TWIRL, MARIO_HAND_FISTS, 0x6000, 0, 1 }, // 11
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0x8000, 0, 1 },  // 12
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0xA000, 0, 1 },  // 13
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0xC000, 0, 1 },  // 14
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0xE000, 0, 1 },  // 15
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0x0000, 0, 1 }, // 16
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0x2000, 0, 1 }, // 17
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0x4000, 0, 1 }, // 18
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0x6000, 0, 1 }, // 19
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0x8000, 0, 1 }, // 20
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0xA000, 0, 1 }, // 21
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0xC000, 0, 1 }, // 22
    { MARIO_ANIM_TWIRL, MARIO_HAND_OPEN, 0xE000, 0, 1 }, // 23
    { MARIO_ANIM_TWIRL_LAND, MARIO_HAND_FISTS, 0xF000, 0, 2 }, // 24
    { MARIO_ANIM_TWIRL_LAND, MARIO_HAND_FISTS, 0xF800, 0, 2 }, // 25
    { MARIO_ANIM_TWIRL_LAND, MARIO_HAND_FISTS, 0xFC00, 0, 2 }, // 26
    { MARIO_ANIM_TWIRL_LAND, MARIO_HAND_FISTS, 0x0000, 0, 2 }, // 27
    { MARIO_ANIM_SINGLE_JUMP, MARIO_HAND_FISTS, 0x0000, 16, 2 }, // 28
    { MARIO_ANIM_SINGLE_JUMP, MARIO_HAND_FISTS, 0x0000, 24, 2 }, // 29
    { MARIO_ANIM_SINGLE_JUMP, MARIO_HAND_FISTS, 0x0000, 28, 2 }, // 30
    { MARIO_ANIM_SINGLE_JUMP, MARIO_HAND_FISTS, 0x0000, 30, 2 }, // 31
    { MARIO_ANIM_SINGLE_JUMP, MARIO_HAND_FISTS, 0x0000, 31, 2 }, // 32
    { MARIO_ANIM_SINGLE_JUMP, MARIO_HAND_FISTS, 0x0000, 32, 2 }, // 33
    { MARIO_ANIM_SINGLE_JUMP, MARIO_HAND_FISTS, 0x0000, 32, 2 }, // 34
    { MARIO_ANIM_SINGLE_JUMP, MARIO_HAND_FISTS, 0x0000, 32, 2 }, // 35
};

static bool omm_sparkly_star_dance_update(struct MarioState *m) {
    static struct Object *celebStar;

    // Enable time stop and spawn the celebration star
    if (m->actionTimer == 0) {
        m->faceAngle[1] = m->area->camera->yaw;
        vec3s_set(m->marioObj->header.gfx.angle, 0, m->area->camera->yaw, 0);
        disable_background_sound();
        music_pause();
        omm_sound_play(OMM_SOUND_EVENT_SPARKLY_STAR_GET, gGlobalSoundArgs);
        celebStar = omm_spawn_sparkly_star_celebration(m->marioObj, omm_sparkly_get_current_mode());
        set_time_stop_flags(TIME_STOP_ENABLED | TIME_STOP_MARIO_AND_DOORS);
        m->marioObj->activeFlags |= ACTIVE_FLAG_INITIATED_TIME_STOP;
    }
    
    // Display the text box "Pink Gold/Crystal/Nebula star"
    else if (m->actionTimer == 28) {
        omm_render_start_you_got_a_star(
            OMM_SPARKLY_STAR_TEXT[omm_sparkly_get_current_mode()],
            omm_level_get_name(gCurrLevelNum, false, false),
            omm_sparkly_get_star_name(
                omm_sparkly_get_current_mode(),
                omm_sparkly_get_star_index(omm_sparkly_get_current_mode(), gCurrLevelNum, gCurrAreaIndex)
            )
        );
    }
    
    // Wa-hoo!
    else if (m->actionTimer == 29) {
        omm_sound_play_character_sound_n64(SOUND_MARIO_HERE_WE_GO, m->marioObj->oCameraToObject);
        set_camera_shake_from_hit(SHAKE_GROUND_POUND);
    }
    
    // Resume action
    else if (m->actionTimer == 90) {
        obj_mark_for_deletion(celebStar);
        m->marioObj->activeFlags &= ~ACTIVE_FLAG_INITIATED_TIME_STOP;
        clear_time_stop_flags(TIME_STOP_ENABLED | TIME_STOP_MARIO_AND_DOORS);
        enable_background_sound();
        music_resume();
        omm_render_stop_you_got_a_star();
        omm_health_fully_heal_mario(m);
        m->healCounter = OMM_O2_REFILL;
        return true;
    }

    // Fix the camera during the animation
    omm_mario_lock_camera(m, true);

    // Animation
    s32 *animData = sOmmSparklyDanceAnimData[omm_min_s(m->actionTimer, 35)];
    omm_mario_set_animation(m, animData[0], animData[4], -1);
    if (m->actionTimer >= 33) set_anim_to_frame(m, 10); // Luigi's freaking flutter jump
    m->marioBodyState->handState = animData[1];
    m->marioObj->header.gfx.angle[1] = m->faceAngle[1] - (s16) animData[2];
    m->marioObj->header.gfx.pos[1] = m->pos[1] + animData[3];
    m->actionTimer++;
    return false;
}
#endif

s32 omm_sparkly_act_star_dance(struct MarioState *m) {
#if OMM_CODE_SPARKLY
    if (omm_sparkly_star_dance_update(m)) {
        mario_set_forward_vel(m, 0.f);
        omm_mario_set_action(m, m->actionArg ? ACT_WATER_IDLE : ACT_FREEFALL, 0, 0);
        return OMM_MARIO_ACTION_RESULT_CANCEL;
    }
#else
    OMM_UNUSED(m);
#endif
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

//
// "Bad" ending cutscenes
//

#if OMM_CODE_SPARKLY
typedef struct {
    struct Object *obj;
    s32 anim;
    s32 jump;
} EndToadStruct;

static void update_end_toad_anim(EndToadStruct *toad) {
    if (toad->obj) {
        geo_obj_init_animation(&toad->obj->header.gfx, (struct Animation **) &toad_seg6_anims_0600FB58[toad->anim]);
        if (obj_is_anim_near_end(toad->obj) && (toad->anim == 0 || toad->anim == 2)) {
            toad->anim++;
        }
    }
}

static void update_end_toad_jump(EndToadStruct *toad) {
    if (toad->obj && toad->jump > 0) {
        toad->obj->oPosY += 60.f * sins(0x1000 * toad->jump);
        toad->jump--;
    }
}

static void set_end_toad_message(const char *msg, s16 duration) {
    static u8 sOmmEndToadMessage[256];
    const u8 *converted = omm_text_convert(msg, false);
    OMM_MEMCPY(sOmmEndToadMessage, converted, omm_text_length(converted) + 1);
    gEndCutsceneStringsEn[OMM_CUTSCENE_MSG_INDEX] = sOmmEndToadMessage;
    set_cutscene_message(SCREEN_WIDTH / 2, 227, OMM_CUTSCENE_MSG_INDEX, duration);
}

static void set_visual_pos(struct Object *o, f32 *value) {
    Vec3s t; find_mario_anim_flags_and_translation(o, o->header.gfx.angle[1], t);
    f32 x = o->header.gfx.pos[0] + t[0];
    f32 y = o->header.gfx.pos[1] + 10.0f;
    f32 z = o->header.gfx.pos[2] + t[2];
    struct Surface *floor;
    *value = find_floor(x, y, z, &floor);
}

static struct Object *get_grand_star_object() {
    for_each_object_with_behavior(grandStar, bhvStaticObject) {
        if (obj_check_model(grandStar, MODEL_STAR) || obj_has_graph_node(grandStar, omm_geo_get_graph_node(omm_geo_star_0_opaque, true))) {
            return grandStar;
        }
    }
    return NULL;
}

static void omm_sparkly_act_end_cutscene_1_update_music(struct MarioState *m) {
#if defined(R96A)
    if (m->actionArg == 3) {
        static u32 sConfigMasterVolume = 0;
        if (sConfigMasterVolume != 0 && configMasterVolume > 0) {
            configMasterVolume -= MIN(configMasterVolume, 1 + (gGlobalTimer & 1));
        }
        if (m->actionTimer == 44) {
            sConfigMasterVolume = configMasterVolume;
        } else if (m->actionTimer == 276) {
            r96_stop_jingle();
            configMasterVolume = sConfigMasterVolume;
            sConfigMasterVolume = 0;
        }
    } else if (m->actionArg == 6 && m->actionTimer == 460) {
        r96_play_jingle(R96_EVENT_CREDITS);
    }
#else
    if (m->actionArg == 3 && m->actionTimer == 44) {
        music_fade_out(SEQ_PLAYER_LEVEL, 192);
    } else if (m->actionArg == 6 && m->actionTimer == 460) {
        play_cutscene_music(SEQUENCE_ARGS(15, SEQ_EVENT_CUTSCENE_CREDITS));
    }
#endif
}

#endif

// Sparkly Stars bad ending (Normal only)
// Toads are waiting for Mario, but Peach isn't here...
s32 omm_sparkly_act_end_cutscene_1(struct MarioState *m) {
#if OMM_CODE_SPARKLY
    static Vp sEndCutsceneVp = { { { 640, 480, 511, 0 }, { 640, 480, 511, 0 } } };
    static EndToadStruct sEndToadL = { NULL, 0, 0 };
    static EndToadStruct sEndToadR = { NULL, 0, 0 };

    // Update the grand star model
    // Use gOmmSparklyStarsMode instead of omm_sparkly_get_current_mode(), as the latter returns 0 during the ending cutscene
    struct Object *grandStar = get_grand_star_object();
    if (grandStar && omm_sparkly_flags_get(OMM_SPARKLY_FLAG_GRAND_STAR)) {
        grandStar->oGraphNode = omm_geo_get_graph_node(OMM_SPARKLY_STAR_OPAQUE_GEO[gOmmSparklyStarsMode], true);
    }

    // Play a different cutscene during the 'bad' ending
    if (omm_sparkly_ending == OMM_SPARKLY_ENDING_BAD) {
        omm_sparkly_act_end_cutscene_1_update_music(m);
        switch (m->actionArg) {
            case 0: {
                return OMM_MARIO_ACTION_RESULT_CONTINUE;
            } break;

            case 1: {
                return OMM_MARIO_ACTION_RESULT_CONTINUE;
            } break;

            case 2: {
                return OMM_MARIO_ACTION_RESULT_CONTINUE;
            } break;

            case 3: {
                switch (m->actionTimer) {
                    case 1: {
                        play_transition(WARP_TRANSITION_FADE_INTO_COLOR, 14, 255, 255, 255);
                    } break;

                    case 2: {
                        play_sound(SOUND_MENU_STAR_SOUND, gGlobalSoundArgs);
                    } break;

                    case 40: {
                        if (grandStar) obj_mark_for_deletion(grandStar);
                        sEndToadL.obj = spawn_object_abs_with_rot(m->marioObj, 0, MODEL_TOAD, bhvStaticObject, -100, 906, -1400, 0, 0, 0);
                        sEndToadR.obj = spawn_object_abs_with_rot(m->marioObj, 0, MODEL_TOAD, bhvStaticObject, +100, 906, -1400, 0, 0, 0);
                        sEndToadL.obj->oOpacity = 255;
                        sEndToadR.obj->oOpacity = 255;
                        sEndToadL.anim = 4;
                        sEndToadR.anim = 5;
                        sEndToadL.jump = 0;
                        sEndToadR.jump = 0;
                    } break;

                    case 44: {
                        play_transition(WARP_TRANSITION_FADE_FROM_COLOR, 192, 255, 255, 255);
                    } break;

                    case 276: {
                        m->actionArg++;
                    } break;
                }
            } break;

            case 4: {
                set_mario_animation(m, MARIO_ANIM_CREDITS_RETURN_FROM_LOOK_UP);
                m->actionState = 0;
                m->actionTimer = 0;
                m->actionArg++;
            } break;

            case 5: {
                return OMM_MARIO_ACTION_RESULT_CONTINUE;
            } break;

            case 6: {
                switch (m->actionTimer) {

                    // Mario scratches his head
                    case 1: {
                        set_mario_animation(m, MARIO_ANIM_MISSING_CAP);
                    } break;

                    // Toad message 1
                    case 60: {
                        set_mario_animation(m, MARIO_ANIM_FIRST_PERSON);
                        sEndToadL.jump = 8;
                        omm_sound_play(OMM_SOUND_TOAD_1, sEndToadL.obj->oCameraToObject);
                        set_end_toad_message(OMM_TEXT_BAD_ENDING_TOAD_1, 30);
                    } break;

                    // Toad message 2
                    case 120: {
                        sEndToadR.jump = 8;
                        omm_sound_play(OMM_SOUND_TOAD_2, sEndToadR.obj->oCameraToObject);
                        set_end_toad_message(OMM_TEXT_BAD_ENDING_TOAD_2, 60);
                    } break;

                    // Toad message 3
                    case 210: {
                        omm_sound_play(OMM_SOUND_TOAD_3, sEndToadL.obj->oCameraToObject);
                        set_end_toad_message(OMM_TEXT_BAD_ENDING_TOAD_3, 60);
                    } break;

                    // Toad message 4, Mario falls on his butt
                    case 300: {
                        omm_sound_play(OMM_SOUND_TOAD_4, sEndToadR.obj->oCameraToObject);
                        set_end_toad_message(OMM_TEXT_BAD_ENDING_TOAD_4, 60);
                        set_mario_animation(m, 50);
                    } break;

                    // Toad message 5, Mario stands up, credits sequence
                    case 460: {
                        sEndToadR.jump = 8;
                        omm_sound_play(OMM_SOUND_TOAD_5, sEndToadR.obj->oCameraToObject);
                        set_end_toad_message(OMM_TEXT_BAD_ENDING_TOAD_5, 60);
                        set_mario_animation(m, 200);
                    } break;

                    // Mario looks at Toads
                    case 480: {
                        set_mario_animation(m, MARIO_ANIM_FIRST_PERSON);
                    } break;

                    // Toad message 6
                    case 540: {
                        sEndToadL.jump = 8;
                        omm_sound_play(OMM_SOUND_TOAD_6, sEndToadL.obj->oCameraToObject);
                        set_end_toad_message(OMM_TEXT_BAD_ENDING_TOAD_6, 60);
                    } break;

                    // Toad message 7
                    case 620: {
                        sEndToadR.jump = 8;
                        omm_sound_play(OMM_SOUND_TOAD_7, sEndToadR.obj->oCameraToObject);
                        set_end_toad_message(OMM_TEXT_BAD_ENDING_TOAD_7, 60);
                    } break;

                    // Right Toad double hop
                    case 628: {
                        sEndToadR.jump = 8;
                    } break;

                    // Toad message 8
                    case 710: {
                        sEndToadL.jump = 8;
                        omm_sound_play(OMM_SOUND_TOAD_8, sEndToadL.obj->oCameraToObject);
                        set_end_toad_message(OMM_TEXT_BAD_ENDING_TOAD_8, 60);
                    } break;

                    // Left Toad double hop
                    case 718: {
                        sEndToadL.jump = 8;
                    } break;

                    // Toad message 9
                    case 790: {
                        omm_sound_play(OMM_SOUND_TOAD_9, sEndToadL.obj->oCameraToObject);
                        set_end_toad_message(OMM_TEXT_BAD_ENDING_TOAD_9, 60);
                    } break;

                    // Toads move, Mario look at the sky
                    case 800: {
                        sEndToadL.anim = 0;
                        sEndToadR.anim = 2;
                        set_mario_animation(m, MARIO_ANIM_CREDITS_START_WALK_LOOK_UP);
                    } break;

                    // Mario moves
                    case 900: {
                        set_mario_animation(m, MARIO_ANIM_CREDITS_LOOK_BACK_THEN_RUN);
                    } break;

                    // Camera starts moving up, birds spawn
                    case 960: {
                    } break;

                    // Credits
                    case 1200: {
                        level_trigger_warp(m, WARP_OP_CREDITS_NEXT);
                        gPaintingMarioYEntry = 1500;
                        m->actionState = 1;
                        m->actionArg = 12;
                        return OMM_MARIO_ACTION_RESULT_BREAK;
                    } break;
                }

                set_visual_pos(m->marioObj, &m->marioObj->header.gfx.pos[1]);
                set_visual_pos(sEndToadL.obj, &sEndToadL.obj->oPosY);
                set_visual_pos(sEndToadR.obj, &sEndToadR.obj->oPosY);
                update_end_toad_jump(&sEndToadL);
                update_end_toad_jump(&sEndToadR);
            } break;
        }

        m->actionTimer++;
        update_end_toad_anim(&sEndToadL);
        update_end_toad_anim(&sEndToadR);
        sEndCutsceneVp.vp.vscale[0] = 640;
        sEndCutsceneVp.vp.vscale[1] = 360;
        sEndCutsceneVp.vp.vtrans[0] = 640;
        sEndCutsceneVp.vp.vtrans[1] = 480;
        override_viewport_and_clip(NULL, &sEndCutsceneVp, 0, 0, 0);
        return OMM_MARIO_ACTION_RESULT_BREAK;
    }
#else
    OMM_UNUSED(m);
#endif
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}

// Sparkly Stars bad ending (Normal only)
// No one is waving at the camera. They're all looking for Peach...
s32 omm_sparkly_act_end_cutscene_2(struct MarioState *m) {
#if OMM_CODE_SPARKLY
    if (omm_sparkly_ending == OMM_SPARKLY_ENDING_BAD) {
        action_init(0, 0, 0, 0, m->statusForCamera->cameraEvent = CAM_EVENT_START_END_WAVING;);
        stop_and_set_height_to_floor(m);
        m->marioObj->oNodeFlags |= GRAPH_RENDER_INVISIBLE;
        if (m->actionTimer++ >= 300) {
            level_trigger_warp(m, WARP_OP_CREDITS_END);
        }
        return OMM_MARIO_ACTION_RESULT_BREAK;
    }
#else
    OMM_UNUSED(m);
#endif
    return OMM_MARIO_ACTION_RESULT_CONTINUE;
}
