#ifndef FRAME_HANDLER_H
#define FRAME_HANDLER_H

#include <stdint.h>
#include "esp_err.h"

#define FRAME_BUFFER_SIZE 16
#define FRAME_QUEUE_LENGTH 8

typedef struct {
  uint8_t data[FRAME_BUFFER_SIZE];
  uint8_t length;
} frame_t;

typedef void (*frame_callback_t)(const frame_t *frame);

/**
 * @brief Initializes the serial frame handler via GPIO.
 * @param data_pin Data GPIO pin.
 * @param clk_pin Clock GPIO pin.
 * @return ESP_OK if successful, ESP-IDF error code otherwise.
 */
esp_err_t frame_handler_init(int data_pin, int clk_pin);

/**
 * @brief Registers a callback that will be called with each received frame.
 * @param callback Callback function.
 */
void on_frame(frame_callback_t callback);

#endif // FRAME_HANDLER_H