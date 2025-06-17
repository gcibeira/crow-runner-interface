#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include <stdint.h>
#include "frame_handler.h"

// Definition of protocol event types
typedef enum {
  PROTO_EVT_KEYPAD,
  PROTO_EVT_LIGHTS,
  PROTO_EVT_BEEP,
  PROTO_EVT_MODE,
  PROTO_EVT_ZONE_ACTIVITY,
  PROTO_EVT_ZONE_BYPASS,
  PROTO_EVT_EVENT_MEMORY,
  PROTO_EVT_CLOCK,
  PROTO_EVT_SYSTEM_STATE,
  PROTO_EVT_UNKNOWN
} protocol_event_type_t;

// Enum for system states
typedef enum {
  SYSTEM_STATE_DISARMED,
  SYSTEM_STATE_ARMED,
  SYSTEM_STATE_ARMING,
  SYSTEM_STATE_STAY,
  SYSTEM_STATE_UNKNOWN
} system_state_t;

// Structure for system state
typedef struct {
  system_state_t state;
} system_state_event_t;

// Structure for zone activity
typedef struct {
  uint8_t active_zones;    // bitmap
  uint8_t triggered_zones; // bitmap
} zone_activity_event_t;

// Structure for keypad event
typedef struct {
  uint8_t key_code;
} keypad_event_t;

// Structure for protocol events
typedef struct {
  protocol_event_type_t type;
  union {
    keypad_event_t keypad;
    system_state_event_t system_state;
    zone_activity_event_t zone_activity;
  } data;
} protocol_event_t;


void process_frame(const frame_t *frame);

// Declaración de callbacks para eventos del protocolo
typedef void (*protocol_event_callback_t)(const protocol_event_t *event);
typedef void (*keypad_event_callback_t)(const keypad_event_t *event);
typedef void (*system_state_event_callback_t)(const system_state_event_t *event);
typedef void (*zone_activity_event_callback_t)(const zone_activity_event_t *event);

void on_event(protocol_event_callback_t cb);
void on_keypad(keypad_event_callback_t cb);
void on_system_state(system_state_event_callback_t cb);
void on_zone_activity(zone_activity_event_callback_t cb);

#endif // PROTOCOL_HANDLER_H