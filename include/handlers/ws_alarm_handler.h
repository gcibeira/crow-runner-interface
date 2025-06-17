#ifndef HTTP_SERVER_WS_ALARM_HANDLER_H
#define HTTP_SERVER_WS_ALARM_HANDLER_H

#include "esp_err.h"
#include "esp_http_server.h"

esp_err_t ws_alarm_handler(httpd_req_t *req);

#endif // HTTP_SERVER_WS_ALARM_HANDLER_H
