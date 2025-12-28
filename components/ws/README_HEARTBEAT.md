# WebSocket心跳机制快速参考

## 心跳机制工作原理

```
客户端                    服务器
  |                         |
  |---- 连接建立 ----------->|
  |                         | 初始化: last_activity = now
  |                         |         last_ping = now
  |                         |         ping_timeout_count = 0
  |                         | 启动定时器(timer_interval)
  |                         |
  |<----- 定时器触发 --------|
  |                         | 检查: now - last_activity > nHeartbeatSeconds?
  |                         |   是 → 关闭连接(心跳超时)
  |                         |   否 → 继续
  |                         |
  |                         | 检查: now - last_ping >= pingIntervalSeconds?
  |                         |   是 → 检查ping_timeout_count >= nPingTimeoutCount?
  |                         |         是 → 关闭连接(ping超时)
  |                         |         否 → 发送ping, ping_timeout_count++
  |<------- PING -----------|
  |                         |
  |------- PONG ----------->|
  |                         | ping_timeout_count = 0
  |                         | last_activity = now
  |                         |
  |---- 发送数据 ----------->|
  |                         | last_activity = now
  |                         | ping_timeout_count = 0
```

## 配置参数说明

| 参数 | 说明 | 推荐值 |
|------|------|--------|
| `pingIntervalSeconds` | ping发送间隔 | 30秒 |
| `nHeartbeatSeconds` | 心跳超时时间 | 120秒 |
| `nPingTimeoutCount` | 最大ping超时次数 | 3次 |

**重要**：确保 `nHeartbeatSeconds > pingIntervalSeconds × nPingTimeoutCount`

## 配置示例

### 示例1：标准配置（推荐）
```cpp
SvrPingCfg pingCfg;
pingCfg.pingIntervalSeconds = 30;   // 每30秒ping一次
pingCfg.nHeartbeatSeconds = 120;    // 120秒无活动超时
pingCfg.nPingTimeoutCount = 3;      // 允许3次ping超时

// 实际行为：
// - 定时器每30秒触发一次（min(30, 120) = 30）
// - 客户端空闲30秒后开始发送ping
// - 如果120秒无活动 → 心跳超时关闭
// - 如果连续3次ping无响应(90秒) → ping超时关闭
```

### 示例2：快速检测（适合内网）
```cpp
SvrPingCfg pingCfg;
pingCfg.pingIntervalSeconds = 10;   // 每10秒ping一次
pingCfg.nHeartbeatSeconds = 30;     // 30秒无活动超时
pingCfg.nPingTimeoutCount = 2;      // 允许2次ping超时

// 实际行为：
// - 定时器每10秒触发一次（min(10, 30) = 10）
// - 客户端空闲10秒后开始发送ping
// - 如果30秒无活动 → 心跳超时关闭
// - 如果连续2次ping无响应(20秒) → ping超时关闭
```

### 示例3：宽松配置（适合移动网络）
```cpp
SvrPingCfg pingCfg;
pingCfg.pingIntervalSeconds = 60;   // 每60秒ping一次
pingCfg.nHeartbeatSeconds = 300;    // 300秒(5分钟)无活动超时
pingCfg.nPingTimeoutCount = 4;      // 允许4次ping超时

// 实际行为：
// - 定时器每60秒触发一次（min(60, 300) = 60）
// - 客户端空闲60秒后开始发送ping
// - 如果300秒无活动 → 心跳超时关闭
// - 如果连续4次ping无响应(240秒) → ping超时关闭
```

## 常见问题

### Q1: 定时器间隔是如何确定的？
**A**: 定时器间隔 = min(pingIntervalSeconds, nHeartbeatSeconds)。使用两者中较小的值，确保能及时发送ping和检查心跳超时。

### Q2: 客户端需要做什么？
**A**: 客户端不需要特殊处理，libwebsockets会自动响应ping。如果客户端有数据发送，也会重置服务器的心跳计时。

### Q3: 什么时候会发送ping？
**A**: 只有在客户端空闲（距离上次活动 ≥ pingIntervalSeconds）时才会发送ping。如果客户端一直在发送数据，说明连接正常，不需要发送ping。

### Q4: 如何调试心跳问题？
**A**:
1. 查看日志中的 "send ping" 和 "recv pong" 消息
2. 检查 `ping_timeout_count` 的值
3. 确认配置参数是否合理

### Q5: 心跳超时和ping超时有什么区别？
**A**:
- **心跳超时**：基于最后活动时间，包括收发数据和pong
- **Ping超时**：基于连续未响应的ping次数
- 两者独立工作，哪个先触发就先关闭连接

### Q6: 如何禁用心跳检测？
**A**: 不建议禁用。如果必须禁用，可以设置很大的值：
```cpp
SvrPingCfg pingCfg;
pingCfg.pingIntervalSeconds = 3600;    // 1小时
pingCfg.nHeartbeatSeconds = 86400;     // 24小时
pingCfg.nPingTimeoutCount = 100;       // 很大的值
```

## 故障排查

### 问题：连接频繁断开
**可能原因**：
1. `nHeartbeatSeconds` 设置太小
2. 网络延迟导致ping/pong超时
3. 客户端没有正确响应ping

**解决方案**：
1. 增大 `nHeartbeatSeconds`
2. 增大 `nPingTimeoutCount`
3. 检查客户端日志

### 问题：死连接无法检测
**可能原因**：
1. `nHeartbeatSeconds` 设置太大
2. `pingIntervalSeconds` 设置太大

**解决方案**：
1. 减小 `nHeartbeatSeconds`
2. 减小 `pingIntervalSeconds`

### 问题：日志中看到 "ping timeout count exceeded"
**说明**：客户端连续多次未响应ping，这是正常的超时检测。

**检查**：
1. 客户端是否真的断开了？
2. 网络是否有问题？
3. 配置是否太严格？

## 性能影响

| 配置 | 网络开销 | CPU开销 | 检测延迟 |
|------|---------|---------|---------|
| 快速检测(10s) | 高 | 中 | 低(10-20s) |
| 标准配置(30s) | 中 | 低 | 中(30-60s) |
| 宽松配置(60s) | 低 | 低 | 高(60-120s) |

**建议**：根据应用场景选择合适的配置，不要盲目追求快速检测。

## 相关文件

- `components/ws/HEARTBEAT_FIX.md` - 详细的修复说明
- `components/ws/BUGFIX_SUMMARY.md` - BUG修复总结
- `components/ws/test/HeartbeatTest.cpp` - 测试程序
- `SOUI/include/interface/ws-i.h` - 接口定义

