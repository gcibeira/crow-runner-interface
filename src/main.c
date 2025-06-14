#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi_manager.h"
#include "http_server.h"
#include "udp_logging.h"
#include <esp_netif_types.h>
#include "esp_spiffs.h"
#include "frame_handler.h"
#include "protocol_handler.h"

static const char *TAG_MAIN = "APP_MAIN";

#define UDP_LOGGING_IP   "192.168.100.11"
#define UDP_LOGGING_PORT 5000

void zone_activity_callback(const zone_activity_event_t *event) {
    udp_logging_send("main: zone activity callback", 28);
}

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_MAIN, "Error inicializando NVS.");
        return;
    }
    ESP_LOGI(TAG_MAIN, "NVS inicializado.");
    
    ESP_LOGI(TAG_MAIN, "Inicializando SPIFFS...");
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/www",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    esp_err_t ret_spiffs = esp_vfs_spiffs_register(&conf);
    if (ret_spiffs != ESP_OK) {
        ESP_LOGE(TAG_MAIN, "Error inicializando SPIFFS (%s)", esp_err_to_name(ret_spiffs));
        return;
    }
    ESP_LOGI(TAG_MAIN, "SPIFFS inicializado correctamente.");
    
    ESP_LOGI(TAG_MAIN, "Inicializando Wi-Fi...");
    ESP_ERROR_CHECK(wifi_manager_init_sta());
    ESP_LOGI(TAG_MAIN, "Wi-Fi conectado.");

    ESP_LOGI(TAG_MAIN, "Iniciando servidor OTA HTTP...");
    ESP_ERROR_CHECK(http_server_start());
    ESP_LOGI(TAG_MAIN, "Servidor OTA HTTP iniciado correctamente.");

    ESP_LOGI(TAG_MAIN, "Inicializando Frame Handler...");
    ESP_ERROR_CHECK(udp_logging_init(UDP_LOGGING_IP, UDP_LOGGING_PORT));
    on_zone_activity(zone_activity_callback);
    protocol_handler_init();
    ESP_ERROR_CHECK(frame_handler_init(23, 18));

    ESP_LOGI(TAG_MAIN, "Sistema iniciado y esperando bits...");
}