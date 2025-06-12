#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief Inicializa el Wi-Fi en modo estación y se conecta a la red configurada.
 *
 * Esta función bloqueará hasta que la conexión sea exitosa o se alcance el número máximo de reintentos.
 * * @return esp_err_t 
 * - ESP_OK: Si la conexión Wi-Fi fue exitosa.
 * - ESP_FAIL: Si no se pudo conectar después de los reintentos.
 * - Otros códigos de error de ESP-IDF en caso de fallos de inicialización.
 */
esp_err_t wifi_manager_init_sta(void);

/**
 * @brief Verifica si el ESP32 está actualmente conectado a la red Wi-Fi.
 * * @return true si está conectado, false en caso contrario.
 */
bool wifi_manager_is_connected(void);

#endif // WIFI_MANAGER_H