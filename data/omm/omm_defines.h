#ifndef OMM_DEFINES_H
#define OMM_DEFINES_H

//
// Utilities
//

#define OMM_EXE_FOLDER                                  sys_exe_path()
#define OMM_USER_FOLDER                                 sys_user_path()
#define OMM_RES_FOLDER                                  "omm"
#define OMM_GFX_FOLDER                                  OMM_RES_FOLDER "/gfx"
#define OMM_SOUND_FOLDER                                OMM_RES_FOLDER "/sound"
#define OMM_CAPPY_FOLDER                                OMM_GFX_FOLDER "/cappy"
#define OMM_AT_STARTUP                                  __attribute__((constructor))
#define OMM_AT_EXIT                                     __attribute__((destructor))
#define OMM_LIKELY(expr)                                (__builtin_expect(!!(expr), 1))
#define OMM_UNLIKELY(expr)                              (__builtin_expect(!!(expr), 0))
#define OMM_UNUSED(x)                                   (void) x
#define OMM_IS_60_FPS                                   (sizeof(struct GraphNodeObject) > 180)
#define OMM_LEVEL_NO_WARP(level)                        (omm_level_get_course(level) == COURSE_NONE)
#define OMM_BOWSER_IN_THE_LEVEL(level)                  (level == LEVEL_BOWSER_1 ? LEVEL_BITDW : (level == LEVEL_BOWSER_2 ? LEVEL_BITFS : (level == LEVEL_BOWSER_3 ? LEVEL_BITS : level)))
#define OMM_STRING(str, size, fmt, ...)                 char str[size]; snprintf(str, size, fmt, __VA_ARGS__);
#define OMM_PRINT_TEXT(x, y, fmt, ...)                  { char _str_[256]; snprintf(_str_, 256, fmt, __VA_ARGS__); print_text(x, y, _str_); }
#define OMM_ARRAY_OF(type)                              (type *) (type[])
#define OMM_ARRAY_SIZE(a)                               (sizeof(a) / sizeof(a[0]))
#define OMM_GET_FLAG(x, f)                              (((x) & (f)) != 0)
#define OMM_SET_FLAG(x, f, v)                           { if (v) { x |= (f); } else { x &= ~(f); } }
#define OMM_RETURN_IF_TRUE(cond, ret, ...)              { if (cond) { __VA_ARGS__; return ret; } }
enum { LEVEL_GROUNDS =                                  LEVEL_CASTLE_GROUNDS };
enum { LEVEL_COURT =                                    LEVEL_CASTLE_COURTYARD };

//
// Memory wrappers
//

#define OMM_MEMNEW(typ, siz)                            calloc(sizeof(typ), siz)
#define OMM_MEMDEL(ptr)                                 { if (ptr) { free(ptr); ptr = NULL; } }
#define OMM_MEMSET(dst, val, siz)                       memset(dst, val, siz)
#define OMM_MEMCPY(dst, src, siz)                       memcpy(dst, src, siz)
#define OMM_MEMMOV(dst, src, siz)                       memmove(dst, src, siz)
#define OMM_MEMDUP(ptr, siz)                            memcpy(calloc(1, siz), ptr, siz)
#define OMM_MEMCMP(pt1, pt2, siz)                       (memcmp(pt1, pt2, siz) == 0)
#define OMM_MEMSWP(pt1, pt2, siz)                       { unsigned char _temp_[siz]; memcpy(&_temp_, &(pt1), siz); memcpy(&(pt1), &(pt2), siz); memcpy(&(pt2), &_temp_, siz); }
#define OMM_MEMREL(dst, src, from, to, typ)             { dst = (typ) (((uintptr_t) (src)) + (((uintptr_t) (to)) - ((uintptr_t) (from)))); }

//
// Inlines
//

static inline s32 omm_min_s         (s32 a, s32 b)                      { return (a < b ? a : b); }
static inline f32 omm_min_f         (f32 a, f32 b)                      { return (a < b ? a : b); }
static inline s32 omm_max_s         (s32 a, s32 b)                      { return (a > b ? a : b); }
static inline f32 omm_max_f         (f32 a, f32 b)                      { return (a > b ? a : b); }
static inline s32 omm_near_s        (s32 x, s32 a, s32 b)               { return (x - a < b - x ? a : b); }
static inline f32 omm_near_f        (f32 x, f32 a, f32 b)               { return (x - a < b - x ? a : b); }
static inline s32 omm_far_s         (s32 x, s32 a, s32 b)               { return (x - a > b - x ? a : b); }
static inline f32 omm_far_f         (f32 x, f32 a, f32 b)               { return (x - a > b - x ? a : b); }
static inline s32 omm_min_3_s       (s32 a, s32 b, s32 c)               { return omm_min_s(omm_min_s(a, b), c); }
static inline f32 omm_min_3_f       (f32 a, f32 b, f32 c)               { return omm_min_f(omm_min_f(a, b), c); }
static inline s32 omm_max_3_s       (s32 a, s32 b, s32 c)               { return omm_max_s(omm_max_s(a, b), c); }
static inline f32 omm_max_3_f       (f32 a, f32 b, f32 c)               { return omm_max_f(omm_max_f(a, b), c); }
static inline s32 omm_clamp_s       (s32 x, s32 a, s32 b)               { return (x < a ? a : (x > b ? b : x)); }
static inline f32 omm_clamp_f       (f32 x, f32 a, f32 b)               { return (x < a ? a : (x > b ? b : x)); }
static inline s32 omm_clamp_0_1_s   (s32 x)                             { return omm_clamp_s(x, 0, 1); }
static inline f32 omm_clamp_0_1_f   (f32 x)                             { return omm_clamp_f(x, 0.f, 1.f); }
static inline s32 omm_lerp_s        (f32 t, s32 a, s32 b)               { return (a + (s32) (((f32) (b - a)) * t)); }
static inline f32 omm_lerp_f        (f32 t, f32 a, f32 b)               { return (a + (b - a) * t); }
static inline f32 omm_invlerp_s     (s32 x, s32 a, s32 b)               { return (f32) (x - a) / (f32) (b - a); }
static inline f32 omm_invlerp_f     (f32 x, f32 a, f32 b)               { return (x - a) / (b - a); }
static inline f32 omm_invlerp_0_1_s (s32 x, s32 a, s32 b)               { return omm_clamp_f(omm_invlerp_s(x, a, b), 0.f, 1.f); }
static inline f32 omm_invlerp_0_1_f (f32 x, f32 a, f32 b)               { return omm_clamp_f(omm_invlerp_f(x, a, b), 0.f, 1.f); }
static inline s32 omm_relerp_s      (s32 x, s32 a, s32 b, s32 c, s32 d) { return omm_lerp_s(omm_invlerp_s(x, a, b), c, d); }
static inline f32 omm_relerp_f      (f32 x, f32 a, f32 b, f32 c, f32 d) { return omm_lerp_f(omm_invlerp_f(x, a, b), c, d); }
static inline s32 omm_relerp_0_1_s  (s32 x, s32 a, s32 b, s32 c, s32 d) { return omm_lerp_s(omm_invlerp_0_1_s(x, a, b), c, d); }
static inline f32 omm_relerp_0_1_f  (f32 x, f32 a, f32 b, f32 c, f32 d) { return omm_lerp_f(omm_invlerp_0_1_f(x, a, b), c, d); }
static inline s32 omm_abs_s         (s32 x)                             { return (x < 0 ? -x : x); }
static inline f32 omm_abs_f         (f32 x)                             { return (x < 0.f ? -x : x); }
static inline s32 omm_sign_s        (s32 x)                             { return (x < 0 ? -1 : +1); }
static inline f32 omm_sign_f        (f32 x)                             { return (x < 0.f ? -1.f : +1.f); }
static inline s32 omm_sign_0_s      (s32 x)                             { return (x == 0 ? 0 : omm_sign_s(x)); }
static inline f32 omm_sign_0_f      (f32 x)                             { return (x == 0.f ? 0.f : omm_sign_f(x)); }
static inline s32 omm_sqr_s         (s32 x)                             { return x * x; }
static inline f32 omm_sqr_f         (f32 x)                             { return x * x; }

//
// Limits
//

#define OMM_COLLISION_LEVEL_BOUNDARY                    0x8000  // [-32768, +32767], Vanilla is [-8192, +8191]
#define OMM_COLLISION_CELL_SIZE                         0x400   // 1024, Vanilla is 1024
#define OMM_COLLISION_CELL_COUNT                        (2 * (OMM_COLLISION_LEVEL_BOUNDARY / OMM_COLLISION_CELL_SIZE))
#define OMM_COLLISION_CELL_BITS                         (OMM_COLLISION_CELL_COUNT - 1)
#define OMM_OBJECT_LOST_COIN                            0x1057

//
// Options
//

