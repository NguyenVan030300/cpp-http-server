cmake_minimum_required(VERSION 3.14)
project(httpsProject CXX)

file(MAKE_DIRECTORY 
include
src
test
)

file(TOUCH "include/httpsProject.hpp")
file(TOUCH "src/httpsProject.cpp")
file(TOUCH "test/httpsProject.cpp")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# 1. Main Application Executable (The actual TCP/HTTP Server)
add_executable(server_app
    src/main.cpp
    src/httpsProject.cpp
    src/Tetris.cpp
)

#sprint 3:
find_package(Threads REQUIRED)
target_link_directories(server_app PRIVATE Threads::Threads)

# 2. GoogleTest Setup
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# 3. Unit Tests Executable (Excludes src/main.cpp so gtest_main provides main())
add_executable(unit_tests
    src/httpsProject.cpp
    test/httpsProject.cpp
    src/Tetris.cpp
)

target_link_libraries(unit_tests PRIVATE gtest_main)

include(GoogleTest)
gtest_discover_tests(unit_tests)