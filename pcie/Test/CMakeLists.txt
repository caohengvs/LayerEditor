cmake_minimum_required(VERSION 3.10)
project(Test)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        nameof.hpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Test
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Test APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Test SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
   else()
        add_executable(Test
            ${PROJECT_SOURCES}
        )
    endif()
endif()
target_include_directories(Test 
PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/HfFeCommonLayer 
PRIVATE PCIEWindow/include
PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/HfFeDataParser 
)
target_link_directories(Test 
PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/HfFeCommonLayer
PRIVATE PCIEWindow/lib
PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/HfFeDataParser 
)
target_link_libraries(Test 
PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
PRIVATE  "$<IF:$<CONFIG:Debug>,HfFeCommonLayerd,HfFeCommonLayer>"
PRIVATE  "$<IF:$<CONFIG:Debug>,HfFeDataParserd,HfFeDataParser>"
 PRIVATE "$<IF:$<CONFIG:Debug>,PCIEWindowd,PCIEWindow>"
 )

#set_target_properties(Test PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
#    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
#    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
#)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Test)
endif()

