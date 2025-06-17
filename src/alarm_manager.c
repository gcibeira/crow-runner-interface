#include "alarm_manager.h"
#include <string.h>

static alarm_state_t alarm_state = {
    .system_state = SYSTEM_STATE_UNKNOWN,
    .active_zones = 0,
    .triggered_zones = 0
};

static alarm_state_changed_callback_t state_changed_callback = NULL;

static void notify_state_changed(void) {
    if (state_changed_callback) {
        state_changed_callback(&alarm_state);
    }
}

static void protocol_event_handler(const protocol_event_t *event) {
    int changed = 0;
    switch (event->type) {
        case PROTO_EVT_SYSTEM_STATE:
            if (alarm_state.system_state != event->data.system_state.state) {
                alarm_state.system_state = event->data.system_state.state;
                changed = 1;
            }
            break;
        case PROTO_EVT_ZONE_ACTIVITY:
            if (alarm_state.active_zones != event->data.zone_activity.active_zones ||
                alarm_state.triggered_zones != event->data.zone_activity.triggered_zones) {
                alarm_state.active_zones = event->data.zone_activity.active_zones;
                alarm_state.triggered_zones = event->data.zone_activity.triggered_zones;
                changed = 1;
            }
            break;
        default:
            break;
    }
    if (changed) {
        notify_state_changed();
    }
}

void alarm_manager_init(void) {
    on_event(protocol_event_handler);
}

void alarm_manager_on_state_changed(alarm_state_changed_callback_t cb) {
    state_changed_callback = cb;
}

const alarm_state_t* alarm_manager_get_state(void) {
    return &alarm_state;
}
