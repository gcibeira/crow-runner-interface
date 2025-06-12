#pragma once
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

esp_err_t udp_logging_init(const char *ip, uint16_t port);
esp_err_t udp_logging_send(const void *data, size_t len);
void udp_logging_close(void);
