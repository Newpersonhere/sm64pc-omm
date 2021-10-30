#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS
#if OMM_CODE_VANILLA

//
// LiveSplit auto-splitter support
// You need an omm.*.asl file to make it work
//
// The way it works is fairly simple
// A string is written into the executable, as well as 4 flag slots
// The volatile keyword is here to prevent the compiler from "optimizing" the array
// The script will attempt to find this string to locate the flag slots
// The flags are set to 1 by the game, read and processed by the AS script, then set back to 0
// The first flag is set to 1 if the player leaves the main menu, i.e. when a save file is selected
// The second flag is set to 1 if the player collects a split star (see below), or touches the Grand Star
// The third flag is set to 1 if the player returns to the main menu
// The fourth flag is currently unused
//
// Split stars: when the game starts, the file "omm/omm_splits.txt" is read
// The file must contain a list of the star counts when the script needs to split
// For instance, if the file contains the values "1 5 9 15 34", the script will trigger a split
// as soon as Mario collects the 1st star, the 5th star, the 9th star and so on
// If the file is missing or badly written, only the Grand Star split will be triggered
//

static volatile u8 sOmmSplitFlags[16] = { 'O', 'M', 'M', 'A', 'U', 'T', 'O', 'S', 'P', 'L', 'I', 'T', 0, 0, 0, 0 };
static OmmArray sOmmSplitStars = NULL;

static void omm_speedrun_update() {
    static s32 pMainMenu = 0;
    static s32 pNumStars = 0;
    static s32 pMarioAct = 0;
    static s32 tMainMenu = 0;

    // Current
    s32 cMainMenu = omm_is_main_menu();
    s32 cNumStars = gMarioState->numStars;
    s32 cMarioAct = gMarioState->action;

    // Start
    if (cMainMenu != pMainMenu && cMainMenu == 0) {
        sOmmSplitFlags[0xC] = 1;
        tMainMenu = 150;
    }

    // Split
    if ((cNumStars != pNumStars && omm_array_find(sOmmSplitStars, cNumStars) != -1) ||
        (cMarioAct != pMarioAct && cMarioAct == ACT_JUMBO_STAR_CUTSCENE)) {
        sOmmSplitFlags[0xD] = 1;
    }

    // Reset
    if (tMainMenu-- <= 0 && cMainMenu != pMainMenu && cMainMenu == 1) {
        sOmmSplitFlags[0xE] = 1;
    }

    // Update
    pMainMenu = cMainMenu;
    pNumStars = cNumStars;
    pMarioAct = cMarioAct;
}

OMM_AT_STARTUP static void omm_speedrun_init() {
    OMM_STRING(filename, 256, "%s/%s/%s", OMM_EXE_FOLDER, OMM_RES_FOLDER, "omm_splits.txt");
    FILE *f = fopen(filename, "r");
    if (f) {
        sOmmSplitStars = omm_array_new(s32);
        char buffer[1024];
        while (fgets(buffer, 1024, f)) {
            for (char *p = buffer; *p != 0;) {
                while (*p != 0 && (*p < '0' || *p > '9')) ++p;
                s32 i; if (sscanf(p, "%d", &i)) { omm_array_add(sOmmSplitStars, i); }
                while (*p >= '0' && *p <= '9') ++p;
            }
        }
        printf("omm_splits.txt file successfully loaded.\n");
        printf("Loaded splits (stars):");
        for (s32 i = 0; i != omm_array_count(sOmmSplitStars); ++i) {
            printf(" %d", omm_array_get(sOmmSplitStars, s32, i));
        }
        printf("\n");
        fflush(stdout);
        fclose(f);
    }
    sOmmSplitFlags[0xC] = 0;
    sOmmSplitFlags[0xD] = 0;
    sOmmSplitFlags[0xE] = 0;
    sOmmSplitFlags[0xF] = 0;
    omm_add_routine(OMM_ROUTINE_TYPE_UPDATE, omm_speedrun_update);
}

#endif
