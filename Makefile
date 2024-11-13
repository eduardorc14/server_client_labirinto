SRC_DIR = src
BUILD_DIR = build

all: $(BUILD_DIR)/client $(BUILD_DIR)/server

$(BUILD_DIR)/client: $(SRC_DIR)/client.c $(BUILD_DIR)/common.o
	gcc -Wall $(SRC_DIR)/client.c $(BUILD_DIR)/common.o -o $(BUILD_DIR)/client

$(BUILD_DIR)/server: $(SRC_DIR)/server.c $(BUILD_DIR)/common.o
	gcc -Wall $(SRC_DIR)/server.c $(BUILD_DIR)/common.o -o $(BUILD_DIR)/server

$(BUILD_DIR)/common.o: $(SRC_DIR)/common.c
	gcc -Wall -c $(SRC_DIR)/common.c -o $(BUILD_DIR)/common.o

clean:
	rm -rf $(BUILD_DIR)/*
