# -----------------------
# Odyssey Mario's Moveset
# -----------------------

# ------
# Source
# ------

SRC_DIRS += data/omm data/omm/system data/omm/object data/omm/mario data/omm/cappy data/omm/capture data/omm/peachy

# -------
# Version
# -------

OMM_VERSION_NUMBER := 6.0.4
OMM_DEVELOPER := PeachyPeach
VERSION_CFLAGS += -DOMM_VERSION="$(OMM_VERSION_NUMBER)"
VERSION_CFLAGS += -DOMM_DEVELOPER="$(OMM_DEVELOPER)"
DEFINES += OMM_VERSION="$(OMM_VERSION_NUMBER)"
DEFINES += OMM_DEVELOPER="$(OMM_DEVELOPER)"
ifeq ($(OMM_BUILDER),1)
CC := $(CROSS)gcc -w
CXX := $(CROSS)g++ -w
else
OMM_PATCH := $(shell python3 omm_patcher.py -p)
endif

# Super Mario 64 Moonshine
ifeq      ($(or $(and $(wildcard actors/Bee/geo.inc.c),1),0),1)
	VERSION_CFLAGS += -DSMMS -DEXTERNAL_DATA
	DEFINES += SMMS=1 EXTERNAL_DATA=1
	OMM_VERSION_SUB := Super Mario 64 Moonshine

# Super Mario Star Road
else ifeq ($(or $(and $(wildcard levels/zerolife/script.c),1),0),1)
	VERSION_CFLAGS += -DSMSR
	DEFINES += SMSR=1
	OMM_VERSION_SUB := Super Mario Star Road
	
# Super Mario 74
else ifeq ($(or $(and $(wildcard text/us/coursesEE.h),1),0),1)
	VERSION_CFLAGS += -DSM74
	DEFINES += SM74=1
	OMM_VERSION_SUB := Super Mario 74

# Render96-alpha
else ifeq ($(or $(and $(wildcard data/r96/r96_defines.h),1),0),1)
	VERSION_CFLAGS += -DR96A
	DEFINES += R96A=1
	OMM_VERSION_SUB := Render96-alpha

# sm64ex-alo (Refresh 14)
else ifeq ($(or $(and $(wildcard src/extras/bettercamera.h),1),0),1)
	VERSION_CFLAGS += -DXALO
	DEFINES += XALO=1
	OMM_VERSION_SUB := Super Mario 64 ex-alo

# sm64ex-nightly
else 
	VERSION_CFLAGS += -DSMEX
	DEFINES += SMEX=1
	OMM_VERSION_SUB := Super Mario 64 ex-nightly
endif

# --------------
# Initialization
# --------------

OMM_DIR_DATA := ./omm
OMM_DIR_OUT := $(BUILD_DIR)/omm
OMM_INIT := mkdir -p $(OMM_DIR_DATA) ; mkdir -p $(OMM_DIR_OUT) ; cp -rf $(OMM_DIR_DATA) $(BUILD_DIR) 2>/dev/null || true ; echo 'ok'
ifneq ($(shell $(call OMM_INIT)),ok)
$(error Could not initialize OMM before building. Aborting make..)
endif

# --------
# Built-in
# --------

VERSION_CFLAGS += -DTEXTURE_FIX
VERSION_CFLAGS += -DEXT_OPTIONS_MENU
VERSION_CFLAGS += -DBETTERCAMERA
VERSION_CFLAGS += -DNODRAWINGDISTANCE
DEFINES += TEXTURE_FIX=1
DEFINES += EXT_OPTIONS_MENU=1
DEFINES += BETTERCAMERA=1
DEFINES += NODRAWINGDISTANCE=1
DEFINES += NO_SEGMENTED_MEMORY=1

# ------
# Macros
# ------

OMM_VANILLA ?= -1
ifneq ($(OMM_VANILLA),-1)
VERSION_CFLAGS += -DOMM_VANILLA=$(OMM_VANILLA)
DEFINES += OMM_VANILLA=$(OMM_VANILLA)
endif

OMM_BOWSER ?= -1
ifneq ($(OMM_BOWSER),-1)
VERSION_CFLAGS += -DOMM_BOWSER=$(OMM_BOWSER)
DEFINES += OMM_BOWSER=$(OMM_BOWSER)
endif

OMM_SPARKLY ?= -1
ifneq ($(OMM_SPARKLY),-1)
VERSION_CFLAGS += -DOMM_SPARKLY=$(OMM_SPARKLY)
DEFINES += OMM_SPARKLY=$(OMM_SPARKLY)
endif

OMM_DEBUG ?= -1
ifneq ($(OMM_DEBUG),-1)
VERSION_CFLAGS += -DOMM_DEBUG=$(OMM_DEBUG)
DEFINES += OMM_DEBUG=$(OMM_DEBUG)
endif

# -------
# Patches
# -------

ifeq ($(or $(and $(wildcard data/dynos.h),1),0),1)
VERSION_CFLAGS += -DDYNOS
DEFINES += DYNOS=1
OMM_PATCHES += [DynOS]
endif

ifeq ($(or $(and $(wildcard src/game/mario_cheats.h),1),0),1)
VERSION_CFLAGS += -DCHEATER
DEFINES += CHEATER=1
OMM_PATCHES += [Cheater]
endif

ifeq ($(or $(and $(wildcard src/game/time_trials.h),1),0),1)
VERSION_CFLAGS += -DTIME_TRIALS
DEFINES += TIME_TRIALS=1
OMM_PATCHES += [Time Trials]
endif

ifeq ($(or $(and $(wildcard src/engine/extended_bounds.h),1),0),1)
VERSION_CFLAGS += -DEXT_BOUNDS
DEFINES += EXT_BOUNDS=1
OMM_PATCHES += [Extended Bounds]
endif

OMM_PATCHES := $(or $(OMM_PATCHES),None)

# ---------
# OMM rules
# ---------

all: OMM_VERSION OMM_EXE_MAP

OMM_VERSION:
	@printf -- \
	"--------------------------------------------------------------------------------------------------------------------------------\n\
	Odyssey Mario's Moveset v$(OMM_VERSION_NUMBER) by $(OMM_DEVELOPER) - $(OMM_VERSION_SUB) detected\n\
	Patches applied: $(OMM_PATCHES)\n\
	--------------------------------------------------------------------------------------------------------------------------------\n"

OMM_EXE_MAP: $(EXE)
	objdump -t $(EXE) > $(OMM_DIR_OUT)/omm.map
