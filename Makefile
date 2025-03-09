CC = gcc
CFLAGS = -Wall -Wextra -std=c11
BUILD_DIR = build
TARGET = $(BUILD_DIR)/main.exe
OBJ = $(BUILD_DIR)/hdlc.o $(BUILD_DIR)/main.o $(BUILD_DIR)/test_data.o

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

$(BUILD_DIR)/hdlc.o: hdlc.c hdlc.h
	$(CC) $(CFLAGS) -c hdlc.c -o $(BUILD_DIR)/hdlc.o

$(BUILD_DIR)/main.o: main.c hdlc.h test_data.h
	$(CC) $(CFLAGS) -c main.c -o $(BUILD_DIR)/main.o

$(BUILD_DIR)/test_data.o: test_data.c test_data.h
	$(CC) $(CFLAGS) -c test_data.c -o $(BUILD_DIR)/test_data.o

clean:
	rm -f $(OBJ) $(TARGET)