#define OMM_MOVESET_CLASSIC                             (gOmmMovesetType == 0)
#define OMM_MOVESET_ODYSSEY                             (gOmmMovesetType != 0)
#define OMM_MOVESET_ODYSSEY_3H                          (gOmmMovesetType == 1)
#define OMM_MOVESET_ODYSSEY_6H                          (gOmmMovesetType == 2)
#define OMM_CAP_CLASSIC                                 (gOmmCapType == 0)
#define OMM_CAP_CAPPY_NO_CAPTURE                        (gOmmCapType == 1)
#define OMM_CAP_CAPPY_CAPTURE                           (gOmmCapType == 2)
#define OMM_STARS_CLASSIC                               (gOmmStarsMode == 0)
#define OMM_STARS_NON_STOP                              (gOmmStarsMode == 1)
#define OMM_STARS_NON_STOP_NOT_ENDING_CUTSCENE          (OMM_STARS_NON_STOP && !omm_is_ending_cutscene())
#define OMM_POWER_UPS_CLASSIC                           (gOmmPowerUpsType == 0)
#define OMM_POWER_UPS_IMPROVED                          (gOmmPowerUpsType == 1)
#define OMM_CAMERA_CLASSIC                              (gOmmCameraMode == 0)
#define OMM_CAMERA_8_DIRECTIONS                         (gOmmCameraMode == 1)
#define OMM_CAMERA_16_DIRECTIONS                        (gOmmCameraMode == 2)
#define OMM_CHEAT_UNLIMITED_CAPPY_BOUNCES               (gOmmCheatUnlimitedCappyBounces == 1)
#define OMM_CHEAT_CAPPY_STAYS_FOREVER                   (gOmmCheatCappyStaysForever == 1)
#define OMM_CHEAT_HOMING_ATTACK_GLOBAL_RANGE            (gOmmCheatHomingAttackGlobalRange == 1)
#define OMM_CHEAT_MARIO_TELEPORTS_TO_CAPPY              (gOmmCheatMarioTeleportsToCappy == 1)
#define OMM_CHEAT_CAPPY_CAN_COLLECT_STARS               (gOmmCheatCappyCanCollectStars == 1)
#define OMM_CHEAT_PLAY_AS_CAPPY                         (gOmmCheatPlayAsCappy == 1)
#define OMM_CHEAT_PEACH_ENDLESS_VIBE_GAUGE              (gOmmCheatPeachEndlessVibeGauge == 1)
#define OMM_CHEAT_GOD_MODE                              (Cheats.EnableCheats && GodModeCheat)
#define OMM_CHEAT_MOON_JUMP                             (Cheats.EnableCheats && Cheats.MoonJump)
#define OMM_CHEATS_ENABLED                              (OMM_CHEAT_UNLIMITED_CAPPY_BOUNCES || \
                                                         OMM_CHEAT_CAPPY_STAYS_FOREVER || \
                                                         OMM_CHEAT_HOMING_ATTACK_GLOBAL_RANGE || \
                                                         OMM_CHEAT_MARIO_TELEPORTS_TO_CAPPY || \
                                                         OMM_CHEAT_CAPPY_CAN_COLLECT_STARS || \
                                                         OMM_CHEAT_PLAY_AS_CAPPY || \
                                                         OMM_CHEAT_PEACH_ENDLESS_VIBE_GAUGE)

//
// Texts
//

