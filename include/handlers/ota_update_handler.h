#ifndef HTTP_SERVER_OTA_UPDATE_HANDLER_H
#define HTTP_SERVER_OTA_UPDATE_HANDLER_H

#include "esp_err.h"
#include "esp_http_server.h"

esp_err_t ota_update_post_handler(httpd_req_t *req);

#endif // HTTP_SERVER_OTA_UPDATE_HANDLER_H
