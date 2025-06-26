cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_GENERATOR_PLATFORM x64)
set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain file")
set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
set(CMAKE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

project(project1 VERSION 0.1.0 LANGUAGES C CXX)
add_subdirectory(Logger)
add_executable(project1 main.cpp)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger/lib
)
target_link_libraries(${PROJECT_NAME} 
    PRIVATE "$<IF:$<CONFIG:Debug>,Loggerd,Logger>"
)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger/include
)
set(OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES 
    VS_DEBUGGER_WORKING_DIRECTORY ${OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR}/bin
    OUTPUT_NAME "$<IF:$<CONFIG:Debug>,${PROJECT_NAME}d,${PROJECT_NAME}>"
)