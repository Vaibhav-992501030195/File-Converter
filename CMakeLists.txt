cmake_minimum_required(VERSION 3.16)
project(UniversalFileConverter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCES
    src/main.cpp
    src/converter.cpp
    src/image_converter.cpp
    src/audio_converter.cpp
    src/video_converter.cpp
    src/document_converter.cpp
    src/data_converter.cpp
    src/validator.cpp
    src/detector.cpp
    src/history.cpp
    src/utils.cpp
)

add_executable(ufc ${SOURCES})

target_compile_options(ufc PRIVATE -Wall -Wextra -Wpedantic)
