﻿#include "page_handle.h"
#include "equip/equip_slot.h"
#include "util/string_util.h"
#include "handle/set_data.h"

namespace handle {
    page_handle* page_handle::get_singleton() {
        static page_handle singleton;
        return std::addressof(singleton);
    }

    void page_handle::init_page(uint32_t a_page,
        const page_setting::position a_position,
        const std::vector<data_helper*>& data_helpers,
        const float a_slot_offset,
        const float a_key_offset,
        const slot_setting::hand_equip a_hand,
        const uint32_t a_opacity,
        key_position*& a_key_pos) {
        logger::trace("init page {}, position {}, data_size for settings {}, hand {} ..."sv,
            a_page,
            static_cast<uint32_t>(a_position),
            data_helpers.size(),
            static_cast<uint32_t>(a_hand));
        if (!this->data_) {
            this->data_ = new page_handle_data();
        }

        page_handle_data* data = this->data_;

        auto* page = new page_setting();
        page->pos = a_position;

        /*auto* fade = new fade_setting();
        fade->action = fade_setting::action::unset;
        fade->alpha = fade_setting::alpha::max;
        fade->current_alpha = static_cast<uint32_t>(fade_setting::alpha::max);

        page->fade_setting = fade;*/

        auto* slots = new std::vector<slot_setting*>;
        for (const auto element : data_helpers) {
            logger::trace("processing form {}, type {}, action {}, left {}"sv,
                element->form != nullptr ? util::string_util::int_to_hex(element->form->GetFormID()) : "0",
                static_cast<int>(element->type),
                static_cast<uint32_t>(element->action_type),
                element->left);
            auto* slot = new slot_setting();
            slot->form = element->form;
            slot->type = element->type;
            slot->action = element->action_type;
            slot->equip = a_hand;
            RE::BGSEquipSlot* equip_slot = nullptr;
            get_equip_slots(element->type, a_hand, equip_slot, element->left);
            get_item_count(element->form, slot->item_count, element->type);
            slot->equip_slot = equip_slot;

            slots->push_back(slot);
        }

        page->slot_settings = *slots;

        float offset_x = 0.f;
        float offset_y = 0.f;

        get_offset_values(a_position, a_slot_offset, offset_x, offset_y);

        auto* offset = new offset_setting();
        offset->offset_slot_x = offset_x;
        offset->offset_slot_y = offset_y;

        get_offset_values(a_position, a_key_offset, offset_x, offset_y);
        offset->offset_key_x = offset_x;
        offset->offset_key_y = offset_y;

        page->offset_setting = offset;

        //TODO for now the right hand or the first setting defines the icon
        page->icon_type = get_icon_type(slots->front()->type, slots->front()->form);
        if (slots->size() == 2 && page->icon_type == ui::icon_image_type::icon_default) {
            logger::debug("Could not find an Icon with first setting, try next");
            page->icon_type = get_icon_type(slots->at(1)->type, slots->at(1)->form);
        }

        page->icon_opacity = a_opacity;

        page->key = a_key_pos->get_key_for_position(a_position);

        data->page_settings[a_page][a_position] = page;

        logger::trace("done setting page {}, position {}."sv,
            a_page,
            static_cast<uint32_t>(a_position));
    }

    void page_handle::set_active_page(const uint32_t a_page,const page_setting::position a_position) const {
        if (!this->data_) {
            return;
        }
        page_handle_data* data = this->data_;

        logger::trace("set active page to {}"sv, a_page);

        data->active_page[static_cast<int32_t>(a_position)] = a_page;
    }

    page_setting* page_handle::get_page_setting(const uint32_t a_page, const page_setting::position a_position) const {
        if (const page_handle_data* data = this->data_;
            data && !data->page_settings.empty() && !data->page_settings.at(a_page).empty() && data->page_settings.
            at(a_page).at(a_position)) {
            return data->page_settings.at(a_page).at(a_position);
        }
        return nullptr;
    }

    std::map<page_setting::position, page_setting*> page_handle::get_page(const uint32_t a_page) const {
        if (const page_handle_data* data = this->data_; data && !data->page_settings.empty()) {
            return data->page_settings.at(a_page);
        }
        return {};
    }

    std::map<uint32_t, std::map<page_setting::position, page_setting*>> page_handle::get_pages() const {
        if (const page_handle_data* data = this->data_; data && !data->page_settings.empty()) {
            return data->page_settings;
        }
        return {};
    }

