cmake_minimum_required(VERSION 3.16)
project(BufferOverflowServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include header-only libraries
include_directories(
    ${CMAKE_SOURCE_DIR}/include/Crow/include
    ${CMAKE_SOURCE_DIR}/include/asio/include
)

# Find OpenSSL (for SHA256)
find_package(OpenSSL REQUIRED)

# Build the server executable
add_executable(app-server
    src/main.cpp
)

# Link against OpenSSL's crypto library
target_link_libraries(app-server
    PRIVATE OpenSSL::Crypto
)
