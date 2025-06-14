#include "frame_handler.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

static const char *TAG = "FRAME_HANDLER";
static QueueHandle_t frame_queue;
static int clk_pin = -1;
static int data_pin = -1;
static frame_callback_t frame_callback = NULL;

void on_frame(frame_callback_t callback) { frame_callback = callback; }

static void IRAM_ATTR frame_process_bit(uint8_t incoming_bit) {
  static volatile bool in_frame = false;
  static volatile uint8_t isr_accumulated_byte = 0;
  static volatile uint8_t isr_bit_count = 0;
  static volatile uint8_t consecutive_ones = 0;
  static volatile frame_t current_frame;

  // Delimiter detection logic
  if (incoming_bit == 1) {
    if (consecutive_ones < 7)
      consecutive_ones++;
    else
      in_frame = false;
  } else {
    // Stuffed bit detected
    if (consecutive_ones == 5) {
      consecutive_ones = 0;
      return;
    }
    // Delimiter detected
    if (consecutive_ones == 6) {
      // End delimiter. Frame complete
      // TODO: Verify if there's residual bits in isr_bit_count
      if (in_frame && current_frame.length > 0) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(frame_queue, (const void *)&current_frame, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken)
          portYIELD_FROM_ISR();
      }

      in_frame = !in_frame;
      consecutive_ones = 0;
      isr_accumulated_byte = 0;
      isr_bit_count = 0;
      current_frame.length = 0;
      return;
    }
    consecutive_ones = 0;
  }

  if (!in_frame)
    return;

  // Accumulate bits into a byte
  isr_accumulated_byte = (isr_accumulated_byte >> 1) | (incoming_bit << 7 & 0x80);
  isr_bit_count++;

  // Store byte in frame buffer
  if (isr_bit_count == 8) {
    uint8_t incoming_byte = isr_accumulated_byte;
    isr_accumulated_byte = 0;
    isr_bit_count = 0;
    if (current_frame.length < FRAME_BUFFER_SIZE)
      current_frame.data[current_frame.length++] = incoming_byte;
    else
      in_frame = false;
  }
}

static void IRAM_ATTR clk_isr_handler(void *arg) {
  uint8_t incoming_bit = (uint8_t)(gpio_get_level(data_pin));
  frame_process_bit(incoming_bit);
}

static void frame_processor_task(void *arg) {
  frame_t received_frame;

  while (true) {
    if (xQueueReceive(frame_queue, &received_frame, portMAX_DELAY) == pdPASS) {
      if (frame_callback && received_frame.length > 0)
        frame_callback(&received_frame);
    }
  }
}

esp_err_t frame_handler_init(int data_pin_param, int clk_pin_param) {
  if (data_pin_param < 0 || clk_pin_param < 0 || data_pin_param == clk_pin_param) {
    ESP_LOGE(TAG, "Invalid GPIO pin numbers");
    return ESP_ERR_INVALID_ARG;
  }

  data_pin = data_pin_param;
  clk_pin = clk_pin_param;

  gpio_config_t io_conf_data = {.pin_bit_mask = (1ULL << data_pin),
                                .mode = GPIO_MODE_INPUT,
                                .pull_up_en = GPIO_PULLUP_DISABLE,
                                .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                .intr_type = GPIO_INTR_DISABLE};
  if (gpio_config(&io_conf_data) != ESP_OK) {
    ESP_LOGE(TAG, "Error configuring DATA_PIN");
    return ESP_FAIL;
  }

  gpio_config_t io_conf_clk = {.pin_bit_mask = (1ULL << clk_pin),
                               .mode = GPIO_MODE_INPUT,
                               .pull_up_en = GPIO_PULLUP_DISABLE,
                               .pull_down_en = GPIO_PULLDOWN_DISABLE,
                               .intr_type = GPIO_INTR_NEGEDGE};
  if (gpio_config(&io_conf_clk) != ESP_OK) {
    ESP_LOGE(TAG, "Error configuring CLK_PIN");
    return ESP_FAIL;
  }

  frame_queue = xQueueCreate(FRAME_QUEUE_LENGTH, sizeof(frame_t));
  if (frame_queue == NULL) {
    ESP_LOGE(TAG, "Could not create frame_queue");
    return ESP_ERR_NO_MEM;
  }

  if (xTaskCreate(frame_processor_task, "frame_processor_task", 1024, NULL, configMAX_PRIORITIES - 5, NULL) != pdPASS) {
    ESP_LOGE(TAG, "Error creating frame_processor_task");
    return ESP_FAIL;
  }

  if (gpio_install_isr_service(0) != ESP_OK) {
    ESP_LOGE(TAG, "Error installing ISR service");
    return ESP_FAIL;
  }
  if (gpio_isr_handler_add(clk_pin, clk_isr_handler, NULL) != ESP_OK) {
    ESP_LOGE(TAG, "Error adding ISR handler");
    return ESP_FAIL;
  }
  ESP_LOGI(TAG, "Frame handler started successfully");
  return ESP_OK;
}
