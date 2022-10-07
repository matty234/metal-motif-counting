add_library(METAL_CPP
        ${CMAKE_CURRENT_SOURCE_DIR}/definition.cpp
        )

target_include_directories(METAL_CPP PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/metal-cpp"
        )

# Metal cpp library (linker)
target_link_libraries(METAL_CPP
        "-framework Metal"
        "-framework Foundation"
        "-framework QuartzCore"
        )
