#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "test_data.h"

#define N 3
#define BUFFER_SIZE 256

uint8_t buffer[BUFFER_SIZE];
uint16_t buffer_index = 0;

void spi_isr(uint8_t spi_data) {
  static uint16_t bit_buffer = 0;      // Acumula bits válidos
  static uint8_t bits_in_buffer = 0;   // Cantidad de bits acumulados
  static uint8_t consecutive_ones = 0; // Contador de unos consecutivos
  static bool in_frame = false;        // Indica si estamos dentro de una trama

  // Procesa cada bit del byte recibido, de MSB a LSB
  for (int i = 7; i >= 0; i--) {
    uint8_t bit = (spi_data >> i) & 0x01;

    if (bit) {
      if (consecutive_ones < 8)
        consecutive_ones++;

      if (in_frame) {
        bit_buffer = (bit_buffer << 1) | 1;
        bits_in_buffer++;
      }
    } else {
      if (consecutive_ones == 6) {
        // Se ha detectado un delimitador (0x7E)
        printf("Delimitador detectado: %s\n",
               in_frame ? "Fin de trama" : "Inicio de trama");
        in_frame = !in_frame;
        bit_buffer = 0;
        bits_in_buffer = 0;
        consecutive_ones = 0;
        continue;
      } else if (consecutive_ones == 5) {
        // Bit stuffed detectado: se omite este 0 y se reinicia el contador
        printf("Bit stuffed detectado\n");
        consecutive_ones = 0;
        continue;
      } else {
        consecutive_ones = 0;
        if (in_frame) {
          bit_buffer = (bit_buffer << 1);
          bits_in_buffer++;
        }
      }
    }

    // Mientras tengamos al menos 8 bits en el bit_buffer, extraemos un byte
    // completo
    /*
    TODO: Ver qué pasa con los bits del delimitador
    */
    while (bits_in_buffer >= 8) {
      uint8_t byte = (bit_buffer >> (bits_in_buffer - 8)) & 0xFF;
      buffer[buffer_index++] = byte;
      bits_in_buffer -= 8;
      printf("Byte extraido: %02X\n", byte);
    }
  }
}

void simulate_spi_data(uint8_t *data, size_t len) {
  for (size_t i = 0; i < len; i++) {
    spi_isr(data[i]);
  }
}

void print_buffer() {
  printf("Buffer reconstruido: \n");
  for (size_t i = 0; i < buffer_index; i++) {
    printf("%02X ", buffer[i]);
  }
  printf("\n");
}

int main() {
  simulate_spi_data(test_data, test_len);
  print_buffer();

  return 0;
}
