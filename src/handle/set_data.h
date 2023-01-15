﻿#pragma once
#include "key_position.h"
#include "data/data_helper.h"

namespace handle {
    class set_data {
    public:
        static void read_and_set_data();

        static void set_new_item_count_if_needed(const RE::TESBoundObject* a_obj, int32_t a_count);

        static void set_single_slot(uint32_t a_page,
            page_setting::position a_pos,
            const std::vector<data_helper*>& a_data);
        
        static void clear_slot(const page_setting::position a_pos);

    private:
        static void set_empty_slot(int a_page, int a_pos, key_position*& a_key_pos);
        static void set_slot(uint32_t a_page,
            page_setting::position a_pos,
            const std::string& a_form,
            uint32_t a_type,
            uint32_t a_hand,
            uint32_t a_action,
            const std::string& a_form_left,
            uint32_t a_type_left,
            uint32_t a_action_left,
            key_position*& a_key_pos);

        static void set_new_item_count(RE::FormID a_form_id, const char* a_name, int32_t a_count);
    };
}
