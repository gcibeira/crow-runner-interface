#include "hdlc.h"
#include <stdio.h>
#include <stdlib.h>

#define N 3
#define BUFFER_SIZE 256

uint8_t buffer[BUFFER_SIZE];
uint16_t buffer_index = 0;

void process_frame(uint8_t *frame, size_t len) {
  // Procesar la trama finalizada (optimizable según requerimientos)
  printf("Procesando trama de longitud %zu: ", len);
  for (size_t i = 0; i < len; i++) {
    printf("%02X ", frame[i]);
  }
  printf("\n");
}

void spi_isr(uint8_t spi_data) {
  static uint16_t bit_buffer = 0;      // Acumula bits válidos
  static uint8_t bits_in_buffer = 0;   // Cantidad de bits acumulados
  static uint8_t consecutive_ones = 0; // Contador de unos consecutivos
  static bool in_frame = false;        // Indica si estamos dentro de una trama

  // Procesa cada bit del byte recibido, de MSB a LSB
  for (int i = 7; i >= 0; i--) {
    uint8_t bit = (spi_data >> i) & 0x01;

    if (bit) {
      if (consecutive_ones < 7) {
        consecutive_ones++;
      } else {
        // 7 ones detected. Re-sync
        in_frame = false;
        buffer_index = 0;
        bit_buffer = 0;
        bits_in_buffer = 0;
        continue;
      }
      if (in_frame) {
        bit_buffer = (bit_buffer << 1) | 1;
        bits_in_buffer++;
      }
    } else {
      if (consecutive_ones == 6) {
        if (in_frame) {
          process_frame(buffer, buffer_index);
          buffer_index = 0;
        }
        in_frame = !in_frame;
        bit_buffer = 0;
        bits_in_buffer = 0;
      } else if (consecutive_ones != 5) {
        if (in_frame) {
          bit_buffer = (bit_buffer << 1);
          bits_in_buffer++;
        }
      }
      consecutive_ones = 0;
    }

    // Al completar 8 bits en bitbuffer, se almacena en el buffer de salida
    while (bits_in_buffer >= 8) {
      uint8_t byte = (bit_buffer >> (bits_in_buffer - 8)) & 0xFF;
      buffer[buffer_index++] = byte;
      bits_in_buffer -= 8;
    }
  }
}

void simulate_spi_data(uint8_t *data, size_t len) {
  for (size_t i = 0; i < len; i++) {
    spi_isr(data[i]);
  }
}