    std::map<page_setting::position, page_setting*> page_handle::get_active_page() const {
        if (const page_handle_data* data = this->data_; data && !data->page_settings.empty()) {
            std::map<page_setting::position, page_setting*> active_page;
            for (int32_t i = 0; i < 4; i++)
            {
                std::map<page_setting::position, page_setting*> tmp = data->page_settings.at(data->active_page[i]);
                active_page[static_cast<page_setting::position>(i)] = tmp[static_cast<page_setting::position>(i)];
            }
            
            return active_page;
        }
        return {};
    }

    uint32_t page_handle::get_active_page_id(const page_setting::position a_position) const {
        if (const page_handle_data* data = this->data_; data) {
            return data->active_page[static_cast<int32_t>(a_position)];
        }
        return {};
    }

    void page_handle::compile_pages() const {
        const auto player = RE::PlayerCharacter::GetSingleton();
        handle::set_data::clear_slot(page_setting::position::right);
        handle::set_data::clear_slot(page_setting::position::left);
        handle::set_data::clear_slot(page_setting::position::bottom);
        handle::set_data::clear_slot(page_setting::position::top);

        int slot_index_right = 0;
        int slot_index_left = 0;
        int slot_index_bottom = 0;
        int slot_index_top = 0;
        for (const auto& [item, inv_data] : player->GetInventory([](const RE::TESBoundObject& a_object) { 
                return a_object.Is(RE::FormType::AlchemyItem,RE::FormType::Weapon,RE::FormType::Armor);
            })) {
            if (const auto& [count, entry] = inv_data; entry->IsFavorited() ) {
                std::vector<data_helper*> data;
                auto item_data_helper = new data_helper();
                item_data_helper->form = RE::TESForm::LookupByID(entry->object->formID);
                item_data_helper->left = false;
                item_data_helper->action_type = handle::slot_setting::acton_type::default_action;
                if(entry->object->IsWeapon()){
                    item_data_helper->type = handle::slot_setting::slot_type::weapon;
                    data.push_back(item_data_helper);
                    handle::set_data::set_single_slot(slot_index_right,page_setting::position::right,data);
                    slot_index_right++;
                }
                if(entry->object->IsArmor()){
                    item_data_helper->type = handle::slot_setting::slot_type::shield;
                    item_data_helper->left = true;
                    data.push_back(item_data_helper);
                    handle::set_data::set_single_slot(slot_index_left,page_setting::position::left,data);
                    slot_index_left++;
                }
                if(entry->object->Is(RE::FormType::AlchemyItem)){
                    item_data_helper->type = handle::slot_setting::slot_type::consumable;
                    data.push_back(item_data_helper);
                    handle::set_data::set_single_slot(slot_index_bottom,page_setting::position::bottom,data);
                    slot_index_bottom++;
                }
            }
        }
        for (auto magic_favorites = RE::MagicFavorites::GetSingleton()->spells; auto form : magic_favorites) {
            if (form->Is(RE::FormType::Spell)) {
                        auto item_data_helper = new data_helper();
                        item_data_helper->form = RE::TESForm::LookupByID(form->formID);
                        item_data_helper->action_type = handle::slot_setting::acton_type::default_action;
                        if (const auto spell = form->As<RE::SpellItem>();
                            spell->GetSpellType() == RE::MagicSystem::SpellType::kPower ||
                            spell->GetSpellType() == RE::MagicSystem::SpellType::kLesserPower){ // IF POWER
                                std::vector<data_helper*> data;
                                item_data_helper->type = handle::slot_setting::slot_type::power;
                                item_data_helper->left = true;
                                data.push_back(item_data_helper);
                                handle::set_data::set_single_slot(slot_index_top,page_setting::position::top,data);
                                slot_index_top++;
                            } else { // IF SPELL
                                std::vector<data_helper*> data;
                                item_data_helper->type = handle::slot_setting::slot_type::magic;
                                item_data_helper->left = true;
                                data.push_back(item_data_helper);
                                handle::set_data::set_single_slot(slot_index_left,page_setting::position::left,data);
                                slot_index_left++;
                            }
            }
            if (form->Is(RE::FormType::Shout)) {
                std::vector<data_helper*> data;
                auto item_data_helper = new data_helper();
                item_data_helper->form = RE::TESForm::LookupByID(form->formID);
                item_data_helper->action_type = handle::slot_setting::acton_type::default_action;
                item_data_helper->type = handle::slot_setting::slot_type::shout;
                item_data_helper->left = true;
                data.push_back(item_data_helper);
                handle::set_data::set_single_slot(slot_index_top,page_setting::position::top,data);
                slot_index_top++;
            }
        }
        page_handle_data* data = this->data_;
        data->active_page_max[static_cast<int32_t>(page_setting::position::right)] = slot_index_right;
        data->active_page_max[static_cast<int32_t>(page_setting::position::left)] = slot_index_left;
        data->active_page_max[static_cast<int32_t>(page_setting::position::bottom)] = slot_index_bottom;
        data->active_page_max[static_cast<int32_t>(page_setting::position::top)] = slot_index_top;
        data->active_page[0] = static_cast<int32_t>(data->active_page[0])<slot_index_top?data->active_page[0]:0;
        data->active_page[1] = static_cast<int32_t>(data->active_page[1])<slot_index_right?data->active_page[1]:0;
        data->active_page[2] = static_cast<int32_t>(data->active_page[2])<slot_index_bottom?data->active_page[2]:0;
        data->active_page[3] = static_cast<int32_t>(data->active_page[3])<slot_index_left?data->active_page[3]:0;
    }
    

