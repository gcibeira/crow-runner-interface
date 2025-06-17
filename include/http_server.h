#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "esp_err.h"
#include "esp_http_server.h"

/**
 * @brief Inicia el servidor HTTP para actualizaciones OTA.
 * * El servidor ofrecerá una página web en la raíz ("/") para subir el firmware
 * y un endpoint en "/ota_update" para manejar la subida del firmware.
 * * @return esp_err_t 
 * - ESP_OK: Si el servidor se inició correctamente.
 * - ESP_FAIL: Si hubo un error al iniciar el servidor.
 */
esp_err_t http_server_start(void);

/**
 * @brief Devuelve el handle del servidor HTTP actual.
 */
httpd_handle_t* get_http_server_handle(void);

/**
 * @brief Detiene el servidor HTTP OTA.
 * * (Opcional: podrías querer implementarlo si necesitas detener el servidor dinámicamente)
 */
// void http_server_stop(void);

#endif // HTTP_SERVER_H