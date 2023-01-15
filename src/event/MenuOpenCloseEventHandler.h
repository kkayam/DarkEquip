#pragma once
#include "handle/page_handle.h"

namespace event {
    class MenuOpenCloseEventHandler final: public RE::BSTEventSink<RE::MenuOpenCloseEvent>
    {
    public:
        static MenuOpenCloseEventHandler* GetSingleton();
        static void Register();

        virtual RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_eventSource) override;
    };
}
