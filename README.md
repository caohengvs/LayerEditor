# LayerEditor

[![GitHub stars](https://img.shields.io/github/stars/caohengvs/LayerEditor)](https://github.com/caohengvs/LayerEditor/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/caohengvs/LayerEditor)](https://github.com/caohengvs/LayerEditor/network)
[![GitHub issues](https://img.shields.io/github/issues/caohengvs/LayerEditor)](https://github.com/caohengvs/LayerEditor/issues)
[![GitHub license](https://img.shields.io/github/license/caohengvs/LayerEditor)](https://github.com/caohengvs/LayerEditor/blob/master/LICENSE)
<!-- [![Build Status](https://github.com/caohengvs/LayerEditor/actions/workflows/CI.yml/badge.svg)](https://github.com/caohengvs/LayerEditor/actions/workflows/CI.yml) -->

图像视频处理应用

## ✨ Features
- vcpkg package managed

## 📋 Dependencies
- spdlog v1.15.3 or later
- CMake 3.20.0 or later
- opencv 4.5.1 or later
- C++17 compiler(msvc)
- Qt5.12.12
- onnxruntime-win-x64-1.21.0 or later(需要自行下载，并放到D:\repos\3rdparty)
- Models(需要自行下载，并放到生成后的运行目录下:your code path\bin\models)，这里用的模型是 [lama_fp32.onnx](https://huggingface.co/Carve/LaMa-ONNX/tree/main)

## 🚀 Quick Start

### Building

```bash
# Clone the repository
git clone https://github.com/caohengvs/LayerEditor.git
cd LayerEditor

# Configure
cmake -B build -S .

# Build
cmake --build build --config Release
```

### Basic Usage

## 📁 Project Structure

```
LayerEditor/
├── Logger/           # 日志库模块（封装spdlog）
│   ├── include/      
│   ├── src/          
│   └── CMakeLists.txt
├── WindowManager/    # 窗口管理模块
│   ├── include/
│   ├── src/
│   └── CMakeLists.txt
├── ImageProcessor/   # 图像处理模块
│   ├── include/
│   ├── src/
│   └── CMakeLists.txt
├── CommonDef/        # 公共定义
│   └── include/
├── main.cpp          # 示例或主程序入口
├── CMakeLists.txt    # 项目主CMake配置
└── README.md         # 项目说明文档
```

## ⚙️ Configuration

### CMake Options
在根目录下，创建UserConfig.cmake.配置自定义环境路径:
```cmake
set(Qt5_ROOT "D:/Qt/5.12.12/msvc2017_64")
set(3rd_PARTY_ROOT "D:/repos/3rdParty")
set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain file")
set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
```

## 📈 Performance


## 🤝 Contributing

1. Fork it (https://github.com/caohengvs/CppTools/fork)
2. Create your feature branch (`git checkout -b feature/amazing`)
3. Commit your changes (`git commit -am 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing`)
5. Create a new Pull Request

## 📝 License

This project is [MIT](https://opensource.org/licenses/MIT) licensed.

## 🌟 Show your support

Give a ⭐️ if this project helped you!

## 📊 Stats

![GitHub Stats](https://github-readme-stats.vercel.app/api?username=caohengvs&show_icons=true)
<!-- ![GitHub Activity Graph](https://github-readme-stats.vercel.app/api/activities?username=caohengvs&hide_title=true&theme=github) -->

## 📫 Contact

- GitHub: [@caohengvs](https://github.com/caohengvs)
- Email: caohengvs@163.com

---
Made with ❤️ by [Curtis](https://github.com/caohengvs)