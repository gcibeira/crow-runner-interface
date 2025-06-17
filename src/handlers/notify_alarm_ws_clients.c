#include "handlers/notify_alarm_ws_clients.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include <string.h>
#include "http_server.h"

void notify_alarm_ws_clients(const alarm_state_t* state) {
  httpd_handle_t server = *get_http_server_handle();
  if (!server)
    return;

  char msg[64];
  snprintf(msg, sizeof(msg), "{\"system_state\":%d,\"active_zones\":%u,\"triggered_zones\":%u}",
           state->system_state, state->active_zones, state->triggered_zones);
  httpd_ws_frame_t ws_pkt = {.type = HTTPD_WS_TYPE_TEXT, .payload = (uint8_t *)msg, .len = strlen(msg)};

  size_t max_clients = CONFIG_LWIP_MAX_SOCKETS;
  int client_fds[CONFIG_LWIP_MAX_SOCKETS];
  esp_err_t err = httpd_get_client_list(server, &max_clients, client_fds);
  if (err != ESP_OK) {
    ESP_LOGE("HTTP_SERVER", "httpd_get_client_list failed: %s", esp_err_to_name(err));
    return;
  }
  ESP_LOGI("HTTP_SERVER", "Sending notification to %u clients", (unsigned)max_clients);

  for (size_t i = 0; i < max_clients; ++i) {
    err = httpd_ws_send_frame_async(server, client_fds[i], &ws_pkt);
    if (err != ESP_OK) {
      ESP_LOGW("HTTP_SERVER", "Error sending WS frame on fd %d: %s", client_fds[i], esp_err_to_name(err));
    }
  }
}

void alarm_state_changed_callback(const alarm_state_t* state) {
  notify_alarm_ws_clients(state);
}
