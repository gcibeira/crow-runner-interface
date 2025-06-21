#ifndef FRAME_HANDLER_H
#define FRAME_HANDLER_H

#include "esp_err.h"
#include <stdint.h>

#define DAT_RX_PIN 23
#define DAT_TX_PIN 19
#define CLK_PIN 18

#define RX_FRAME_BUFFER_SIZE 16
#define RX_FRAME_QUEUE_LENGTH 8
#define TX_BITSTREAM_SIZE (RX_FRAME_BUFFER_SIZE * 10 + 16) // 8 bits + 2 stuffed bits per byte + 16 bits for delimiters
#define DAT_IDLE_LEVEL 0
#define TIME_BETWEEN_FRAMES_MS 250

typedef struct {
  uint8_t data[RX_FRAME_BUFFER_SIZE];
  uint8_t length;
} frame_t;

/**
 * @brief Initializes the serial frame handler via GPIO.
 * @return ESP_OK if successful, ESP-IDF error code otherwise.
 */
esp_err_t frame_handler_init();

/**
 * @brief Adds a frame to the TX frame queue, adding delimiters and stuffed bits.
 * @param frame Pointer to the frame to be transmitted.
 */
void frame_handler_send(const frame_t *frame);

#endif // FRAME_HANDLER_H