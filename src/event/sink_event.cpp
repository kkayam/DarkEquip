#include "sink_event.h"
#include "equip_event.h"
#include "key_manager.h"
#include "MenuOpenCloseEventHandler.h"

namespace event {
    void sink_events() {
        key_manager::sink();
        equip_event::sink();
        MenuOpenCloseEventHandler::Register();

        logger::info("added all sinks.");
    }
}
