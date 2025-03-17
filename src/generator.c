#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 16
#define OFFSET 3

void generate_random_array(unsigned char *array, size_t size) {
    for (size_t i = 0; i < size; i++) {
        array[i] = rand() % 256;
    }
}

void shift_array_right(unsigned char *array, size_t size, int n) {
    int bit_shift = n % 8;
    unsigned char carry = 0xFF << (8 - bit_shift);

    for (int i = 0; i <= size-1; i++) {
        unsigned char new_carry = array[i] << (8 - bit_shift);
        array[i] = (array[i] >> bit_shift) | carry;
        carry = new_carry;
    }
}

void print_array(const unsigned char *array, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("0x%02X, ", array[i]);
    }
    printf("\n");
}

int main() {
    unsigned char array[ARRAY_SIZE];
    int n;

    srand(time(NULL));

    generate_random_array(array, ARRAY_SIZE);
    printf("Original array:\n");
    print_array(array, ARRAY_SIZE);

    shift_array_right(array, ARRAY_SIZE, OFFSET);
    printf("Shifted array:\n");
    print_array(array, ARRAY_SIZE);

    return 0;
}