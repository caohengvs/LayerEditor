cmake_minimum_required(VERSION 3.20.0)
project(ImageProcessor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 
set(onnxruntime_ROOT ${3rd_PARTY_ROOT}/onnxruntime-win-x64-1.21.0)

file(GLOB SOURCES "src/*.cpp" "include/*.hpp")

find_package(OpenCV REQUIRED)
find_package(magic_enum CONFIG REQUIRED)

add_library(${PROJECT_NAME} SHARED
    ${SOURCES}
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Logger/lib
    ${onnxruntime_ROOT}/lib
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE "$<IF:$<CONFIG:Debug>,Loggerd,Logger>"
    PRIVATE ${OpenCV_LIBS}
    PRIVATE onnxruntime
    PRIVATE magic_enum::magic_enum
) 

target_include_directories(${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Logger/include>
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${onnxruntime_ROOT}/include
)

include(${CMAKE_ROOT_DIR}/ProjectConfig.cmake)


include(${CMAKE_CURRENT_SOURCE_DIR}/ProjectTestConfig.cmake)