#ifndef OMM_PERRY_H
#define OMM_PERRY_H

#include "types.h"

typedef struct OmmPerryModifiers {
    u8  colors[9];
    f32 swordHitbox;
    f32 shockwaveRadius;
    f32 shockwaveHeight;
    f32 shockwaveForwardVel;
    f32 blastRadius;
    u32 swordFlags;
    u32 shockwaveFlags;
    u32 blastFlags;
} OmmPerryModifiers;
extern const OmmPerryModifiers gOmmPerryModifiers[5];

#define __OMM_PERRY_SWORD_HITBOX                120
#define __OMM_PERRY_SWORD_INT_FLAGS             (OBJ_INT_ATTACK_WEAK | OBJ_INT_ATTACK_BREAKABLE | OBJ_INT_COLLECT_COINS | OBJ_INT_COLLECT_TRIGGERS | OBJ_INT_COLLECT_STARS | OBJ_INT_COLLECT_CAPS | OBJ_INT_PERRY_ATTACK)
#define __OMM_PERRY_SHOCKWAVE_RADIUS            150
#define __OMM_PERRY_SHOCKWAVE_HEIGHT            100
#define __OMM_PERRY_SHOCKWAVE_DURATION          30
#define __OMM_PERRY_SHOCKWAVE_FORWARD_VEL       80
#define __OMM_PERRY_SHOCKWAVE_INT_FLAGS         (OBJ_INT_ATTACK_WEAK | OBJ_INT_ATTACK_BREAKABLE | OBJ_INT_COLLECT_COINS | OBJ_INT_COLLECT_TRIGGERS | OBJ_INT_PERRY_ATTACK)
#define __OMM_PERRY_BLAST_RADIUS                300
#define __OMM_PERRY_BLAST_INT_FLAGS             (OBJ_INT_ATTACK_WEAK | OBJ_INT_ATTACK_STRONG | OBJ_INT_ATTACK_BREAKABLE | OBJ_INT_COLLECT_COINS | OBJ_INT_COLLECT_TRIGGERS | OBJ_INT_PERRY_ATTACK)

#define OMM_PERRY_SWORD_HITBOX                  (__OMM_PERRY_SWORD_HITBOX * gOmmPerryModifiers[o->oAnimState].swordHitbox)
#define OMM_PERRY_SWORD_INT_FLAGS               (__OMM_PERRY_SWORD_INT_FLAGS | gOmmPerryModifiers[o->oAnimState].swordFlags)
#define OMM_PERRY_SHOCKWAVE_RADIUS              ((o->oPerryShockwaveBlast ? 1.10f : 1.00f) * (__OMM_PERRY_SHOCKWAVE_RADIUS * gOmmPerryModifiers[o->oAnimState].shockwaveRadius))
#define OMM_PERRY_SHOCKWAVE_HEIGHT              ((o->oPerryShockwaveBlast ? 1.10f : 1.00f) * (__OMM_PERRY_SHOCKWAVE_HEIGHT * gOmmPerryModifiers[o->oAnimState].shockwaveHeight))
#define OMM_PERRY_SHOCKWAVE_DURATION            (__OMM_PERRY_SHOCKWAVE_DURATION)
#define OMM_PERRY_SHOCKWAVE_FORWARD_VEL         ((o->oPerryShockwaveBlast ? 1.05f : 1.00f) * (__OMM_PERRY_SHOCKWAVE_FORWARD_VEL * gOmmPerryModifiers[o->oAnimState].shockwaveForwardVel))
#define OMM_PERRY_SHOCKWAVE_INT_FLAGS           (o->oPerryShockwaveBlast ? OMM_PERRY_BLAST_INT_FLAGS : (__OMM_PERRY_SHOCKWAVE_INT_FLAGS | gOmmPerryModifiers[o->oAnimState].shockwaveFlags))
#define OMM_PERRY_BLAST_RADIUS                  (__OMM_PERRY_BLAST_RADIUS * gOmmPerryModifiers[o->oAnimState].blastRadius)
#define OMM_PERRY_BLAST_INT_FLAGS               (__OMM_PERRY_BLAST_INT_FLAGS | gOmmPerryModifiers[o->oAnimState].blastFlags)
#define OMM_PERRY_COLOR_WHITE                   (gOmmPerryModifiers[o->oAnimState].colors + 0)
#define OMM_PERRY_COLOR_FRONT                   (gOmmPerryModifiers[o->oAnimState].colors + 3)
#define OMM_PERRY_COLOR_BACK                    (gOmmPerryModifiers[o->oAnimState].colors + 6)
#define OMM_PERRY_COLOR(type, channel)          (gOmmPerryModifiers[o->oAnimState].colors[(3 * (type)) + (channel)])

#define OMM_PERRY_CHARGE_ACTION                 (10)
#define OMM_PERRY_CHARGE_START                  (20)
#define OMM_PERRY_CHARGE_FULL                   (45)
#define OMM_PERRY_CHARGE_END                    (60)
#define OMM_PERRY_CHARGED                       (gOmmPerryCharge > OMM_PERRY_CHARGE_FULL)
#define OMM_PERRY_CHARGING                      (gMarioState->action == ACT_OMM_PEACH_PERRY_CHARGE_GROUND || gMarioState->action == ACT_OMM_PEACH_PERRY_CHARGE_AIR)
#define gOmmPerryCharge                         gOmmData->mario->peach.perryCharge
#define gOmmPerryBlast                          gOmmData->mario->peach.perryBlast

void omm_peach_update_perry_graphics(struct MarioState *m, Mat4 transform, Vec3f translation, Vec3s rotation);

#endif
