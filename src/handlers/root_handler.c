#include "handlers/root_handler.h"
#include "esp_log.h"
#include <stdio.h>

esp_err_t root_get_handler(httpd_req_t *req) {
  ESP_LOGI("HTTP_SERVER", "Serving OTA HTML page");

  FILE *f = fopen("/www/index.html", "r");
  if (!f) {
    ESP_LOGE("HTTP_SERVER", "Could not open /index.html");
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Could not open index.html");
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "text/html");
  char buf[1024];
  size_t n;
  while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
    if (httpd_resp_send_chunk(req, buf, n) != ESP_OK) {
      ESP_LOGE("HTTP_SERVER", "Error sending HTML chunk");
      fclose(f);
      httpd_resp_sendstr_chunk(req, NULL); // End response
      return ESP_FAIL;
    }
  }
  fclose(f);
  httpd_resp_sendstr_chunk(req, NULL); // End response
  return ESP_OK;
}
