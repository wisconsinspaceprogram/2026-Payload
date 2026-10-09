cmake_minimum_required(VERSION 3.14)
project(WispPayload CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra)

add_library(blackboard src/blackboard/blackboard.cpp)
target_include_directories(blackboard PUBLIC src)

add_executable(main_control src/main_control.cpp)
target_link_libraries(main_control PRIVATE blackboard)
