cmake_minimum_required(VERSION 3.30)
project(crapple C)

set(CMAKE_C_STANDARD 11)

# Find SDL2 package
find_package(SDL2 REQUIRED)

# Add executable
add_executable(crapple main.c)

# Link SDL2 to your executable
target_link_libraries(crapple PRIVATE SDL2::SDL2)

