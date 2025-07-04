cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(ImageProcessor)

file(GLOB SOURCES "src/*.cpp" "include/*.hpp")

set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/3rdParty/opencv)
find_package(OpenCV REQUIRED)

add_library(${PROJECT_NAME} SHARED
    ${SOURCES}
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Logger/lib
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE "$<IF:$<CONFIG:Debug>,Loggerd,Logger>"
    PRIVATE ${OpenCV_LIBS}
) 

target_include_directories(${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Logger/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${OpenCV_INCLUDE_DIRS}
    $<INSTALL_INTERFACE:include>
)

include(${CMAKE_ROOT_DIR}/ProjectConfig.cmake)


include(${CMAKE_CURRENT_SOURCE_DIR}/ProjectTestConfig.cmake)