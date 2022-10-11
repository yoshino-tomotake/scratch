cmake_minimum_required(VERSION 3.0)

project(labcpp)
set(CMAKE_CXX_STANDARD 17)
aux_source_directory(. SRC_DIR)

add_executable(main ${SRC_DIR})