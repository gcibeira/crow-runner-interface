#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "frame_parser.h"
#include "wifi_manager.h"
#include "http_server.h"
#include "udp_logging.h"
#include <esp_netif_types.h>
#include "esp_spiffs.h"
//#include "protocol_handler.h"

static const char *TAG_MAIN = "APP_MAIN";

#define UDP_LOGGING_IP   "192.168.100.11"
#define UDP_LOGGING_PORT 5000

static void frame_udp_callback(const frame_t *received_frame) {
    udp_logging_send(received_frame->data, received_frame->length);
}

/*static void app_protocol_event_callback(const protocol_event_t *event) {
    switch (event->type) {
        case PROTO_EVT_KEYPAD:
            ESP_LOGI(TAG_MAIN, "Tecla presionada: 0x%02x", event->data.keypad.key_code);
            break;
        case PROTO_EVT_LIGHTS:
            ESP_LOGI(TAG_MAIN, "Luz %d estado %d", event->data.lights.light_id, event->data.lights.state);
            break;
        default:
            ESP_LOGI(TAG_MAIN, "Evento desconocido");
            break;
    }
}*/

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

    ESP_LOGI(TAG_MAIN, "Inicializando Frame Parser...");
    ESP_ERROR_CHECK(udp_logging_init(UDP_LOGGING_IP, UDP_LOGGING_PORT));
    ESP_ERROR_CHECK(frame_parser_init(23, 18));
    frame_parser_register_callback(frame_udp_callback);
    ESP_LOGI(TAG_MAIN, "Sistema iniciado y esperando bits...");


    /*protocol_handler_init();
    protocol_handler_register_callback(app_protocol_event_callback);*/
}