OMM_TEXT_(BLANK,                                        "")
OMM_TEXT_(SELECT_CHARACTER,                             "SELECT CHARACTER")
OMM_TEXT_(MARIO,                                        "Mario")
OMM_TEXT_(LUIGI,                                        "Luigi")
OMM_TEXT_(WARIO,                                        "Wario")
OMM_TEXT_(PEACH,                                        "Peach")
OMM_TEXT_(CAPPY,                                        "Cappy")
OMM_TEXT_(TIARA,                                        "Tiara")
OMM_TEXT_(MARIO_UPPER,                                  "MARIO")
OMM_TEXT_(LUIGI_UPPER,                                  "LUIGI")
OMM_TEXT_(WARIO_UPPER,                                  "WARIO")
OMM_TEXT_(PEACH_UPPER,                                  "PEACH")
OMM_TEXT_(CAPPY_UPPER,                                  "CAPPY")
OMM_TEXT_(TIARA_UPPER,                                  "TIARA")
OMM_TEXT_(MARIO_LOWER,                                  "mario")
OMM_TEXT_(LUIGI_LOWER,                                  "luigi")
OMM_TEXT_(WARIO_LOWER,                                  "wario")
OMM_TEXT_(PEACH_LOWER,                                  "peach")
OMM_TEXT_(CAPPY_LOWER,                                  "cappy")
OMM_TEXT_(TIARA_LOWER,                                  "tiara")
OMM_TEXT_(LOCKED,                                       "?????")
OMM_TEXT_(PAUSE,                                        "PAUSE")
OMM_TEXT_(COURSE,                                       "COURSE")
OMM_TEXT_(ACT,                                          "ACT")
OMM_TEXT_(MY_STARS,                                     STAR "S")
OMM_TEXT_(MY_SCORE,                                     "SCORE")
OMM_TEXT_(CONTINUE,                                     "CONTINUE")
OMM_TEXT_(RESTART_LEVEL,                                "RESTART LEVEL")
OMM_TEXT_(EXIT_LEVEL,                                   "EXIT LEVEL")
OMM_TEXT_(RETURN_TO_CASTLE,                             "RETURN TO CASTLE")
OMM_TEXT_(COURSE_COMPLETE_GOT_A_STAR,                   "YOU GOT A " STAR)
OMM_TEXT_(COURSE_COMPLETE_CONGRATULATIONS,              "CONGRATULATIONS")
OMM_TEXT_(COURSE_COMPLETE_HI_SCORE,                     "HI SCORE!")
OMM_TEXT_(YOU_GOT_A_STAR,                               "YOU GOT A " STAR)
OMM_TEXT_(SPARKLY_STAR_1,                               "PINK GOLD " STAR)
OMM_TEXT_(SPARKLY_STAR_2,                               "CRYSTAL " STAR)
OMM_TEXT_(SPARKLY_STAR_3,                               "NEBULA " STAR)
OMM_TEXT_(SPARKLY_STARS_1,                              "PINK GOLD " STAR "S")
OMM_TEXT_(SPARKLY_STARS_2,                              "CRYSTAL " STAR "S")
OMM_TEXT_(SPARKLY_STARS_3,                              "NEBULA " STAR "S")
OMM_TEXT_(LEVEL_CASTLE_INSIDE,                          "CASTLE INSIDE")
OMM_TEXT_(LEVEL_CASTLE_UPSTAIRS,                        "CASTLE UPSTAIRS")
OMM_TEXT_(LEVEL_CASTLE_BASEMENT,                        "CASTLE BASEMENT")
OMM_TEXT_(LEVEL_CASTLE_GROUNDS,                         "CASTLE GROUNDS")
OMM_TEXT_(LEVEL_CASTLE_COURTYARD,                       "CASTLE COURTYARD")
OMM_TEXT_(LEVEL_BOWSER_4,                               "BOWSER 4")
OMM_TEXT_(LEVEL_UNKNOWN,                                "???")
OMM_TEXT_(LEVEL_EMPTY,                                  "")
OMM_TEXT_(LEVEL_CASTLE,                                 "CASTLE")
OMM_TEXT_(LEVEL_BOWSER_1,                               "BOWSER 1")
OMM_TEXT_(LEVEL_BOWSER_2,                               "BOWSER 2")
OMM_TEXT_(LEVEL_BOWSER_3,                               "BOWSER 3")
OMM_TEXT_(LEVEL_100_COINS_STAR,                         "100 COINS " STAR)
OMM_TEXT_(LEVEL_RED_COINS_STAR,                         "RED COINS " STAR)
OMM_TEXT_(LEVEL_ONE_SECRET_STAR,                        "ONE OF THE CASTLE'S SECRET " STAR "S!")
OMM_TEXT_(LEVEL_STAR__,                                 STAR "  ")
#if defined(SMSR)
OMM_TEXT_(LEVEL_STAR_REPLICA,                           "STAR REPLICA")
#endif
OMM_TEXT_(LEVEL_SECRET_STARS,                           "SECRET " STAR "S")
OMM_TEXT_(BAD_ENDING_TOAD_1,                            "Mario!")
OMM_TEXT_(BAD_ENDING_TOAD_2,                            "The Princess... where is she?")
OMM_TEXT_(BAD_ENDING_TOAD_3,                            "The Grand " Star " didn't free her... Why?")
OMM_TEXT_(BAD_ENDING_TOAD_4,                            "Is there... nothing we can do?")
OMM_TEXT_(BAD_ENDING_TOAD_5,                            "No... we can't give up now!")
OMM_TEXT_(BAD_ENDING_TOAD_6,                            "Yeah! We won't let Bowser win, right?")
OMM_TEXT_(BAD_ENDING_TOAD_7,                            "Mario! We'll help you to save her... again!")
OMM_TEXT_(BAD_ENDING_TOAD_8,                            "Sure! I want to eat her delicious cake!")
OMM_TEXT_(BAD_ENDING_TOAD_9,                            "Come on, Mario! Let's find Princess Peach!")
OMM_TEXT_(OPT_MENU_TITLE,                               "ODYSSEY MARIO MOUESET") // There is no 'V' in the HUD font, but the 'U' looks close enough
OMM_TEXT_(OPT_MENU_LABEL,                               "ODYSSEY MARIO'S MOVESET")
OMM_TEXT_(OPT_CHARACTER_LABEL,                          "Character")
OMM_TEXT_(OPT_MOVESET_LABEL,                            "Moveset")
OMM_TEXT_(OPT_MOVESET_CLASSIC,                          "Classic")
OMM_TEXT_(OPT_MOVESET_ODYSSEY_3H,                       "Odyssey - 3-Health")
OMM_TEXT_(OPT_MOVESET_ODYSSEY_6H,                       "Odyssey - 6-Health")
OMM_TEXT_(OPT_CAP_LABEL,                                "Cap")
OMM_TEXT_(OPT_CAP_CLASSIC,                              "Classic")
OMM_TEXT_(OPT_CAP_NO_CAPTURE,                           "Cappy - No Capture")
OMM_TEXT_(OPT_CAP_CAPTURE,                              "Cappy - Capture")
OMM_TEXT_(OPT_STARS_LABEL,                              Star "s")
OMM_TEXT_(OPT_STARS_CLASSIC,                            "Classic")
OMM_TEXT_(OPT_STARS_NON_STOP,                           "Non-Stop")
OMM_TEXT_(OPT_POWER_UPS_LABEL,                          "Power-ups")
OMM_TEXT_(OPT_POWER_UPS_CLASSIC,                        "Classic")
OMM_TEXT_(OPT_POWER_UPS_IMPROVED,                       "Improved")
OMM_TEXT_(OPT_CAMERA_LABEL,                             "Camera")
OMM_TEXT_(OPT_CAMERA_CLASSIC,                           "Classic")
OMM_TEXT_(OPT_CAMERA_8_DIR,                             "8-Directions")
OMM_TEXT_(OPT_CAMERA_16_DIR,                            "16-Directions")
OMM_TEXT_(OPT_SPARKLY_STARS_LABEL,                      "Sparkly " Star "s")
OMM_TEXT_(OPT_SPARKLY_STARS_MODE_0,                     "Disabled")
OMM_TEXT_(OPT_SPARKLY_STARS_MODE_1,                     "Normal Mode")
OMM_TEXT_(OPT_SPARKLY_STARS_MODE_2,                     "Hard Mode")
OMM_TEXT_(OPT_SPARKLY_STARS_MODE_3,                     "Lunatic Mode")
OMM_TEXT_(OPT_CHEATS_TITLE,                             "CHEATS")
OMM_TEXT_(OPT_CHEATS_LABEL,                             "Cheats")
OMM_TEXT_(OPT_CHEAT_UNLIMITED_CAPPY_BOUNCES,            "Unlimited Cappy Bounces")
OMM_TEXT_(OPT_CHEAT_CAPPY_STAYS_FOREVER,                "Cappy Stays Forever")
OMM_TEXT_(OPT_CHEAT_GLOBAL_HOMING_ATTACK_RANGE,         "Homing Attack Global Range")
OMM_TEXT_(OPT_CHEAT_MARIO_TELEPORTS_TO_CAPPY,           "Mario Teleports to Cappy")
OMM_TEXT_(OPT_CHEAT_CAPPY_CAN_COLLECT_STARS,            "Cappy Can Collect Stars")
OMM_TEXT_(OPT_CHEAT_PLAY_AS_CAPPY,                      "Play As Cappy")
OMM_TEXT_(OPT_CHEAT_PEACH_ENDLESS_VIBE_GAUGE,           "Peach Endless Vibe Gauge")
OMM_TEXT_(OPT_EXTRAS_TITLE,                             "EoTRAS") // Not a typo, the 'o' character is the 'X' glyph of the HUD font
OMM_TEXT_(OPT_EXTRAS_LABEL,                             "Extras")
OMM_TEXT_(OPT_CAPPY_EYES_ON_MARIOS_CAP,                 "Cappy Eyes on Mario's Cap")
OMM_TEXT_(OPT_COLORED_STARS,                            "Colored " Star "s")
OMM_TEXT_(OPT_3D_COINS,                                 "3D Coins")
OMM_TEXT_(OPT_VANISHING_HUD,                            "Vanishing HUD")
OMM_TEXT_(OPT_REVEAL_SECRETS,                           "Reveal Secrets")
OMM_TEXT_(OPT_RED_COINS_RADAR,                          "Red Coins Radar")
OMM_TEXT_(OPT_SHOW_STAR_NUMBER,                         "Show Star Number")
OMM_TEXT_(OPT_INVISIBLE_MODE,                           "Invisible Mode")
OMM_TEXT_(OPT_CRYSTAL_STARS_REWARD,                     "Crystal Stars Reward")
OMM_TEXT_(OPT_SHORTCUTS_TITLE,                          "SHORTCUTS")
OMM_TEXT_(OPT_SHORTCUTS_LABEL,                          "Shortcuts")
OMM_TEXT_(OPT_SHORTCUTS_OPTION,                         "Option ")
OMM_TEXT_(OPT_SHORTCUTS_SET_TO,                         " set to ")
OMM_TEXT_(OPT_CONTROLS_TITLE,                           "CONTROLS")
OMM_TEXT_(OPT_CONTROLS_LABEL,                           "CONTROLS")
OMM_TEXT_(OPT_CONTROLS_A_BUTTON,                        "A Button")
OMM_TEXT_(OPT_CONTROLS_B_BUTTON,                        "B Button")
OMM_TEXT_(OPT_CONTROLS_X_BUTTON,                        "X Button")
OMM_TEXT_(OPT_CONTROLS_Y_BUTTON,                        "Y Button")
OMM_TEXT_(OPT_CONTROLS_START_BUTTON,                    "Start Button")
OMM_TEXT_(OPT_CONTROLS_L_TRIGGER,                       "L Trigger")
OMM_TEXT_(OPT_CONTROLS_R_TRIGGER,                       "R Trigger")
OMM_TEXT_(OPT_CONTROLS_Z_TRIGGER,                       "Z Trigger")
OMM_TEXT_(OPT_CONTROLS_C_UP,                            "C-Up")
OMM_TEXT_(OPT_CONTROLS_C_DOWN,                          "C-Down")
OMM_TEXT_(OPT_CONTROLS_C_LEFT,                          "C-Left")
OMM_TEXT_(OPT_CONTROLS_C_RIGHT,                         "C-Right")
OMM_TEXT_(OPT_CONTROLS_D_UP,                            "D-Up")
OMM_TEXT_(OPT_CONTROLS_D_DOWN,                          "D-Down")
OMM_TEXT_(OPT_CONTROLS_D_LEFT,                          "D-Left")
OMM_TEXT_(OPT_CONTROLS_D_RIGHT,                         "D-Right")
OMM_TEXT_(OPT_CONTROLS_STICK_UP,                        "Stick Up")
OMM_TEXT_(OPT_CONTROLS_STICK_DOWN,                      "Stick Down")
OMM_TEXT_(OPT_CONTROLS_STICK_LEFT,                      "Stick Left")
OMM_TEXT_(OPT_CONTROLS_STICK_RIGHT,                     "Stick Right")
OMM_TEXT_(OPT_CONTROLS_STICK_DEADZONE,                  "Stick Deadzone")
OMM_TEXT_(OPT_CONTROLS_RESET,                           "Reset Controls")
OMM_TEXT_(OPT_WARP_TO_LEVEL_TITLE,                      "WARP TO LEUEL") // There is no 'V' in the HUD font, but the 'U' looks close enough
OMM_TEXT_(OPT_WARP_TO_LEVEL_LABEL,                      "WARP TO LEVEL")
OMM_TEXT_(OPT_WARP_TO_LEVEL_LEVEL,                      "Level")
OMM_TEXT_(OPT_WARP_TO_LEVEL_AREA,                       "Area")
OMM_TEXT_(OPT_WARP_TO_LEVEL_ACT,                        Star)
OMM_TEXT_(OPT_WARP_TO_LEVEL_WARP,                       "Warp")
OMM_TEXT_(OPT_RETURN_TO_MAIN_MENU_LABEL,                "RETURN TO MAIN MENU")
OMM_TEXT_(OPT_PRESS_A,                                  "([A])>")
OMM_TEXT_(OPT_DISABLED,                                 "Disabled")
OMM_TEXT_(OPT_ENABLED,                                  "Enabled")
OMM_TEXT_(OPT_NONE,                                     "NONE")
OMM_TEXT_(OPT_DOT_DOT_DOT,                              "...")
#if defined(SM74)
OMM_TEXT_(SM74_SUPER_MARIO_74,                          "Super Mario 74")
OMM_TEXT_(SM74_NORMAL_EDITION,                          "Normal Edition")
OMM_TEXT_(SM74_EXTREME_EDITION,                         "Extreme Edition")
OMM_TEXT_(SM74_SWAP_VERSION,                            "SWAP VERSION")
OMM_TEXT_(SM74_OPT_WARP_EDITION,                        "Edition")
OMM_TEXT_(SM74_OPT_WARP_NORMAL,                         "Normal")
OMM_TEXT_(SM74_OPT_WARP_EXTREME,                        "Extreme")
#endif
#if !defined(SMSR)
OMM_TEXT_(MENU_HARD_MODE_ON,                            "Hard Mode ON")
OMM_TEXT_(MENU_HARD_MODE_OFF,                           "Hard Mode OFF")
#endif
#if OMM_CODE_DEBUG
OMM_TEXT_(OPT_DEBUG_TITLE,                              "DEBUG")
OMM_TEXT_(OPT_DEBUG_LABEL,                              "Debug")
OMM_TEXT_(OPT_DEBUG_HITBOX,                             "Display Hitboxes")
OMM_TEXT_(OPT_DEBUG_HURTBOX,                            "Display Hurtboxes")
OMM_TEXT_(OPT_DEBUG_WALLBOX,                            "Display Wallboxes")
OMM_TEXT_(OPT_DEBUG_MARIO,                              "Display Mario Values")
OMM_TEXT_(OPT_DEBUG_CAPPY,                              "Display Cappy Values")
OMM_TEXT_(OPT_DEBUG_PROFILER,                           "Display Execution Time")
#if OMM_CODE_DEV_DEBUG
OMM_TEXT_(OPT_DEBUG_GAME_SPEED,                         "Game Speed")
OMM_TEXT_(OPT_DEBUG_GAME_SPEED_30_FPS,                  "30 FPS")
OMM_TEXT_(OPT_DEBUG_GAME_SPEED_20_FPS,                  "20 FPS")
OMM_TEXT_(OPT_DEBUG_GAME_SPEED_15_FPS,                  "15 FPS")
OMM_TEXT_(OPT_DEBUG_GAME_SPEED_10_FPS,                  "10 FPS")
OMM_TEXT_(OPT_DEBUG_GAME_SPEED_5_FPS,                   "5 FPS")
#endif
#endif

