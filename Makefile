CC = gcc
CFLAGS = -Wall -Wextra -std=c11
BUILD_DIR = build
BIN_DIR = bin
SRC_DIR = src
TARGET = $(BIN_DIR)/main.exe
OBJ = $(BUILD_DIR)/hdlc.o $(BUILD_DIR)/main.o $(BUILD_DIR)/test_data.o

all: $(BUILD_DIR) $(BIN_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

$(BUILD_DIR)/hdlc.o: $(SRC_DIR)/hdlc.c $(SRC_DIR)/hdlc.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/hdlc.c -o $(BUILD_DIR)/hdlc.o

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c $(SRC_DIR)/hdlc.h $(SRC_DIR)/test_data.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/main.c -o $(BUILD_DIR)/main.o

$(BUILD_DIR)/test_data.o: $(SRC_DIR)/test_data.c $(SRC_DIR)/test_data.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/test_data.c -o $(BUILD_DIR)/test_data.o

clean:
	rm -f $(OBJ) $(TARGET)
