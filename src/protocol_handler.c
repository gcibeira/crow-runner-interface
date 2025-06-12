#include "protocol_handler.h"
#include <string.h>

static protocol_event_callback_t app_callback = NULL;

static void protocol_handler_frame_callback(const frame_t *frame) {
    protocol_event_t event = {0};
    event.raw_frame = frame;

    // Ejemplo simple: identifica tipo por primer byte
    switch (frame->data[0]) {
        case 0xd1:
            event.type = PROTO_EVT_KEYPAD;
            event.data.keypad.key_code = frame->data[2];
            break;
        case 0x10:
            event.type = PROTO_EVT_LIGHTS;
            event.data.lights.light_id = frame->data[4];
            event.data.lights.state = frame->data[3];
            break;
        // Agrega más casos según tu protocolo
        default:
            event.type = PROTO_EVT_UNKNOWN;
            break;
    }

    if (app_callback) app_callback(&event);
}

void protocol_handler_init(void) {
    frame_parser_register_callback(protocol_handler_frame_callback);
}

void protocol_handler_register_callback(protocol_event_callback_t cb) {
    app_callback = cb;
}