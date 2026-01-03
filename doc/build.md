# SOUI4 构建指南

SOUI4 是一个轻量级的跨平台客户端开发框架，核心编译后体积约 1MB，支持 Windows、Linux 和 macOS 平台。

## 环境准备

### Windows

1. **CMake**：版本 3.16 或更高，从 [CMake 官网](https://cmake.org/download/) 下载
2. **Git**：用于克隆项目及子模块
3. **编译器**：
   - Visual Studio 2019 或更高版本，或
   - MinGW-w64 (推荐使用 MSYS2 提供的版本)
4. **可选工具**：
   - Visual Studio Code 或 Visual Studio
   - Python (用于某些辅助脚本)

### Linux

1. **基础构建工具**：
   ```bash
   # Ubuntu/Debian
   sudo apt install cmake git build-essential libxcb1-dev libxcb-render0-dev libgl1-mesa-dev freeglut3-dev uuid-dev libasound2-dev pkg-config
   
   # CentOS/RHEL/Fedora
   sudo yum install cmake git  build-essential libxcb-devel xcb-util-renderutil-devel mesa-libGL-devel libuuid-devel pkg-config
   ```

2. **可选工具**：
   - Ninja (更快的构建系统)

### macOS

1. **基础构建工具**：
   ```bash
   # 安装 Homebrew (如果尚未安装)
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   
   # 安装必要工具
   brew tap homebrew/core && brew tap homebrew/cask
   brew install cmake ninja pkgconf glfw3 glew
   ```

## 获取源码

```bash
git clone https://gitee.com/setoutsoft/soui4.git
cd soui4
git submodule update --init
```

> **注意**：确保子模块正确初始化，特别是 swinx 子模块，否则 Linux/macOS 构建会失败。

## CMake 配置选项

SOUI4 提供了多个 CMake 配置选项：

- `SOUI_BUILD_DEMOS`：构建演示程序 (默认 ON)
- `SOUI_SHARED_CRT`：使用共享 CRT 运行时库 (默认 ON)
- `SOUI_USE_UNICODE`：使用 Unicode (Windows 默认 ON, 其他平台 OFF)
- `SOUI_WCHAR_AS_DEFAULT`：使用 wchar_t 作为内部类型 (默认 ON)
- `SOUI_ENABLE_CORE_LIB`：将 'core' 编译为静态库 (默认 OFF)
- `SOUI_ENABLE_COM_LIB`：将 'components' 编译为静态库 (默认 OFF)
- `SOUI_XP_TOOLSET`：使用 XP 工具集 (默认 ON)
- `SOUI_ENABLE_SPECTRE`：启用 Spectre 缓解 (默认 OFF)
- `SOUI_BUILD_WS`：构建 WebSocket 模块 (默认 ON，如果可用)
- `SOUI_BUILD_RICHEDIT`：使用源码中的 richedit (Windows 默认 OFF, 其他平台 ON)
- `SOUI_ENABLE_ACC`：启用 Acc (Windows 默认 ON, 其他平台 OFF)
- `SOUI_ENABLE_SPY`：启用 Swnd Spy (Windows 默认 ON, 其他平台 OFF)

## Windows 构建

### 使用 Visual Studio

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

### 使用 MinGW

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
make -j8
```

### 使用 Ninja (更快的构建)

```bash
mkdir build && cd build
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Linux/macOS 构建

```bash
# Linux
git submodule update --init
mkdir build && cd build
cmake .. && make -j8

# macOS
git submodule update --init
mkdir build && cd build
cmake .. -G Ninja && cmake --build .
```

## 交叉编译

### Linux ARM 32位交叉编译

```bash
# 安装交叉编译工具链
sudo apt install gcc-arm-linux-gnueabihf

# 创建工具链文件 arm-toolchain.cmake
cat > arm-toolchain.cmake << EOF
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
EOF

# 配置和构建
mkdir build-arm && cd build-arm
cmake .. -DCMAKE_TOOLCHAIN_FILE=../arm-toolchain.cmake
make -j8
```

## 使用 CMake GUI (Windows)

1. 运行 CMake GUI
2. 设置源码目录为 SOUI4 根目录
3. 设置构建目录 (例如 build)
4. 点击 Configure，选择生成器 (如 Visual Studio 16 2019)
5. 根据需要调整选项
6. 点击 Generate 生成项目文件
7. 点击 Open Project 打开项目

## 构建输出

构建完成后，输出文件位于：

- **Windows**: `bin/` 目录 (可执行文件) 和 `lib/` 目录 (库文件)
- **Linux/macOS**: `build/` 目录下的相应位置

## 常见问题

### 1. VS2013 及以下版本兼容性

VS2013 及以下版本不支持 WebSocket 模块，如果需要使用该功能，请升级到 VS2015 或更高版本。

### 2. MSYS 环境问题

在 MSYS 环境下，CMake 会错误地检测系统为 Linux，项目会自动覆盖系统识别为 Windows。

### 3. swinx 子模块

swinx 子模块必须正确初始化，否则 Linux/macOS 构建会失败。使用以下命令确保初始化：

```bash
git submodule update --init --recursive
```

## 部署

### Windows 部署

- 将生成的二进制文件和资源文件（fonts、themes、scripts）打包发布
- 注意 CRT 运行时依赖（静态/动态链接配置）

### Linux/macOS 部署

- 链接 swinx 库以提供 Windows API 兼容层
- 确保必要的运行时库已安装

## 调试配置

SOUI4 支持标准的 C++ 调试工具：

- **Windows**: Visual Studio 调试器或 CDB
- **Linux**: GDB
- **macOS**: LLDB

在构建时使用 Debug 配置以包含调试信息：

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
```