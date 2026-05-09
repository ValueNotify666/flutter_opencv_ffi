# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

project(flutter_opencv_ffi_library VERSION 0.0.1 LANGUAGES CXX)

add_library(flutter_opencv_ffi SHARED
  "flutter_opencv_ffi.cpp"
)

set_target_properties(flutter_opencv_ffi PROPERTIES
  PUBLIC_HEADER flutter_opencv_ffi.h
  OUTPUT_NAME "flutter_opencv_ffi"
)

target_compile_definitions(flutter_opencv_ffi PUBLIC DART_SHARED_LIB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (DEFINED OHOS_ARCH)
  set(OPENCV_OHOS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../ohos/opencv-mobile-4.13.0-harmonyos")
  set(OpenCV_DIR "${OPENCV_OHOS_ROOT}/${OHOS_ARCH}/lib/cmake/opencv4")
  find_package(OpenCV REQUIRED)

  target_compile_definitions(flutter_opencv_ffi PRIVATE HAVE_OPENCV)
  target_include_directories(flutter_opencv_ffi PRIVATE ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(flutter_opencv_ffi PRIVATE opencv_imgproc opencv_core)
endif()

if (ANDROID)
  # Support Android 15 16k page size
  target_link_options(flutter_opencv_ffi PRIVATE "-Wl,-z,max-page-size=16384")
endif()
