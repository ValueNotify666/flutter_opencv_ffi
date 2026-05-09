cmake_minimum_required(VERSION 3.10)

project(flutter_opencv_ffi_library VERSION 0.0.1 LANGUAGES CXX)

add_library(flutter_opencv_ffi SHARED
  "../src/flutter_opencv_ffi.cpp"
  "src/main/cpp/openmp_compat.cpp"
)

set_target_properties(flutter_opencv_ffi PROPERTIES
  PUBLIC_HEADER ../src/flutter_opencv_ffi.h
  OUTPUT_NAME "flutter_opencv_ffi"
)

target_compile_definitions(flutter_opencv_ffi PUBLIC DART_SHARED_LIB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenCV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/opencv-mobile-4.13.0-android/sdk/native/jni")
find_package(OpenCV REQUIRED)

target_compile_definitions(flutter_opencv_ffi PRIVATE HAVE_OPENCV)
target_include_directories(flutter_opencv_ffi PRIVATE ${OpenCV_INCLUDE_DIRS})

if("${ANDROID_ABI}" STREQUAL "arm64-v8a")
  set(OPENMP_ARCH "aarch64")
elseif("${ANDROID_ABI}" STREQUAL "armeabi-v7a")
  set(OPENMP_ARCH "arm")
elseif("${ANDROID_ABI}" STREQUAL "x86")
  set(OPENMP_ARCH "i386")
elseif("${ANDROID_ABI}" STREQUAL "x86_64")
  set(OPENMP_ARCH "x86_64")
endif()

file(GLOB OMP_STATIC_LIB "${ANDROID_NDK}/toolchains/llvm/prebuilt/*/lib/clang/*/lib/linux/${OPENMP_ARCH}/libomp.a")
if(OMP_STATIC_LIB)
  target_link_libraries(flutter_opencv_ffi PRIVATE opencv_imgproc opencv_core ${OMP_STATIC_LIB})
else()
  target_link_libraries(flutter_opencv_ffi PRIVATE opencv_imgproc opencv_core omp)
endif()

# Support Android 15 16k page size
target_link_options(flutter_opencv_ffi PRIVATE "-Wl,-z,max-page-size=16384")
