#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include <stdint.h>
#include "frame_parser.h"

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

typedef struct {
    protocol_event_type_t type;
    union {
        struct { uint8_t key_code; } keypad;
        struct { uint8_t light_id; uint8_t state; } lights;
        // Agrega aquí más estructuras según tus necesidades
    } data;
    const frame_t *raw_frame;
} protocol_event_t;

typedef void (*protocol_event_callback_t)(const protocol_event_t *event);

void protocol_handler_init(void);
void protocol_handler_register_callback(protocol_event_callback_t cb);

#endif // PROTOCOL_HANDLER_H