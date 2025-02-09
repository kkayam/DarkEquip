﻿#include "shout.h"

namespace magic {
    std::vector<RE::TESForm*> shout::get_shouts() {
        //easier just to use items that have been favourited, just filter them
        std::vector<RE::TESForm*> shout_list;

        for (auto magic_favorites = RE::MagicFavorites::GetSingleton()->spells; auto form : magic_favorites) {
            if (form->Is(RE::FormType::Shout)) {
                logger::trace("shout name is {}"sv, form->GetName());
                shout_list.push_back(form);
            }
        }

        logger::trace("shout list is size {}. return."sv, shout_list.size());
        return shout_list;
    }

    void shout::equip_shout(RE::TESForm* a_form, RE::PlayerCharacter*& a_player) {
        logger::trace("try to equip shout {}"sv, a_form->GetName());

        if (!a_form->Is(RE::FormType::Shout)) {
            logger::warn("object {} is not a shout. return."sv, a_form->GetName());
            return;
        }

        if (const auto selected_power = a_player->GetActorRuntimeData().selectedPower;
            selected_power != nullptr) {
            logger::trace("current selected power is {}, is shout {}, is spell {}"sv,
                selected_power->GetName(),
                selected_power->Is(RE::FormType::Shout),
                selected_power->Is(RE::FormType::Spell));
            if (selected_power->formID == a_form->formID) {
                logger::debug("no need to equip shout {}, it is already equipped. return."sv, a_form->GetName());
                return;
            }
        }

        const auto shout = a_form->As<RE::TESShout>();
        RE::ActorEquipManager::GetSingleton()->EquipShout(a_player, shout);
        logger::trace("equipped shout {}. return."sv, a_form->GetName());
    }
}