    uint32_t page_handle::get_next_page_id(const page_setting::position a_position) const {
        page_handle_data* data = this->data_;
        if (data->active_page_max[static_cast<int32_t>(a_position)] == 0) return 0;
        return (data->active_page[static_cast<int32_t>(a_position)]+1)%(data->active_page_max[static_cast<int32_t>(a_position)]);
    }

    void page_handle::get_offset_values(const page_setting::position a_position,
        const float a_setting,
        float& offset_x,
        float& offset_y) {
        offset_x = 0.f;
        offset_y = 0.f;
        // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
        // ReSharper disable once CppIncompleteSwitchStatement
        switch (a_position) {
            case page_setting::position::top:
                offset_y = -a_setting+20;
                break;
            case page_setting::position::right:
                offset_x = a_setting;
                break;
            case page_setting::position::bottom:
                offset_y = a_setting-20;
                break;
            case page_setting::position::left:
                offset_x = -a_setting;
                break;
        }
    }

    void page_handle::get_equip_slots(const slot_setting::slot_type a_type,
        const slot_setting::hand_equip a_hand,
        RE::BGSEquipSlot*& a_slot,
        const bool a_left) {
        a_slot = nullptr;
        if ((a_type == slot_setting::slot_type::magic || a_type == slot_setting::slot_type::weapon) && a_hand
            ==
            slot_setting::hand_equip::single || a_type == slot_setting::slot_type::unset) {
            a_slot = a_left ? item::equip_slot::get_left_hand_slot() : item::equip_slot::get_right_hand_slot();
        }
    }

    ui::icon_image_type page_handle::get_icon_type(const slot_setting::slot_type a_type, RE::TESForm*& a_form) {
        auto icon = ui::icon_image_type::icon_default;
        switch (a_type) {
            case slot_setting::slot_type::weapon:
                get_icon_for_weapon_type(a_form, icon);
                break;
            case slot_setting::slot_type::magic:
                get_icon_for_spell(a_form, icon);
                break;
            case slot_setting::slot_type::shout:
                icon = ui::icon_image_type::shout;
                break;
            case slot_setting::slot_type::power:
                icon = ui::icon_image_type::power;
                break;
            case slot_setting::slot_type::consumable:
                get_icon_for_consumable(a_form, icon);
                break;
            case slot_setting::slot_type::shield:
                //kinda useless atm, icon is set by the first setting, basically right hand
                icon = ui::icon_image_type::shield;
                break;
            case slot_setting::slot_type::armor:
                get_icon_for_item(a_form, icon);
                break;
            case slot_setting::slot_type::scroll:
                icon = ui::icon_image_type::scroll;
                break;
            case slot_setting::slot_type::unset:
            case slot_setting::slot_type::misc:
                icon = ui::icon_image_type::icon_default;
                break;
        }
        return icon;
    }

    void page_handle::get_icon_for_weapon_type(RE::TESForm*& a_form, ui::icon_image_type& a_icon) {
        if (!a_form->IsWeapon()) {
            return;
        }
        switch (const auto weapon = a_form->As<RE::TESObjectWEAP>(); weapon->GetWeaponType()) {
            case RE::WEAPON_TYPE::kHandToHandMelee:
                break;
            case RE::WEAPON_TYPE::kOneHandSword:
                a_icon = ui::icon_image_type::sword_one_handed;
                break;
            case RE::WEAPON_TYPE::kOneHandDagger:
                a_icon = ui::icon_image_type::dagger;
                break;
            case RE::WEAPON_TYPE::kOneHandAxe:
                a_icon = ui::icon_image_type::axe_one_handed;
                break;
            case RE::WEAPON_TYPE::kOneHandMace:
                a_icon = ui::icon_image_type::mace;
                break;
            case RE::WEAPON_TYPE::kTwoHandSword:
                a_icon = ui::icon_image_type::sword_two_handed;
                break;
            case RE::WEAPON_TYPE::kTwoHandAxe:
                a_icon = ui::icon_image_type::axe_two_handed;
                break;
            case RE::WEAPON_TYPE::kBow:
                a_icon = ui::icon_image_type::bow;
                break;
            case RE::WEAPON_TYPE::kStaff:
                a_icon = ui::icon_image_type::staff;
                break;
            case RE::WEAPON_TYPE::kCrossbow:
                a_icon = ui::icon_image_type::crossbow;
                break;
        }
    }

