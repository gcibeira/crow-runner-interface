#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include "frame_handler.h"
#include <stdint.h>

// Definición de tipos de eventos del protocolo
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

// Enum para los estados del sistema
typedef enum {
  SYSTEM_STATE_DISARMED,
  SYSTEM_STATE_ARMED,
  SYSTEM_STATE_ARMING,
  SYSTEM_STATE_STAY,
  SYSTEM_STATE_UNKNOWN
} system_state_t;

// Estructura para estado del sistema
typedef struct {
  system_state_t state;
} system_state_event_t;

// Estructura para actividad de zonas
typedef struct {
  uint8_t active_zones;    // bitmap
  uint8_t triggered_zones; // bitmap
} zone_activity_event_t;

// Estructura para evento de teclado
typedef struct {
  uint8_t key_code;
} keypad_event_t;

// Estructura para eventos del protocolo
typedef struct {
  protocol_event_type_t type;
  union {
    keypad_event_t keypad;
    system_state_event_t system_state;
    zone_activity_event_t zone_activity;
  } data;
} protocol_event_t;

typedef void (*protocol_event_callback_t)(const protocol_event_t *event);

// Callbacks específicos por tipo de evento
typedef void (*keypad_event_callback_t)(const keypad_event_t *event);
typedef void (*system_state_event_callback_t)(const system_state_event_t *event);
typedef void (*zone_activity_event_callback_t)(const zone_activity_event_t *event);

void protocol_handler_init(void);
void on_event(protocol_event_callback_t cb);
void on_keypad(keypad_event_callback_t cb);
void on_system_state(system_state_event_callback_t cb);
void on_zone_activity(zone_activity_event_callback_t cb);

static void dispatch_event(const protocol_event_t *event);


#endif // PROTOCOL_HANDLER_H