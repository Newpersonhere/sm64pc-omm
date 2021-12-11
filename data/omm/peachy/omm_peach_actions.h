#ifndef OMM_PEACH_ACTIONS_H
#define OMM_PEACH_ACTIONS_H

//
// Peach
//

#define OMM_PEACH_FLOAT_DURATION                    (30)

//
// Perry
//

#define OMM_PEACH_PERRY_HITBOX                      (120.f)
#define OMM_PEACH_PERRY_SHOCKWAVE_HITBOX_RADIUS     (OMM_ARRAY_OF(f32) { 150, 150, 180, 120, 150 })[o->oPerryShockwaveType]
#define OMM_PEACH_PERRY_SHOCKWAVE_HITBOX_HEIGHT     (OMM_ARRAY_OF(f32) { 100, 100, 120,  80, 100 })[o->oPerryShockwaveType]
#define OMM_PEACH_PERRY_SHOCKWAVE_DURATION          (OMM_ARRAY_OF(s32) {  30,  30,  30,  30,  30 })[o->oPerryShockwaveType]
#define OMM_PEACH_PERRY_SHOCKWAVE_FORWARD_VEL       (OMM_ARRAY_OF(f32) {  80,  80,  60, 100,  80 })[o->oPerryShockwaveType]

//
// Colors
//

#define OMM_PEACH_COLOR_PERRY_FRONT                 0xFF, 0xF0, 0x90
#define OMM_PEACH_COLOR_PERRY_BACK                  0xFF, 0xF0, 0x80
#define OMM_PEACH_COLOR_VIBE_FRONT                  0xFF, 0xA0, 0xFF
#define OMM_PEACH_COLOR_VIBE_BACK                   0xFF, 0x80, 0xFF
#define OMM_PEACH_COLOR_VIBE_JOY_FRONT              0xFF, 0xF0, 0x70
#define OMM_PEACH_COLOR_VIBE_JOY_BACK               0xFF, 0xF0, 0x60
#define OMM_PEACH_COLOR_VIBE_RAGE_FRONT             0xFF, 0xA0, 0x00
#define OMM_PEACH_COLOR_VIBE_RAGE_BACK              0xFF, 0x00, 0x00
#define OMM_PEACH_COLOR_VIBE_GLOOM_FRONT            0x60, 0x90, 0xFF
#define OMM_PEACH_COLOR_VIBE_GLOOM_BACK             0x40, 0x80, 0xFF
#define OMM_PEACH_COLOR_VIBE_CALM_FRONT             0x40, 0xF0, 0x40
#define OMM_PEACH_COLOR_VIBE_CALM_BACK              0x20, 0xE0, 0x40

//
// Vibes
//

#define OMM_PEACH_VIBE_TYPE_NONE                    (0)
#define OMM_PEACH_VIBE_TYPE_JOY                     (1)
#define OMM_PEACH_VIBE_TYPE_RAGE                    (2)
#define OMM_PEACH_VIBE_TYPE_GLOOM                   (3)
#define OMM_PEACH_VIBE_TYPE_CALM                    (4)
#define OMM_PEACH_VIBE_TYPE_COUNT                   (5)

#define OMM_PEACH_VIBE_NO_SLEEPING                  (omm_peach_vibe_is_rage() || omm_peach_vibe_is_gloom() || omm_peach_vibe_is_calm())
#define OMM_PEACH_VIBE_NO_PANTING                   (omm_peach_vibe_is_rage() || omm_peach_vibe_is_gloom() || omm_peach_vibe_is_calm())
#define OMM_PEACH_VIBE_NO_SHIVERING                 (omm_peach_vibe_is_rage() || omm_peach_vibe_is_gloom() || omm_peach_vibe_is_calm())
#define OMM_PEACH_VIBE_NO_COUGHING                  (omm_peach_vibe_is_rage() || omm_peach_vibe_is_calm())
#define OMM_PEACH_VIBE_NO_HEAD_TURN                 (omm_peach_vibe_is_rage() || omm_peach_vibe_is_gloom())

