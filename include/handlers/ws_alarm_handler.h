#ifndef HTTP_SERVER_WS_ALARM_HANDLER_H
#define HTTP_SERVER_WS_ALARM_HANDLER_H

#include "esp_err.h"
#include "esp_http_server.h"

esp_err_t ws_alarm_handler(httpd_req_t *req);

// Procesa comandos recibidos por WebSocket (ej: activar/desactivar alarma)
void ws_alarm_handle_command(const char* command);

#endif // HTTP_SERVER_WS_ALARM_HANDLER_H
