import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_opencv_ffi/flutter_opencv_ffi.dart'
    as flutter_opencv_ffi;
import 'package:image/image.dart' as img;

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  static const String _assetPath = 'images/ae0b4515633b54ddda77514a07d00e7.jpg';

  String _opencvVersion = '';
  String _status = '处理中...';
  Uint8List? _originalBytes;
  Uint8List? _edgeBytes;

  @override
  void initState() {
    super.initState();
    _runEdgeDetection();
  }

  Future<void> _runEdgeDetection() async {
    try {
      final ByteData data = await rootBundle.load(_assetPath);
      final Uint8List jpgBytes = data.buffer.asUint8List();
      final img.Image? decoded = img.decodeImage(jpgBytes);

      if (decoded == null) {
        throw StateError('图片解码失败');
      }

      final Uint8List rgba = decoded.getBytes(order: img.ChannelOrder.rgba);
      final Uint8List edgeRgba = flutter_opencv_ffi.detectEdgesRgba(
        rgba,
        decoded.width,
        decoded.height,
      );

      final img.Image edgeImage = img.Image.fromBytes(
        width: decoded.width,
        height: decoded.height,
        bytes: edgeRgba.buffer,
        order: img.ChannelOrder.rgba,
        numChannels: 4,
      );
      final Uint8List edgePng = Uint8List.fromList(img.encodePng(edgeImage));
      final String version = flutter_opencv_ffi.opencvVersion();

      if (!mounted) {
        return;
      }

      setState(() {
        _originalBytes = jpgBytes;
        _edgeBytes = edgePng;
        _opencvVersion = version;
        _status = '完成';
      });
    } catch (error) {
      if (!mounted) {
        return;
      }

      setState(() {
        _status = '失败：$error';
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    debugPrint('状态：$_status');
    debugPrint('OpenCV：${_opencvVersion.isEmpty ? '-' : _opencvVersion}');
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('OpenCV FFI Edge Detection')),
        body: ListView(
          padding: const EdgeInsets.all(16),
          children: <Widget>[
            Text('状态：$_status'),
            const SizedBox(height: 8),
            Text('OpenCV：${_opencvVersion.isEmpty ? '-' : _opencvVersion}'),
            const SizedBox(height: 16),
            const Text('原图'),
            const SizedBox(height: 8),
            if (_originalBytes != null) Image.memory(_originalBytes!),
            const SizedBox(height: 16),
            const Text('边缘检测结果'),
            const SizedBox(height: 8),
            if (_edgeBytes != null) Image.memory(_edgeBytes!),
          ],
        ),
      ),
    );
  }
}
