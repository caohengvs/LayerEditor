cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(Logger)

find_package(spdlog CONFIG REQUIRED)

file(GLOB SOURCES "src/*.cpp")

add_library(${PROJECT_NAME} SHARED
    ${SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/CommonDef/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)

include(${CMAKE_ROOT_DIR}/ProjectConfig.cmake)