#include "alarm_manager.h"
#include "frame_handler.h"
#include <stddef.h>

static alarm_state_t alarm_state = {.system_state = SYSTEM_STATE_UNKNOWN, .active_zones = 0, .triggered_zones = 0};
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

esp_err_t alarm_manager_init() {
  esp_err_t err = frame_handler_init();
  if (err != ESP_OK) {
    return err;
  }
  on_event(protocol_event_handler);
  return ESP_OK;
}

void alarm_manager_on_state_changed(alarm_state_changed_callback_t cb) { state_changed_callback = cb; }

const alarm_state_t *alarm_manager_get_state(void) { return &alarm_state; }

void alarm_manager_send_frame_sequence(const frame_t *frames, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    frame_handler_send(&frames[i]);
  }
}

void alarm_manager_activate(void) {
  frame_t frame = {.data = {0xd1, 0x00, 0x0e}, .length = 3};
  frame_handler_send(&frame);
}

void alarm_manager_deactivate(void) {
  // Send user code
  uint8_t code[ALARM_USER_CODE_LENGTH] = ALARM_USER_CODE;
  frame_t frames[ALARM_USER_CODE_LENGTH + 1];
  for (size_t i = 0; i < ALARM_USER_CODE_LENGTH; ++i) {
    frames[i].data[0] = 0xd1;
    frames[i].data[1] = 0x00;
    frames[i].data[2] = code[i];
    frames[i].length = 3;
  }
  // Enter key (código 0x11)
  frames[ALARM_USER_CODE_LENGTH].data[0] = 0xd1;
  frames[ALARM_USER_CODE_LENGTH].data[1] = 0x00;
  frames[ALARM_USER_CODE_LENGTH].data[2] = 0x11;
  frames[ALARM_USER_CODE_LENGTH].length = 3;
  alarm_manager_send_frame_sequence(frames, ALARM_USER_CODE_LENGTH + 1);
}
