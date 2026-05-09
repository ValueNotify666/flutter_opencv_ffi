#include "flutter_opencv_ffi.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

#ifdef HAVE_OPENCV
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#endif

FFI_PLUGIN_EXPORT int sum(int a, int b) { return a + b; }

FFI_PLUGIN_EXPORT int sum_long_running(int a, int b) {
#if _WIN32
  Sleep(5000);
#else
  usleep(5000 * 1000);
#endif
  return a + b;
}

FFI_PLUGIN_EXPORT const char* opencv_version(void) {
#ifdef HAVE_OPENCV
  return CV_VERSION;
#else
  return "OpenCV unavailable";
#endif
}

#ifdef HAVE_OPENCV
static void write_rect_8_points(const cv::Rect& rect, float* points) {
  const float left = static_cast<float>(rect.x);
  const float top = static_cast<float>(rect.y);
  const float right = static_cast<float>(rect.x + rect.width);
  const float bottom = static_cast<float>(rect.y + rect.height);
  const float center_x = (left + right) * 0.5f;
  const float center_y = (top + bottom) * 0.5f;
  const float values[16] = {
      left, top,
      center_x, top,
      right, top,
      right, center_y,
      right, bottom,
      center_x, bottom,
      left, bottom,
      left, center_y,
  };
  std::memcpy(points, values, sizeof(values));
}

static void write_rotated_rect_8_points(const cv::RotatedRect& rect, float* points) {
  cv::Point2f corners[4];
  rect.points(corners);

  cv::Point2f top_left = corners[0];
  cv::Point2f top_right = corners[0];
  cv::Point2f bottom_right = corners[0];
  cv::Point2f bottom_left = corners[0];
  double min_sum = corners[0].x + corners[0].y;
  double max_sum = min_sum;
  double min_diff = corners[0].x - corners[0].y;
  double max_diff = min_diff;
  for (int i = 1; i < 4; i++) {
    const double sum = corners[i].x + corners[i].y;
    const double diff = corners[i].x - corners[i].y;
    if (sum < min_sum) {
      min_sum = sum;
      top_left = corners[i];
    }
    if (sum > max_sum) {
      max_sum = sum;
      bottom_right = corners[i];
    }
    if (diff > max_diff) {
      max_diff = diff;
      top_right = corners[i];
    }
    if (diff < min_diff) {
      min_diff = diff;
      bottom_left = corners[i];
    }
  }

  const cv::Point2f top_center = (top_left + top_right) * 0.5f;
  const cv::Point2f right_center = (top_right + bottom_right) * 0.5f;
  const cv::Point2f bottom_center = (bottom_left + bottom_right) * 0.5f;
  const cv::Point2f left_center = (top_left + bottom_left) * 0.5f;
  const float values[16] = {
      top_left.x, top_left.y,
      top_center.x, top_center.y,
      top_right.x, top_right.y,
      right_center.x, right_center.y,
      bottom_right.x, bottom_right.y,
      bottom_center.x, bottom_center.y,
      bottom_left.x, bottom_left.y,
      left_center.x, left_center.y,
  };
  std::memcpy(points, values, sizeof(values));
}

