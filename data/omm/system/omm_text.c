#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

//
// String conversion (very basic)
//

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

u8 *omm_text_convert(const char *str, bool heapAlloc) {
    u8 *str64 = omm_memory_new(heapAlloc ? NULL : gOmmMemoryPoolStrings, 0x800, NULL);
    s32 i = 0;
    for (; *str != 0;) {
        str = omm_text_add_char(str64, str, &i);
    }
    str64[i] = 0xFF;
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

static void omm_text_replace_mario_by_wario(u8 *str64) {
    omm_text_replace(str64, OMM_TEXT_MARIO, OMM_TEXT_WARIO);
    omm_text_replace(str64, OMM_TEXT_MARIO_UPPER, OMM_TEXT_WARIO_UPPER);
    omm_text_replace(str64, OMM_TEXT_MARIO_LOWER, OMM_TEXT_WARIO_LOWER);
}

static void omm_text_replace_mario_by_luigi(u8 *str64) {
    omm_text_replace(str64, OMM_TEXT_MARIO, OMM_TEXT_LUIGI);
    omm_text_replace(str64, OMM_TEXT_MARIO_UPPER, OMM_TEXT_LUIGI_UPPER);
    omm_text_replace(str64, OMM_TEXT_MARIO_LOWER, OMM_TEXT_LUIGI_LOWER);
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

static struct DialogEntry *omm_dialog_get_entry(s16 dialogId) {
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
    return &sOmmDialogEntries[dialogId];
}

struct DialogEntry *omm_get_dialog_entry(void **dialogTable, s16 dialogId) {
    if (dialogId >= OMM_DIALOG_START_INDEX && dialogId < OMM_DIALOG_END_INDEX) {
        struct DialogEntry *dialog = omm_dialog_get_entry(dialogId - OMM_DIALOG_START_INDEX);
        if (gDialogBoxOpenTimer == 90.f && dialog->unused > 1) {
            play_sound(dialog->unused, gGlobalSoundArgs);
        }
        return dialog;
    }
    return (struct DialogEntry *) dialogTable[dialogId];
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
        case DIALOG_163: gDialogID = omm_get_bowser_dialog(false, DIALOG_121); break;
    }

    // Ending cutscene dialog
    if (gCutsceneMsgIndex != -1) {

        // Sparkly Stars bad ending (Normal only)
        // Toads are waiting for Mario, but Peach isn't here...
        if (gCutsceneMsgIndex > 8) {
            switch (gCutsceneMsgIndex) {
                case 9:  gEndCutsceneStringsEn[8] = omm_text_convert(OMM_TEXT_BAD_ENDING_TOAD_0, false); break;
                case 10: gEndCutsceneStringsEn[8] = omm_text_convert(OMM_TEXT_BAD_ENDING_TOAD_1, false); break;
                case 11: gEndCutsceneStringsEn[8] = omm_text_convert(OMM_TEXT_BAD_ENDING_TOAD_2, false); break;
                case 12: gEndCutsceneStringsEn[8] = omm_text_convert(OMM_TEXT_BAD_ENDING_TOAD_3, false); break;
                case 13: gEndCutsceneStringsEn[8] = omm_text_convert(OMM_TEXT_BAD_ENDING_TOAD_4, false); break;
                case 14: gEndCutsceneStringsEn[8] = omm_text_convert(OMM_TEXT_BAD_ENDING_TOAD_5, false); break;
                case 15: gEndCutsceneStringsEn[8] = omm_text_convert(OMM_TEXT_BAD_ENDING_TOAD_6, false); break;
                case 16: gEndCutsceneStringsEn[8] = omm_text_convert(OMM_TEXT_BAD_ENDING_TOAD_7, false); break;
                case 17: gEndCutsceneStringsEn[8] = omm_text_convert(OMM_TEXT_BAD_ENDING_TOAD_8, false); break;
                default: gEndCutsceneStringsEn[8] = omm_text_convert(OMM_TEXT_LEVEL_EMPTY, false); break;
            }
            gCutsceneMsgIndex = 8;
        }

        // Replace every instance of 'Mario' and 'Cappy'
        // by the selected character name and cap name
        omm_text_replace_names(gEndCutsceneStringsEn[gCutsceneMsgIndex], true);
    }
}
