﻿#pragma once

namespace config {
    class mcm_setting {
    public:
        static void read_setting();

        static uint32_t get_top_action_key();
        static uint32_t get_right_action_key();
        static uint32_t get_bottom_action_key();
        static uint32_t get_left_action_key();
        static uint32_t get_toggle_key();
        static uint32_t get_controller_set();
        static float get_config_button_hold_time();

        static float get_hud_image_scale_width();
        static float get_hud_image_scale_height();
        static float get_hud_image_position_width();
        static float get_hud_image_position_height();
        static float get_hud_slot_position_offset();
        static float get_hud_key_position_offset();
        static float get_icon_scale_width();
        static float get_icon_scale_height();
        static uint32_t get_icon_opacity();
        static uint32_t get_slot_button_feedback();
        static float get_key_icon_scale_width();
        static float get_key_icon_scale_height();
        static float get_slot_count_text_offset();
        static float get_slot_count_text_font_size();
        static bool get_draw_toggle_button();
        static float get_toggle_key_offset_x();
        static float get_toggle_key_offset_y();

        static bool get_action_check();
        static bool get_empty_hand_setting();
        static bool get_hide_outside_combat();
        static float get_fade_timer_outside_combat();
        static bool get_disable_input_quick_loot();
    };
}
