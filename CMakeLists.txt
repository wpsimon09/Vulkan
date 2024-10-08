cmake_minimum_required(VERSION 3.20)
project(LearnVulkan)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

set(TARGET LearnVulkan)
set(SOURCES src/main.cpp
        Includes/VulkanApp.cpp
        Includes/VulkanApp.hpp
        Includes/DebugInfoLog.hpp
        Includes/Utils.hpp
        Includes/Structs.hpp
        Includes/Utils.hpp
        Includes/Camera/Camera.cpp
        Includes/Camera/Camera.hpp
        Includes/VertexData.hpp
        Includes/stb/stb_image.h
        Includes/stb/stb_image.cpp
        Includes/Material/Material.cpp
        Includes/Material/Material.hpp
        Includes/tiny_obj_loader/tiny_obj_loader.h
        Includes/tiny_obj_loader/tiny_obj_loader.cpp)

set(CFLAGS_RELEASE "-O2")
set(CFLAGS_DEBUG "-g")

# Find Vulkan, GLFW, and GLM
find_package(Vulkan REQUIRED)
find_package(assimp REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)

# Add executable
add_executable(${TARGET} ${SOURCES})

# Include GLM headers
target_include_directories(${TARGET} PRIVATE ${GLM_INCLUDE_DIRS})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
        ${CMAKE_SOURCE_DIR}/Includes
)

# Set compile options based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${TARGET} PRIVATE ${CFLAGS_RELEASE})
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${TARGET} PRIVATE ${CFLAGS_DEBUG})
endif()



# Link libraries
target_link_libraries(${TARGET} PRIVATE glfw Vulkan::Vulkan assimp)

# Custom target for running the executable
add_custom_target(run
        COMMAND ./${TARGET}
        COMMAND ./Shaders/compile.sh
        DEPENDS ${TARGET}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Custom target for cleaning the build
add_custom_target(reset
        COMMAND ${CMAKE_COMMAND} -E remove ${TARGET}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)



