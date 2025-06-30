cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(WindowManager)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Widgets Core Gui REQUIRED)
file(GLOB SOURCES "src/*.cpp" "*.cpp" "include/*.hpp")

add_library(${PROJECT_NAME} SHARED
    ${SOURCES}
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Logger/lib
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE Qt5::Widgets
    PRIVATE "$<IF:$<CONFIG:Debug>,Loggerd,Logger>"
) 

target_include_directories(${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/CommonDef/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Logger/include>
    $<INSTALL_INTERFACE:include>
)

include(${CMAKE_ROOT_DIR}/ProjectConfig.cmake)