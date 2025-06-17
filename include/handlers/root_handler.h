#ifndef HTTP_SERVER_ROOT_HANDLER_H
#define HTTP_SERVER_ROOT_HANDLER_H

#include "esp_err.h"
#include "esp_http_server.h"

esp_err_t root_get_handler(httpd_req_t *req);

#endif // HTTP_SERVER_ROOT_HANDLER_H
