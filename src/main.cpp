/*
 * Copyright (c) 2020-2023 Gustavo Valiente gustavo.valiente@protonmail.com
 * zlib License, see LICENSE file.
 */

#include "bn_core.h"
#include "bn_math.h"
#include "bn_keypad.h"
#include "bn_display.h"
#include "bn_sprite_actions.h"
#include "bn_affine_bg_ptr.h"
#include "bn_sprite_animate_actions.h"

#include "bn_sprite_text_generator.h"
#include "bn_affine_bg_pa_register_hbe_ptr.h"
#include "bn_affine_bg_pc_register_hbe_ptr.h"
#include "bn_affine_bg_dx_register_hbe_ptr.h"
#include "bn_affine_bg_dy_register_hbe_ptr.h"

#include "bn_sprite_items_boat.h"
#include "bn_sprite_items_hud.h"
#include "bn_sprite_items_compass.h"
#include "bn_sprite_items_needle.h"

#include "bn_sprite_items_island.h"



#include "bn_affine_bg_items_land.h"

#include "common_info.h"
#include "common_variable_8x16_sprite_font.h"

#define INCLUDE_HUD 1

namespace
{
    struct camera
    {
        bn::fixed x = 440;
        bn::fixed y = 250;
        bn::fixed z = 0;
        int phi = 10;
        int cos = 0;
        int sin = 0;
    };

    void sprite_boat_thing(camera camera) {
        bn::sprite_ptr boat_sprite = bn::sprite_items::boat.create_sprite(0, 48);
        //bn::sprite_animate_action<4> action = bn::create_sprite_animate_action_forever(
        //        boat_sprite, 16, bn::sprite_items::boat.tiles_item(), 0, 1, 2, 3);

        bn::sprite_animate_action<4> action = bn::create_sprite_animate_action_forever(boat_sprite, 16, bn::sprite_items::boat.tiles_item(), 0, 1,2,3);

        action.update();

        // boat_sprite.set_visible(true);
        // boat_sprite.set_z_order(-1000);
        float y_float = camera.y.to_float();
        float scale_val = (500.0)/( y_float);
        boat_sprite.set_scale(scale_val);


        bn::core::update();
    }

    void update_camera(camera& camera)
    {
        bn::fixed dir_x = 0;
        bn::fixed dir_z = 0;

        if(bn::keypad::left_held())
        {
            // dir_x -= bn::fixed::from_data(32);
        }
        else if(bn::keypad::right_held())
        {
            // dir_x += bn::fixed::from_data(32);
        }

        if(bn::keypad::down_held())
        {
            //dir_z += bn::fixed::from_data(32);
        }
        //else if(bn::keypad::up_held())
        //{
            dir_z -= bn::fixed::from_data(48);
        //}

        if(bn::keypad::b_held())
        {
            camera.y -= bn::fixed::from_data(2048);

            if(camera.y < 0)
            {
                camera.y = 0;
            }
        }
        else if(bn::keypad::a_held())
        {
            camera.y += bn::fixed::from_data(2048);
        }

        if(bn::keypad::left_held())
        {
            camera.phi -= 1;

            if(camera.phi < 0)
            {
                camera.phi += 2048;
            }
        }
        else if(bn::keypad::right_held())
        {
            camera.phi += 1;

            if(camera.phi >= 2048)
            {
                camera.phi -= 2048;
            }
        }

        camera.cos = bn::lut_cos(camera.phi).data() >> 4;
        camera.sin = bn::lut_sin(camera.phi).data() >> 4;
        camera.x += (dir_x * camera.cos) - (dir_z * camera.sin);
        camera.z += (dir_x * camera.sin) + (dir_z * camera.cos);
    }