//
// Dialogs
//

#define OMM_DIALOG_START_INDEX                          DIALOG_COUNT
#define OMM_DIALOG_BOWSER_1_INTRO                       (OMM_DIALOG_START_INDEX + 0)
#define OMM_DIALOG_BOWSER_1_HINT_1                      (OMM_DIALOG_START_INDEX + 1)
#define OMM_DIALOG_BOWSER_1_HINT_2                      (OMM_DIALOG_START_INDEX + 2)
#define OMM_DIALOG_BOWSER_1_HINT_3                      (OMM_DIALOG_START_INDEX + 3)
#define OMM_DIALOG_BOWSER_2_INTRO                       (OMM_DIALOG_START_INDEX + 4)
#define OMM_DIALOG_BOWSER_3_INTRO                       (OMM_DIALOG_START_INDEX + 5)
#define OMM_DIALOG_BOWSER_3_DEFEAT                      (OMM_DIALOG_START_INDEX + 6)

#define OMM_DIALOG_SPARKLY_STAR_NAMES                   (OMM_DIALOG_START_INDEX + 6)
#define OMM_DIALOG_SPARKLY_STAR_1_NAMES                 (OMM_DIALOG_START_INDEX + 7)
#define OMM_DIALOG_SPARKLY_STAR_2_NAMES                 (OMM_DIALOG_START_INDEX + 8)
#define OMM_DIALOG_SPARKLY_STAR_3_NAMES                 (OMM_DIALOG_START_INDEX + 9)

#define OMM_DIALOG_SPARKLY_STAR_1_BOB                   (OMM_DIALOG_START_INDEX + 10)
#define OMM_DIALOG_SPARKLY_STAR_1_WF                    (OMM_DIALOG_START_INDEX + 11)
#define OMM_DIALOG_SPARKLY_STAR_1_JRB                   (OMM_DIALOG_START_INDEX + 12)
#define OMM_DIALOG_SPARKLY_STAR_1_BBH                   (OMM_DIALOG_START_INDEX + 13)
#define OMM_DIALOG_SPARKLY_STAR_1_LLL                   (OMM_DIALOG_START_INDEX + 14)
#define OMM_DIALOG_SPARKLY_STAR_1_SL                    (OMM_DIALOG_START_INDEX + 15)
#define OMM_DIALOG_SPARKLY_STAR_1_THI                   (OMM_DIALOG_START_INDEX + 16)
#define OMM_DIALOG_SPARKLY_STAR_1_TTC                   (OMM_DIALOG_START_INDEX + 17)
#define OMM_DIALOG_SPARKLY_STAR_1_RR                    (OMM_DIALOG_START_INDEX + 18)
#define OMM_DIALOG_SPARKLY_STAR_1_BITDW                 (OMM_DIALOG_START_INDEX + 19)
#define OMM_DIALOG_SPARKLY_STAR_1_BITFS                 (OMM_DIALOG_START_INDEX + 20)
#define OMM_DIALOG_SPARKLY_STAR_1_BITS                  (OMM_DIALOG_START_INDEX + 21)
#define OMM_DIALOG_SPARKLY_STAR_1_TOTWC                 (OMM_DIALOG_START_INDEX + 22)
#define OMM_DIALOG_SPARKLY_STAR_1_VCUTM                 (OMM_DIALOG_START_INDEX + 23)
#define OMM_DIALOG_SPARKLY_STAR_1_COTMC                 (OMM_DIALOG_START_INDEX + 24)
#define OMM_DIALOG_SPARKLY_STAR_1_PSS                   (OMM_DIALOG_START_INDEX + 25)
#define OMM_DIALOG_SPARKLY_STAR_1_WMOTR                 (OMM_DIALOG_START_INDEX + 26)
#define OMM_DIALOG_SPARKLY_STAR_1_SA                    (OMM_DIALOG_START_INDEX + 27)
#define OMM_DIALOG_SPARKLY_STAR_1_GROUNDS               (OMM_DIALOG_START_INDEX + 28)
#define OMM_DIALOG_SPARKLY_STAR_1_INSIDE                (OMM_DIALOG_START_INDEX + 29)
#define OMM_DIALOG_SPARKLY_STAR_1_BASEMENT              (OMM_DIALOG_START_INDEX + 30)
#define OMM_DIALOG_SPARKLY_STAR_1_COURT                 (OMM_DIALOG_START_INDEX + 31)
#define OMM_DIALOG_SPARKLY_STAR_1_BOWSER_4_UNLOCKED     (OMM_DIALOG_START_INDEX + 32)
#define OMM_DIALOG_SPARKLY_STAR_1_BOWSER_4_INTRO        (OMM_DIALOG_START_INDEX + 33)
#define OMM_DIALOG_SPARKLY_STAR_1_BOWSER_4_DEFEAT       (OMM_DIALOG_START_INDEX + 34)

#define OMM_DIALOG_SPARKLY_STAR_2_BOB                   (OMM_DIALOG_START_INDEX + 35)
#define OMM_DIALOG_SPARKLY_STAR_2_WF                    (OMM_DIALOG_START_INDEX + 36)
#define OMM_DIALOG_SPARKLY_STAR_2_JRB                   (OMM_DIALOG_START_INDEX + 37)
#define OMM_DIALOG_SPARKLY_STAR_2_BBH                   (OMM_DIALOG_START_INDEX + 38)
#define OMM_DIALOG_SPARKLY_STAR_2_HMC                   (OMM_DIALOG_START_INDEX + 39)
#define OMM_DIALOG_SPARKLY_STAR_2_LLL                   (OMM_DIALOG_START_INDEX + 40)
#define OMM_DIALOG_SPARKLY_STAR_2_SSL                   (OMM_DIALOG_START_INDEX + 41)
#define OMM_DIALOG_SPARKLY_STAR_2_SL                    (OMM_DIALOG_START_INDEX + 42)
#define OMM_DIALOG_SPARKLY_STAR_2_WDW                   (OMM_DIALOG_START_INDEX + 43)
#define OMM_DIALOG_SPARKLY_STAR_2_THI                   (OMM_DIALOG_START_INDEX + 44)
#define OMM_DIALOG_SPARKLY_STAR_2_TTC                   (OMM_DIALOG_START_INDEX + 45)
#define OMM_DIALOG_SPARKLY_STAR_2_RR                    (OMM_DIALOG_START_INDEX + 46)
#define OMM_DIALOG_SPARKLY_STAR_2_BITDW                 (OMM_DIALOG_START_INDEX + 47)
#define OMM_DIALOG_SPARKLY_STAR_2_BITFS                 (OMM_DIALOG_START_INDEX + 48)
#define OMM_DIALOG_SPARKLY_STAR_2_BITS                  (OMM_DIALOG_START_INDEX + 49)
#define OMM_DIALOG_SPARKLY_STAR_2_TOTWC                 (OMM_DIALOG_START_INDEX + 50)
#define OMM_DIALOG_SPARKLY_STAR_2_VCUTM                 (OMM_DIALOG_START_INDEX + 51)
#define OMM_DIALOG_SPARKLY_STAR_2_COTMC                 (OMM_DIALOG_START_INDEX + 52)
#define OMM_DIALOG_SPARKLY_STAR_2_PSS                   (OMM_DIALOG_START_INDEX + 53)
#define OMM_DIALOG_SPARKLY_STAR_2_WMOTR                 (OMM_DIALOG_START_INDEX + 54)
#define OMM_DIALOG_SPARKLY_STAR_2_SA                    (OMM_DIALOG_START_INDEX + 55)
#define OMM_DIALOG_SPARKLY_STAR_2_GROUNDS               (OMM_DIALOG_START_INDEX + 56)
#define OMM_DIALOG_SPARKLY_STAR_2_INSIDE                (OMM_DIALOG_START_INDEX + 57)
#define OMM_DIALOG_SPARKLY_STAR_2_UPSTAIRS              (OMM_DIALOG_START_INDEX + 58)
#define OMM_DIALOG_SPARKLY_STAR_2_COURT                 (OMM_DIALOG_START_INDEX + 59)
#define OMM_DIALOG_SPARKLY_STAR_2_BOWSER_4_UNLOCKED     (OMM_DIALOG_START_INDEX + 60)
#define OMM_DIALOG_SPARKLY_STAR_2_BOWSER_4_INTRO        (OMM_DIALOG_START_INDEX + 61)
#define OMM_DIALOG_SPARKLY_STAR_2_BOWSER_4_DEFEAT       (OMM_DIALOG_START_INDEX + 62)

