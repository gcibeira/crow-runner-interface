#include "handlers/ws_alarm_handler.h"
#include "alarm_manager.h"
#include "esp_log.h"
#include <string.h>

esp_err_t ws_alarm_handler(httpd_req_t *req) {
  if (req->method == HTTP_GET) {
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    const alarm_state_t *state = alarm_manager_get_state();
    char msg[64];
    snprintf(msg, sizeof(msg), "{\"system_state\":%d,\"active_zones\":%u,\"triggered_zones\":%u}", state->system_state,
             state->active_zones, state->triggered_zones);
    ws_pkt.payload = (uint8_t *)msg;
    ws_pkt.len = strlen(msg);
    httpd_ws_send_frame(req, &ws_pkt);
    return ESP_OK;
  }

  httpd_ws_frame_t ws_pkt;
  memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
  ws_pkt.type = HTTPD_WS_TYPE_TEXT;

  esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
  if (ret != ESP_OK) {
    ESP_LOGE("HTTP_SERVER", "Error getting WS frame size: %s", esp_err_to_name(ret));
    return ret;
  }
  if (ws_pkt.len > 128)
    return ESP_FAIL;

  uint8_t buf[128] = {0};
  ws_pkt.payload = buf;
  ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
  if (ret != ESP_OK) {
    ESP_LOGE("HTTP_SERVER", "Error receiving WS frame: %s", esp_err_to_name(ret));
    return ret;
  }
  buf[ws_pkt.len] = 0;

  // Procesar comando recibido
  ws_alarm_handle_command((const char *)buf);

  return ESP_OK;
}

// Procesa comandos recibidos por WebSocket (ej: activar/desactivar alarma)
void ws_alarm_handle_command(const char *json) {
  if (strstr(json, "\"command\"") && strstr(json, "\"activate_alarm\"")) {
      alarm_manager_activate();
  } else if (strstr(json, "\"command\"") && strstr(json, "\"deactivate_alarm\"")) {
      alarm_manager_deactivate();
      } else {
    ESP_LOGW("WS_CMD", "Comando WebSocket desconocido o mal formado: %s", json);
  }
}
