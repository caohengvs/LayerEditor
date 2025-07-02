# Logger Library

[![GitHub stars](https://img.shields.io/github/stars/yourusername/project1)](https://github.com/yourusername/project1/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/yourusername/project1)](https://github.com/yourusername/project1/network)
[![GitHub issues](https://img.shields.io/github/issues/yourusername/project1)](https://github.com/yourusername/project1/issues)
[![GitHub license](https://img.shields.io/github/license/yourusername/project1)](https://github.com/yourusername/project1/blob/master/LICENSE)
[![Build Status](https://github.com/yourusername/project1/workflows/CI/badge.svg)](https://github.com/yourusername/project1/actions)

A thread-safe, asynchronous logging library built as a wrapper around spdlog.

## ✨ Features
待补充

## 📋 Dependencies
- spdlog v1.15.3 or later
- vcpkg package manager
- CMake 3.20.0 or later
- opencv 4.5.1 or later
- C++17 compiler(msvc)
- Qt5.12.12

## 🚀 Quick Start

### Building

```bash
# Clone the repository
git clone https://github.com/caohengvs/CppTools.git
cd CppTools

# Configure with vcpkg
cmake -B build -S .

# Build
cmake --build build --config Release
```

### Basic Usage

## 📁 Project Structure

```
CppTools/
├── Logger/           # 日志库模块（封装spdlog）
│   ├── include/      # 头文件（如CLogger.hpp、ILogger.hpp等）
│   ├── src/          # 源文件（如CLogger.cpp）
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
``

## ⚙️ Configuration

### CMake Options

```cmake
set(CMAKE_TOOLCHAIN_FILE "your vcpkg installed path/vcpkg.cmake")
set(VCPKG_TARGET_TRIPLET "x64-windows")
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

![GitHub Stats](https://github-readme-stats.vercel.app/api?username=yourusername&show_icons=true)
![GitHub Activity Graph](https://activity-graph.herokuapp.com/graph?username=yourusername&theme=github)

## 📫 Contact

- GitHub: [@caohengvs](https://github.com/caohengvs)
- Email: caohengvs@163.com

---
Made with ❤️ by [curtis](https://github.com/caohengvs)