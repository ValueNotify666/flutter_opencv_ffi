// You have generated a new plugin project without specifying the `--platforms`
// flag. An FFI plugin project that supports no platforms is generated.
// To add platforms, run `flutter create -t plugin_ffi --platforms <platforms> .`
// in this directory. You can also find a detailed instruction on how to
// add platforms in the `pubspec.yaml` at
// https://flutter.dev/to/pubspec-plugin-platforms.

import 'dart:async';
import 'dart:ffi';
import 'dart:io';
import 'dart:isolate';
import 'dart:typed_data';

import 'package:ffi/ffi.dart';
import 'flutter_opencv_ffi_bindings_generated.dart';

/// A very short-lived native function.
///
/// For very short-lived functions, it is fine to call them on the main isolate.
/// They will block the Dart execution while running the native function, so
/// only do this for native functions which are guaranteed to be short-lived.
int sum(int a, int b) => _bindings.sum(a, b);

class OpenCvImageResult {
  const OpenCvImageResult({
    required this.rgba,
    required this.width,
    required this.height,
  });

  final Uint8List rgba;
  final int width;
  final int height;
}

/// A longer lived native function, which occupies the thread calling it.
///
/// Do not call these kind of native functions in the main isolate. They will
/// block Dart execution. This will cause dropped frames in Flutter applications.
/// Instead, call these native functions on a separate isolate.
///
/// Modify this to suit your own use case. Example use cases:
///
/// 1. Reuse a single isolate for various different kinds of requests.
/// 2. Use multiple helper isolates for parallel execution.
Future<int> sumAsync(int a, int b) async {
  final SendPort helperIsolateSendPort = await _helperIsolateSendPort;
  final int requestId = _nextSumRequestId++;
  final _SumRequest request = _SumRequest(requestId, a, b);
  final Completer<int> completer = Completer<int>();
  _sumRequests[requestId] = completer;
  helperIsolateSendPort.send(request);
  return completer.future;
}

String opencvVersion() {
  return _bindings.opencv_version().cast<Utf8>().toDartString();
}

Uint8List detectEdgesRgba(
  Uint8List rgba,
  int width,
  int height, {
  int lowThreshold = 50,
  int highThreshold = 150,
}) {
  final Pointer<Uint8> inputPointer = calloc<Uint8>(rgba.length);
  final Pointer<Int32> outputLengthPointer = calloc<Int32>();
  Pointer<Uint8> outputPointer = nullptr;

  try {
    inputPointer.asTypedList(rgba.length).setAll(0, rgba);
    outputPointer = _bindings.opencv_detect_edges_rgba(
      inputPointer,
      width,
      height,
      lowThreshold,
      highThreshold,
      outputLengthPointer,
    );

    if (outputPointer == nullptr || outputLengthPointer.value <= 0) {
      throw StateError('OpenCV edge detection failed.');
    }

    return Uint8List.fromList(
      outputPointer.asTypedList(outputLengthPointer.value),
    );
  } finally {
    calloc.free(inputPointer);
    calloc.free(outputLengthPointer);
    if (outputPointer != nullptr) {
      _bindings.opencv_free(outputPointer);
    }
  }
}

Float32List detectDocument8PointsRgba(
  Uint8List rgba,
  int width,
  int height,
) {
  final Pointer<Uint8> inputPointer = calloc<Uint8>(rgba.length);
  final Pointer<Float> pointsPointer = calloc<Float>(16);

  try {
    inputPointer.asTypedList(rgba.length).setAll(0, rgba);
    final int result = _bindings.opencv_detect_document_8_points_rgba(
      inputPointer,
      width,
      height,
      pointsPointer,
    );

    if (result == 0) {
      throw StateError('OpenCV document 8 points detection failed.');
    }

    return Float32List.fromList(pointsPointer.asTypedList(16));
  } finally {
    calloc.free(inputPointer);
    calloc.free(pointsPointer);
  }
}

