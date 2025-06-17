#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "esp_err.h"
#include "esp_http_server.h"

/**
 * @brief Starts the HTTP server.
 * * The server exposes a web page at the root ("/"), a page to upload firmware ("/ota"),
 * an endpoint at "/ota_update" to handle firmware upload, an endpoint to get the
 * alarm state ("/alarm"), and a WebSocket for alarm notifications ("/ws_alarm").
 * * @return esp_err_t
 * - ESP_OK: If the server started successfully.
 * - ESP_FAIL: If there was an error starting the server.
 */
esp_err_t http_server_start(void);

/**
 * @brief Returns the current HTTP server handle.
 */
httpd_handle_t get_http_server_handle(void);

/**
 * @brief Stops the HTTP OTA server.
 * * (Optional: you may want to implement this if you need to stop the server dynamically)
 */
// void http_server_stop(void);

#endif // HTTP_SERVER_H