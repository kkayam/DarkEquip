﻿#include "potion.h"
#include "inventory.h"
#include "handle/page_handle.h"
#include "handle/page/page_setting.h"

namespace item {
    void potion::consume_potion(handle::slot_setting*& a_slot, RE::PlayerCharacter*& a_player) {
        logger::trace("try to consume {}"sv, a_slot->form->GetName());


        RE::TESBoundObject* obj = nullptr;
        uint32_t left = 0;
        for (auto potential_items = inventory::get_inventory(a_player, RE::FormType::AlchemyItem);
             const auto& [item, inv_data] : potential_items) {
            if (const auto& [num_items, entry] = inv_data; entry->object->formID == a_slot->form->formID) {
                obj = item;
                left = inv_data.first;
                break;
            }
        }


        if (obj == nullptr || left == 0) {
            logger::warn("could not find selected potion, maybe it all have been consumed"sv);
            const auto handler = handle::page_handle::get_singleton();
            handler->set_active_page(handler->get_next_page_id(handle::page_setting::position::bottom),handle::page_setting::position::bottom);
            //TODO update ui in this case
            return;
        }

        if (!obj->Is(RE::FormType::AlchemyItem)) {
            logger::warn("object {} is not an alchemy item. return."sv, obj->GetName());
            return;
        }

        logger::trace("calling drink/eat potion/food {}, count left {}"sv, obj->GetName(), left);

        const auto equip_manager = RE::ActorEquipManager::GetSingleton();
        equip_manager->EquipObject(a_player, obj);
        if (left == 1) {
            const auto handler = handle::page_handle::get_singleton();
            handler->set_active_page(handler->get_next_page_id(handle::page_setting::position::bottom),handle::page_setting::position::bottom);
        }
        logger::trace("drank/ate potion/food {}. return."sv, obj->GetName());
    }
}
