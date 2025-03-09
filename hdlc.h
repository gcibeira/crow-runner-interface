#ifndef HDLC_H
#define HDLC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void process_frame(uint8_t *frame, size_t len);
void spi_isr(uint8_t spi_data);
void simulate_spi_data(uint8_t *data, size_t len);

#endif // HDLC_H
