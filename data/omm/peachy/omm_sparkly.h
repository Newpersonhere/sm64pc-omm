#ifndef OMM_SPARKLY_H
#define OMM_SPARKLY_H

#include "data/omm/omm_includes.h"
#include "omm_sparkly_data.h"

//
// Save data
//

bool omm_sparkly_is_mode_unlocked(s32 mode);
bool omm_sparkly_is_timer_started(s32 mode);
bool omm_sparkly_is_star_collected(s32 mode, s32 index);
bool omm_sparkly_is_bowser_4_unlocked(s32 mode);
bool omm_sparkly_is_grand_star_collected(s32 mode);
bool omm_sparkly_is_mode_completed(s32 mode);
s32  omm_sparkly_get_timer(s32 mode);
void omm_sparkly_unlock_mode(s32 mode);
void omm_sparkly_start_timer(s32 mode);
void omm_sparkly_collect_star(s32 mode, s32 index);
void omm_sparkly_unlock_bowser_4(s32 mode);
void omm_sparkly_collect_grand_star(s32 mode);
void omm_sparkly_clear_mode(s32 mode);
bool omm_sparkly_read_save(const char **tokens);
void omm_sparkly_write_save(char **buffer);

//
// Sparkly Stars data
//

bool omm_sparkly_is_enabled();
bool omm_sparkly_is_bowser_4();
bool omm_sparkly_is_mode_selected(s32 mode);
s32  omm_sparkly_get_current_mode();
s32  omm_sparkly_get_star_index(s32 mode, s32 level, s32 area);
s32  omm_sparkly_get_star_level(s32 mode, s32 index);
s32  omm_sparkly_get_star_area(s32 mode, s32 index);
u8  *omm_sparkly_get_star_name(s32 mode, s32 index);
s32  omm_sparkly_get_star_count(s32 mode);

//
// Context data
//

bool omm_sparkly_context_get_flag(s32 flag);
void omm_sparkly_context_set_flag(s32 flag, bool cond, bool unsetIfFalse);
void omm_sparkly_context_unset_flag(s32 flag, bool cond);
s32  omm_sparkly_context_get_counter(s32 counter);
void omm_sparkly_context_set_counter(s32 counter, s32 value);
void omm_sparkly_context_inc_counter(s32 counter, s32 inc);
s32  omm_sparkly_context_dif_counter(s32 counter);
bool omm_sparkly_context_check_star(struct Object *star);
bool omm_sparkly_context_is_state_ok();
void omm_sparkly_context_set_state(s32 state, bool sound);
void omm_sparkly_context_reset_data();
void omm_sparkly_context_restart();
void omm_sparkly_context_update(s32 flag);
void omm_sparkly_context_update_state(struct MarioState *m);
void omm_sparkly_context_spawn_star(struct MarioState *m);
void omm_sparkly_context_spawn_sign(struct MarioState *m);
s32  omm_sparkly_context_get_remaining_flames();
s32  omm_sparkly_context_get_remaining_boxes();

//
// Misc
//

bool omm_sparkly_check_cheats(struct MarioState *m);
void omm_sparkly_turn_off_cheats();
void omm_sparkly_set_opt_mode(s32 mode);
void omm_sparkly_disable();

//
// Actions and interactions
//

s32  omm_sparkly_act_star_dance(struct MarioState *m);
s32  omm_sparkly_act_end_cutscene_1(struct MarioState *m);
s32  omm_sparkly_act_end_cutscene_2(struct MarioState *m);
bool omm_sparkly_interact_star(struct MarioState *m, struct Object *o);
void omm_sparkly_interact_grand_star(struct MarioState *m, struct Object *o);

//
// Endings
//

void omm_sparkly_ending_update_cutscene();
void omm_sparkly_ending_render_screen();

#endif
