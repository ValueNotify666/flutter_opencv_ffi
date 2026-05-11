#import <Foundation/Foundation.h>

#include "../../src/flutter_opencv_ffi.h"

// FFI plugins on iOS are statically linked. Without an Objective-C object file,
// the linker can drop the C/C++ translation units because Dart resolves them
// only at runtime via dlsym. This anchor is loaded by -ObjC and keeps the
// exported symbols reachable from DynamicLibrary.process().
__attribute__((used)) static const void* flutter_opencv_ffi_exported_symbols[] = {
    (const void*)&sum,
    (const void*)&sum_long_running,
    (const void*)&opencv_version,
    (const void*)&opencv_detect_edges_rgba,
    (const void*)&opencv_detect_document_8_points_rgba,
    (const void*)&opencv_crop_enhance_document_8_points_rgba,
    (const void*)&opencv_free,
};

@interface FlutterOpencvFfiPlugin : NSObject
@end

@implementation FlutterOpencvFfiPlugin
@end