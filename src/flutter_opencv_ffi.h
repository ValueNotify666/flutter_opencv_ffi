#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT
#endif

#if __cplusplus
extern "C" {
#endif

// A very short-lived native function.
//
// For very short-lived functions, it is fine to call them on the main isolate.
// They will block the Dart execution while running the native function, so
// only do this for native functions which are guaranteed to be short-lived.
FFI_PLUGIN_EXPORT int sum(int a, int b);

// A longer lived native function, which occupies the thread calling it.
//
// Do not call these kind of native functions in the main isolate. They will
// block Dart execution. This will cause dropped frames in Flutter applications.
// Instead, call these native functions on a separate isolate.
FFI_PLUGIN_EXPORT int sum_long_running(int a, int b);

FFI_PLUGIN_EXPORT const char* opencv_version(void);

FFI_PLUGIN_EXPORT uint8_t* opencv_detect_edges_rgba(
    const uint8_t* rgba,
    int32_t width,
    int32_t height,
    int32_t low_threshold,
    int32_t high_threshold,
    int32_t* output_length);

FFI_PLUGIN_EXPORT int32_t opencv_detect_document_8_points_rgba(
    const uint8_t* rgba,
    int32_t width,
    int32_t height,
    float* points);

FFI_PLUGIN_EXPORT uint8_t* opencv_crop_enhance_document_8_points_rgba(
    const uint8_t* rgba,
    int32_t width,
    int32_t height,
    const float* points,
    int32_t* output_width,
    int32_t* output_height,
    int32_t* output_length);

FFI_PLUGIN_EXPORT void opencv_free(uint8_t* pointer);

#if __cplusplus
}
#endif
