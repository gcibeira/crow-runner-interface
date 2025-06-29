#include "protocol_handler.h"

static protocol_event_callback_t protocol_event_callback = NULL;
static keypad_event_callback_t keypad_callback = NULL;
static system_state_event_callback_t system_state_callback = NULL;
static zone_activity_event_callback_t zone_activity_callback = NULL;

static void dispatch_event(protocol_event_t *event) {
  // Generic callback for all protocol events
  if (protocol_event_callback) {
    protocol_event_callback(event);
  }
  // Specific callbacks for each event type
  switch (event->type) {
    case PROTO_EVT_KEYPAD:
      if (keypad_callback) {
        keypad_callback(&event->data.keypad);
      }
      break;
    case PROTO_EVT_SYSTEM_STATE:
      if (system_state_callback) {
        system_state_callback(&event->data.system_state);
      }
      break;
    case PROTO_EVT_ZONE_ACTIVITY:
      if (zone_activity_callback) {
        zone_activity_callback(&event->data.zone_activity);
      }
      break;
    default:
      break;
  }
}

void process_frame(const frame_t *frame) {
  protocol_event_t event = {0};

  switch (frame->data[0]) {
  case 0xd1:
    event.type = PROTO_EVT_KEYPAD;
    event.data.keypad.key_code = frame->data[2];
    break;
  case 0x11:
    event.type = PROTO_EVT_SYSTEM_STATE;
    if (frame->data[1] == 0x00 && frame->data[2] == 0x00 && frame->data[3] == 0x00 && frame->data[4] == 0x00) {
      event.data.system_state.state = SYSTEM_STATE_DISARMED;
    } else if (frame->data[1] == 0x01) {
      event.data.system_state.state = SYSTEM_STATE_ARMED;
    } else if (frame->data[2] == 0x01) {
      event.data.system_state.state = SYSTEM_STATE_ARMING;
    } else if (frame->data[3] == 0x01) {
      event.data.system_state.state = SYSTEM_STATE_STAY;
    } else {
      event.data.system_state.state = SYSTEM_STATE_UNKNOWN;
    }
    break;
  case 0x12:
    event.type = PROTO_EVT_ZONE_ACTIVITY;
    event.data.zone_activity.active_zones = frame->data[2];
    event.data.zone_activity.triggered_zones = frame->data[3];
    break;
  default:
    event.type = PROTO_EVT_UNKNOWN;
    break;
  }
  
  dispatch_event(&event);
}

void on_event(protocol_event_callback_t cb) { protocol_event_callback = cb; }

void on_keypad(keypad_event_callback_t cb) { keypad_callback = cb; }

void on_system_state(system_state_event_callback_t cb) { system_state_callback = cb; }

void on_zone_activity(zone_activity_event_callback_t cb) { zone_activity_callback = cb; }