static bool detect_best_document_rect(
    const uint8_t* rgba,
    int32_t width,
    int32_t height,
    cv::RotatedRect* best_rect) {
  if (rgba == nullptr || best_rect == nullptr || width <= 0 || height <= 0) {
    return false;
  }

  cv::Mat source(height, width, CV_8UC4, const_cast<uint8_t*>(rgba));
  cv::Mat input;
  const double max_detection_side = 960.0;
  const double source_max_side = static_cast<double>(std::max(width, height));
  const double scale =
      source_max_side > max_detection_side ? max_detection_side / source_max_side : 1.0;
  if (scale < 1.0) {
    cv::resize(source, input, cv::Size(), scale, scale, cv::INTER_AREA);
  } else {
    input = source;
  }
  const int work_width = input.cols;
  const int work_height = input.rows;
  const cv::Rect work_bounds(0, 0, work_width, work_height);
  const double work_area = static_cast<double>(work_width) * work_height;

  cv::Mat gray;
  cv::cvtColor(input, gray, cv::COLOR_RGBA2GRAY);

  cv::Mat blurred;
  cv::GaussianBlur(gray, blurred, cv::Size(3, 3), 0.8);

  cv::Mat text_enhanced;
  cv::morphologyEx(
      blurred,
      text_enhanced,
      cv::MORPH_BLACKHAT,
      cv::getStructuringElement(
          cv::MORPH_RECT,
          cv::Size(std::max(13, work_width / 85), std::max(7, work_height / 170))));
  cv::Mat text_mask;
  cv::threshold(text_enhanced, text_mask, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
  cv::morphologyEx(
      text_mask,
      text_mask,
      cv::MORPH_OPEN,
      cv::getStructuringElement(
          cv::MORPH_RECT,
          cv::Size(2, 2)));
  cv::dilate(
      text_mask,
      text_mask,
      cv::getStructuringElement(
          cv::MORPH_RECT,
          cv::Size(std::max(2, work_width / 520), std::max(2, work_height / 520))));

  cv::Mat text_cluster_mask;
  cv::dilate(
      text_mask,
      text_cluster_mask,
      cv::getStructuringElement(
          cv::MORPH_RECT,
          cv::Size(std::max(21, work_width / 50), std::max(13, work_height / 80))));
  cv::morphologyEx(
      text_cluster_mask,
      text_cluster_mask,
      cv::MORPH_CLOSE,
      cv::getStructuringElement(
          cv::MORPH_RECT,
          cv::Size(std::max(17, work_width / 70), std::max(11, work_height / 100))));

  cv::Mat bright_mask;
  cv::threshold(blurred, bright_mask, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(text_cluster_mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

  double best_score = 0.0;
  cv::RotatedRect selected;

  for (const std::vector<cv::Point>& cluster_contour : contours) {
    cv::Rect cluster_rect = cv::boundingRect(cluster_contour) & work_bounds;
    if (cluster_rect.width <= 0 || cluster_rect.height <= 0) {
      continue;
    }
    const double cluster_area = static_cast<double>(cluster_rect.area());
    if (cluster_area < work_area * 0.0004 || cluster_area > work_area * 0.20) {
      continue;
    }

    cv::Rect score_rect(
        cluster_rect.x - std::max(6, cluster_rect.width / 2),
        cluster_rect.y - std::max(6, cluster_rect.height / 2),
        cluster_rect.width * 2,
        cluster_rect.height * 2);
    score_rect = score_rect & work_bounds;
    if (score_rect.width <= 0 || score_rect.height <= 0) {
      continue;
    }

    const cv::Mat text_roi = text_mask(score_rect);
    const cv::Mat bright_roi = bright_mask(score_rect);
    const int text_pixels = cv::countNonZero(text_roi);
    const int bright_pixels = cv::countNonZero(bright_roi);
    const double score_area = static_cast<double>(score_rect.area());
    if (text_pixels < std::max(16, static_cast<int>(score_area * 0.003))) {
      continue;
    }

    std::vector<std::vector<cv::Point>> text_contours;
    cv::Mat text_roi_copy = text_roi.clone();
    cv::findContours(text_roi_copy, text_contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    int text_component_count = 0;
    double text_component_area = 0.0;
    std::vector<cv::Point> text_points;
    for (const std::vector<cv::Point>& text_contour : text_contours) {
      cv::Rect text_rect = cv::boundingRect(text_contour);
      const double component_area = static_cast<double>(text_rect.area());
      if (component_area < 3 || component_area > score_area * 0.12) {
        continue;
      }
      const double component_aspect =
          static_cast<double>(text_rect.width) / std::max(1, text_rect.height);
      if (component_aspect < 0.08 || component_aspect > 18.0) {
        continue;
      }
      text_component_count++;
      text_component_area += component_area;
      for (const cv::Point& point : text_contour) {
        text_points.push_back(cv::Point(point.x + score_rect.x, point.y + score_rect.y));
      }
    }
    if (text_component_count < 4 || text_points.size() < 8) {
      continue;
    }

    const double text_density =
        static_cast<double>(text_pixels) / std::max(1.0, score_area);
    const double bright_ratio =
        static_cast<double>(bright_pixels) / std::max(1.0, score_area);
    if (bright_ratio < 0.18) {
      continue;
    }

    const double aspect =
        static_cast<double>(cluster_rect.width) / std::max(1, cluster_rect.height);
    const double aspect_score =
        1.0 - std::min(1.0, std::abs(aspect - 1.8) / 3.2);
    const double text_amount_score =
        std::min(1.0, static_cast<double>(text_pixels) / (work_area * 0.018));
    const double text_component_score =
        std::min(1.0, static_cast<double>(text_component_count) / 24.0);
    const double text_density_score = std::min(1.0, text_density / 0.10);
    const double bright_score = std::min(1.0, bright_ratio / 0.70);
    const double compact_score =
        1.0 - std::min(1.0, cluster_area / std::max(1.0, work_area * 0.14));
    const double score =
        text_amount_score * 3.00 + text_component_score * 2.20 +
        text_density_score * 1.35 + bright_score * 0.60 +
        aspect_score * 0.35 + compact_score * 1.40 +
        std::min(1.0, text_component_area / std::max(1.0, score_area * 0.16)) * 0.55;

    if (score > best_score) {
      best_score = score;
      selected = cv::minAreaRect(text_points);
      selected.size.width *= 1.08f;
      selected.size.height *= 1.16f;
    }
  }

  if (best_score <= 0.0) {
    return false;
  }

  if (scale < 1.0) {
    selected.center.x = static_cast<float>(selected.center.x / scale);
    selected.center.y = static_cast<float>(selected.center.y / scale);
    selected.size.width = static_cast<float>(selected.size.width / scale);
    selected.size.height = static_cast<float>(selected.size.height / scale);
  }

  *best_rect = selected;
  return true;
}

static cv::Point2f point_from_8(const float* points, int index) {
  return cv::Point2f(points[index * 2], points[index * 2 + 1]);
}

static cv::Point2f average_points(
    const cv::Point2f& a,
    const cv::Point2f& b) {
  return cv::Point2f((a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f);
}
#endif

FFI_PLUGIN_EXPORT uint8_t* opencv_detect_edges_rgba(
    const uint8_t* rgba,
    int32_t width,
    int32_t height,
    int32_t low_threshold,
    int32_t high_threshold,
    int32_t* output_length) {
  if (output_length == nullptr) {
    return nullptr;
  }
  *output_length = 0;

  if (rgba == nullptr || width <= 0 || height <= 0) {
    return nullptr;
  }

  const int32_t byte_length = width * height * 4;
  if (byte_length <= 0) {
    return nullptr;
  }

  uint8_t* output = static_cast<uint8_t*>(malloc(byte_length));
  if (output == nullptr) {
    return nullptr;
  }

#ifdef HAVE_OPENCV
  cv::Mat input(height, width, CV_8UC4, const_cast<uint8_t*>(rgba));
  cv::Mat gray;
  cv::cvtColor(input, gray, cv::COLOR_RGBA2GRAY);

  cv::Mat blurred;
  cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.4);

  cv::Mat edges;
  cv::Canny(blurred, edges, low_threshold, high_threshold);

  cv::Mat result(height, width, CV_8UC4);
  std::memcpy(result.data, rgba, byte_length);

  const int horizontal_kernel_width = std::max(9, width / 45);
  const int vertical_kernel_height = std::max(5, height / 120);
  cv::Mat merged;
  cv::morphologyEx(
      edges,
      merged,
      cv::MORPH_CLOSE,
      cv::getStructuringElement(
          cv::MORPH_RECT,
          cv::Size(horizontal_kernel_width, vertical_kernel_height)));
  cv::dilate(
      merged,
      merged,
      cv::getStructuringElement(
          cv::MORPH_RECT,
          cv::Size(std::max(5, width / 100), std::max(5, height / 100))));

  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(merged, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

  const double image_area = static_cast<double>(width) * height;
  double best_score = 0.0;
  cv::Rect best_rect;

  for (const std::vector<cv::Point>& contour : contours) {
    cv::Rect rect = cv::boundingRect(contour) & cv::Rect(0, 0, width, height);
    const double rect_area = static_cast<double>(rect.area());
    if (rect.width <= 0 || rect.height <= 0) {
      continue;
    }
    if (rect_area < image_area * 0.015 || rect_area > image_area * 0.75) {
      continue;
    }

    const double aspect = static_cast<double>(rect.width) / rect.height;
    if (aspect < 0.20 || aspect > 2.20) {
      continue;
    }

    const cv::Mat edge_roi = edges(rect);
    const double edge_density =
        static_cast<double>(cv::countNonZero(edge_roi)) / rect_area;
    if (edge_density < 0.015 || edge_density > 0.45) {
      continue;
    }

    const double contour_area = std::max(1.0, cv::contourArea(contour));
    const double rectangularity = std::min(1.0, contour_area / rect_area);
    const double area_ratio = rect_area / image_area;
    const double aspect_score =
        1.0 - std::min(1.0, std::abs(aspect - 0.65) / 1.55);
    const double density_score = std::min(1.0, edge_density / 0.12);
    const double score =
        density_score * 0.50 + area_ratio * 1.80 + rectangularity * 0.35 +
        aspect_score * 0.35;

    if (score > best_score) {
      best_score = score;
      best_rect = rect;
    }
  }

  if (best_score > 0.0) {
    const int thickness = std::max(3, std::min(width, height) / 120);
    cv::rectangle(result, best_rect, cv::Scalar(255, 0, 0, 255), thickness);
  }

  std::memcpy(output, result.data, byte_length);
#else
  std::memcpy(output, rgba, byte_length);
#endif

  *output_length = byte_length;
  return output;
}

FFI_PLUGIN_EXPORT int32_t opencv_detect_document_8_points_rgba(
    const uint8_t* rgba,
    int32_t width,
    int32_t height,
    float* points) {
  if (points == nullptr) {
    return 0;
  }

#ifdef HAVE_OPENCV
  cv::RotatedRect rect;
  if (!detect_best_document_rect(rgba, width, height, &rect)) {
    return 0;
  }

  write_rotated_rect_8_points(rect, points);
  return 1;
#else
  return 0;
#endif
}

FFI_PLUGIN_EXPORT uint8_t* opencv_crop_enhance_document_8_points_rgba(
    const uint8_t* rgba,
    int32_t width,
    int32_t height,
    const float* points,
    int32_t* output_width,
    int32_t* output_height,
    int32_t* output_length) {
  if (output_width == nullptr || output_height == nullptr ||
      output_length == nullptr) {
    return nullptr;
  }
  *output_width = 0;
  *output_height = 0;
  *output_length = 0;

  if (rgba == nullptr || points == nullptr || width <= 0 || height <= 0) {
    return nullptr;
  }

#ifdef HAVE_OPENCV
  const cv::Point2f top_left = point_from_8(points, 0);
  const cv::Point2f top_right = point_from_8(points, 2);
  const cv::Point2f bottom_right = point_from_8(points, 4);
  const cv::Point2f bottom_left = point_from_8(points, 6);

  const double top_width = cv::norm(top_right - top_left);
  const double bottom_width = cv::norm(bottom_right - bottom_left);
  const double left_height = cv::norm(bottom_left - top_left);
  const double right_height = cv::norm(bottom_right - top_right);
  int32_t target_width =
      std::max(1, static_cast<int32_t>(std::round((top_width + bottom_width) * 0.5)));
  int32_t target_height =
      std::max(1, static_cast<int32_t>(std::round((left_height + right_height) * 0.5)));
  const int32_t byte_length = target_width * target_height * 4;
  if (byte_length <= 0) {
    return nullptr;
  }

  cv::Point2f source_points[4] = {
      top_left,
      top_right,
      bottom_right,
      bottom_left,
  };
  cv::Point2f target_points[4] = {
      cv::Point2f(0, 0),
      cv::Point2f(static_cast<float>(target_width - 1), 0),
      cv::Point2f(static_cast<float>(target_width - 1),
                  static_cast<float>(target_height - 1)),
      cv::Point2f(0, static_cast<float>(target_height - 1)),
  };

  cv::Mat input(height, width, CV_8UC4, const_cast<uint8_t*>(rgba));
  cv::Mat transform = cv::getPerspectiveTransform(source_points, target_points);
  cv::Mat warped;
  cv::warpPerspective(
      input,
      warped,
      transform,
      cv::Size(target_width, target_height),
      cv::INTER_LINEAR,
      cv::BORDER_REPLICATE);

  cv::Mat gray;
  cv::cvtColor(warped, gray, cv::COLOR_RGBA2GRAY);
  cv::Mat enhanced;
  cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
  clahe->apply(gray, enhanced);
  cv::Mat blurred;
  cv::GaussianBlur(enhanced, blurred, cv::Size(0, 0), 1.0);
  cv::addWeighted(enhanced, 1.35, blurred, -0.35, 0, enhanced);

  cv::Mat rgba_output;
  cv::cvtColor(enhanced, rgba_output, cv::COLOR_GRAY2RGBA);

  uint8_t* output = static_cast<uint8_t*>(malloc(byte_length));
  if (output == nullptr) {
    return nullptr;
  }
  std::memcpy(output, rgba_output.data, byte_length);

  *output_width = target_width;
  *output_height = target_height;
  *output_length = byte_length;
  return output;
#else
  return nullptr;
#endif
}

FFI_PLUGIN_EXPORT void opencv_free(uint8_t* pointer) {
  free(pointer);
}
