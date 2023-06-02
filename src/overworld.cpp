// #! user/bin/env gcc
/*
 * Copyright (c) 2020-2023 Gustavo Valiente gustavo.valiente@protonmail.com
 * zlib License, see LICENSE file.
 */

#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_display.h"
#include "bn_blending.h"
#include "bn_unique_ptr.h"
#include "bn_fixed_point.h"
#include "bn_rect_window.h"
#include "bn_affine_bg_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_sprite_animate_actions.h"
#include "bn_affine_bg_pa_register_hbe_ptr.h"
#include "bn_affine_bg_pd_register_hbe_ptr.h"
#include "bn_affine_bg_dx_register_hbe_ptr.h"
#include "bn_affine_bg_dy_register_hbe_ptr.h"

#include "common_info.h"
#include "common_variable_8x16_sprite_font.h"

#include "load_attributes.h"

#include "bn_sprite_items_link.h"
#include "bn_sprite_items_barrel.h"

#include "bn_sprite_items_chuchu.h"
#include "bn_sprite_items_bokoblin.h"

#include "bn_sprite_items_hud.h"


#include "bn_affine_bg_items_land.h"
#include "bn_affine_bg_items_clouds.h"

int main()
{
    bn::core::init();

    /*constexpr bn::string_view info_text_lines[] = {
        "PAD: move link",
        "PAD+A: move link faster",
        "",
        "START: go to next scene",
    };*/

    bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);
    // common::info info("World map", info_text_lines, text_generator);

    bn::affine_bg_ptr land_bg = bn::affine_bg_items::land.create_bg(0, 0);
    land_bg.set_pivot_position(1432, 874);

    int x_limit = (land_bg.dimensions().width() - bn::display::width()) / 2; // Limit of how far the character can move in x
    int y_limit = (land_bg.dimensions().height() - bn::display::height()) / 2; // Limit of how far the character can move in y

    bn::affine_bg_ptr clouds_bg = bn::affine_bg_items::clouds.create_bg(0, 0);
    clouds_bg.set_priority(2);
    clouds_bg.set_blending_enabled(true);
    bn::blending::set_transparency_alpha(0.5);

    
    
    bn::rect_window rect_window = bn::rect_window::external();
    rect_window.set_boundaries(attributes_window_height - (bn::display::height() / 2),
                                   -bn::display::width() / 2,
                                   (bn::display::height() / 2) - attributes_window_height,
                                   bn::display::width() / 2);

    
    bn::sprite_ptr chuchu_sprite = bn::sprite_items::bokoblin.create_sprite(0, 0);

    bn::window outside_window = bn::window::outside();
    outside_window.set_show_bg(land_bg, false);
    outside_window.set_show_bg(clouds_bg, false);
    outside_window.set_show_sprites(false);

     
    
    bn::unique_ptr<bn::array<bn::affine_bg_mat_attributes, bn::display::height()>> land_attributes_ptr(
            new bn::array<bn::affine_bg_mat_attributes, bn::display::height()>());
    
    
    bn::array<bn::affine_bg_mat_attributes, bn::display::height()>& land_attributes = *land_attributes_ptr;
    
    bn::affine_bg_pa_register_hbe_ptr land_pa_hbe =
            bn::affine_bg_pa_register_hbe_ptr::create(land_bg, land_attributes._data);
    bn::affine_bg_pd_register_hbe_ptr land_pd_hbe =
            bn::affine_bg_pd_register_hbe_ptr::create(land_bg, land_attributes._data);
    bn::affine_bg_dx_register_hbe_ptr land_dx_hbe =
            bn::affine_bg_dx_register_hbe_ptr::create(land_bg, land_attributes._data);
    bn::affine_bg_dy_register_hbe_ptr land_dy_hbe =
            bn::affine_bg_dy_register_hbe_ptr::create(land_bg, land_attributes._data);

    bn::unique_ptr<bn::array<bn::affine_bg_mat_attributes, bn::display::height()>> clouds_attributes_ptr(
            new bn::array<bn::affine_bg_mat_attributes, bn::display::height()>());
    bn::array<bn::affine_bg_mat_attributes, bn::display::height()>& clouds_attributes = *clouds_attributes_ptr;
    bn::affine_bg_pa_register_hbe_ptr clouds_pa_hbe =
            bn::affine_bg_pa_register_hbe_ptr::create(clouds_bg, clouds_attributes._data);
    bn::affine_bg_pd_register_hbe_ptr clouds_pd_hbe =
            bn::affine_bg_pd_register_hbe_ptr::create(clouds_bg, clouds_attributes._data);
    bn::affine_bg_dx_register_hbe_ptr clouds_dx_hbe =
            bn::affine_bg_dx_register_hbe_ptr::create(clouds_bg, clouds_attributes._data);
    bn::affine_bg_dy_register_hbe_ptr clouds_dy_hbe =
            bn::affine_bg_dy_register_hbe_ptr::create(clouds_bg, clouds_attributes._data);

    bn::sprite_ptr link_sprite = bn::sprite_items::link.create_sprite(0, 0);
    bn::sprite_animate_action<4> link_animate_action = bn::create_sprite_animate_action_forever(
                link_sprite, 12, bn::sprite_items::link.tiles_item(), 0, 1, 2, 3);

    union direction
    {
       struct
       {
          unsigned up: 1;
          unsigned down: 1;
          unsigned left: 1;
          unsigned right: 1;
       } keys;
       int data = 0;
    };

    direction last_direction;
    last_direction.keys.down = true;

    bool first_frame = true;
    
    float cc_pivot_x = 1500;
    float cc_pivot_y = 874;
    
    float scale_x = 1.55;
    float scale_y = 1.0;
    float frame = 0;
    
    bn::sprite_ptr hud_sprite = bn::sprite_items::hud.create_sprite(-88, -24);
    bn::sprite_animate_action<2> chuchu_animation = bn::create_sprite_animate_action_forever(
                chuchu_sprite, 12, bn::sprite_items::bokoblin.tiles_item(), 0, 1);

    
    hud_sprite.set_bg_priority(0);
    
    //bn::sprite_ptr barrel_sprite = bn::sprite_items::barrel.create_sprite(0, 0);
    //barrel_sprite.set_priority(0);

    
    // clouds_bg.set_visible(false);


    while(true)
    {
        frame ++;
        cc_pivot_x = 1000.0-(frame/7.0);
        cc_pivot_y = 440.0;
        chuchu_sprite.set_x(cc_pivot_x*scale_x-scale_x*land_bg.pivot_x());
        chuchu_sprite.set_y(cc_pivot_y*scale_y-scale_y*land_bg.pivot_y());
        
        chuchu_sprite.set_scale(bn::max(bn::fixed(0.1),(cc_pivot_y)/land_bg.pivot_y()));

        
        bn::fixed_point old_pivot_position = land_bg.pivot_position();
        direction new_direction;
        int inc = bn::keypad::a_held() ? 2 : 1;
        bool key_held = false;

        if(bn::keypad::left_held())
        {
            land_bg.set_pivot_x(bn::max(land_bg.pivot_x().right_shift_integer() - inc, 0 - x_limit));
            new_direction.keys.left = true;
            key_held = true;
        }
        else if(bn::keypad::right_held())
        {
            land_bg.set_pivot_x(bn::min(land_bg.pivot_x().right_shift_integer() + inc, x_limit - 1));
            new_direction.keys.right = true;
            key_held = true;
        }

        if(bn::keypad::up_held())
        {
            land_bg.set_pivot_y(bn::max(land_bg.pivot_y().right_shift_integer() - inc, 0 - y_limit));
            new_direction.keys.up = true;
            key_held = true;
        }
        else if(bn::keypad::down_held())
        {
            land_bg.set_pivot_y(bn::min(land_bg.pivot_y().right_shift_integer() + inc, y_limit - 1));
            new_direction.keys.down = true;
            key_held = true;
        }

        clouds_bg.set_pivot_position(clouds_bg.pivot_position() + land_bg.pivot_position() - old_pivot_position +
                                     bn::fixed_point(0.1, 0.1));

        load_attributes(land_bg.mat_attributes(), land_attributes._data);
        load_attributes(clouds_bg.mat_attributes(), clouds_attributes._data);
        

        if(first_frame)
        {
            land_pa_hbe.reload_attributes_ref();
            land_pd_hbe.reload_attributes_ref();
            clouds_pa_hbe.reload_attributes_ref();
            clouds_pd_hbe.reload_attributes_ref();
            first_frame = false;
        }

        land_dx_hbe.reload_attributes_ref();
        land_dy_hbe.reload_attributes_ref();
        clouds_dx_hbe.reload_attributes_ref();
        clouds_dy_hbe.reload_attributes_ref();

        if(key_held && last_direction.data != new_direction.data)
        {
            if(new_direction.keys.left)
            {
                link_animate_action = bn::create_sprite_animate_action_forever(
                            link_sprite, 12, bn::sprite_items::link.tiles_item(), 8, 9, 10, 11);
            }
            else if(new_direction.keys.right)
            {
                link_animate_action = bn::create_sprite_animate_action_forever(
                            link_sprite, 12, bn::sprite_items::link.tiles_item(), 12, 13, 14, 15);
            }

            if(new_direction.keys.up)
            {
                link_animate_action = bn::create_sprite_animate_action_forever(
                            link_sprite, 12, bn::sprite_items::link.tiles_item(), 4, 5, 6, 7);
            }
            else if(new_direction.keys.down)
            {
                link_animate_action = bn::create_sprite_animate_action_forever(
                            link_sprite, 12, bn::sprite_items::link.tiles_item(), 0, 1, 2, 3);
            }

            last_direction = new_direction;
        }

        if (key_held) {
            
            for(int index = 0; index < inc; ++index)
            {
                link_animate_action.update();
            }
        }

        // info.update();
        chuchu_animation.update();
        bn::core::update();
    }
}
