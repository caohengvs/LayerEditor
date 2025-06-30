#include "ImageProcessor.hpp"
#include <opencv2/opencv.hpp>

int ImageProcessor::processImage(const std::string& imagePath)
{
   // --- 配置部分：根据你的水印特性调整 ---

    // 2. 水印颜色范围 (HSV 颜色空间)
    // 这是最关键的参数，需要根据你的水印颜色进行调整。
    // 你可以使用图像编辑软件（如 Photoshop, GIMP）查看水印的 HSV 值，
    // 或者通过尝试不同的值来找到最佳范围。
    //
    // 示例：假设水印是蓝色
    // H (Hue): 色相 (0-179)
    // S (Saturation): 饱和度 (0-255)
    // V (Value/Brightness): 亮度 (0-255)
    //
    // 如果水印是白色或灰色（高亮度，低饱和度）：
    cv::Scalar lower_watermark_color = cv::Scalar(0, 0, 200);
    cv::Scalar upper_watermark_color = cv::Scalar(179, 50, 255);
    //
    // 如果水印是黑色（低亮度）：
    // cv::Scalar lower_watermark_color = cv::Scalar(0, 0, 0);
    // cv::Scalar upper_watermark_color = cv::Scalar(179, 255, 50);
    //
    // 如果水印是特定颜色，例如蓝色：
    // cv::Scalar lower_watermark_color = cv::Scalar(100, 50, 50);  // 示例：蓝色下限 (H,S,V)
    // cv::Scalar upper_watermark_color = cv::Scalar(130, 255, 255); // 示例：蓝色上限 (H,S,V)


    // 3. 形态学操作核大小和迭代次数 (用于优化掩码)
    // 调整这些值可以帮助连接水印碎片或去除背景中的噪点
    int kernel_size = 5; // 建议3到7，越大效果越平滑，可能去除更多细节
    int dilate_iterations = 1; // 膨胀次数，可以帮助扩大水印区域，确保完全覆盖
    int erode_iterations = 1;  // 腐蚀次数，可以帮助收缩水印区域，去除边缘噪点


    // 4. 图像修复半径和算法
    int inpaintRadius = 3; // 修复半径：影响周围多少像素被考虑，建议3到5
    // cv::INPAINT_TELEA: 推荐，通常对大多数图像效果更好，尤其是平滑区域和纹理。
    // cv::INPAINT_NS: 适用于有清晰边缘或结构的区域，可能保留更多锐度。
    int inpaintAlgorithm = cv::INPAINT_TELEA;

    // --- 代码主逻辑 ---

    // 1. 加载 PNG 图像 (包括 alpha 通道，尽管在此示例中主要使用颜色检测)
    cv::Mat src = cv::imread(imagePath, cv::IMREAD_UNCHANGED);
    if (src.empty()) {
        std::cerr << "Error: Could not open or find the image at " << imagePath << std::endl;
        return -1;
    }

    // 提取 RGB/BGR 部分进行颜色处理
    cv::Mat bgr_src;
    if (src.channels() == 4) { // 如果是 RGBA
        cv::cvtColor(src, bgr_src, cv::COLOR_BGRA2BGR);
    } else { // 如果是 RGB
        bgr_src = src.clone();
    }

    // 2. 将图像转换为 HSV 颜色空间，以便进行颜色阈值处理
    cv::Mat hsv_src;
    cv::cvtColor(bgr_src, hsv_src, cv::COLOR_BGR2HSV);

    // 3. 基于颜色范围创建水印的初始掩码
    cv::Mat watermark_mask;
    cv::inRange(hsv_src, lower_watermark_color, upper_watermark_color, watermark_mask);

    // 4. 应用形态学操作优化掩码
    // 创建形态学操作的核
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernel_size, kernel_size));

    // 闭运算：先膨胀后腐蚀，用于填充小孔和连接附近的区域
    cv::morphologyEx(watermark_mask, watermark_mask, cv::MORPH_CLOSE, kernel);

    // 膨胀：扩大水印区域，确保完全覆盖水印
    cv::dilate(watermark_mask, watermark_mask, kernel, cv::Point(-1,-1), dilate_iterations);

    // 腐蚀 (可选，如果膨胀过度导致去除背景)：收缩水印区域，去除边缘噪点
    // cv::erode(watermark_mask, watermark_mask, kernel, cv::Point(-1,-1), erode_iterations);


    // 5. 应用图像修复
    cv::Mat dst_inpainted;
    cv::inpaint(src, watermark_mask, dst_inpainted, inpaintRadius, inpaintAlgorithm);

    // 6. 显示结果
    cv::imshow("Original Image", src);
    cv::imshow("Generated Watermark Mask", watermark_mask);
    cv::imshow("Inpainted Result", dst_inpainted);

    // 等待按键，然后关闭所有窗口
    cv::waitKey(0);
    cv::destroyAllWindows();

    // 可选：保存结果图像
    // cv::imwrite("inpainted_image.png", dst_inpainted);

    return 0;
}

void ImageProcessor::saveImage(const std::string& outputPath)
{
}