#define OMM_PEACH_VIBE_GAUGE_MAX                    (9000)
#define OMM_PEACH_VIBE_GAUGE_LIMIT                  ((OMM_PEACH_VIBE_GAUGE_MAX * 90) / 100) // Vibes cannot be activated under 10% Vibe power
#define OMM_PEACH_VIBE_GAUGE_INC                    (10) // Normal regen: fully recovers in 30 seconds (900 frames)
#define OMM_PEACH_VIBE_GAUGE_IDLE_INC               (20) // Idle regen: fully recovers in 15 seconds (450 frames)
#define OMM_PEACH_VIBE_GAUGE_HEART_INC              (30) // Standing inside a recovery heart recovers 10% per second (30 frames)
#define OMM_PEACH_VIBE_GAUGE_SPARKLE_INC            (450) // Collecting a Vibe sparkle restores 5% of the Vibe gauge
#define OMM_PEACH_VIBE_GAUGE_JOY_MOVE_DEC           (10) // 30 seconds (900 frames)
#define OMM_PEACH_VIBE_GAUGE_JOY_FLY_DEC            (60) //  5 seconds (150 frames)
#define OMM_PEACH_VIBE_GAUGE_JOY_ATTACK_DEC         (20) // 15 seconds (450 frames)
#define OMM_PEACH_VIBE_GAUGE_RAGE_DEC               (15) // 20 seconds (600 frames)
#define OMM_PEACH_VIBE_GAUGE_RAGE_SHOCKWAVE_DEC     (450) // Releasing a shockwave drains 5% of the Vibe gauge
#define OMM_PEACH_VIBE_GAUGE_GLOOM_DEC              (12) // 25 seconds (750 frames)
#define OMM_PEACH_VIBE_GAUGE_CALM_O2_DEC            (20) // 15 seconds (450 frames), recovers 6 O2 every frame (+5~15% per second)
#define OMM_PEACH_VIBE_GAUGE_CALM_HP_DEC            (30) // 10 seconds (300 frames), recovers 1 HP every 2 seconds (60 frames)
#define OMM_PEACH_VIBE_GAUGE_CALM_FREEZE_DEC        (10) // 30 seconds (900 frames), prevents the effect of frozen water

#define OMM_PEACH_VIBE_COOLDOWN                     (20)
#define OMM_PEACH_VIBE_MUSIC_FREQ_MOD_JOY           (1.50f)
#define OMM_PEACH_VIBE_MUSIC_FREQ_MOD_RAGE          (0.40f)
#define OMM_PEACH_VIBE_MUSIC_FREQ_MOD_GLOOM         (0.75f)
#define OMM_PEACH_VIBE_MUSIC_FREQ_MOD_CALM          (1.20f)
#define OMM_PEACH_VIBE_MUSIC_VOLUME_MOD_JOY         (1.00f)
#define OMM_PEACH_VIBE_MUSIC_VOLUME_MOD_RAGE        (1.40f)
#define OMM_PEACH_VIBE_MUSIC_VOLUME_MOD_GLOOM       (1.20f)
#define OMM_PEACH_VIBE_MUSIC_VOLUME_MOD_CALM        (1.00f)
#define OMM_PEACH_VIBE_MUSIC_STRM_FREQ_MOD_JOY      (1.40f)
#define OMM_PEACH_VIBE_MUSIC_STRM_FREQ_MOD_RAGE     (0.50f)
#define OMM_PEACH_VIBE_MUSIC_STRM_FREQ_MOD_GLOOM    (0.80f)
#define OMM_PEACH_VIBE_MUSIC_STRM_FREQ_MOD_CALM     (1.10f)
#define OMM_PEACH_VIBE_MUSIC_STRM_VOLUME_MOD_JOY    (1.00f)
#define OMM_PEACH_VIBE_MUSIC_STRM_VOLUME_MOD_RAGE   (1.30f)
#define OMM_PEACH_VIBE_MUSIC_STRM_VOLUME_MOD_GLOOM  (1.15f)
#define OMM_PEACH_VIBE_MUSIC_STRM_VOLUME_MOD_CALM   (1.00f)
#define OMM_PEACH_VIBE_MUSIC_R96A_FREQ_MOD_JOY      (1.40f)
#define OMM_PEACH_VIBE_MUSIC_R96A_FREQ_MOD_RAGE     (0.50f)
#define OMM_PEACH_VIBE_MUSIC_R96A_FREQ_MOD_GLOOM    (0.80f)
#define OMM_PEACH_VIBE_MUSIC_R96A_FREQ_MOD_CALM     (1.10f)
#define OMM_PEACH_VIBE_MUSIC_R96A_VOLUME_MOD_JOY    (1.00f)
#define OMM_PEACH_VIBE_MUSIC_R96A_VOLUME_MOD_RAGE   (1.30f)
#define OMM_PEACH_VIBE_MUSIC_R96A_VOLUME_MOD_GLOOM  (1.15f)
#define OMM_PEACH_VIBE_MUSIC_R96A_VOLUME_MOD_CALM   (1.00f)
#define OMM_PEACH_VIBE_SOUND_PITCH_MOD_JOY          (1.08f)
#define OMM_PEACH_VIBE_SOUND_PITCH_MOD_RAGE         (0.95f)
#define OMM_PEACH_VIBE_SOUND_PITCH_MOD_GLOOM        (0.90f)
#define OMM_PEACH_VIBE_SOUND_PITCH_MOD_CALM         (1.02f)

