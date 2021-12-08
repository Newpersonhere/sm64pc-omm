#ifndef EXTENDED_BOUNDS_H
#define EXTENDED_BOUNDS_H

/*
    Better Extended Bounds + Collsion Patches
        Thanks to anonymous_moose for the original Better Extended Bounds patch
        Thanks to someone2639 for the shiftable segments patch
        Thanks to Wiseguy for the Surface Pool Full error code and 4x bounds fix
        Thanks to FramePerfection/chaosBrick for the wall collision rewrite
        Thanks to Arthurtilly for the platform displacement rewrite
        Thanks to falcobuster for the long render distance fix
        Thanks to JoshDuman, Kaze, Arceveti and various people for the QoL collision code changes
        Patch compilated by AloXado320

    0: Regular bounds
        Same as vanilla sm64, boundaries are (-8192 to 8191)
        16x16 collision cells.
    1: Regular bounds (performance)
        Same boundaries as vanilla (-8192 to 8191), but with twice the amount of collision cells
        Trades more RAM usage for faster collision calculations.
    2: 2x extended bounds
        level boundaries are twice as big (-16384 to 16383)
        Collision calculations remain as fast as vanilla, at the cost of using more RAM.
        32x32 collision cells.
    3: 4x extended bounds
        level boundaries are 4 times as big (-32768 to 32767)
        Collision calculations remain as fast as vanilla, at the cost of using far more RAM (16 times vanilla).
        64x64 collision cells.
    4: 4x extended bounds (Memory saver)
        level boundaries are 4 times as big (-32768 to 32767)
        Collision cells are the same size as vanilla, at the cost of being much slower.
        32x32 collision cells.

    If you see "SURFACE POOL FULL" or "SURFACE NODE POOL FULL" in game, you should increase
    SURFACE_POOL_SIZE or SURFACE_NODE_POOL_SIZE, respectively, or reduce the amount of
    collision surfaces in your level.
*/

//set this to the extended bounds mode you want, then do "make clean".
#define EXTENDED_BOUNDS_MODE 2

// Coordinate overflow fix by falcobuster (World Scale).
// Your levels will render correctly on console and LLE emulators
// even when using 2x or 4x bounds, while not hurting anything on HLE plugins.

// SURFACE_POOL_SIZE and SURFACE_NODE_POOL_SIZE only matter on N64
// On non-N64 targets, surface sizes are allocated using SYSTEM_MALLOC
// If SYSTEM_MALLOC is not defined on PC Port, then fixed pools are used instead

// SURFACE_POOL_SIZE
// The maximum amount of collision surfaces (static and dynamic combined)
// The vanilla value is 2300

// SURFACE_NODE_POOL_SIZE
// Make this approximately (amount of collision cells) + (SURFACE_POOL_SIZE * 3)
// The vanilla value is 7000

// Cell and height limits
// Default cell/floor limit is 20000/-11000 respectively
#define CELL_HEIGHT_LIMIT           20000
#define FLOOR_LOWER_LIMIT           -11000
#define FLOOR_LOWER_LIMIT_MISC      (FLOOR_LOWER_LIMIT + 1000)
#define FLOOR_LOWER_LIMIT_SHADOW    FLOOR_LOWER_LIMIT_MISC

/* Miscellaneous Tweaks */

// Show ingame text info if any surface pool is full (N64 and Non-sys malloc only)
#ifndef SYSTEM_MALLOC
#define SURFACE_POOLS_FULL_MESSAGES

#ifdef SURFACE_POOLS_FULL_MESSAGES
// Sanity checks if pool gets past the size
#define NOT_ENOUGH_ROOM_FOR_SURFACES (1 << 0)
#define NOT_ENOUGH_ROOM_FOR_NODES    (1 << 1)
#endif

#endif

// Set vanilla camera course specific functions
#define CAMERA_SPECIFIC_FUNCTION_DEFINES

// Platform displacement 2 also known as momentum patch.
// Makes Mario keep the momemtum from moving platforms.
// Doesn't break treadmills anymore!
// (Arthurtilly, HackerSM64)
#define PLATFORM_DISPLACEMENT_2

// Water surfaces (Thecozies)
#define NEW_WATER_SURFACES

// Improved wall collision detection, with rounded corners
// Fixes Mario jittering when walking into multiple walls at once
// (frameperfection/Frame#5375)
#define BETTER_WALL_COLLISION

// Ceiling margin from BWC
// (frameperfection/Frame#5375)
#define CEILING_MARGINS

// For static floor checks only
#define FIND_FLOOR_EXCLUDE_DYNAMIC_CHECK

// Push Mario out of sloped ceilings instead of stopping or bonking
// helps prevent unwanted bonks and softlocks
// (Arceveti)
#define BETTER_CEILING_HANDLING

