#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

static bool sOmmPeachIsSelected = false;

bool omm_peach_is_unlocked() {
    return omm_sparkly_is_mode_completed(OMM_SPARKLY_MODE_NORMAL);
}

bool omm_peach_is_selected() {
    return omm_peach_is_unlocked() && sOmmPeachIsSelected;
}

bool omm_peach_select(bool select) {
    if (omm_peach_is_unlocked()) {
        sOmmPeachIsSelected = select;
        gSaveFileModified = true;
        return sOmmPeachIsSelected;
    }
    return false;
}
