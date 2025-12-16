# 中国象棋游戏 (SOUI框架移植版)

这是一个将传统的tkchess象棋游戏移植到SOUI4框架的项目。

## 项目结构

```
cnchess/
├── algorithm/          # 象棋算法实现
│   ├── ChessAlgorithm.h
│   ├── ChessAlgorithm.cpp
│   └── CMakeLists.txt
├── client/             # 客户端主程序
│   ├── uires/          # UI资源文件
│   ├── MainDlg.h       # 主对话框声明
│   ├── MainDlg.cpp     # 主对话框实现
│   ├── ChessGame.h     # 游戏逻辑声明
│   ├── ChessGame.cpp   # 游戏逻辑实现
│   ├── main.cpp        # 程序入口
│   ├── stdafx.h        # 预编译头文件
│   ├── stdafx.cpp      # 预编译头源文件
│   └── CMakeLists.txt
├── server/             # 服务器程序
│   ├── ChessServer.h   # 服务器主类声明
│   ├── ChessServer.cpp # 服务器主类实现
│   ├── ChessGame.h     # 游戏逻辑声明
│   ├── ChessGame.cpp   # 游戏逻辑实现
│   ├── server.cpp      # 服务器入口
│   ├── stdafx.h        # 预编译头文件
│   ├── stdafx.cpp      # 预编译头源文件
│   ├── config.ini      # 服务器配置文件
│   └── CMakeLists.txt
├── test/               # 算法测试模块
│   ├── ChessAlgorithmTest.cpp # 算法测试实现
│   └── CMakeLists.txt  # 测试模块构建配置
└── CMakeLists.txt      # 顶层CMake配置
```

## 编译说明

在项目根目录执行以下命令：

```bash
mkdir build
cd build
cmake ..
make
```

或者使用Visual Studio打开sln文件进行编译。

## 当前进度

- [x] 项目基本框架搭建
- [x] CMakeLists.txt配置文件创建
- [x] 主程序入口(main.cpp)
- [x] 主对话框(MainDlg.h/cpp)
- [x] 游戏逻辑框架(ChessGame.h/cpp)
- [x] 象棋算法基础类(ChessAlgorithm.h/cpp)
- [x] UI资源文件结构
- [x] 服务器框架(ChessServer.h/cpp)
- [x] 服务器游戏逻辑(ChessGame.h/cpp)
- [x] 算法测试模块(test/)
- [ ] 棋子资源图片
- [ ] 完整的游戏逻辑实现
- [ ] 网络对战功能
- [ ] AI对手功能

## 服务器使用说明

1. 编译服务器程序
2. 修改config.ini配置文件
3. 运行服务器程序: ./chess_server [config_file]

## 测试模块使用说明

1. 编译测试程序
2. 运行测试程序: ./cnchess_test

## 后续工作

1. 实现完整的象棋规则判定
2. 添加棋子图片资源
3. 实现用户交互逻辑
4. 添加音效和动画效果
5. 实现保存/加载棋局功能
6. 添加网络对战功能
7. 添加AI对手功能

## 使用的SOUI特性

- XML布局系统
- 事件映射机制
- 网格布局(Grid Layout)
- 窗口管理
- 资源管理系统
- WebSocket网络通信
- Google Test单元测试框架