#define OMM_DIALOG_SPARKLY_STAR_3_GROUNDS               (OMM_DIALOG_START_INDEX + 62)

#define OMM_DIALOG_SPARKLY_MIPS_1                       (OMM_DIALOG_START_INDEX + 63)
#define OMM_DIALOG_SPARKLY_MIPS_2                       (OMM_DIALOG_START_INDEX + 64)
#define OMM_DIALOG_SPARKLY_MIPS_3                       (OMM_DIALOG_START_INDEX + 65)
#define OMM_DIALOG_SPARKLY_ANTI_CHEAT_INTRO             (OMM_DIALOG_START_INDEX + 66)
#define OMM_DIALOG_SPARKLY_ANTI_CHEAT_0                 (OMM_DIALOG_START_INDEX + 67)
#define OMM_DIALOG_SPARKLY_ANTI_CHEAT_1                 (OMM_DIALOG_START_INDEX + 68)
#define OMM_DIALOG_SPARKLY_ANTI_CHEAT_2                 (OMM_DIALOG_START_INDEX + 69)
#define OMM_DIALOG_SPARKLY_ANTI_CHEAT_3                 (OMM_DIALOG_START_INDEX + 70)
#define OMM_DIALOG_SPARKLY_ANTI_CHEAT_4                 (OMM_DIALOG_START_INDEX + 71)
#define OMM_DIALOG_SPARKLY_ANTI_CHEAT_5                 (OMM_DIALOG_START_INDEX + 72)
#define OMM_DIALOG_SPARKLY_ANTI_CHEAT_6                 (OMM_DIALOG_START_INDEX + 73)
#define OMM_DIALOG_SPARKLY_ANTI_CHEAT_7                 (OMM_DIALOG_START_INDEX + 74)
#define OMM_DIALOG_SPARKLY_ANTI_CHEAT_END_0             (OMM_DIALOG_START_INDEX + 75)
#define OMM_DIALOG_SPARKLY_ANTI_CHEAT_END_1             (OMM_DIALOG_START_INDEX + 76)
#define OMM_DIALOG_SPARKLY_ANTI_CHEAT_END_2             (OMM_DIALOG_START_INDEX + 77)
#define OMM_DIALOG_END_INDEX                            (OMM_DIALOG_START_INDEX + 78)
#define OMM_DIALOG_COUNT                                (OMM_DIALOG_END_INDEX - OMM_DIALOG_START_INDEX)

//
// Textures
//

#define OMM_GFX                                         "OMM_GFX/"
#define OMM_TEXTURE_(id, name)                          static const char OMM_TEXTURE_##id[] = OMM_GFX name;

OMM_TEXTURE_(STAR_BODY_0,                               "star/omm_texture_star_body_0")
OMM_TEXTURE_(STAR_BODY_1,                               "star/omm_texture_star_body_1")
OMM_TEXTURE_(STAR_BODY_2,                               "star/omm_texture_star_body_2")
OMM_TEXTURE_(STAR_BODY_3,                               "star/omm_texture_star_body_3")
OMM_TEXTURE_(STAR_BODY_4,                               "star/omm_texture_star_body_4")
OMM_TEXTURE_(STAR_BODY_5,                               "star/omm_texture_star_body_5")
OMM_TEXTURE_(STAR_BODY_6,                               "star/omm_texture_star_body_6")
OMM_TEXTURE_(STAR_BODY_7,                               "star/omm_texture_star_body_7")
OMM_TEXTURE_(STAR_BODY_8,                               "star/omm_texture_star_body_8")
OMM_TEXTURE_(STAR_BODY_9,                               "star/omm_texture_star_body_9")
OMM_TEXTURE_(STAR_BODY_10,                              "star/omm_texture_star_body_10")
OMM_TEXTURE_(STAR_BODY_11,                              "star/omm_texture_star_body_11")
OMM_TEXTURE_(STAR_BODY_12,                              "star/omm_texture_star_body_12")
OMM_TEXTURE_(STAR_BODY_13,                              "star/omm_texture_star_body_13")
OMM_TEXTURE_(STAR_BODY_14,                              "star/omm_texture_star_body_14")
OMM_TEXTURE_(STAR_BODY_15,                              "star/omm_texture_star_body_15")
OMM_TEXTURE_(STAR_BODY_16,                              "star/omm_texture_star_body_16")
OMM_TEXTURE_(STAR_BODY_17,                              "star/omm_texture_star_body_17")
OMM_TEXTURE_(STAR_BODY_18,                              "star/omm_texture_star_body_18")
OMM_TEXTURE_(STAR_BODY_19,                              "star/omm_texture_star_body_19")
OMM_TEXTURE_(STAR_FULL_0,                               "star/omm_texture_star_full_0")
OMM_TEXTURE_(STAR_FULL_1,                               "star/omm_texture_star_full_1")
OMM_TEXTURE_(STAR_FULL_2,                               "star/omm_texture_star_full_2")
OMM_TEXTURE_(STAR_FULL_3,                               "star/omm_texture_star_full_3")
OMM_TEXTURE_(STAR_FULL_4,                               "star/omm_texture_star_full_4")
OMM_TEXTURE_(STAR_FULL_5,                               "star/omm_texture_star_full_5")
OMM_TEXTURE_(STAR_FULL_6,                               "star/omm_texture_star_full_6")
OMM_TEXTURE_(STAR_FULL_7,                               "star/omm_texture_star_full_7")
OMM_TEXTURE_(STAR_FULL_8,                               "star/omm_texture_star_full_8")
OMM_TEXTURE_(STAR_FULL_9,                               "star/omm_texture_star_full_9")
OMM_TEXTURE_(STAR_FULL_10,                              "star/omm_texture_star_full_10")
OMM_TEXTURE_(STAR_FULL_11,                              "star/omm_texture_star_full_11")
OMM_TEXTURE_(STAR_FULL_12,                              "star/omm_texture_star_full_12")
OMM_TEXTURE_(STAR_FULL_13,                              "star/omm_texture_star_full_13")
OMM_TEXTURE_(STAR_FULL_14,                              "star/omm_texture_star_full_14")
OMM_TEXTURE_(STAR_FULL_15,                              "star/omm_texture_star_full_15")
OMM_TEXTURE_(STAR_FULL_16,                              "star/omm_texture_star_full_16")
OMM_TEXTURE_(STAR_FULL_17,                              "star/omm_texture_star_full_17")
OMM_TEXTURE_(STAR_FULL_18,                              "star/omm_texture_star_full_18")
OMM_TEXTURE_(STAR_FULL_19,                              "star/omm_texture_star_full_19")
OMM_TEXTURE_(STAR_EYE,                                  "star/omm_texture_star_eye")

OMM_TEXTURE_(CAPPY_EYE_32,                              "cappy/omm_texture_cappy_eye_32")
OMM_TEXTURE_(CAPPY_EYE_64,                              "cappy/omm_texture_cappy_eye_64")
OMM_TEXTURE_(CAPPY_EYE_128,                             "cappy/omm_texture_cappy_eye_128")
OMM_TEXTURE_(CAPPY_METAL_32,                            "cappy/omm_texture_cappy_metal_32")
OMM_TEXTURE_(CAPPY_METAL_64,                            "cappy/omm_texture_cappy_metal_64")
OMM_TEXTURE_(CAPPY_METAL_128,                           "cappy/omm_texture_cappy_metal_128")

