CC = clang
CFLAGS = -Wall -Wextra -O2 -I$(SRC_DIR)/glad/include
LDFLAGS = -lGL -lglfw -lm -lfftw3 -lpulse -lpulse-simple -lpthread

VER_MAJOR = 0
VER_MINOR = 1
VER_PATCH = 0

SRC_DIR := src
BUILD_DIR := target

TARGET = visualizer-$(VER_MAJOR).$(VER_MINOR).$(VER_PATCH)

SRCS = $(wildcard $(SRC_DIR)/*.c)
GLAD_SRC = $(SRC_DIR)/glad/src/gl.c
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
OBJS += $(BUILD_DIR)/glad.o

.PHONY: all clean run

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/glad.o: $(GLAD_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -rf $(BUILD_DIR)

