# Logger Library

[![GitHub stars](https://img.shields.io/github/stars/yourusername/project1)](https://github.com/yourusername/project1/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/yourusername/project1)](https://github.com/yourusername/project1/network)
[![GitHub issues](https://img.shields.io/github/issues/yourusername/project1)](https://github.com/yourusername/project1/issues)
[![GitHub license](https://img.shields.io/github/license/yourusername/project1)](https://github.com/yourusername/project1/blob/master/LICENSE)
[![Build Status](https://github.com/yourusername/project1/workflows/CI/badge.svg)](https://github.com/yourusername/project1/actions)

A thread-safe, asynchronous logging library built as a wrapper around spdlog.

## ✨ Features

- 🔄 Asynchronous logging support
- 🔒 Thread-safe singleton design
- 📚 Multiple logger instances support
- ⚙️ Default logger configuration
- 📍 Source location tracking
- 🎨 Both formatted and stream-style logging
- 🔌 DLL export support for Windows

## 📋 Dependencies
- spdlog v1.15.3 or later
- vcpkg package manager
- CMake 3.20.0 or later
- opencv 4.5.1 or later
- C++17 compiler

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

```cpp
#include "logger/ILogger.hpp"

int main() {
    // Get logger instance
    auto& logger = CHUtils::ILogger::s_GetInstance();
    
    // Create and set default logger
    logger.addLogger("main_logger", "logs/main.log");
    logger.setDefaultLogger("main_logger");
    
    // Log messages
    logger.info({"main.cpp", __LINE__, __FUNCTION__}, "Hello, World!");
    logger.info({"main.cpp", __LINE__, __FUNCTION__}, "Value is {}", 42);
    
    // Clean up
    CHUtils::ILogger::s_DestroyInstance();
    return 0;
}
```

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

The library supports both synchronous and asynchronous logging modes:

### Default Settings
- ✅ Async mode enabled
- 📊 Thread pool size: 8192
- 🧵 Worker threads: 1
- 🐛 Debug level logging enabled
- 🔄 Auto-flush on debug level
- ⏱️ 500ms flush interval

### CMake Options

```cmake
set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_TARGET_TRIPLET "x64-windows")
```

## 📈 Performance

- Asynchronous logging for minimal impact on application performance
- Thread-safe operations without locks in the fast path
- Efficient memory usage with thread pool

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