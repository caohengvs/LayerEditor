project(test_imageProcessor)

add_executable(${PROJECT_NAME}
    mainTest.cpp
)
target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Logger/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE "$<IF:$<CONFIG:Debug>,ImageProcessord,ImageProcessor>"
) 

target_include_directories(${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Logger/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
set(OUTPUT_DIR ${CMAKE_SOURCE_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR}/bin
    OUTPUT_NAME "$<IF:$<CONFIG:Debug>,${PROJECT_NAME}d,${PROJECT_NAME}>")