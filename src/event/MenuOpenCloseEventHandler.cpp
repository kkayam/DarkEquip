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
        if (a_event && !a_event->opening ) {
            if(a_event->menuName == "InventoryMenu" || a_event->menuName == "MagicMenu"){
                const auto handler = handle::page_handle::get_singleton();
                handler->compile_pages();
                handler->refresh_active_page();
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }
}
