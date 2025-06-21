#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi_manager.h"
#include "http_server.h"
#include <esp_netif_types.h>
#include "esp_spiffs.h"
#include "alarm_manager.h"

#define UDP_LOGGING_IP   "192.168.100.11"
#define UDP_LOGGING_PORT 5000

static const char *TAG_MAIN = "APP_MAIN";

void app_main(void) {
    // Initialize NVS Flash
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_MAIN, "Error initializing NVS.");
        return;
    }
    
    // Initialize SPIFFS File System
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/www",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    esp_err_t ret_spiffs = esp_vfs_spiffs_register(&conf);
    if (ret_spiffs != ESP_OK) {
        ESP_LOGE(TAG_MAIN, "Error initializing SPIFFS (%s)", esp_err_to_name(ret_spiffs));
        return;
    }

    // Initialize Wifi Manager
    ESP_ERROR_CHECK(wifi_manager_init_sta());
    
    // Start HTTP Server
    ESP_ERROR_CHECK(http_server_start());

    // Initialize Alarm manager
    ESP_ERROR_CHECK(alarm_manager_init());

    ESP_LOGI(TAG_MAIN, "System started and waiting for events...");
}