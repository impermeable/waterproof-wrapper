# Version 3.11 is required for the FetchContent module.
cmake_minimum_required(VERSION 3.11)

project(wpwrapper VERSION 0.1.0 LANGUAGES CXX)

# Force the compiler to use standard C++17.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Configure external dependencies.
add_subdirectory(external)

# Configure the main application target.
add_subdirectory(src)

# Configure the unit test target.
#add_subdirectory(test)
