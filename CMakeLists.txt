cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
# enable incomplete C++20 support
add_subdirectory(metalcpp)

# compile metal to air and embed it into the binary

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shaders/compute.air COMMAND xcrun -sdk macosx metal -c ${CMAKE_CURRENT_SOURCE_DIR}/compute.metal -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/compute.air DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/compute.metal)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shaders/compute.metallib COMMAND xcrun -sdk macosx metallib ${CMAKE_CURRENT_BINARY_DIR}/shaders/compute.air -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/compute.metallib DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/shaders/compute.air)
add_custom_target(shaders ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/shaders/compute.metallib)


add_executable(main main.cpp gpuprocessing.cpp readsource.cpp)

target_link_libraries(main METAL_CPP)
add_dependencies(main shaders)