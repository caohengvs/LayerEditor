cmake_minimum_required(VERSION 3.20.0)
cmake_policy(SET CMP0074 NEW)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) 

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/UserConfig.cmake")
    message(STATUS "apply UserConfig.cmake")
    include("${CMAKE_CURRENT_SOURCE_DIR}/UserConfig.cmake")
else()
    set(Qt5_ROOT "D:/Qt/5.12.12/msvc2017_64") # Qt SDK 路径
    set(3rd_PARTY_ROOT "D:/repos/3rdParty") # 额外的第三方包路径
    set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain file") # vcpkg 安装路径
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
endif()

set(CMAKE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

project(LayerEditor VERSION 0.1.0 LANGUAGES C CXX)


find_package(Qt5 COMPONENTS Widgets Core Gui REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger/include
    ${CMAKE_CURRENT_SOURCE_DIR}/WindowManager/include
    ${CMAKE_SOURCE_DIR}/CommonDef/include
)

add_subdirectory(Logger)
add_subdirectory(WindowManager)
add_subdirectory(ImageProcessor)
add_executable(${PROJECT_NAME} main.cpp)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/WindowManager/lib
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE "$<IF:$<CONFIG:Debug>,Loggerd,Logger>"
    PRIVATE "$<IF:$<CONFIG:Debug>,WindowManagerd,WindowManager>"
    PRIVATE Qt5::Widgets
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger/include
    ${CMAKE_CURRENT_SOURCE_DIR}/WindowManager/include
)


set(OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt PATHS ${Qt5_ROOT}/bin)

    if(WINDEPLOYQT_EXECUTABLE)
        message(STATUS "找到 windeployqt: ${WINDEPLOYQT_EXECUTABLE}")
        add_custom_command(
            TARGET ${PROJECT_NAME}# 关联到你的可执行文件目标
            POST_BUILD                    # 在目标构建完成后运行
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                    ${OUTPUT_DIR}/bin # 可选：如果你的项目有 QML 文件，告诉 windeployqt 扫描源目录
        )
    else()
        message(WARNING "未找到 windeployqt。部署可能需要手动步骤。")
    endif()
endif()


set_target_properties(${PROJECT_NAME} PROPERTIES 
    VS_DEBUGGER_WORKING_DIRECTORY ${OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR}/bin
    OUTPUT_NAME "$<IF:$<CONFIG:Debug>,${PROJECT_NAME}d,${PROJECT_NAME}>"
)
