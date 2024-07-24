# Compiler flags for release and debug builds
CFLAGS_RELEASE = -std=c++17 -O2
CFLAGS_DEBUG = -std=c++17 -g

# Linker flags
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi 

# Target executable
TARGET = build/VulkanTest

# Default build (release)
all: $(TARGET)

# Release build
release: CFLAGS = $(CFLAGS_RELEASE)
release: $(TARGET)

# Debug build
debug: CFLAGS = $(CFLAGS_DEBUG)
debug: $(TARGET)

# Link and compile
$(TARGET): src/main.cpp
	g++ $(CFLAGS) -o $(TARGET) src/main.cpp $(LDFLAGS)

# Phony targets
.PHONY: run clean release debug

# Run the compiled program
run: $(TARGET)
	./$(TARGET)

# Clean the build
clean:
	rm -f $(TARGET)