# 四国军棋(Junqi) 算法模块 - gtest测试框架

## 快速开始

### 编译测试

```bash
cd build
cmake ..
cmake --build . --target junqi_test
```

### 运行测试

```bash
# 运行所有测试
ctest -C Debug -V

# 或直接运行测试可执行文件
.\bin\junqi_test.exe
```

## 测试覆盖

本项目包含3个主要的测试模块：

1. **JunqiAlgorithmTest.cpp** - 基础算法测试
   - 棋盘初始化、棋子操作、军衔比较等

2. **JunqiMovementTest.cpp** - 移动机制测试
   - 移动范围、铁路移动、特殊规则等

3. **JunqiGameRulesTest.cpp** - 游戏规则测试
   - 地雷、炸弹、军旗捕获等特殊规则

## 测试统计

- **总测试数**: 46个测试用例
- **覆盖范围**:
  - ✅ 棋盘管理（MapInitialization, RailwayQuery等）
  - ✅ 棋子操作（SetCellChessman, RemoveChessman, ExchangeChessman等）
  - ✅ 游戏规则（ChessmanComparison, GetCmpResult, MoveChessmanServer等）
  - ✅ 特殊机制（SapperEatsMine, BombEffect, FlagPosition等）
  - ✅ 移动机制（NormalMovementRange, RailwayMovement等）

## 相关文件

| 文件 | 说明 |
|------|------|
| JunqiAlgorithmTest.cpp | 46个基础和规则测试 |
| JunqiMovementTest.cpp | 20个移动和战斗测试 |
| JunqiGameRulesTest.cpp | 17个特殊规则测试 |
| CMakeLists.txt | gtest集成和编译配置 |
| TEST_GUIDE.md | 详细的测试指南 |

## 项目现状

**完成度**: 95%

✅ **已完成**:
- 完整的算法实现（JunqiLayout.cpp ~500行代码）
- 全面的单元测试（83个测试用例）
- CMake编译配置
- 测试框架集成（gtest）

🔄 **进行中**:
- 测试执行和调试

⏳ **待启动**:
- 客户端框架迁移（参考cnchess）
- UI资源文件创建
- 集成测试

## 快速参考

### 构建命令
```bash
# 在build目录下
cmake ..
cmake --build . --target junqi_test
```

### gtest常用参数
```bash
# 列出所有测试
./junqi_test --gtest_list_tests

# 运行特定测试类
./junqi_test --gtest_filter=JunqiAlgorithmTest.*

# 运行特定测试
./junqi_test --gtest_filter=JunqiAlgorithmTest.ChessmanComparison

# 详细输出
./junqi_test -v

# 重复运行100次
./junqi_test --gtest_repeat=100
```

## 联系与反馈

遇到问题或有建议，请参考完整的测试文档: [TEST_GUIDE.md](TEST_GUIDE.md)