OMM_TEXTURE_(HUD_0,                                     "hud/omm_texture_hud_0")
OMM_TEXTURE_(HUD_1,                                     "hud/omm_texture_hud_1")
OMM_TEXTURE_(HUD_2,                                     "hud/omm_texture_hud_2")
OMM_TEXTURE_(HUD_3,                                     "hud/omm_texture_hud_3")
OMM_TEXTURE_(HUD_4,                                     "hud/omm_texture_hud_4")
OMM_TEXTURE_(HUD_5,                                     "hud/omm_texture_hud_5")
OMM_TEXTURE_(HUD_6,                                     "hud/omm_texture_hud_6")
OMM_TEXTURE_(HUD_7,                                     "hud/omm_texture_hud_7")
OMM_TEXTURE_(HUD_8,                                     "hud/omm_texture_hud_8")
OMM_TEXTURE_(HUD_9,                                     "hud/omm_texture_hud_9")
OMM_TEXTURE_(HUD_M,                                     "hud/omm_texture_hud_m")
OMM_TEXTURE_(HUD_S,                                     "hud/omm_texture_hud_s")
OMM_TEXTURE_(HUD_J,                                     "hud/omm_texture_hud_j")
OMM_TEXTURE_(HUD_Q,                                     "hud/omm_texture_hud_q")
OMM_TEXTURE_(HUD_V,                                     "hud/omm_texture_hud_v")
OMM_TEXTURE_(HUD_X,                                     "hud/omm_texture_hud_x")
OMM_TEXTURE_(HUD_Z,                                     "hud/omm_texture_hud_z")
OMM_TEXTURE_(HUD_SLASH,                                 "hud/omm_texture_hud_slash")
OMM_TEXTURE_(HUD_HYPHEN,                                "hud/omm_texture_hud_hyphen")
OMM_TEXTURE_(HUD_DOTS,                                  "hud/omm_texture_hud_dots")
OMM_TEXTURE_(HUD_COIN,                                  "hud/omm_texture_hud_coin")
OMM_TEXTURE_(HUD_CAMERA,                                "hud/omm_texture_hud_camera")
OMM_TEXTURE_(HUD_CAMERA_8,                              "hud/omm_texture_hud_camera_8")
OMM_TEXTURE_(HUD_CAMERA_16,                             "hud/omm_texture_hud_camera_16")
OMM_TEXTURE_(HUD_CAMERA_NEAR,                           "hud/omm_texture_hud_camera_near")
OMM_TEXTURE_(HUD_CAMERA_FAR,                            "hud/omm_texture_hud_camera_far")
OMM_TEXTURE_(HUD_ARROW,                                 "hud/omm_texture_hud_arrow")
OMM_TEXTURE_(HUD_CAP_MARIO,                             "hud/omm_texture_hud_cap_mario")
OMM_TEXTURE_(HUD_CAP_MARIO_METAL,                       "hud/omm_texture_hud_cap_mario_metal")
OMM_TEXTURE_(HUD_CAPPY_MARIO,                           "hud/omm_texture_hud_cappy_mario")
OMM_TEXTURE_(HUD_CAPPY_MARIO_METAL,                     "hud/omm_texture_hud_cappy_mario_metal")
OMM_TEXTURE_(HUD_CAP_PEACH,                             "hud/omm_texture_hud_cap_peach")
OMM_TEXTURE_(HUD_CAP_PEACH_METAL,                       "hud/omm_texture_hud_cap_peach_metal")
OMM_TEXTURE_(HUD_CAPPY_PEACH,                           "hud/omm_texture_hud_cappy_peach")
OMM_TEXTURE_(HUD_CAPPY_PEACH_METAL,                     "hud/omm_texture_hud_cappy_peach_metal")
OMM_TEXTURE_(HUD_CAP_LUIGI,                             "hud/omm_texture_hud_cap_luigi")
OMM_TEXTURE_(HUD_CAP_LUIGI_METAL,                       "hud/omm_texture_hud_cap_luigi_metal")
OMM_TEXTURE_(HUD_CAPPY_LUIGI,                           "hud/omm_texture_hud_cappy_luigi")
OMM_TEXTURE_(HUD_CAPPY_LUIGI_METAL,                     "hud/omm_texture_hud_cappy_luigi_metal")
OMM_TEXTURE_(HUD_CAP_WARIO,                             "hud/omm_texture_hud_cap_wario")
OMM_TEXTURE_(HUD_CAP_WARIO_METAL,                       "hud/omm_texture_hud_cap_wario_metal")
OMM_TEXTURE_(HUD_CAPPY_WARIO,                           "hud/omm_texture_hud_cappy_wario")
OMM_TEXTURE_(HUD_CAPPY_WARIO_METAL,                     "hud/omm_texture_hud_cappy_wario_metal")
OMM_TEXTURE_(HUD_CAP_WINGS,                             "hud/omm_texture_hud_cap_wings")
OMM_TEXTURE_(HUD_VIBE_NORMAL,                           "hud/omm_texture_hud_vibe_normal")
OMM_TEXTURE_(HUD_VIBE_JOY,                              "hud/omm_texture_hud_vibe_joy")
OMM_TEXTURE_(HUD_VIBE_RAGE,                             "hud/omm_texture_hud_vibe_rage")
OMM_TEXTURE_(HUD_VIBE_GLOOM,                            "hud/omm_texture_hud_vibe_gloom")
OMM_TEXTURE_(HUD_VIBE_CALM,                             "hud/omm_texture_hud_vibe_calm")

OMM_TEXTURE_(BOWSER_FIRE_RED_1,                         "bowser/omm_texture_bowser_fire_red_1")
OMM_TEXTURE_(BOWSER_FIRE_RED_2,                         "bowser/omm_texture_bowser_fire_red_2")
OMM_TEXTURE_(BOWSER_FIRE_BLUE_1,                        "bowser/omm_texture_bowser_fire_blue_1")
OMM_TEXTURE_(BOWSER_FIRE_BLUE_2,                        "bowser/omm_texture_bowser_fire_blue_2")
OMM_TEXTURE_(BOWSER_FIRE_MAGIC_1,                       "bowser/omm_texture_bowser_fire_magic_1")
OMM_TEXTURE_(BOWSER_FIRE_MAGIC_2,                       "bowser/omm_texture_bowser_fire_magic_2")
OMM_TEXTURE_(BOWSER_FIRE_BREATH,                        "bowser/omm_texture_bowser_fire_breath")
OMM_TEXTURE_(BOWSER_4_0,                                "bowser/omm_texture_bowser_4_0")
OMM_TEXTURE_(BOWSER_4_1,                                "bowser/omm_texture_bowser_4_1")
OMM_TEXTURE_(BOWSER_4_2,                                "bowser/omm_texture_bowser_4_2")
OMM_TEXTURE_(BOWSER_4_3,                                "bowser/omm_texture_bowser_4_3")
OMM_TEXTURE_(BOWSER_4_4,                                "bowser/omm_texture_bowser_4_4")
OMM_TEXTURE_(BOWSER_4_5,                                "bowser/omm_texture_bowser_4_5")

OMM_TEXTURE_(ENDING_0_MARIO,                            "ending/omm_texture_ending_0_mario")
OMM_TEXTURE_(ENDING_0_LUIGI,                            "ending/omm_texture_ending_0_luigi")
OMM_TEXTURE_(ENDING_0_WARIO,                            "ending/omm_texture_ending_0_wario")
OMM_TEXTURE_(ENDING_1,                                  "ending/omm_texture_ending_1")

OMM_TEXTURE_(MENU_MARIO_HEAD,                           "menu/omm_texture_menu_mario_head")
OMM_TEXTURE_(MENU_MARIO_BACK,                           "menu/omm_texture_menu_mario_back")
OMM_TEXTURE_(MENU_MARIO_DARK,                           "menu/omm_texture_menu_mario_dark")
OMM_TEXTURE_(MENU_PEACH_HEAD,                           "menu/omm_texture_menu_peach_head")
OMM_TEXTURE_(MENU_PEACH_BACK,                           "menu/omm_texture_menu_peach_back")
OMM_TEXTURE_(MENU_PEACH_DARK,                           "menu/omm_texture_menu_peach_dark")
OMM_TEXTURE_(MENU_LUIGI_HEAD,                           "menu/omm_texture_menu_luigi_head")
OMM_TEXTURE_(MENU_LUIGI_BACK,                           "menu/omm_texture_menu_luigi_back")
OMM_TEXTURE_(MENU_LUIGI_DARK,                           "menu/omm_texture_menu_luigi_dark")
OMM_TEXTURE_(MENU_WARIO_HEAD,                           "menu/omm_texture_menu_wario_head")
OMM_TEXTURE_(MENU_WARIO_BACK,                           "menu/omm_texture_menu_wario_back")
OMM_TEXTURE_(MENU_WARIO_DARK,                           "menu/omm_texture_menu_wario_dark")
OMM_TEXTURE_(MENU_FONT,                                 "menu/omm_texture_menu_font")

OMM_TEXTURE_(SPARKLY_STAR_1_HINT_FRONT,                 "sparkly/omm_texture_sparkly_star_1_hint_front")
OMM_TEXTURE_(SPARKLY_STAR_1_HINT_BACK,                  "sparkly/omm_texture_sparkly_star_1_hint_back")
OMM_TEXTURE_(SPARKLY_STAR_1_BLOCK_FRONT,                "sparkly/omm_texture_sparkly_star_1_block_front")
OMM_TEXTURE_(SPARKLY_STAR_1_BLOCK_SIDE,                 "sparkly/omm_texture_sparkly_star_1_block_side")
OMM_TEXTURE_(SPARKLY_STAR_1_SPARKLE_1,                  "sparkly/omm_texture_sparkly_star_1_sparkle_1")
OMM_TEXTURE_(SPARKLY_STAR_1_SPARKLE_2,                  "sparkly/omm_texture_sparkly_star_1_sparkle_2")
OMM_TEXTURE_(SPARKLY_STAR_2_HINT_FRONT,                 "sparkly/omm_texture_sparkly_star_2_hint_front")
OMM_TEXTURE_(SPARKLY_STAR_2_HINT_BACK,                  "sparkly/omm_texture_sparkly_star_2_hint_back")
OMM_TEXTURE_(SPARKLY_STAR_2_BLOCK_FRONT,                "sparkly/omm_texture_sparkly_star_2_block_front")
OMM_TEXTURE_(SPARKLY_STAR_2_BLOCK_SIDE,                 "sparkly/omm_texture_sparkly_star_2_block_side")
OMM_TEXTURE_(SPARKLY_STAR_2_SPARKLE_1,                  "sparkly/omm_texture_sparkly_star_2_sparkle_1")
OMM_TEXTURE_(SPARKLY_STAR_2_SPARKLE_2,                  "sparkly/omm_texture_sparkly_star_2_sparkle_2")
OMM_TEXTURE_(SPARKLY_STAR_3_HINT_FRONT,                 "sparkly/omm_texture_sparkly_star_3_hint_front")
OMM_TEXTURE_(SPARKLY_STAR_3_HINT_BACK,                  "sparkly/omm_texture_sparkly_star_3_hint_back")
OMM_TEXTURE_(SPARKLY_STAR_3_BLOCK_FRONT,                "sparkly/omm_texture_sparkly_star_3_block_front")
OMM_TEXTURE_(SPARKLY_STAR_3_BLOCK_SIDE,                 "sparkly/omm_texture_sparkly_star_3_block_side")
OMM_TEXTURE_(SPARKLY_STAR_3_SPARKLE_1,                  "sparkly/omm_texture_sparkly_star_3_sparkle_1")
OMM_TEXTURE_(SPARKLY_STAR_3_SPARKLE_2,                  "sparkly/omm_texture_sparkly_star_3_sparkle_2")

