#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// String conversion (very basic)
//

#if !defined(R96A)
static const struct { const char *str; u8 c64; } sSm64CharMap[] = {
    { "0",   0x00 }, { "1",  0x01 }, { "2",   0x02 }, { "3",   0x03 }, { "4",   0x04 }, { "5",   0x05 },
    { "6",   0x06 }, { "7",  0x07 }, { "8",   0x08 }, { "9",   0x09 }, { "A",   0x0A }, { "B",   0x0B },
    { "C",   0x0C }, { "D",  0x0D }, { "E",   0x0E }, { "F",   0x0F }, { "G",   0x10 }, { "H",   0x11 },
    { "I",   0x12 }, { "J",  0x13 }, { "K",   0x14 }, { "L",   0x15 }, { "M",   0x16 }, { "N",   0x17 },
    { "O",   0x18 }, { "P",  0x19 }, { "Q",   0x1A }, { "R",   0x1B }, { "S",   0x1C }, { "T",   0x1D },
    { "U",   0x1E }, { "V",  0x1F }, { "W",   0x20 }, { "X",   0x21 }, { "Y",   0x22 }, { "Z",   0x23 },
    { "a",   0x24 }, { "b",  0x25 }, { "c",   0x26 }, { "d",   0x27 }, { "e",   0x28 }, { "f",   0x29 },
    { "g",   0x2A }, { "h",  0x2B }, { "i",   0x2C }, { "j",   0x2D }, { "k",   0x2E }, { "l",   0x2F },
    { "m",   0x30 }, { "n",  0x31 }, { "o",   0x32 }, { "p",   0x33 }, { "q",   0x34 }, { "r",   0x35 },
    { "s",   0x36 }, { "t",  0x37 }, { "u",   0x38 }, { "v",   0x39 }, { "w",   0x3A }, { "x",   0x3B },
    { "y",   0x3C }, { "z",  0x3D }, { "\'",  0x3E }, { ".",   0x3F }, { "^",   0x50 }, { "|",   0x51 },
    { "<",   0x52 }, { ">",  0x53 }, { "[A]", 0x54 }, { "[B]", 0x55 }, { "[C]", 0x56 }, { "[Z]", 0x57 },
    { "[R]", 0x58 }, { ",",  0x6F }, { " ",   0x9E }, { "-",   0x9F }, { "/",   0xD0 }, { "[%]", 0xE0 },
    { "(",   0xE1 }, { ")(", 0xE2 }, { ")",   0xE3 }, { "+",   0xE4 }, { "&",   0xE5 }, { ":",   0xE6 },
    { "!",   0xF2 }, { "%",  0xF3 }, { "?",   0xF4 }, { "~",   0xF7 }, { "$",   0xF9 }, { "@",   0xFA },
    { "*",   0xFB }, { "=",  0xFD }, { "\n",  0xFE }, { NULL,  0xFF },
};

static const char *omm_text_add_char(u8 *str64, const char *str, s32 *i) {
    for (s32 j = 0; sSm64CharMap[j].str; ++j) {
        if (strstr(str, sSm64CharMap[j].str) == str) {
            str64[(*i)++] = sSm64CharMap[j].c64;
            return str + strlen(sSm64CharMap[j].str);
        }
    }
    str64[(*i)++] = 0x9E;
    return str + 1;
}
#endif

u8 *omm_text_convert(const char *str, bool heapAlloc) {
    u8 *str64 = omm_memory_new(heapAlloc ? NULL : gOmmMemoryPoolStrings, 0x800, NULL);
#if defined(R96A)
    extern struct LanguageEntry *current_language;
    u8 *translated = get_key_string(str);
    if (translated == current_language->none) {
        translated = getTranslatedText((char *) str);
        memcpy(str64, translated, omm_text_length(translated) + 1);
        free(translated);
    } else {
        memcpy(str64, translated, omm_text_length(translated) + 1);
    }
#else
    s32 i = 0;
    for (; *str != 0;) {
        str = omm_text_add_char(str64, str, &i);
    }
    str64[i] = 0xFF;
#endif
    return str64;
}

