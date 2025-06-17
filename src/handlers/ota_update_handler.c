#include "handlers/ota_update_handler.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

#define OTA_BUF_SIZE 2048
static char ota_write_data[OTA_BUF_SIZE + 1] = {0};

esp_err_t ota_update_post_handler(httpd_req_t *req) {
  esp_ota_handle_t ota_handle = 0;
  const esp_partition_t *update_partition = NULL;
  int received_len = 0;
  int total_len = req->content_len;

  ESP_LOGI("HTTP_SERVER", "Starting OTA update. Total firmware size: %d bytes", total_len);

  update_partition = esp_ota_get_next_update_partition(NULL);
  if (update_partition == NULL) {
    ESP_LOGE("HTTP_SERVER", "Failed to get update partition.");
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OTA partition error");
    return ESP_FAIL;
  }
  ESP_LOGI("HTTP_SERVER", "Writing to partition: subtype %d, offset 0x%" PRIX32, update_partition->subtype, update_partition->address);

  esp_err_t err = esp_ota_begin(update_partition, total_len, &ota_handle);
  if (err != ESP_OK) {
    ESP_LOGE("HTTP_SERVER", "esp_ota_begin error: %s", esp_err_to_name(err));
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Error starting OTA");
    return ESP_FAIL;
  }
  ESP_LOGI("HTTP_SERVER", "esp_ota_begin successful.");

  int data_read;
  while (true) {
    data_read = httpd_req_recv(req, ota_write_data, OTA_BUF_SIZE);
    if (data_read < 0) {
      if (data_read == HTTPD_SOCK_ERR_TIMEOUT) {
        ESP_LOGW("HTTP_SERVER", "Timeout receiving data. If the file is large, consider increasing socket timeout.");
        continue;
      }
      ESP_LOGE("HTTP_SERVER", "Error receiving firmware data: %d", data_read);
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Data reception error");
      esp_ota_abort(ota_handle);
      return ESP_FAIL;
    } else if (data_read == 0) {
      if (received_len < total_len) {
        ESP_LOGE("HTTP_SERVER", "Incomplete reception. Received %d of %d bytes.", received_len, total_len);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Incomplete firmware");
        esp_ota_abort(ota_handle);
        return ESP_FAIL;
      }
      ESP_LOGI("HTTP_SERVER", "Reception completed by stream close. Total %d bytes.", received_len);
      break;
    }
    err = esp_ota_write(ota_handle, (const void *)ota_write_data, data_read);
    if (err != ESP_OK) {
      ESP_LOGE("HTTP_SERVER", "esp_ota_write error: %s", esp_err_to_name(err));
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OTA write error");
      esp_ota_abort(ota_handle);
      return ESP_FAIL;
    }
    received_len += data_read;
    ESP_LOGD("HTTP_SERVER", "Written %d bytes, total %d/%d", data_read, received_len, total_len);

    if (received_len == total_len) {
      ESP_LOGI("HTTP_SERVER", "Reception completed. Total bytes received: %d", received_len);
      break;
    }
  }

  err = esp_ota_end(ota_handle);
  if (err != ESP_OK) {
    if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
      ESP_LOGE("HTTP_SERVER", "Image validation failed (bad checksum, etc.)");
    }
    ESP_LOGE("HTTP_SERVER", "esp_ota_end error: %s", esp_err_to_name(err));
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OTA end error (validation)");
    return ESP_FAIL;
  }
  ESP_LOGI("HTTP_SERVER", "esp_ota_end successful.");

  err = esp_ota_set_boot_partition(update_partition);
  if (err != ESP_OK) {
    ESP_LOGE("HTTP_SERVER", "esp_ota_set_boot_partition error: %s", esp_err_to_name(err));
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Boot partition set error");
    return ESP_FAIL;
  }
  ESP_LOGI("HTTP_SERVER", "esp_ota_set_boot_partition successful.");

  const char *resp_str = "OTA update successful! Restarting ESP32...";
  httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
  ESP_LOGI("HTTP_SERVER", "%s", resp_str);

  vTaskDelay(pdMS_TO_TICKS(2000));
  esp_restart();

  return ESP_OK;
}