OMM_TEXTURE_(EFFECT_METAL_SPARKLE_1,                    "effect/omm_texture_effect_metal_sparkle_1")
OMM_TEXTURE_(EFFECT_METAL_SPARKLE_2,                    "effect/omm_texture_effect_metal_sparkle_2")
OMM_TEXTURE_(EFFECT_METAL_SPARKLE_3,                    "effect/omm_texture_effect_metal_sparkle_3")
OMM_TEXTURE_(EFFECT_METAL_SPARKLE_4,                    "effect/omm_texture_effect_metal_sparkle_4")
OMM_TEXTURE_(EFFECT_METAL_SPARKLE_5,                    "effect/omm_texture_effect_metal_sparkle_5")
OMM_TEXTURE_(EFFECT_METAL_SPARKLE_6,                    "effect/omm_texture_effect_metal_sparkle_6")
OMM_TEXTURE_(EFFECT_METAL_SPARKLE_7,                    "effect/omm_texture_effect_metal_sparkle_7")
OMM_TEXTURE_(EFFECT_METAL_SPARKLE_8,                    "effect/omm_texture_effect_metal_sparkle_8")
OMM_TEXTURE_(EFFECT_METAL_SPARKLE_9,                    "effect/omm_texture_effect_metal_sparkle_9")
OMM_TEXTURE_(EFFECT_METAL_SPARKLE_10,                   "effect/omm_texture_effect_metal_sparkle_10")
OMM_TEXTURE_(EFFECT_METAL_SPARKLE_11,                   "effect/omm_texture_effect_metal_sparkle_11")
OMM_TEXTURE_(EFFECT_METAL_SPARKLE_12,                   "effect/omm_texture_effect_metal_sparkle_12")
OMM_TEXTURE_(EFFECT_METAL_SPARKLE_13,                   "effect/omm_texture_effect_metal_sparkle_13")
OMM_TEXTURE_(EFFECT_METAL_SPARKLE_14,                   "effect/omm_texture_effect_metal_sparkle_14")
OMM_TEXTURE_(EFFECT_METAL_SPARKLE_15,                   "effect/omm_texture_effect_metal_sparkle_15")
OMM_TEXTURE_(EFFECT_METAL_SPARKLE_16,                   "effect/omm_texture_effect_metal_sparkle_16")
OMM_TEXTURE_(EFFECT_BREAK_PARTICLE_1,                   "effect/omm_texture_effect_break_particle_1")
OMM_TEXTURE_(EFFECT_BREAK_PARTICLE_2,                   "effect/omm_texture_effect_break_particle_2")
OMM_TEXTURE_(EFFECT_BREAK_PARTICLE_3,                   "effect/omm_texture_effect_break_particle_3")
OMM_TEXTURE_(EFFECT_BREAK_PARTICLE_4,                   "effect/omm_texture_effect_break_particle_4")
OMM_TEXTURE_(EFFECT_BREAK_PARTICLE_5,                   "effect/omm_texture_effect_break_particle_5")
OMM_TEXTURE_(EFFECT_PROBLEM,                            "effect/omm_texture_effect_problem")
OMM_TEXTURE_(EFFECT_FREEZE,                             "effect/omm_texture_effect_freeze")
OMM_TEXTURE_(EFFECT_LAVA,                               "effect/omm_texture_effect_lava")
OMM_TEXTURE_(EFFECT_PERRY_SHOCKWAVE,                    "effect/omm_texture_effect_perry_shockwave")
OMM_TEXTURE_(EFFECT_VIBE_SPARKLE,                       "effect/omm_texture_effect_vibe_sparkle")
OMM_TEXTURE_(EFFECT_VIBE_JOY,                           "effect/omm_texture_effect_vibe_joy")
OMM_TEXTURE_(EFFECT_VIBE_JOY_TORNADO,                   "effect/omm_texture_effect_vibe_joy_tornado")
OMM_TEXTURE_(EFFECT_VIBE_JOY_SPARKLE,                   "effect/omm_texture_effect_vibe_joy_sparkle")
OMM_TEXTURE_(EFFECT_VIBE_RAGE,                          "effect/omm_texture_effect_vibe_rage")
OMM_TEXTURE_(EFFECT_VIBE_RAGE_AURA,                     "effect/omm_texture_effect_vibe_rage_aura")
OMM_TEXTURE_(EFFECT_VIBE_RAGE_SHOCKWAVE_1,              "effect/omm_texture_effect_vibe_rage_shockwave_1")
OMM_TEXTURE_(EFFECT_VIBE_RAGE_SHOCKWAVE_2,              "effect/omm_texture_effect_vibe_rage_shockwave_2")
OMM_TEXTURE_(EFFECT_VIBE_GLOOM,                         "effect/omm_texture_effect_vibe_gloom")
OMM_TEXTURE_(EFFECT_VIBE_CALM,                          "effect/omm_texture_effect_vibe_calm")
OMM_TEXTURE_(EFFECT_VIBE_CALM_AURA,                     "effect/omm_texture_effect_vibe_calm_aura")
OMM_TEXTURE_(EFFECT_VIBE_CALM_SPARKLE,                  "effect/omm_texture_effect_vibe_calm_sparkle")

//
// Sounds
//

#define OMM_SOUND_PEACH_JUMP_YAH                        0x00
#define OMM_SOUND_PEACH_JUMP_WAH                        0x01
#define OMM_SOUND_PEACH_JUMP_HOO                        0x02
#define OMM_SOUND_PEACH_JUMP_YAHOO                      0x03
#define OMM_SOUND_PEACH_JUMP_WAHA                       0x04
#define OMM_SOUND_PEACH_JUMP_YIPPEE                     0x05
#define OMM_SOUND_PEACH_LAND_HAHA                       0x06
#define OMM_SOUND_PEACH_PUNCH_YAH                       0x07
#define OMM_SOUND_PEACH_PUNCH_WAH                       0x08
#define OMM_SOUND_PEACH_PUNCH_HOO                       0x09
#define OMM_SOUND_PEACH_GRAB_HRMM                       0x0A
#define OMM_SOUND_PEACH_THROW_WAH2                      0x0B
#define OMM_SOUND_PEACH_GROUND_POUND_WAH                0x0C
#define OMM_SOUND_PEACH_DIVE_HOOHOO                     0x0D
#define OMM_SOUND_PEACH_TWIRL_BOUNCE                    0x0E
#define OMM_SOUND_PEACH_SO_LONGA_BOWSER                 0x0F
#define OMM_SOUND_PEACH_LEDGE_GRAB_WHOA                 0x10
#define OMM_SOUND_PEACH_LEDGE_CLIMB_EEUH                0x11
#define OMM_SOUND_PEACH_LEDGE_CLIMB_UH2                 0x12
#define OMM_SOUND_PEACH_LEDGE_MISS_UH                   0x13
#define OMM_SOUND_PEACH_BONK_DOH                        0x14
#define OMM_SOUND_PEACH_BONK_OOOF                       0x15
#define OMM_SOUND_PEACH_ATTACKED                        0x16
#define OMM_SOUND_PEACH_ON_FIRE                         0x17
#define OMM_SOUND_PEACH_DYING                           0x18
#define OMM_SOUND_PEACH_DROWNING                        0x19
#define OMM_SOUND_PEACH_FALLING_WAAAOOOW                0x1A
#define OMM_SOUND_PEACH_MAMA_MIA                        0x1B
#define OMM_SOUND_PEACH_LETS_A_GO                       0x1C
#define OMM_SOUND_PEACH_HERE_WE_GO                      0x1D
#define OMM_SOUND_PEACH_PANTING1                        0x1E
#define OMM_SOUND_PEACH_PANTING2                        0x1F
#define OMM_SOUND_PEACH_PANTING3                        0x20
#define OMM_SOUND_PEACH_COUGHING1                       0x21
#define OMM_SOUND_PEACH_COUGHING2                       0x22
#define OMM_SOUND_PEACH_COUGHING3                       0x23
#define OMM_SOUND_PEACH_YAWNING                         0x24
#define OMM_SOUND_PEACH_VIBE_JOY                        0x25
#define OMM_SOUND_PEACH_VIBE_RAGE                       0x26
#define OMM_SOUND_PEACH_VIBE_GLOOM                      0x27
#define OMM_SOUND_PEACH_VIBE_CALM                       0x28

