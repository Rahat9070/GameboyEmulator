cmake_minimum_required(VERSION 3.10)
project(GameboyEmulator)

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} src)

# Add source files
set(SOURCES
    main.cpp
    src/Cartridge/cartridge.cpp
    src/CPU/CPU.cpp
    src/MBC/MBC.cpp
    src/MMU/MMU.cpp
    src/Render/render.cpp
)

# Create executable
add_executable(GameboyEmulator ${SOURCES})

# Link SDL2
target_link_libraries(GameboyEmulator ${SDL2_LIBRARIES})