u8 *omm_text_copy(const u8 *str64, bool heapAlloc) {
    u8 *strCopy = omm_memory_new(heapAlloc ? NULL : gOmmMemoryPoolStrings, 0x800, NULL);
    OMM_MEMCPY(strCopy, str64, omm_text_length(str64) + 1);
    return strCopy;
}

u8 *omm_text_capitalize(u8 *str64) {
    for (u8 *p = str64; *p != 0xFF; ++p) {
        if (*p >= 0x24 && *p <= 0x3D) {
            *p -= 26;
        }
    }
    return str64;
}

u8 *omm_text_decapitalize(u8 *str64) {
    bool wasSpace = true;
    for (u8 *p = str64; *p != 0xFF; ++p) {
        if (*p >= 0x0A && *p <= 0x23) {
            if (wasSpace) wasSpace = false;
            else *p += 26;
        } else if (*p >= 0x3F) {
            wasSpace = true;
        }
    }
    return str64;
}

u8 *omm_text_replace_char(u8 *str64, u8 from, u8 to) {
    for (u8 *p = str64; *p != 0xFF; ++p) {
        if (*p == from) {
            *p = to;
        }
    }
    return str64;
}

static void omm_text_replace(u8 *str64, const char *from, const char *to) {
    const u8 *from64 = omm_text_convert(from, false);
    const u8 *to64 = omm_text_convert(to, false);
    s32 length = omm_text_length(from64);
    for (; *str64 != 0xFF;) {
        if (OMM_MEMCMP(str64, from64, length)) {
            OMM_MEMCPY(str64, to64, length);
            str64 += length;
        } else {
            str64++;
        }
    }
}

static void omm_text_replace_mario_by_peach(u8 *str64) {
    omm_text_replace(str64, OMM_TEXT_MARIO, OMM_TEXT_PEACH);
    omm_text_replace(str64, OMM_TEXT_MARIO_UPPER, OMM_TEXT_PEACH_UPPER);
    omm_text_replace(str64, OMM_TEXT_MARIO_LOWER, OMM_TEXT_PEACH_LOWER);
    omm_text_replace(str64, OMM_TEXT_CAPPY, OMM_TEXT_TIARA);
    omm_text_replace(str64, OMM_TEXT_CAPPY_UPPER, OMM_TEXT_TIARA_UPPER);
    omm_text_replace(str64, OMM_TEXT_CAPPY_LOWER, OMM_TEXT_TIARA_LOWER);
}

static void omm_text_replace_mario_by_luigi(u8 *str64) {
    omm_text_replace(str64, OMM_TEXT_MARIO, OMM_TEXT_LUIGI);
    omm_text_replace(str64, OMM_TEXT_MARIO_UPPER, OMM_TEXT_LUIGI_UPPER);
    omm_text_replace(str64, OMM_TEXT_MARIO_LOWER, OMM_TEXT_LUIGI_LOWER);
}

static void omm_text_replace_mario_by_wario(u8 *str64) {
    omm_text_replace(str64, OMM_TEXT_MARIO, OMM_TEXT_WARIO);
    omm_text_replace(str64, OMM_TEXT_MARIO_UPPER, OMM_TEXT_WARIO_UPPER);
    omm_text_replace(str64, OMM_TEXT_MARIO_LOWER, OMM_TEXT_WARIO_LOWER);
}

u8 *omm_text_replace_names(u8 *str64, bool inplace) {
    if (!inplace) {
        u8 *temp = (u8 *) omm_memory_new(gOmmMemoryPoolStrings, omm_text_length(str64) + 1, NULL);
        OMM_MEMCPY(temp, str64, omm_text_length(str64) + 1);
        str64 = temp;
    }
    switch (omm_player_get_selected_index()) {
        case OMM_PLAYER_PEACH: omm_text_replace_mario_by_peach(str64); break;
        case OMM_PLAYER_LUIGI: omm_text_replace_mario_by_luigi(str64); break;
        case OMM_PLAYER_WARIO: omm_text_replace_mario_by_wario(str64); break;
    }
    return str64;
}