OpenCvImageResult cropEnhanceDocument8PointsRgba(
  Uint8List rgba,
  int width,
  int height,
  Float32List points,
) {
  if (points.length != 16) {
    throw ArgumentError.value(points.length, 'points.length', 'must be 16');
  }

  final Pointer<Uint8> inputPointer = calloc<Uint8>(rgba.length);
  final Pointer<Float> pointsPointer = calloc<Float>(16);
  final Pointer<Int32> outputWidthPointer = calloc<Int32>();
  final Pointer<Int32> outputHeightPointer = calloc<Int32>();
  final Pointer<Int32> outputLengthPointer = calloc<Int32>();
  Pointer<Uint8> outputPointer = nullptr;

  try {
    inputPointer.asTypedList(rgba.length).setAll(0, rgba);
    pointsPointer.asTypedList(16).setAll(0, points);
    outputPointer = _bindings.opencv_crop_enhance_document_8_points_rgba(
      inputPointer,
      width,
      height,
      pointsPointer,
      outputWidthPointer,
      outputHeightPointer,
      outputLengthPointer,
    );

    if (outputPointer == nullptr ||
        outputWidthPointer.value <= 0 ||
        outputHeightPointer.value <= 0 ||
        outputLengthPointer.value <= 0) {
      throw StateError('OpenCV crop enhance document failed.');
    }

    return OpenCvImageResult(
      rgba: Uint8List.fromList(
        outputPointer.asTypedList(outputLengthPointer.value),
      ),
      width: outputWidthPointer.value,
      height: outputHeightPointer.value,
    );
  } finally {
    calloc.free(inputPointer);
    calloc.free(pointsPointer);
    calloc.free(outputWidthPointer);
    calloc.free(outputHeightPointer);
    calloc.free(outputLengthPointer);
    if (outputPointer != nullptr) {
      _bindings.opencv_free(outputPointer);
    }
  }
}

const String _libName = 'flutter_opencv_ffi';

/// The dynamic library in which the symbols for [FlutterOpencvFfiBindings] can be found.
final DynamicLibrary _dylib = () {
  if (Platform.isMacOS || Platform.isIOS) {
    return DynamicLibrary.open('$_libName.framework/$_libName');
  }
  if (Platform.isAndroid || Platform.isLinux || Platform.isOhos) {
    return DynamicLibrary.open('lib$_libName.so');
  }
  if (Platform.isWindows) {
    return DynamicLibrary.open('$_libName.dll');
  }
  throw UnsupportedError('Unknown platform: ${Platform.operatingSystem}');
}();

/// The bindings to the native functions in [_dylib].
final FlutterOpencvFfiBindings _bindings = FlutterOpencvFfiBindings(_dylib);


/// A request to compute `sum`.
///
/// Typically sent from one isolate to another.
class _SumRequest {
  final int id;
  final int a;
  final int b;

  const _SumRequest(this.id, this.a, this.b);
}

/// A response with the result of `sum`.
///
/// Typically sent from one isolate to another.
class _SumResponse {
  final int id;
  final int result;

  const _SumResponse(this.id, this.result);
}

/// Counter to identify [_SumRequest]s and [_SumResponse]s.
int _nextSumRequestId = 0;

/// Mapping from [_SumRequest] `id`s to the completers corresponding to the correct future of the pending request.
final Map<int, Completer<int>> _sumRequests = <int, Completer<int>>{};

/// The SendPort belonging to the helper isolate.
Future<SendPort> _helperIsolateSendPort = () async {
  // The helper isolate is going to send us back a SendPort, which we want to
  // wait for.
  final Completer<SendPort> completer = Completer<SendPort>();

  // Receive port on the main isolate to receive messages from the helper.
  // We receive two types of messages:
  // 1. A port to send messages on.
  // 2. Responses to requests we sent.
  final ReceivePort receivePort = ReceivePort()
    ..listen((dynamic data) {
      if (data is SendPort) {
        // The helper isolate sent us the port on which we can sent it requests.
        completer.complete(data);
        return;
      }
      if (data is _SumResponse) {
        // The helper isolate sent us a response to a request we sent.
        final Completer<int> completer = _sumRequests[data.id]!;
        _sumRequests.remove(data.id);
        completer.complete(data.result);
        return;
      }
      throw UnsupportedError('Unsupported message type: ${data.runtimeType}');
    });

  // Start the helper isolate.
  await Isolate.spawn((SendPort sendPort) async {
    final ReceivePort helperReceivePort = ReceivePort()
      ..listen((dynamic data) {
        // On the helper isolate listen to requests and respond to them.
        if (data is _SumRequest) {
          final int result = _bindings.sum_long_running(data.a, data.b);
          final _SumResponse response = _SumResponse(data.id, result);
          sendPort.send(response);
          return;
        }
        throw UnsupportedError('Unsupported message type: ${data.runtimeType}');
      });

    // Send the port to the main isolate on which we can receive requests.
    sendPort.send(helperReceivePort.sendPort);
  }, receivePort.sendPort);

  // Wait until the helper isolate has sent us back the SendPort on which we
  // can start sending requests.
  return completer.future;
}();
