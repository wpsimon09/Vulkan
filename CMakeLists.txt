cmake_minimum_required(VERSION 3.20)
project(LearnVulkan)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

set(TARGET VulkanTest)

set(SOURCES "src/main.cpp")

set(CFLAGS_RELEASE "-O2")
set(CFLAGS_DEBUG "-g")


add_executable(${TARGET} ${SOURCES})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${TARGET} PRIVATE ${CFLAGS_RELEASE})
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${TARGET} PRIVATE ${CFLAGS_DEBUG})
endif()

target_link_libraries(${TARGET} PRIVATE ${LDFLAGS})

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES} Vulkan::Vulkan )

add_custom_target(run
        COMMAND ./${TARGET}
        DEPENDS ${TARGET}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(reset
        COMMAND ${CMAKE_COMMAND} -E remove ${TARGET}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)