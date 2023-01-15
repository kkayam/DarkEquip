#include "MenuOpenCloseEventHandler.h"

namespace event {
    MenuOpenCloseEventHandler* MenuOpenCloseEventHandler::GetSingleton()
    {
        static MenuOpenCloseEventHandler singleton;
        return std::addressof(singleton);
    }

    void MenuOpenCloseEventHandler::Register()
    {
        auto ui = RE::UI::GetSingleton();
        ui->AddEventSink(GetSingleton());
    }

    RE::BSEventNotifyControl MenuOpenCloseEventHandler::ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
    {
        if (a_event && !a_event->opening && a_event->menuName == "InventoryMenu") {
            handle::page_handle::compile_pages_inventory();
        }
        if (a_event && !a_event->opening && a_event->menuName == "MagicMenu") {
            RE::DebugNotification("menu closed");
        }
        return RE::BSEventNotifyControl::kContinue;
    }
}
