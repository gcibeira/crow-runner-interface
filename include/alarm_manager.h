#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

#include <stdint.h>
#include "protocol_handler.h"

// Estructura para mantener el estado de la alarma
typedef struct {
    system_state_t system_state;
    uint8_t active_zones;
    uint8_t triggered_zones;
} alarm_state_t;

// Callback para notificar cambios de estado de la alarma
typedef void (*alarm_state_changed_callback_t)(const alarm_state_t* state);

// Inicializa el alarm manager y registra los callbacks necesarios
void alarm_manager_init(void);

// Obtiene el estado actual de la alarma
const alarm_state_t* alarm_manager_get_state(void);

// Registra un callback que será llamado cuando cambie el estado de la alarma
void alarm_manager_on_state_changed(alarm_state_changed_callback_t cb);

#endif // ALARM_MANAGER_H
