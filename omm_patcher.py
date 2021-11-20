import os
import sys


def patch_string_in_file(filepath, location, where, what, how, unpatch):
    try:
        file = open(filepath, "r", newline="\n", encoding="utf-8", errors="ignore")
        data = file.read()
        file.close()
        
        i_location = data.find(location)
        if i_location == -1:
            print("[x] ERROR: File \"" + filepath + "\": Cannot find string \"" + location + "\".")
            return 0
            
        i_what = data.find(what, i_location - len(what) if how == -1 else 0)
        if unpatch:
            if i_what == -1:
                print("(!) Warning: File \"" + filepath + "\": " + location + " ... " + where + " has already been unpatched.")
                return 1
        else:
            if i_what != -1:
                print("(!) Warning: File \"" + filepath + "\": " + location + " ... " + where + " has already been patched.")
                return 1
            
        i_where = data.find(where, i_location)
        if i_where == -1 and (not unpatch or how != 0):
            print("[x] ERROR: File \"" + filepath + "\": Cannot find string \"" + where + "\".")
            return 0
            
        if how == -1:
            if unpatch:
                new_data = data[:i_what] + data[i_where:]
            else:
                new_data = data[:i_where] + what + data[i_where:]
        elif how == +1:
            if unpatch:
                new_data = data[:i_where + len(where)] + data[i_what + len(what):]
            else:
                new_data = data[:i_where + len(where)] + what + data[i_where + len(where):]
        elif how == 0:
            if unpatch:
                new_data = data[:i_what] + where + data[i_what + len(what):]
            else:
                new_data = data[:i_where] + what + data[i_where + len(where):]
        else:
            print("[x] ERROR: File \"" + filepath + "\": Argument 'how' must be either 0, -1 or +1.")
            return 0
            
        try:
            new_file = open(filepath, "w", newline="\n", encoding="utf-8", errors="ignore")
            new_file.write(new_data)
            new_file.close()
            if unpatch:
                print("File \"" + filepath + "\": " + location + " ... " + where + " has successfully been unpatched.")
            else:
                print("File \"" + filepath + "\": " + location + " ... " + where + " has successfully been patched.")
            return 1
            
        except IOError:
            print("[x] ERROR: File \"" + filepath + "\" cannot be modified.")
            return 0
            
    except IOError:
        print("[x] ERROR: File \"" + filepath + "\" does not exist.")
        return 0
        
        
def delete_file(filepath, unpatch):
    if not unpatch:
        try:
            os.remove(filepath)
            print("File \"" + filepath + "\" has successfully been deleted.")
            return 1
        except OSError:
            print("[x] ERROR: File \"" + filepath + "\" cannot be deleted.")
            return 0
    return 1
            
            
def scan_directories(filename, function, *args):
    unpatch = None
    if len(sys.argv) < 2 or sys.argv[1] not in ["-p", "-u"]:
        print("You must provide a valid command as first argument.")
        print("List of available commands:")
        print("-p : Apply the patch")
        print("-u : Remove the patch")
        sys.exit(0)
        return 0
        
    if sys.argv[1] == "-p":
        unpatch = False
    elif sys.argv[1] == "-u":
        unpatch = True
        
    filename.replace('/', os.sep)
    for root, _, files in os.walk("."):
        for file in files:
            for ignore in [".rej", ".porig"]:
                if not ignore in file:
                    filepath = root + os.sep + file
                    if (os.sep + filename) in filepath:
                        return function(filepath, *args, unpatch)
                
                
def do_patch_file(filename, location, where, what, how):
    return scan_directories(filename, patch_string_in_file, location, where, what, how)
    
    
def do_undef_code(filename, start, end, index=0):
    if (do_patch_file(filename, start, start, "#if 0 // {} [{}]\n".format(filename, index), -1) != 0):
        do_patch_file(filename, end, end, "#endif // {} [{}]\n".format(filename, index), -1)
    
    
def do_delete_file(filename):
    scan_directories(filename, delete_file)
    
    
