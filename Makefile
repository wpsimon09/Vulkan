CFLAGS_RELEASE = -std=c++17 -O2

LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi 

VulkanTest: src/main.cpp
	g++ $(CFLAGS_RELEASE) -o build/VulkanTest src/main.cpp $(LDFLAGS)

.PHONY: test clean

run: VulkanTest
	./build/VulkanTest

clean:
	rm -f ./build/VulkanTest