#define OMM_SOUND_TOAD_1                                0x40
#define OMM_SOUND_TOAD_2                                0x41
#define OMM_SOUND_TOAD_3                                0x42
#define OMM_SOUND_TOAD_4                                0x43
#define OMM_SOUND_TOAD_5                                0x44
#define OMM_SOUND_TOAD_6                                0x45
#define OMM_SOUND_TOAD_7                                0x46
#define OMM_SOUND_TOAD_8                                0x47
#define OMM_SOUND_TOAD_9                                0x48

#define OMM_SOUND_EVENT_CAPTURE                         0x50
#define OMM_SOUND_EVENT_UNCAPTURE                       0x51
#define OMM_SOUND_EVENT_LIFE_UP                         0x52
#define OMM_SOUND_EVENT_SPARKLY_STAR_GET                0x53
#define OMM_SOUND_EVENT_DEATH_MARIO                     0x54
#define OMM_SOUND_EVENT_DEATH_MARIO_WATER               0x55
#define OMM_SOUND_EVENT_DEATH_MARIO_FALL                0x56
#define OMM_SOUND_EVENT_DEATH_PEACH                     0x57
#define OMM_SOUND_EVENT_DEATH_PEACH_WATER               0x58
#define OMM_SOUND_EVENT_DEATH_PEACH_FALL                0x59
#define OMM_SOUND_EVENT_DEATH_LUIGI                     0x5A
#define OMM_SOUND_EVENT_DEATH_LUIGI_WATER               0x5B
#define OMM_SOUND_EVENT_DEATH_LUIGI_FALL                0x5C
#define OMM_SOUND_EVENT_DEATH_WARIO                     0x5D
#define OMM_SOUND_EVENT_DEATH_WARIO_WATER               0x5E
#define OMM_SOUND_EVENT_DEATH_WARIO_FALL                0x5F

#define OMM_SOUND_EFFECT_DAMAGE                         0x80
#define OMM_SOUND_EFFECT_HEAL                           0x81
#define OMM_SOUND_EFFECT_FREEZE_1                       0x82
#define OMM_SOUND_EFFECT_FREEZE_2                       0x83
#define OMM_SOUND_EFFECT_PROPELLER_1                    0x84
#define OMM_SOUND_EFFECT_PROPELLER_2                    0x85
#define OMM_SOUND_EFFECT_PROPELLER_3                    0x86
#define OMM_SOUND_EFFECT_MIDAIR_SPIN                    0x87
#define OMM_SOUND_EFFECT_PEACH_GLIDE                    0x88
#define OMM_SOUND_EFFECT_PEACH_VIBE                     0x89

//
// Playable characters
//

#define OMM_PLAYER_MARIO                                0
#define OMM_PLAYER_PEACH                                1
#define OMM_PLAYER_LUIGI                                2
#define OMM_PLAYER_WARIO                                3
#define OMM_PLAYER_IS_MARIO                             omm_player_is_selected(OMM_PLAYER_MARIO)
#define OMM_PLAYER_IS_PEACH                             omm_player_is_selected(OMM_PLAYER_PEACH)
#define OMM_PLAYER_IS_LUIGI                             omm_player_is_selected(OMM_PLAYER_LUIGI)
#define OMM_PLAYER_IS_WARIO                             omm_player_is_selected(OMM_PLAYER_WARIO)

//
// Routines
//

#define OMM_ROUTINE_TYPE_LEVEL_ENTRY                    0
#define OMM_ROUTINE_TYPE_UPDATE                         1
#define OMM_ROUTINE_TYPE_GFX                            2
#define OMM_ROUTINE_LEVEL_ENTRY(func)                   static void func(void); OMM_AT_STARTUP static void func##_init() { omm_add_routine(OMM_ROUTINE_TYPE_LEVEL_ENTRY, func); } static void func(void)
#define OMM_ROUTINE_UPDATE(func)                        static void func(void); OMM_AT_STARTUP static void func##_init() { omm_add_routine(OMM_ROUTINE_TYPE_UPDATE, func); } static void func(void)
#define OMM_ROUTINE_GFX(func)                           static void func(void); OMM_AT_STARTUP static void func##_init() { omm_add_routine(OMM_ROUTINE_TYPE_GFX, func); } static void func(void)

//
// Level script
//

#define OMM_LEVEL_SCRIPT_CONTINUE                       0
#define OMM_LEVEL_SCRIPT_NEXT_CMD                       1
#define OMM_LEVEL_SCRIPT_RETURN                         2
#define OMM_LEVEL_SCRIPT_STOP                           3

//
// Custom object fields
//

#define gCurrGraphNodeObject                            ((struct Object *) gCurGraphNodeObject)
#define oCameraToObject                                 header.gfx.cameraToObject
#define oGraphNode                                      header.gfx.sharedChild
#define oNodeFlags                                      header.gfx.node.flags
#define oScaleX                                         header.gfx.scale[0]
#define oScaleY                                         header.gfx.scale[1]
#define oScaleZ                                         header.gfx.scale[2]
#define oWall                                           OBJECT_FIELD_SURFACE(0x19)
#define oGeoData                                        OBJECT_FIELD_VPTR(0x47)
#define oCappyLifeTimer                                 OBJECT_FIELD_S32(0x1B)
#define oCappyBehavior                                  OBJECT_FIELD_S16(0x1C, 0)
#define oCappyFlags                                     OBJECT_FIELD_S16(0x1C, 1)
#define oCappyYaw                                       OBJECT_FIELD_S16(0x1D, 0)
#define oCappyThrowStrength                             OBJECT_FIELD_S16(0x1D, 1)
#define oGoombaStackParent                              OBJECT_FIELD_OBJ(0x00)
#define oMipsCurrentWaypoint                            OBJECT_FIELD_S32(0x1D)
#define oMipsTargetWaypoint                             OBJECT_FIELD_S32(0x1E)
#define oMipsGrabbedCounter                             OBJECT_FIELD_S32(0x1F)
#define oMipsEscapePath                                 OBJECT_FIELD_S32(0x20)
#define oMipsEscapeIndex                                OBJECT_FIELD_S32(0x21)
#define oCelebStarRadius                                OBJECT_FIELD_F32(0x1B)
#define oCelebStarHeight                                OBJECT_FIELD_F32(0x1C)
#define oFlamingBobombAura                              OBJECT_FIELD_OBJ(0x00)
#define oFlamingBobombIndex                             OBJECT_FIELD_S32(0x1B)
#define oFlamingBobombMaxHeight                         OBJECT_FIELD_F32(0x1C)
#define oBowserMine                                     OBJECT_FIELD_VPTR(0x1B)
#define oBowserHit                                      OBJECT_FIELD_S32(0x1C)
#define oBowserFireDuration                             OBJECT_FIELD_S32(0x1B)
#define oIsBowserClone                                  OBJECT_FIELD_S32(0x1D)
#define oChainChompFreeState                            OBJECT_FIELD_S32(0x1F)
#define oChainChompFreeTimer                            OBJECT_FIELD_S32(0x20)
#define oChainChompFreeAngle                            OBJECT_FIELD_S32(0x21)
#define oShockwaveFireRadius                            OBJECT_FIELD_F32(0x1B)
#define oShockwaveFireWidth                             OBJECT_FIELD_F32(0x1C)
#define oShockwaveFireHeight                            OBJECT_FIELD_F32(0x1D)
#define oShockwaveFireSpeed                             OBJECT_FIELD_F32(0x1E)
#define oShockwaveFireDuration                          OBJECT_FIELD_S32(0x1F)
#define oRisingLavaMinY                                 OBJECT_FIELD_F32(0x1B)
#define oRisingLavaMaxY                                 OBJECT_FIELD_F32(0x1C)
#define oRisingLavaVelY                                 OBJECT_FIELD_F32(0x1D)
#define oRisingLavaRadius                               OBJECT_FIELD_F32(0x1E)
#define oRisingLavaRotVel                               OBJECT_FIELD_S32(0x1F)
#define oRisingLavaShake                                OBJECT_FIELD_S32(0x20)
#define oSparklyStarMode                                OBJECT_FIELD_S32(0x1C)
#define oSparklyStarPosX                                OBJECT_FIELD_F32(0x1E)
#define oSparklyStarPosY                                OBJECT_FIELD_F32(0x1F)
#define oSparklyStarPosZ                                OBJECT_FIELD_F32(0x20)
#define oPerryFlags                                     OBJECT_FIELD_S32(0x1B)
#define oPerryShockwaveType                             OBJECT_FIELD_S32(0x1B)
#define oPerryShockwaveDelay                            OBJECT_FIELD_S32(0x1C)
#define oPerryShockwaveAngleYaw                         OBJECT_FIELD_S32(0x1D)
#define oPerryShockwaveClockwise                        OBJECT_FIELD_S32(0x1E)
#define oMrIBeamPower                                   OBJECT_FIELD_F32(0x17)
#define oMenuButtonCharacterIndex                       OBJECT_FIELD_S32(0x21)
#define oBehaviorTypes                                  unused1

#endif // OMM_DEFINES_H
