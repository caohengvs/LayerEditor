cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(Logger)

file(GLOB SOURCES "src/*.cpp")

add_library(${PROJECT_NAME} SHARED
    ${SOURCES}
)
target_compile_definitions(${PROJECT_NAME} PRIVATE SPDLOG_HEADER_ONLY)


target_include_directories(${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/CommonDef/include>
    $<INSTALL_INTERFACE:include>
)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only)

include(${CMAKE_ROOT_DIR}/ProjectConfig.cmake)