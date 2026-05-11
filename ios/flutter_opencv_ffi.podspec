Pod::Spec.new do |s|
  s.name             = 'flutter_opencv_ffi'
  s.version          = '0.0.1'
  s.summary          = 'Flutter FFI plugin with OpenCV.'
  s.description      = <<-DESC
Flutter FFI plugin with OpenCV support for iOS.
DESC
  s.homepage         = 'https://github.com/ValueNotify666/flutter_opencv_ffi'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'flutter_opencv_ffi' => 'dev@example.com' }
  s.source           = { :path => '.' }
  s.source_files     = 'Classes/**/*.{h,m,mm,cc,cpp}'
  s.public_header_files = 'Classes/**/*.h'
  s.platform         = :ios, '12.0'
  s.dependency 'Flutter'

  # Prefer xcframework (device + simulator). Fall back to framework for local tests.
  xcframework_path = File.join(File.dirname(__FILE__), 'opencv2.xcframework')
  framework_path = File.join(File.dirname(__FILE__), 'opencv2.framework')

  if File.exist?(xcframework_path)
    s.vendored_frameworks = 'opencv2.xcframework'
  elsif File.exist?(framework_path)
    s.vendored_frameworks = 'opencv2.framework'
  else
    raise 'Missing OpenCV binary: add ios/opencv2.xcframework (recommended) or ios/opencv2.framework'
  end

  s.pod_target_xcconfig = {
    'DEFINES_MODULE' => 'YES',
    'CLANG_CXX_LANGUAGE_STANDARD' => 'c++17',
    'GCC_PREPROCESSOR_DEFINITIONS' => '$(inherited) HAVE_OPENCV=1'
  }
end