s32 omm_text_length(const u8 *str64) {
    s32 length = 0;
    for (; str64 && *str64 != 255; str64++, length++);
    return length;
}

//
// Dialogs
//

#include "omm_text_dialogs.inl"
_Static_assert(OMM_DIALOG_END_INDEX <= 256, "Last dialog ID is over 255");
_Static_assert(OMM_ARRAY_SIZE(sOmmDialogEntriesRaw) == OMM_DIALOG_COUNT, "Missing dialogs");

struct DialogEntry *omm_get_dialog_entry(void **dialogTable, s16 dialogId) {
#if !defined(R96A)
    static struct DialogEntry *sOmmDialogEntries = NULL;
    if (!sOmmDialogEntries) {
        sOmmDialogEntries = (struct DialogEntry *) OMM_MEMNEW(struct DialogEntry, OMM_DIALOG_COUNT);
        for (s32 i = 0; i != OMM_DIALOG_COUNT; ++i) {
            struct OmmDialogEntry *raw = &sOmmDialogEntriesRaw[i];
            sOmmDialogEntries[raw->id - OMM_DIALOG_START_INDEX].unused      = raw->soundBits;
            sOmmDialogEntries[raw->id - OMM_DIALOG_START_INDEX].linesPerBox = raw->linesPerBox;
            sOmmDialogEntries[raw->id - OMM_DIALOG_START_INDEX].leftOffset  = raw->leftOffset;
            sOmmDialogEntries[raw->id - OMM_DIALOG_START_INDEX].width       = raw->downOffset;
            sOmmDialogEntries[raw->id - OMM_DIALOG_START_INDEX].str         = omm_text_convert(raw->str, true);
        }
    }
    struct DialogEntry *dialog = NULL;
    if (dialogId >= OMM_DIALOG_START_INDEX && dialogId < OMM_DIALOG_END_INDEX) {
        dialog = &sOmmDialogEntries[dialogId - OMM_DIALOG_START_INDEX];
    } else {
        dialog = (struct DialogEntry *) dialogTable[dialogId];
    }
#else
    OMM_UNUSED(dialogTable);
    struct DialogEntry *dialog = dialogPool[dialogId];
#endif
    if (dialog->unused > 1 && gDialogAngle == 90.f) {
        play_sound(dialog->unused, gGlobalSoundArgs);
    }
    return dialog;
}

static s16 omm_get_bowser_dialog(bool isIntro, s16 defaultDialog) {
    struct Object *o = obj_get_first_with_behavior(omm_bhv_bowser);
    if (o && o->oBehParams) {
        switch (gCurrLevelNum) {
            case LEVEL_BOWSER_1: return (isIntro ? OMM_DIALOG_BOWSER_1_INTRO : defaultDialog);
            case LEVEL_BOWSER_2: return (isIntro ? OMM_DIALOG_BOWSER_2_INTRO : defaultDialog);
            case LEVEL_BOWSER_3: return (isIntro ? OMM_DIALOG_BOWSER_3_INTRO : OMM_DIALOG_BOWSER_3_DEFEAT);
            case LEVEL_GROUNDS:  return (isIntro ? OMM_SPARKLY_BOWSER_4_DIALOG_INTRO[omm_sparkly_get_current_mode()] : OMM_SPARKLY_BOWSER_4_DIALOG_DEFEAT[omm_sparkly_get_current_mode()]);
        }
    }
    return defaultDialog;
}

OMM_ROUTINE_GFX(omm_update_dialogs) {
    
    // Dialog box
    switch (gDialogID) {

        // OMM Bowser dialogs
        case DIALOG_067: gDialogID = omm_get_bowser_dialog(true, DIALOG_067); break;
        case DIALOG_092: gDialogID = omm_get_bowser_dialog(true, DIALOG_092); break;
        case DIALOG_093: gDialogID = omm_get_bowser_dialog(true, DIALOG_093); break;
        case DIALOG_121: gDialogID = omm_get_bowser_dialog(false, DIALOG_121); break;
        case DIALOG_163: gDialogID = omm_get_bowser_dialog(false, DIALOG_163); break;
    }

    // Ending cutscene dialog
    if (gCutsceneMsgIndex != -1) {

        // Replace 'Mario' and 'Cappy' by the selected character name and cap name
        omm_text_replace_names(gEndCutsceneStringsEn[gCutsceneMsgIndex], true);
    }
}