    void update_hbe_values(const camera& camera, int16_t* pa_values, int16_t* pc_values, int* dx_values,
                           int* dy_values)
    {
        int camera_x = camera.x.data();
        int camera_y = camera.y.data() >> 4;
        int camera_z = camera.z.data();
        int camera_cos = camera.cos;
        int camera_sin = camera.sin;
        int y_shift = 160;

        for(int index = 0; index < bn::display::height(); ++index)
        {
            int reciprocal = bn::reciprocal_lut[index].data() >> 4;
            int lam = camera_y * reciprocal >> 12;
            int lcf = lam * camera_cos >> 8;
            int lsf = lam * camera_sin >> 8;

            pa_values[index] = int16_t(lcf >> 4);
            pc_values[index] = int16_t(lsf >> 4);

            int lxr = (bn::display::width() / 2) * lcf;
            int lyr = y_shift * lsf;
            dx_values[index] = (camera_x - lxr + lyr) >> 4;

            lxr = (bn::display::width() / 2) * lsf;
            lyr = y_shift * lcf;
            dy_values[index] = (camera_z - lxr - lyr) >> 4;
        }
    }
}

int main() {
    bn::core::init();

    bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);

    /*
    constexpr bn::string_view info_text_lines[] = {
        "Left/Right: move camera x",
        "Up/Down: move camera z",
        "B/A: move camera y",
        "L/R: move camera phi",
    };
        */

    //common::info info("", info_text_lines, text_generator);

    bn::affine_bg_ptr bg = bn::affine_bg_items::land.create_bg(-376, -406);// 336

    int16_t pa_values[bn::display::height()];
    bn::affine_bg_pa_register_hbe_ptr pa_hbe = bn::affine_bg_pa_register_hbe_ptr::create(bg, pa_values);

    int16_t pc_values[bn::display::height()];
    bn::affine_bg_pc_register_hbe_ptr pc_hbe = bn::affine_bg_pc_register_hbe_ptr::create(bg, pc_values);

    int dx_values[bn::display::height()];
    bn::affine_bg_dx_register_hbe_ptr dx_hbe = bn::affine_bg_dx_register_hbe_ptr::create(bg, dx_values);

    int dy_values[bn::display::height()];
    bn::affine_bg_dy_register_hbe_ptr dy_hbe = bn::affine_bg_dy_register_hbe_ptr::create(bg, dy_values);

    camera camera;


    bn::sprite_ptr boat_sprite = bn::sprite_items::boat.create_sprite(0, 48);
    bn::sprite_animate_action<4> action = bn::create_sprite_animate_action_forever(boat_sprite, 16,
                                                                                   bn::sprite_items::boat.tiles_item(),
                                                                                   0, 1, 2, 3);


    bn::sprite_ptr hud_sprite = bn::sprite_items::hud.create_sprite(-88, -48);

    bn::sprite_ptr compass_sprite = bn::sprite_items::compass.create_sprite(-110, -48);
    bn::sprite_ptr needle_sprite = bn::sprite_items::needle.create_sprite(-110, -48);


    //bn::sprite_ptr horizon[10];
    //for (int i = 0; i <10; i++) {
    // bn::sprite_ptr island = bn::sprite_items::island.create_sprite(0, -72);
    // island.set_scale(0.5);
    // }


    while(true)
    {
        update_camera(camera);
        update_hbe_values(camera, pa_values, pc_values, dx_values, dy_values);
        pa_hbe.reload_values_ref();
        pc_hbe.reload_values_ref();
        dx_hbe.reload_values_ref();
        dy_hbe.reload_values_ref();
        //bn::core::update();

        needle_sprite.set_rotation_angle(((float) camera.phi) * 360.0 / 2048.0);

        if (( -camera.z / 4000.0) -0.3 > 0) {
            //island.set_scale((-camera.z / 4000.0) - 0.3);
        }

        action.update();

        // boat_sprite.set_visible(true);
        // boat_sprite.set_z_order(-1000);
        float y_float = camera.y.to_float();
        float scale_val = (500.0)/( y_float);
        boat_sprite.set_scale(scale_val);


        bn::core::update();



        // info.update();
        //bn::core::update();
    }
}
