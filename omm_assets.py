import os
import shutil

def copy_asset(filename):
    directory = filename[:filename.rfind("/")]
    os.makedirs("build/us_pc/omm/gfx/assets/" + directory, exist_ok=True)
    shutil.copyfile(filename + ".png", "build/us_pc/omm/gfx/assets/" + filename + ".png")
    
if __name__ == "__main__":
    copy_asset("actors/coin/coin_front.ia16")
    copy_asset("actors/coin/coin_tilt_right.ia16")
    copy_asset("actors/coin/coin_side.ia16")
    copy_asset("actors/coin/coin_tilt_left.ia16")
    copy_asset("actors/flame/flame_0.ia16")
    copy_asset("actors/flame/flame_1.ia16")
    copy_asset("actors/flame/flame_2.ia16")
    copy_asset("actors/flame/flame_3.ia16")
    copy_asset("actors/flame/flame_4.ia16")
    copy_asset("actors/flame/flame_5.ia16")
    copy_asset("actors/flame/flame_6.ia16")
    copy_asset("actors/flame/flame_7.ia16")
    copy_asset("actors/sparkle/sparkle_0.rgba16")
    copy_asset("actors/sparkle/sparkle_1.rgba16")
    copy_asset("actors/sparkle/sparkle_2.rgba16")
    copy_asset("actors/sparkle/sparkle_3.rgba16")
    copy_asset("actors/sparkle/sparkle_4.rgba16")
    copy_asset("actors/sparkle/sparkle_5.rgba16")
    copy_asset("actors/sparkle_animation/sparkle_animation_0.ia16")
    copy_asset("actors/sparkle_animation/sparkle_animation_1.ia16")
    copy_asset("actors/sparkle_animation/sparkle_animation_2.ia16")
    copy_asset("actors/sparkle_animation/sparkle_animation_3.ia16")
    copy_asset("actors/sparkle_animation/sparkle_animation_4.ia16")
    copy_asset("actors/exclamation_box/exclamation_box_front.rgba16")
    copy_asset("actors/mushroom_1up/1-up_mushroom.rgba16")
    copy_asset("textures/intro_raw/red_star_0.rgba16")
    copy_asset("textures/intro_raw/red_star_1.rgba16")
    copy_asset("textures/intro_raw/red_star_2.rgba16")
    copy_asset("textures/intro_raw/red_star_3.rgba16")
    copy_asset("textures/intro_raw/red_star_4.rgba16")
    copy_asset("textures/intro_raw/red_star_5.rgba16")
    copy_asset("textures/intro_raw/red_star_6.rgba16")
    copy_asset("textures/intro_raw/red_star_7.rgba16")