// Properly fixes false ledge grabs
// Does nothing if BETTER_WALL_COLLISION is enabled.
// (Arceveti)
#define FIX_LEDGE_GRABS

// Experimental: Automatic World Scale
//#define AUTOMATIC_WORLD_SCALE

// Check for floors/ceilings from halfway up Mario's hitbox rather than from the bottom
// (Arceveti)
#define CENTERED_COLLISION

#ifdef CENTERED_COLLISION
// Step height for objects. Comment out the define to use vanilla behavior. (Arceveti)
#define OBJ_STEP_HEIGHT 40.0f
#endif

// Maximum number of walls to check (vanilla is 4, higher is recommended)
#define MAX_REFEREMCED_WALLS 16

// Include steep floors when checking for walls when underwater
// This fixes the weirdness caused by swimming into such slopes (as seen with the JRB pillars in vanilla)
// May cause performance issues when underwater due to also iterating through the floors partition
// in addition to the regular walls partition when checking wall collisions.
// (Arceveti)
#define UNDERWATER_STEEP_FLOORS_AS_WALLS
#define MIN_UNDERWATER_FLOOR_NORMAL_Y 0.1f

// Minimum Y normal for floors (vanilla is 0.01f)
#define MIN_FLOOR_NORMAL_Y 0.02f

// Minimum Y normal for ceilings (vanilla is -0.01f, -0.2f is recommended)
#define MAX_CEIL_NORMAL_Y -0.2f

// Number of null floors to check ahead through during a qstep.
// Fixes many instances of invisible walls but may cause minor
// issues on actual level boundaries, such as extra knockback
// (Arceveti)
#define NULL_FLOOR_STEPS 4

// Number of swimming steps per frame (Vanilla is 1)
#define WATER_NUM_STEPS  4

// Number of ground steps per frame (Vanilla is 4)
#define GROUND_NUM_STEPS 4

// Number of air steps per frame (Vanilla is 4)
#define AIR_NUM_STEPS    4

/* Don't touch the stuff past this point unless you know what you're doing! */

// Default value to check if the user set a proper extended bounds mode
#define LEVEL_BOUNDARY_MAX 0x0000

// Scales the world down by this factor, increasing how far you can render on
// console (and PC) in exchange for a slight loss in precision.
#define WORLD_SCALE 1.0f

#if EXTENDED_BOUNDS_MODE == 0 // Vanilla
    #undef LEVEL_BOUNDARY_MAX // Undefine the old value to avoid compiler warnings
    #undef WORLD_SCALE
    #define LEVEL_BOUNDARY_MAX      0x2000L
    #define CELL_SIZE               0x400
    #define SURFACE_POOL_SIZE       4000
    #define SURFACE_NODE_POOL_SIZE  12000
    #define WORLD_SCALE             1.0f
    #define 
#elif EXTENDED_BOUNDS_MODE == 1 // Vanilla size with performance but more RAM usage
    #undef LEVEL_BOUNDARY_MAX
    #undef WORLD_SCALE
    #define LEVEL_BOUNDARY_MAX      0x2000L
    #define CELL_SIZE               0x200
    #define SURFACE_POOL_SIZE       4000
    #define SURFACE_NODE_POOL_SIZE  16000
    #define WORLD_SCALE             1.0f
#elif EXTENDED_BOUNDS_MODE == 2 // 2x bounds
    #undef LEVEL_BOUNDARY_MAX
    #undef WORLD_SCALE
    #define LEVEL_BOUNDARY_MAX      0x4000L
    #define CELL_SIZE               0x400
    #define SURFACE_POOL_SIZE       8200
    #define SURFACE_NODE_POOL_SIZE  22000
    #define WORLD_SCALE             2.0f
#elif EXTENDED_BOUNDS_MODE == 3 // 4x bounds
    #undef LEVEL_BOUNDARY_MAX
    #undef WORLD_SCALE
    #define LEVEL_BOUNDARY_MAX      0x8000L
    #define CELL_SIZE               0x400
    #define SURFACE_POOL_SIZE       8200
    #define SURFACE_NODE_POOL_SIZE  24600
    #define WORLD_SCALE             4.0f
#elif EXTENDED_BOUNDS_MODE == 4 // 4x bounds with big cells (similar to old ext bounds)
    #undef LEVEL_BOUNDARY_MAX
    #undef WORLD_SCALE
    #define LEVEL_BOUNDARY_MAX      0x8000L
    #define CELL_SIZE               0x800
    #define SURFACE_POOL_SIZE       8200
    #define SURFACE_NODE_POOL_SIZE  20600
    #define WORLD_SCALE             4.0f
#endif

#define NUM_CELLS (2 * LEVEL_BOUNDARY_MAX / CELL_SIZE)
#define NUM_CELLS_INDEX (NUM_CELLS - 1)

#endif // EXTENDED_BOUNDS_H
