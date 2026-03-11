CXX = clang++
CC = clang
CXXFLAGS = -Wall -Wextra -O2 -I$(GLAD_DIR)/include -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CFLAGS = -Wall -Wextra -O2 -I$(GLAD_DIR)/include
LDFLAGS = -lGL -lglfw -lm -lfftw3 -lpulse -lpulse-simple -lpthread

VER_MAJOR = 0
VER_MINOR = 1
VER_PATCH = 0

SRC_DIR := src
BUILD_DIR := target
THIRD_PARTY_DIR := third_party
GLAD_DIR := $(THIRD_PARTY_DIR)/glad
IMGUI_DIR := $(THIRD_PARTY_DIR)/imgui

TARGET = visualizer-$(VER_MAJOR).$(VER_MINOR).$(VER_PATCH)

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
GLAD_SRC = $(GLAD_DIR)/src/gl.c
IMGUI_SRCS = imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp
IMGUI_BACKEND_SRCS = imgui_impl_glfw.cpp imgui_impl_opengl3.cpp

OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
OBJS += $(BUILD_DIR)/glad.o
OBJS += $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(IMGUI_SRCS))
OBJS += $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(IMGUI_BACKEND_SRCS))

.PHONY: all clean run

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(SRC_DIR)/config.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/glad.o: $(GLAD_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/imgui.o: $(IMGUI_DIR)/imgui.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/imgui_draw.o: $(IMGUI_DIR)/imgui_draw.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/imgui_tables.o: $(IMGUI_DIR)/imgui_tables.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/imgui_widgets.o: $(IMGUI_DIR)/imgui_widgets.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/imgui_impl_glfw.o: $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/imgui_impl_opengl3.o: $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -rf $(BUILD_DIR)

