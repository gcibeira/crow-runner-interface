#include "http_server.h"
#include "esp_flash_partitions.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "freertos/FreeRTOS.h" // Para vTaskDelay
#include "freertos/task.h"     // Para vTaskDelay
#include <inttypes.h>

static const char *TAG = "HTTP_SERVER";

#define OTA_BUF_SIZE 2048 // Tamaño del buffer para leer los datos del firmware
static char ota_write_data[OTA_BUF_SIZE + 1] = {
    0}; // Buffer para chunks de firmware

// --- Manejador para la carga del firmware OTA ---
static esp_err_t ota_update_post_handler(httpd_req_t *req) {
  esp_ota_handle_t ota_handle = 0;
  const esp_partition_t *update_partition = NULL;
  int received_len = 0;
  int total_len = req->content_len;

  ESP_LOGI(TAG,
           "Iniciando actualización OTA. Tamaño total del firmware: %d bytes",
           total_len);

  update_partition = esp_ota_get_next_update_partition(NULL);
  if (update_partition == NULL) {
    ESP_LOGE(TAG, "Fallo al obtener la partición de actualización.");
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "Error de partición OTA");
    return ESP_FAIL;
  }
  ESP_LOGI(TAG, "Escribiendo en la partición: subtype %d, offset 0x%" PRIX32,
           update_partition->subtype, update_partition->address);

  // Usar OTA_SIZE_UNKNOWN si el Content-Length no es fiable o no está presente
  // pero con el script JS lo estamos enviando.
  esp_err_t err = esp_ota_begin(update_partition, total_len, &ota_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error en esp_ota_begin: %s", esp_err_to_name(err));
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "Error al iniciar OTA");
    return ESP_FAIL;
  }
  ESP_LOGI(TAG, "esp_ota_begin exitoso.");

  int data_read;
  while (true) {
    data_read = httpd_req_recv(req, ota_write_data, OTA_BUF_SIZE);
    if (data_read < 0) { // Error en la recepción
      if (data_read == HTTPD_SOCK_ERR_TIMEOUT) {
        ESP_LOGW(TAG, "Timeout al recibir datos. Si el archivo es grande, "
                      "considera aumentar el timeout del socket.");
        // Continuar si aún no se ha recibido todo. Si es persistente, fallará.
        // El servidor HTTPD tiene timeouts configurables.
        continue;
      }
      ESP_LOGE(TAG, "Error al recibir datos del firmware: %d", data_read);
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                          "Error de recepción de datos");
      esp_ota_abort(ota_handle);
      return ESP_FAIL;
    } else if (data_read ==
               0) { // Conexión cerrada por el cliente (o fin de datos)
      if (received_len < total_len) {
        ESP_LOGE(TAG, "Recepción incompleta. Recibidos %d de %d bytes.",
                 received_len, total_len);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Firmware incompleto");
        esp_ota_abort(ota_handle);
        return ESP_FAIL;
      }
      ESP_LOGI(TAG,
               "Recepción completada por cierre de stream. Total %d bytes.",
               received_len);
      break; // Salir del bucle
    }
    // data_read > 0
    err = esp_ota_write(ota_handle, (const void *)ota_write_data, data_read);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Error en esp_ota_write: %s", esp_err_to_name(err));
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                          "Error al escribir datos OTA");
      esp_ota_abort(ota_handle);
      return ESP_FAIL;
    }
    received_len += data_read;
    ESP_LOGD(TAG, "Escritos %d bytes, total %d/%d", data_read, received_len,
             total_len);

    if (received_len == total_len) {
      ESP_LOGI(TAG, "Recepción completada. Total de bytes recibidos: %d",
               received_len);
      break; // Salir del bucle
    }
  }

  err = esp_ota_end(ota_handle);
  if (err != ESP_OK) {
    if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
      ESP_LOGE(TAG,
               "La validación de la imagen falló (checksum incorrecto, etc.)");
    }
    ESP_LOGE(TAG, "Error en esp_ota_end: %s", esp_err_to_name(err));
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "Error al finalizar OTA (validación)");
    return ESP_FAIL;
  }
  ESP_LOGI(TAG, "esp_ota_end exitoso.");

  err = esp_ota_set_boot_partition(update_partition);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error en esp_ota_set_boot_partition: %s",
             esp_err_to_name(err));
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "Error al establecer partición de arranque");
    return ESP_FAIL;
  }
  ESP_LOGI(TAG, "esp_ota_set_boot_partition exitoso.");

  const char *resp_str = "Actualizacion OTA exitosa! Reiniciando ESP32...";
  httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
  ESP_LOGI(TAG, "%s", resp_str);

  vTaskDelay(pdMS_TO_TICKS(2000));
  esp_restart();

  return ESP_OK;
}

// --- Manejador para servir la página HTML ---
static esp_err_t root_get_handler(httpd_req_t *req) {
  ESP_LOGI(TAG, "Sirviendo página HTML de OTA");

  FILE *f = fopen("/www/index.html", "r");
  if (!f) {
    ESP_LOGE(TAG, "No se pudo abrir /index.html");
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No se pudo abrir index.html");
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "text/html");
  char buf[1024];
  size_t n;
  while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
    if (httpd_resp_send_chunk(req, buf, n) != ESP_OK) {
      ESP_LOGE(TAG, "Error enviando chunk HTML");
      fclose(f);
      httpd_resp_sendstr_chunk(req, NULL); // Terminar respuesta
      return ESP_FAIL;
    }
  }
  fclose(f);
  httpd_resp_sendstr_chunk(req, NULL); // Terminar respuesta
  return ESP_OK;
}

static httpd_handle_t server = NULL;

esp_err_t http_server_start(void) {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 8;
  config.lru_purge_enable = true;
  // Aumentar el timeout de recepción si se esperan subidas largas y lentas
  // config.recv_wait_timeout = 10; // Segundos
  // config.send_wait_timeout = 10; // Segundos

  ESP_LOGI(TAG, "Iniciando servidor HTTP en el puerto: '%d'",
           config.server_port);
  esp_err_t ret = httpd_start(&server, &config);
  if (ret == ESP_OK) {
    httpd_uri_t uri_root_get = {.uri = "/",
                                .method = HTTP_GET,
                                .handler = root_get_handler,
                                .user_ctx = NULL};
    httpd_register_uri_handler(server, &uri_root_get);

    httpd_uri_t uri_ota_update_post = {.uri = "/ota_update",
                                       .method = HTTP_POST,
                                       .handler = ota_update_post_handler,
                                       .user_ctx = NULL};
    httpd_register_uri_handler(server, &uri_ota_update_post);
    ESP_LOGI(TAG, "Servidor HTTP iniciado y handlers registrados.");
    return ESP_OK;
  }

  ESP_LOGE(TAG, "Error al iniciar el servidor HTTP: %s", esp_err_to_name(ret));
  return ESP_FAIL;
}

// void http_server_stop(void) {
//     if (server) {
//         httpd_stop(server);
//         ESP_LOGI(TAG, "Servidor HTTP detenido.");
//         server = NULL;
//     }
// }