//
// Stationary
//

#define ACT_OMM_PEACH_ATTACK_GROUND                 (0x30 | ACT_GROUP_STATIONARY | ACT_FLAG_OMM_ACTION | ACT_FLAG_STATIONARY | ACT_FLAG_ATTACKING)

//
// Moving
//

#define ACT_OMM_PEACH_ATTACK_FAST                   (0x30 | ACT_GROUP_MOVING | ACT_FLAG_OMM_ACTION | ACT_FLAG_MOVING | ACT_FLAG_ATTACKING)

//
// Airborne
//

#define ACT_OMM_PEACH_FLOAT                         (0x30 | ACT_GROUP_AIRBORNE | ACT_FLAG_OMM_ACTION | ACT_FLAG_AIR | ACT_FLAG_ALLOW_VERTICAL_WIND_ACTION)
#define ACT_OMM_PEACH_GLIDE                         (0x31 | ACT_GROUP_AIRBORNE | ACT_FLAG_OMM_ACTION | ACT_FLAG_AIR | ACT_FLAG_ALLOW_VERTICAL_WIND_ACTION)
#define ACT_OMM_PEACH_ATTACK_AIR                    (0x32 | ACT_GROUP_AIRBORNE | ACT_FLAG_OMM_ACTION | ACT_FLAG_AIR | ACT_FLAG_ALLOW_VERTICAL_WIND_ACTION | ACT_FLAG_ATTACKING)
#define ACT_OMM_PEACH_VIBE_JOY_MOVE                 (0x33 | ACT_GROUP_AIRBORNE | ACT_FLAG_OMM_ACTION | ACT_FLAG_AIR | ACT_FLAG_ALLOW_VERTICAL_WIND_ACTION)
#define ACT_OMM_PEACH_VIBE_JOY_FLY                  (0x34 | ACT_GROUP_AIRBORNE | ACT_FLAG_OMM_ACTION | ACT_FLAG_AIR | ACT_FLAG_ALLOW_VERTICAL_WIND_ACTION)
#define ACT_OMM_PEACH_VIBE_JOY_ATTACK               (0x35 | ACT_GROUP_AIRBORNE | ACT_FLAG_OMM_ACTION | ACT_FLAG_AIR | ACT_FLAG_ALLOW_VERTICAL_WIND_ACTION | ACT_FLAG_ATTACKING)

//
// Submerged
//

//
// Cutscene
//

//
// Automatic
//

//
// Object
//

//
// Metal water
//

#endif // OMM_PEACH_ACTIONS_H
