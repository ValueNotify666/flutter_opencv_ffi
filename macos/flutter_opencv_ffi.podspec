Pod::Spec.new do |s|
  s.name             = 'flutter_opencv_ffi'
  s.version          = '0.0.1'
  s.summary          = 'Flutter FFI plugin with OpenCV.'
  s.description      = <<-DESC
Flutter FFI plugin with OpenCV support for macOS.
DESC
  s.homepage         = 'https://github.com/ValueNotify666/flutter_opencv_ffi'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'flutter_opencv_ffi' => 'dev@example.com' }
  s.source           = { :path => '.' }
  # Include the bridge implementation so the macOS framework exports the FFI symbols.
  s.source_files     = 'Classes/**/*.{h,mm,m,cpp,cc}'
  s.public_header_files = 'Classes/**/*.h'
  s.platform         = :osx, '10.14'
  s.dependency 'FlutterMacOS'
  s.frameworks       = 'Accelerate'

  # Add OpenCV framework - ensure it's properly linked
  s.vendored_frameworks = 'opencv2.framework'
  # Use pod_target_xcconfig to set framework search paths for linking
  s.pod_target_xcconfig = {
    'DEFINES_MODULE' => 'YES',
    'CLANG_CXX_LANGUAGE_STANDARD' => 'c++17',
    'FRAMEWORK_SEARCH_PATHS' => '$(PODS_TARGET_SRCROOT)',
  }
end
