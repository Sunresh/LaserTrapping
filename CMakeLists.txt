project(Onemore)

set(OpenCV_DIR "C:/opencv/build")
find_package(OpenCV REQUIRED)

find_library(NIDAQmx_LIBRARY NIDAQmx PATHS "C:/Program Files (x86)/National Instruments/Shared/ExternalCompilerSupport/C/lib64/msvc")
include_directories("C:/Program Files (x86)/National Instruments/NI-DAQ/DAQmx ANSI C Dev/include")
include_directories("C:/opencv/build/include")
add_executable(${PROJECT_NAME}  "main.cpp"   "preference.cpp" "preference.h"    "depositionclass.h" "depositionclass.cpp"   "MainWindow.cpp" "MainWindow.h"  )
link_directories("C:/opencv/build/x64/v16/lib")
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${NIDAQmx_LIBRARY} )
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
set(WITH_GSTREAMER OFF)
