set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Onemore)

set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/opencv/build")
find_package(OpenCV REQUIRED)

include_directories("C:/opencv/build/include")
add_executable(${PROJECT_NAME}  "main.cpp" "preference.h" "depositionclass.h" "MainWindow.h" "brightnessclass.h" )
link_directories("C:/opencv/build/x64/v16/lib")
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} )
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
set(WITH_GSTREAMER OFF)


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/opencv/build/x64/vc16/bin/opencv_world4110.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/opencv/build/x64/vc16/bin/opencv_world4110d.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/opencv/build/x64/vc16/bin/opencv_videoio_ffmpeg4110_64.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

function(create_class class_name)
    set(HEADER_FILE ${class_name}.h)
    set(SOURCE_FILE ${class_name}.cpp)

    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${HEADER_FILE})
        set(HEADER_CONTENT "#ifndef ${class_name}_H\n#define ${class_name}_H\n\nclass ${class_name} {\npublic:\n    ${class_name}();\n    ~${class_name}();\n\nprivate:\n\n};\n\n#endif\n")
        file(WRITE ${HEADER_FILE} "${HEADER_CONTENT}")
    endif()

    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
        set(SOURCE_CONTENT "#include \"${class_name}.h\"\n\n${class_name}::${class_name}() {\n    // Constructor implementation\n}\n\n${class_name}::~${class_name}() {\n    // Destructor implementation\n}\n")
        file(WRITE ${SOURCE_FILE} "${SOURCE_CONTENT}")
    endif()
endfunction()
#create_class(YourClass)