#if defined(R96A)

//
// For Render96: Auto-generates AM_us.omm.json at execution time
//

OMM_AT_STARTUP static void omm_r96a_generate_json() {
    OMM_STRING(filename, 256, "%s/res/texts/AM_us.omm.json", OMM_EXE_FOLDER);
    FILE *f = fopen(filename, "w");
    if (f) {

        // Header
        fprintf(f, "{\n");
        fprintf(f, "  \"manifest\": {\n");
        fprintf(f, "    \"langName\": \"English\",\n");
        fprintf(f, "    \"langLogo\": \"none\"\n");
        fprintf(f, "  },\n");

        // Dialogs
        fprintf(f, "  \"dialogs\": [\n");
        for (s32 i = 0; i != OMM_ARRAY_SIZE(sOmmDialogEntriesRaw); ++i) {
            fprintf(f, "    {\n");
            fprintf(f, "      \"ID\": %d,\n", sOmmDialogEntriesRaw[i].id);
            if (sOmmDialogEntriesRaw[i].soundBits != 0) {
                fprintf(f, "      \"sound\": \"%d, %d, 0x%02X, 0x%02X, %d\",\n",
                    (s32) ((sOmmDialogEntriesRaw[i].soundBits >> 28) & 0x0F),
                    (s32) ((sOmmDialogEntriesRaw[i].soundBits >> 24) & 0x0F),
                     (u8) ((sOmmDialogEntriesRaw[i].soundBits >> 16) & 0xFF),
                     (u8) ((sOmmDialogEntriesRaw[i].soundBits >>  8) & 0xFF),
                     (u8) ((sOmmDialogEntriesRaw[i].soundBits >>  4) & 0x0F)
                );
            }
            fprintf(f, "      \"linesPerBox\": %d,\n", sOmmDialogEntriesRaw[i].linesPerBox);
            fprintf(f, "      \"leftOffset\": %d,\n", sOmmDialogEntriesRaw[i].leftOffset);
            fprintf(f, "      \"width\": %d,\n", sOmmDialogEntriesRaw[i].downOffset);
            fprintf(f, "      \"lines\": [\n");
            fprintf(f, "        \"");
            for (const char *c = sOmmDialogEntriesRaw[i].str;; ++c) {
                if (*c == 0) {
                    fprintf(f, "\"\n");
                    fprintf(f, "      ]\n");
                    break;
                } else if (*c == '\n') {
                    fprintf(f, "\",\n");
                    fprintf(f, "        \"");
                } else {
                    fprintf(f, "%c", *c);
                }
            }
            if (i < (s32) OMM_ARRAY_SIZE(sOmmDialogEntriesRaw) - 1) {
                fprintf(f, "    },\n");
            } else {
                fprintf(f, "    }\n");
            }
        }
        fprintf(f, "  ],\n");
        
        // Strings
        fprintf(f, "  \"strings\": {\n");
        #undef OMM_TEXT_
        #define OMM_TEXT_(id, str) { "OMM_TEXT_" #id, str },
        static const char *sOmmStrings[][2] = {
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
        };
        for (s32 i = 0; i != OMM_ARRAY_SIZE(sOmmStrings); ++i) {
            fprintf(f, "    \"%s\": \"%s\"", sOmmStrings[i][0], sOmmStrings[i][1]);
            if (i < (s32) OMM_ARRAY_SIZE(sOmmStrings) - 1) {
                fprintf(f, ",\n");
            } else {
                fprintf(f, "\n");
            }
        }
        fprintf(f, "  }\n");

        // Footer
        fprintf(f, "}\n");
        fclose(f);
    }
}

#endif