#ifndef FRAME_HANDLER_H
#define FRAME_HANDLER_H

#include "esp_err.h"
#include <stdint.h>

#define FRAME_BUFFER_SIZE 16
#define FRAME_QUEUE_LENGTH 8
#define MAX_BITSTREAM_BITS (FRAME_BUFFER_SIZE * 10 + 16)
#define DAT_RX_PIN 23
#define DAT_TX_PIN 19
#define CLK_PIN 18
#define DAT_IDLE_LEVEL 0

typedef struct {
  uint8_t data[FRAME_BUFFER_SIZE];
  uint8_t length;
} frame_t;

/**
 * @brief Initializes the serial frame handler via GPIO.
 * @return ESP_OK if successful, ESP-IDF error code otherwise.
 */
esp_err_t frame_handler_init();

/**
 * @brief Sends a frame over the serial bus, adding delimiters and stuffed bits.
 * @param frame Pointer to the frame to be sent.
 */
void frame_handler_send(const frame_t *frame);

#endif // FRAME_HANDLER_H