# flutter_opencv_ffi

[English](#english) | [中文](#中文)

---

<a name="english"></a>
## English

A Flutter FFI plugin that provides OpenCV image processing capabilities for HarmonyOS (OHOS), Android, iOS, and other platforms.

### Features

- **Document Detection**: Automatic 8-point document/label detection with text-density based object selection
- **Perspective Crop**: Warp and crop image regions using 8-point perspective transformation
- **Image Enhancement**: Grayscale conversion, CLAHE contrast enhancement, and sharpening
- **Edge Detection**: Canny edge detection with contour-based object selection
- **FFI Native Integration**: Direct native code invocation via Dart FFI for high performance

### 8-Point Order

The 8 points are ordered as follows:

```
0: topLeft       1: topCenter      2: topRight
7: leftCenter                        3: rightCenter
6: bottomLeft   5: bottomCenter   4: bottomRight
```

### API

```dart
// Get OpenCV version
String version = FlutterOpencvFfi.opencvVersion();

// Detect document 8 points from RGBA bytes
Float32List points = FlutterOpencvFfi.detectDocument8PointsRgba(
  rgba: rgbaBytes,
  width: width,
  height: height,
);

// Crop and enhance document using 8 points
OpenCvImageResult result = FlutterOpencvFfi.cropEnhanceDocument8PointsRgba(
  rgba: rgbaBytes,
  width: width,
  height: height,
  points: points,
);

// Edge detection
Uint8List edges = FlutterOpencvFfi.detectEdgesRgba(
  rgba: rgbaBytes,
  width: width,
  height: height,
  lowThreshold: 45,
  highThreshold: 135,
);
```

### Detection Algorithm

The `detectDocument8PointsRgba` function:

1. Downscales large images to max side 960px for performance
2. Converts RGBA to grayscale
3. Extracts dark text using morphological blackhat + OTSU thresholding
4. Groups text into clusters using dilation and morphological operations
5. Scores clusters by text pixel density, component count, and compactness
6. Selects the best text cluster and expands its rotated minAreaRect
7. Maps back to original image coordinates
8. Outputs 8 points (4 corners + 4 edge midpoints)

### Project Structure

- `src/`: Native C++ source code with OpenCV integration
- `lib/`: Dart FFI bindings and API
- `ohos/`: HarmonyOS native build files with OpenCV static libraries
- `android/`, `ios/`, `windows/`, etc.: Platform-specific build configurations

### Native Build

The plugin uses prebuilt OpenCV mobile static libraries for HarmonyOS under `ohos/opencv-mobile-4.13.0-harmonyos`.

### Threading

- Short-running functions can be called from any isolate
- Long-running functions (detection, crop/enhance) should be called on a helper isolate to avoid frame drops

### Getting Started

Add this to your `pubspec.yaml`:

```yaml
dependencies:
  flutter_opencv_ffi:
    path: ../flutter_opencv_ffi
```

---

<a name="中文"></a>
## 中文

一个提供 OpenCV 图像处理能力的 Flutter FFI 插件，支持鸿蒙（OHOS）、Android、iOS 等平台。

### 功能特性

- **文档检测**：基于文字密度的 8 点文档/标签自动检测
- **透视裁剪**：使用 8 点透视变换裁剪图像区域
- **图像增强**：灰度转换、CLAHE 对比度增强、锐化处理
- **边缘检测**：Canny 边缘检测与基于轮廓的物体选择
- **FFI 原生集成**：通过 Dart FFI 直接调用原生代码，高性能

### 8 点顺序

8 个点的顺序如下：

```
0: 左上角        1: 上边中点       2: 右上角
7: 左边中点                         3: 右边中点
6: 左下角        5: 下边中点       4: 右下角
```

### API

```dart
// 获取 OpenCV 版本
String version = FlutterOpencvFfi.opencvVersion();

// 从 RGBA 字节数据检测文档 8 点
Float32List points = FlutterOpencvFfi.detectDocument8PointsRgba(
  rgba: rgbaBytes,
  width: width,
  height: height,
);

// 使用 8 点裁剪并增强文档
OpenCvImageResult result = FlutterOpencvFfi.cropEnhanceDocument8PointsRgba(
  rgba: rgbaBytes,
  width: width,
  height: height,
  points: points,
);

// 边缘检测
Uint8List edges = FlutterOpencvFfi.detectEdgesRgba(
  rgba: rgbaBytes,
  width: width,
  height: height,
  lowThreshold: 45,
  highThreshold: 135,
);
```

### 检测算法

`detectDocument8PointsRgba` 函数：

1. 将大图降采样到最大边 960px 以提升性能
2. 将 RGBA 转为灰度图
3. 使用形态学 blackhat + OTSU 阈值提取深色文字
4. 通过膨胀和形态学操作将文字分组成簇
5. 按文字像素密度、组件数量和紧凑度对簇评分
6. 选择最佳文字簇并扩展其旋转最小外接矩形
7. 映射回原图坐标
8. 输出 8 点（4 个角点 + 4 个边中点）

### 项目结构

- `src/`：原生 C++ 源码，集成 OpenCV
- `lib/`：Dart FFI 绑定和 API
- `ohos/`：鸿蒙原生构建文件，包含 OpenCV 静态库
- `android/`、`ios/`、`windows/` 等：各平台构建配置

### 原生构建

插件使用预编译的 OpenCV Mobile 静态库，位于 `ohos/opencv-mobile-4.13.0-harmonyos`。

### 线程使用

- 短时函数可在任何 isolate 直接调用
- 长时函数（检测、裁剪/增强）应在辅助 isolate 调用，避免掉帧

### 快速开始

在 `pubspec.yaml` 中添加：

```yaml
dependencies:
  flutter_opencv_ffi:
    path: ../flutter_opencv_ffi
```
