cmake_minimum_required(VERSION 3.16)
project(BufferOverflowServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include header-only libraries
include_directories(
    ${CMAKE_SOURCE_DIR}/include/Crow/include
    ${CMAKE_SOURCE_DIR}/include/asio/include
)

# Build the server executable
add_executable(app-server
    src/main.cpp
)
