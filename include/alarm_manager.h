#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

#include <stdint.h>
#include "protocol_handler.h"
#include "frame_handler.h"
#include "esp_err.h"
#include "secrets.h"

// Structure to maintain the alarm state
typedef struct {
    system_state_t system_state;
    uint8_t active_zones;
    uint8_t triggered_zones;
} alarm_state_t;

// Callback to notify alarm state changes
typedef void (*alarm_state_changed_callback_t)(const alarm_state_t* state);

// Initializes the alarm manager and frame handler internally with the specified pins
esp_err_t alarm_manager_init();

// Gets the current alarm state
const alarm_state_t* alarm_manager_get_state(void);

// Registers a callback that will be called when the alarm state changes
void alarm_manager_on_state_changed(alarm_state_changed_callback_t cb);

// Stubs para activar/desactivar alarma desde comandos
void alarm_manager_activate(void);
void alarm_manager_deactivate(void);

// Prototipo para enviar una secuencia de frames
void alarm_manager_send_frame_sequence(const frame_t* frames, size_t count);

#endif // ALARM_MANAGER_H
