#include "http_server.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "protocol_handler.h"
#include "alarm_manager.h"
#include <inttypes.h>
#include <stdbool.h>
// Handler headers
#include "handlers/root_handler.h"
#include "handlers/ota_handler.h"
#include "handlers/alarm_handler.h"
#include "handlers/ws_alarm_handler.h"
#include "handlers/ota_update_handler.h"
#include "handlers/notify_alarm_ws_clients.h"

static httpd_handle_t server = NULL;

// Expose server for notify_alarm_ws_clients
httpd_handle_t* get_http_server_handle() { return &server; }

esp_err_t http_server_start(void) {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 8;
  config.lru_purge_enable = true;

  ESP_LOGI("HTTP_SERVER", "Starting HTTP server on port: '%d'", config.server_port);
  esp_err_t ret = httpd_start(&server, &config);
  if (ret == ESP_OK) {
    httpd_uri_t uri_root_get = {.uri = "/", .method = HTTP_GET, .handler = root_get_handler, .user_ctx = NULL};
    httpd_register_uri_handler(server, &uri_root_get);

    httpd_uri_t uri_ota_get = {.uri = "/ota", .method = HTTP_GET, .handler = ota_get_handler, .user_ctx = NULL};
    httpd_register_uri_handler(server, &uri_ota_get);

    httpd_uri_t uri_alarm_get = {.uri = "/alarm", .method = HTTP_GET, .handler = alarm_get_handler, .user_ctx = NULL};
    httpd_register_uri_handler(server, &uri_alarm_get);

    httpd_uri_t uri_ws_alarm = {
        .uri = "/ws_alarm", .method = HTTP_GET, .handler = ws_alarm_handler, .is_websocket = true, .user_ctx = NULL};
    httpd_register_uri_handler(server, &uri_ws_alarm);

    httpd_uri_t uri_ota_update_post = {
        .uri = "/ota_update", .method = HTTP_POST, .handler = ota_update_post_handler, .user_ctx = NULL};
    httpd_register_uri_handler(server, &uri_ota_update_post);

    alarm_manager_on_state_changed(alarm_state_changed_callback);
    ESP_LOGI("HTTP_SERVER", "HTTP server started and handlers registered.");
    return ESP_OK;
  }

  ESP_LOGE("HTTP_SERVER", "Error starting HTTP server: %s", esp_err_to_name(ret));
  return ESP_FAIL;
}