#include "udp_logging.h"
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "esp_log.h"

static int udp_sock = -1;
static struct sockaddr_in dest_addr;
static const char *TAG = "UDP_LOG";

esp_err_t udp_logging_init(const char *ip, uint16_t port) {
    if (udp_sock != -1) {
        close(udp_sock);
        udp_sock = -1;
    }
    udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (udp_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return ESP_FAIL;
    }
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &dest_addr.sin_addr.s_addr);
    ESP_LOGI(TAG, "UDP socket created to %s:%d", ip, port);
    return ESP_OK;
}

esp_err_t udp_logging_send(const void *data, size_t len) {
    if (udp_sock < 0) return ESP_FAIL;
    int sent = sendto(udp_sock, data, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (sent < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        return ESP_FAIL;
    }
    return ESP_OK;
}

void udp_logging_close() {
    if (udp_sock != -1) {
        close(udp_sock);
        udp_sock = -1;
    }
}
