cmake_minimum_required(VERSION 3.15)

project(webserver)

set(CXX_STANDARD 14)

include_directories(${PROJECT_SOURCE_DIR}/include)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/ SRC)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(server ${SRC})