    void page_handle::get_icon_for_spell(RE::TESForm*& a_form, ui::icon_image_type& a_icon) {
        if (!a_form->Is(RE::FormType::Spell)) {
            return;
        }
        const auto spell = a_form->As<RE::SpellItem>();
        const auto effect = spell->GetCostliestEffectItem()->baseEffect;
        auto actor_value = effect->GetMagickSkill();
        if (actor_value == RE::ActorValue::kNone) {
            actor_value = effect->data.primaryAV;
        }

        switch (actor_value) {
            case RE::ActorValue::kAlteration:
            case RE::ActorValue::kConjuration:
            case RE::ActorValue::kDestruction:
                switch (effect->data.resistVariable) {
                    case RE::ActorValue::kResistFire:
                        a_icon = ui::icon_image_type::spell_fire;
                        break;
                    case RE::ActorValue::kResistFrost:
                        a_icon = ui::icon_image_type::spell_frost;
                        break;
                    case RE::ActorValue::kResistShock:
                        a_icon = ui::icon_image_type::spell_shock;
                        break;
                    default:
                        a_icon = ui::icon_image_type::spell_default_alt;
                }
                break;
            case RE::ActorValue::kIllusion:
                a_icon = ui::icon_image_type::spell_default;
                break;
            case RE::ActorValue::kRestoration:
                //might not fit all spells
                a_icon = ui::icon_image_type::spell_heal;
                break;
            default:
                a_icon = ui::icon_image_type::spell_default;
        }
    }

    void page_handle::get_icon_for_consumable(RE::TESForm*& a_form, ui::icon_image_type& a_icon) {
        if (!a_form->Is(RE::FormType::AlchemyItem)) {
            return;
        }
        const auto alchemy_potion = a_form->As<RE::AlchemyItem>();

        if (alchemy_potion->IsFood()) {
            a_icon = ui::icon_image_type::food;
            return;
        }
        if (alchemy_potion->IsPoison()) {
            a_icon = ui::icon_image_type::poison_default;
            return;
        }

        const auto effect = alchemy_potion->GetCostliestEffectItem()->baseEffect;
        auto actor_value = effect->GetMagickSkill();

        if (actor_value == RE::ActorValue::kNone) {
            actor_value = effect->data.primaryAV;
        }

        switch (actor_value) {
            case RE::ActorValue::kHealth:
            case RE::ActorValue::kHealRateMult:
            case RE::ActorValue::kHealRate:
                a_icon = ui::icon_image_type::potion_health;
                break;
            default:
                a_icon = ui::icon_image_type::potion_default;
        }
    }

    void page_handle::get_item_count(RE::TESForm*& a_form,
        int32_t& a_count,
        const slot_setting::slot_type a_type) {
        if (a_type == slot_setting::slot_type::unset || a_form == nullptr) {
            a_count = 0;
            return;
        }
        if (a_type == slot_setting::slot_type::consumable || a_type == slot_setting::slot_type::scroll) {
            const auto player = RE::PlayerCharacter::GetSingleton();
            for (auto potential_items = player->GetInventory(); const auto& [item, invData] : potential_items) {
                if (invData.second->object->formID == a_form->formID) {
                    a_count = invData.first;
                    break;
                }
            }
        } else {
            a_count = 0;
        }
        logger::trace("Item {}, count {}"sv, a_form->GetName(), a_count);
    }

    void page_handle::get_icon_for_item(RE::TESForm*& a_form, ui::icon_image_type& a_icon) {
        if (!a_form->IsArmor()) {
            return;
        }
        switch (const auto armor = a_form->As<RE::TESObjectARMO>(); armor->GetArmorType()) {
            case RE::BIPED_MODEL::ArmorType::kLightArmor:
                a_icon = ui::icon_image_type::armor_light;
                break;
            case RE::BIPED_MODEL::ArmorType::kHeavyArmor:
                a_icon = ui::icon_image_type::armor_heavy;
                break;
            case RE::BIPED_MODEL::ArmorType::kClothing:
                a_icon = ui::icon_image_type::armor_clothing;
                break;
        }
    }
}
