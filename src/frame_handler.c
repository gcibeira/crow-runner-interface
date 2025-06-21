#include "frame_handler.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "protocol_handler.h"

static const char *TAG = "FRAME_HANDLER";

static QueueHandle_t rx_frame_queue;
static QueueHandle_t tx_frame_queue;
static SemaphoreHandle_t tx_done_sem;
static SemaphoreHandle_t bus_free_sem;
static volatile const uint8_t *tx_bits;
static volatile size_t tx_len;
static volatile size_t tx_idx;

static void prepare_bitstream(const frame_t *frame, uint8_t *out_buffer, size_t *out_bits) {
  if (!frame || !out_buffer || !out_bits) {
    ESP_LOGE(TAG, "prepare_bitstream: invalid parameters");
    return;
  }

  size_t bit_idx = 0;
  const uint8_t DELIM = 0x7E; // 01111110
  int consecutive_ones = 0;

  // 1) Delimitador de inicio
  for (int i = 0; i <= 7; ++i) {
    out_buffer[bit_idx++] = (DELIM >> i) & 0x1;
  }

  // 2) Datos con bit-stuffing
  for (size_t byte_i = 0; byte_i < frame->length; ++byte_i) {
    uint8_t b = frame->data[byte_i];
    for (int bit = 0; bit <= 7; ++bit) {
      uint8_t v = (b >> bit) & 0x1;
      out_buffer[bit_idx++] = v;

      if (v) {
        consecutive_ones++;
        if (consecutive_ones == 5) {
          // tras 5 unos, insertamos un '0' stuffed
          out_buffer[bit_idx++] = 0;
          consecutive_ones = 0;
        }
      } else {
        // cualquier '0' reinicia el conteo
        consecutive_ones = 0;
      }
    }
  }

  // 3) Delimitador de fin
  // resetear contador para no interferir
  consecutive_ones = 0;
  for (int i = 0; i <= 8; ++i) {
    out_buffer[bit_idx++] = (DELIM >> i) & 0x1;
  }

  // 4) Salida
  *out_bits = bit_idx;
}

static void IRAM_ATTR frame_process_bit(uint8_t incoming_bit) {
  static volatile bool inside_frame = false;
  static volatile uint8_t isr_accumulated_byte = 0;
  static volatile uint8_t isr_bit_count = 0;
  static volatile uint8_t consecutive_ones = 0;
  static volatile frame_t current_frame;

  // Delimiter detection logic
  if (incoming_bit == 1) {
    if (consecutive_ones < 7)
      consecutive_ones++;
    else {
      inside_frame = false;
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      xSemaphoreGiveFromISR(bus_free_sem, &xHigherPriorityTaskWoken);
      if (xHigherPriorityTaskWoken)
        portYIELD_FROM_ISR();
    }
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
      if (inside_frame && current_frame.length > 0) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(rx_frame_queue, (const void *)&current_frame, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken)
          portYIELD_FROM_ISR();
      }

      inside_frame = !inside_frame;
      consecutive_ones = 0;
      isr_accumulated_byte = 0;
      isr_bit_count = 0;
      current_frame.length = 0;
      return;
    }
    if (!inside_frame) {
      BaseType_t xHigherPriorityTaskWoken = pdFALSE;
      xSemaphoreTakeFromISR(bus_free_sem, &xHigherPriorityTaskWoken);
      if (xHigherPriorityTaskWoken)
        portYIELD_FROM_ISR();
    }
    
    consecutive_ones = 0;
  }

  if (!inside_frame)
    return;

  // Accumulate bits into a byte
  isr_accumulated_byte = (isr_accumulated_byte >> 1) | (incoming_bit << 7 & 0x80);
  isr_bit_count++;

  // Store byte in frame buffer
  if (isr_bit_count == 8) {
    uint8_t incoming_byte = isr_accumulated_byte;
    isr_accumulated_byte = 0;
    isr_bit_count = 0;
    if (current_frame.length < RX_FRAME_BUFFER_SIZE)
      current_frame.data[current_frame.length++] = incoming_byte;
    else
      inside_frame = false;
  }
}

static void IRAM_ATTR clk_rx_isr_handler(void *arg) {
  uint8_t incoming_bit = (uint8_t)(gpio_get_level(DAT_RX_PIN));
  frame_process_bit(incoming_bit);
}

static void IRAM_ATTR clk_tx_isr_handler(void *arg) {
  if (tx_idx < tx_len) {
    gpio_set_level(DAT_TX_PIN, !tx_bits[tx_idx]);
    tx_idx++;
  } else {
    gpio_set_level(DAT_TX_PIN, DAT_IDLE_LEVEL);
    // Notify that transmission is done
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(tx_done_sem, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken)
      portYIELD_FROM_ISR();
  }
}

static void frame_processor_task(void *arg) {
  frame_t received_frame;

  while (true) {
    if (xQueueReceive(rx_frame_queue, &received_frame, portMAX_DELAY) == pdPASS) {
      if (received_frame.length > 0)
        process_frame(&received_frame);
    }
  }
}