if __name__ == "__main__":

    # File deletion
    do_delete_file("surface_collision.c")
    do_delete_file("surface_load.c")
    do_delete_file("mario_step.c")
    do_delete_file("save_file.c")
    do_delete_file("star_select.c")
    
    # Code deletion
    do_undef_code("bettercamera.c", "s32 ray_surface_intersect", "static s32 puppycam_check_volume_bounds")
    do_undef_code("interaction.c", "u32 determine_interaction", "u32 attack_object")
    do_undef_code("mario.c", "struct Surface *resolve_and_return_wall_collisions", "s32 mario_facing_downhill", 0)
    do_undef_code("mario.c", "#ifdef BETTER_WALL_COLLISION", "s32 mario_facing_downhill", 1)
    do_undef_code("mario_actions_submerged.c", "static f32 get_buoyancy", "static BAD_RETURN(u32) update_water_pitch")

    # Code patches
    do_patch_file("dynos_misc.cpp", "&__Actors()", "define_actor(yoshi_egg_geo),", "\nOMM_DYNOS_ACTORS,", +1)
    do_patch_file("ultra64.h", "_ULTRA64_H_", "#include <PR/libultra.h>", "\n#include \"data/omm/omm_macros.h\"", +1)
    do_patch_file("sm64.h", "SM64_H", "#include \"macros.h\"", "\n#include \"data/omm/omm_includes.h\"", +1)
    do_patch_file("levels/castle_grounds/script.c", "level_castle_grounds_entry", "END_AREA(),", "\n    JUMP_LINK(omm_bowser_4_level_script),", +1)
    do_patch_file("audio/external.c", "play_sound(s32 soundBits, f32 *pos)", "{", "\n    OMM_RETURN_IF_TRUE(omm_sound_play_character_sound_n64(soundBits, pos),,);", +1)
    do_patch_file("audio/seqplayer.c", "sequence_channel_process_script(struct SequenceChannel *seqChannel)", "case 0x00: // chan_testlayerfinished", "\n                        if (loBits >= LAYERS_MAX) break;", +1)
    do_patch_file("level_script.c", "level_cmd_place_object", "struct SpawnInfo *spawnInfo;", "\n    val7 |= 0xFF * OMM_STARS_NON_STOP_NOT_ENDING_CUTSCENE;", +1)
    do_patch_file("level_script.c", "level_script_execute(struct LevelCommand *cmd)", "{", "\n    omm_update();", +1)
    do_patch_file("level_script.c", "level_script_execute(struct LevelCommand *cmd)", "LevelScriptJumpTable[sCurrentCmd->type]();", "omm_update_current_cmd();", 0)
    do_patch_file("level_script.c", "level_script_execute(struct LevelCommand *cmd)", "render_game();", "omm_update_gfx();\n    ", -1)
    do_patch_file("level_script.c", "level_script_execute(struct LevelCommand *cmd)", "render_game();", "\n    omm_debug_end_counter();", +1)
    do_patch_file("camera.c", "find_in_bounds_yaw_wdw_bob_thi(Vec3f pos, Vec3f origin, s16 yaw)", "{", "\n    return (yaw);", +1)
    do_patch_file("options_menu.c", "static struct SubMenu *currentMenu = &menuMain", ";", "\n#include \"data/omm/system/omm_options_menu.inl\"", +1)
    do_patch_file("behaviors/coin.inc.c", "bhv_coin_loop(void)", "cur_obj_update_floor_and_walls();", "obj_safe_step(o, 0);\n    ", -1)
    do_patch_file("behaviors/coin.inc.c", "bhv_coin_loop(void)", "cur_obj_move_standard(-62);", "\n    obj_safe_step(o, 1);", +1)
    do_patch_file("behaviors/moving_coin.inc.c", "coin_step(s16 *collisionFlagsPtr)", "*collisionFlagsPtr = object_step();", "obj_safe_step(o, 0);\n    ", -1)
    do_patch_file("behaviors/moving_coin.inc.c", "coin_step(s16 *collisionFlagsPtr)", "*collisionFlagsPtr = object_step();", "\n    obj_safe_step(o, 1);", +1)
    do_patch_file("behaviors/goomba.inc.c", "bhv_goomba_update(void)", "cur_obj_scale(o->oGoombaScale);", "obj_safe_step(o, 0);\n        ", -1)
    do_patch_file("behaviors/goomba.inc.c", "bhv_goomba_update(void)", "cur_obj_move_standard(-78);", "\n        obj_safe_step(o, 1);", +1)
    do_patch_file("level_update.c", "level_trigger_warp(struct MarioState *m, s32 warpOp)", "{", "\n    OMM_RETURN_IF_TRUE(omm_mario_check_death_warp(m, warpOp), 0,);", +1)
    do_patch_file("mario_actions_airborne.c", "update_air_with_turn(struct MarioState *m)", "{", "\n    OMM_RETURN_IF_TRUE(OMM_LIKELY(omm_mario_update_air_with_turn(m)),,);", +1)
    do_patch_file("mario_actions_airborne.c", "update_air_without_turn(struct MarioState *m)", "{", "\n    OMM_RETURN_IF_TRUE(OMM_LIKELY(omm_mario_update_air_without_turn(m)),,);", +1)
    do_patch_file("mario_actions_airborne.c", "common_air_action_step(struct MarioState *m, u32 landAction, s32 animation, u32 stepArg)", "case AIR_STEP_HIT_WALL:", "\n            OMM_RETURN_IF_TRUE(omm_mario_try_to_perform_wall_slide(m), AIR_STEP_NONE, set_mario_animation(m, animation););", +1)
    do_patch_file("mario_actions_cutscene.c", "check_for_instant_quicksand(struct MarioState *m)", "&& m->action != ACT_QUICKSAND_DEATH) {", "\n        if (OMM_MOVESET_ODYSSEY) { return (find_floor_height(m->pos[0], m->pos[1], m->pos[2]) >= m->pos[1]) && mario_update_quicksand(m, 0.f); }", +1)
    do_patch_file("mario_actions_moving.c", "apply_slope_accel(struct MarioState *m)", "} else ", "if (!omm_peach_vibe_is_gloom()) ", +1)
    do_patch_file("mario_actions_moving.c", "update_walking_speed(struct MarioState *m)", "{", "\n    OMM_RETURN_IF_TRUE(OMM_LIKELY(omm_mario_update_walking_speed(m)),,);", +1)
    do_patch_file("object_list_processor.h", "OBJECT_LIST_PROCESSOR_H", "#define OBJECT_POOL_CAPACITY 960", "#define OBJECT_POOL_CAPACITY 2048", 0)
    do_patch_file("object_list_processor.h", "OBJECT_LIST_PROCESSOR_H", "#define OBJECT_POOL_CAPACITY 240", "#define OBJECT_POOL_CAPACITY 1024", 0)
    do_patch_file("object_helpers.c", "geo_switch_area(s32 callContext, struct GraphNode *node", "if (gMarioObject == NULL) {", "gFindFloorForCutsceneStar = TRUE;\n        ", -1)
    do_patch_file("rendering_graph_node.c", "geo_process_switch(struct GraphNodeSwitchCase *node)", "geo_process_node_and_siblings(selectedChild);", "\n        omm_cappy_gfx_draw_eyes(node, geo_append_display_list);", +1)
    do_patch_file("rendering_graph_node.c", "geo_process_animated_part(struct GraphNodeAnimatedPart *node)", "mtxf_rotate_xyz_and_translate(matrix, translation, rotation);", "omm_geo_fix_marios_anim_translation_y(gCurrGraphNodeObject, node, &translation[1]);\n    ", -1)
    do_patch_file("rendering_graph_node.c", "geo_process_animated_part(struct GraphNodeAnimatedPart *node)", "mtxf_rotate_xyz_and_translate(matrix, translationInterpolated, rotationInterpolated);", "omm_geo_fix_marios_anim_translation_y(gCurrGraphNodeObject, node, &translationInterpolated[1]);\n    ", -1)
    do_patch_file("rendering_graph_node.c", "geo_process_shadow(struct GraphNodeShadow *node)", "vec3f_copy(shadowPos, gCurGraphNodeObject->pos);", "\n            obj_fix_shadow_pos(gCurGraphNodeObject, shadowPos);", +1)
    do_patch_file("rendering_graph_node.c", "obj_is_in_view(struct GraphNodeObject *node, Mat4 matrix)", "{", "\n    OMM_RETURN_IF_TRUE(obj_is_always_rendered((struct Object *) node), TRUE,);", +1)
    do_patch_file("rendering_graph_node.c", "geo_process_node_and_siblings(struct GraphNode *firstNode)", "{", "\n    if (!omm_sanity_check_graph_node(firstNode)) return;", +1)
    do_patch_file("rendering_graph_node.c", "geo_process_node_and_siblings(struct GraphNode *firstNode)", "do {", "\n        if (!omm_sanity_check_graph_node(curGraphNode)) break;", +1)
    do_patch_file("gd_memory.c", "#ifndef USE_SYSTEM_MALLOC", "#ifndef USE_SYSTEM_MALLOC", "#undef USE_SYSTEM_MALLOC\n", -1)
    do_patch_file("gfx_pc.c", "gfx_sp_tri1(uint8_t vtx1_idx, uint8_t vtx2_idx, uint8_t vtx3_idx)", "import_texture(i);", "extern bool omm_gfx_import_texture(struct GfxRenderingAPI *, void *, int, void **, const void *, int, int); if (!omm_gfx_import_texture(gfx_rapi, (void *) gfx_texture_cache_lookup, i, (void **) &rendering_state.textures[i], (const void *) rdp.loaded_texture[i].addr, rdp.texture_tile.fmt, rdp.texture_tile.siz)) { import_texture(i) ; }", 0)
    
    
    