static void frame_sender_task(void *arg) {
  frame_t frame;
  uint8_t bit_buf[TX_BITSTREAM_SIZE];
  size_t total_bits;

  while (true) {
    if (xQueueReceive(tx_frame_queue, &frame, portMAX_DELAY) == pdPASS) {
      prepare_bitstream(&frame, bit_buf, &total_bits);

      // Wait for bus to be free and switch to TX mode
      xSemaphoreTake(bus_free_sem, portMAX_DELAY);
      gpio_isr_handler_remove(CLK_PIN);
      tx_bits = bit_buf;
      tx_len = total_bits;
      tx_idx = 0;
      gpio_set_intr_type(CLK_PIN, GPIO_INTR_POSEDGE);
      gpio_isr_handler_add(CLK_PIN, clk_tx_isr_handler, NULL);

      // Wait for transmission to be complete and switch back to RX mode
      xSemaphoreTake(tx_done_sem, portMAX_DELAY);
      gpio_isr_handler_remove(CLK_PIN);
      gpio_set_intr_type(CLK_PIN, GPIO_INTR_NEGEDGE);
      gpio_isr_handler_add(CLK_PIN, clk_rx_isr_handler, NULL);
      vTaskDelay(pdMS_TO_TICKS(TIME_BETWEEN_FRAMES_MS));
    }
  }
}

void frame_handler_send(const frame_t *frame) {
  if (!frame || frame->length == 0) {
    ESP_LOGE(TAG, "invalid frame");
    return;
  }

  if (xQueueSend(tx_frame_queue, frame, portMAX_DELAY) != pdPASS) {
    ESP_LOGE(TAG, "Error sending frame to tx queue");
    return;
  }
}

esp_err_t frame_handler_init() {
  gpio_config_t io_conf_rx_dat = {.pin_bit_mask = (1ULL << DAT_RX_PIN),
                                  .mode = GPIO_MODE_INPUT,
                                  .pull_up_en = GPIO_PULLUP_DISABLE,
                                  .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                  .intr_type = GPIO_INTR_DISABLE};
  if (gpio_config(&io_conf_rx_dat) != ESP_OK) {
    ESP_LOGE(TAG, "Error configuring DAT_RX_PIN");
    return ESP_FAIL;
  }

  gpio_config_t io_conf_tx_dat = {.pin_bit_mask = (1ULL << DAT_TX_PIN),
                                  .mode = GPIO_MODE_OUTPUT,
                                  .pull_up_en = GPIO_PULLUP_DISABLE,
                                  .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                  .intr_type = GPIO_INTR_DISABLE};
  if (gpio_config(&io_conf_tx_dat) != ESP_OK) {
    ESP_LOGE(TAG, "Error configuring DAT_TX_PIN");
    return ESP_FAIL;
  }
  gpio_set_level(DAT_TX_PIN, DAT_IDLE_LEVEL); // Set TX pin idle initially

  gpio_config_t io_conf_clk = {.pin_bit_mask = (1ULL << CLK_PIN),
                               .mode = GPIO_MODE_INPUT,
                               .pull_up_en = GPIO_PULLUP_DISABLE,
                               .pull_down_en = GPIO_PULLDOWN_DISABLE,
                               .intr_type = GPIO_INTR_NEGEDGE};
  if (gpio_config(&io_conf_clk) != ESP_OK) {
    ESP_LOGE(TAG, "Error configuring CLK_PIN");
    return ESP_FAIL;
  }

  rx_frame_queue = xQueueCreate(RX_FRAME_QUEUE_LENGTH, sizeof(frame_t));
  if (rx_frame_queue == NULL) {
    ESP_LOGE(TAG, "Could not create rx frame_queue");
    return ESP_ERR_NO_MEM;
  }

  tx_frame_queue = xQueueCreate(RX_FRAME_QUEUE_LENGTH, sizeof(frame_t));
  if (tx_frame_queue == NULL) {
    ESP_LOGE(TAG, "Could not create tx frame_queue");
    return ESP_ERR_NO_MEM;
  }

  tx_done_sem = xSemaphoreCreateBinary();
  if (tx_done_sem == NULL) {
    ESP_LOGE(TAG, "Could not create tx_done semaphore");
    return ESP_ERR_NO_MEM;
  }

  bus_free_sem = xSemaphoreCreateBinary();
  if (bus_free_sem == NULL) {
    ESP_LOGE(TAG, "Could not create bus_free semaphore");
    return ESP_ERR_NO_MEM;
  }

  if (xTaskCreate(frame_processor_task, "frame_processor_task", 4096, NULL, configMAX_PRIORITIES - 5, NULL) != pdPASS) {
    ESP_LOGE(TAG, "Error creating frame_processor_task");
    return ESP_FAIL;
  }

  if (xTaskCreate(frame_sender_task, "frame_sender_task", 4096, NULL, configMAX_PRIORITIES - 5, NULL) != pdPASS) {
    ESP_LOGE(TAG, "Error creating frame_sender_task");
    return ESP_FAIL;
  }

  if (gpio_install_isr_service(0) != ESP_OK) {
    ESP_LOGE(TAG, "Error installing ISR service");
    return ESP_FAIL;
  }
  if (gpio_isr_handler_add(CLK_PIN, clk_rx_isr_handler, NULL) != ESP_OK) {
    ESP_LOGE(TAG, "Error adding ISR handler");
    return ESP_FAIL;
  }
  ESP_LOGI(TAG, "Frame handler started successfully");
  return ESP_OK;
}
