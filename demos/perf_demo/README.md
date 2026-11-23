# perf_demo —— X11 / Wayland 后端绘制性能对比

同一份源码，按后端编两个可执行文件，用同一组绘制负载对比 `swinx` 两个 Linux
后端的开销。测的是 **backend 实现差异**（cairo 绘制 → surface 提交 → SHM 拷贝），
不是 SOUI 布局或业务逻辑。

**每个后端只跑一次**：一次输出全部场景、`repeat=1/4` 两组样本，以及由它们推导出的
归因指标（每帧固定成本 / 纯布局 / 字形栅格 / 每趟净成本 / 每次调用成本），同时落盘
成 `perf_<backend>.txt`；两份文件再用 `--compare` 并排比出 delta 与 ratio。

## 构建

```bash
# X11
cmake -S . -B build-x11 -DSOUI_LINUX_BACKEND=x11
cmake --build build-x11 -j --target perf_demo

# Wayland
cmake -S . -B build-wl -DSOUI_LINUX_BACKEND=wayland
cmake --build build-wl -j --target perf_demo
```

两个目录必须**分别 configure**：后端开关决定参与编译的 swinx 源文件。

## 运行

```bash
# 后端 A：一次跑完全部用例，写出 perf_x11.txt（默认文件名 = perf_<backend>.txt）
cd build-x11 && ./bin/perf_demo

# 后端 B：同上，写出 perf_wayland.txt
cd build-wl && ./bin/perf_demo

# 并排比较（不开窗、不绘制，只读两份结果文件）
./bin/perf_demo --compare=../build-x11/perf_x11.txt,./perf_wayland.txt
```

> 可执行文件与 `demo` 输出在同一目录。若构建把可执行文件放在 build 根目录而不是
> `bin/`，把路径换成 `./build-x11/perf_demo`（`find build-x11 -name perf_demo` 可确认）。

参数：

| 参数 | 说明 |
|---|---|
| `--frames=N` | 每个用例的采样帧数，默认 100（每用例前另丢弃 3 帧暖机） |
| `--repeat=1,4` | 每次 `WM_PAINT` 内重复绘制的次数列表，默认 `1,4`。用来把「每帧固定成本」和「每趟图元成本」解成两个数 |
| `--case=a,b` | 只跑指定场景（`all`/`fill`/`text`/`blit`/`lines`/`empty`/`textmeasure`） |
| `--save=FILE` | 结果落盘路径，默认 `perf_<backend>.txt` |
| `--warmup=N` | 每个用例前丢弃的暖机帧数，默认 3 |
| `--hold` | 跑完保持窗口（ESC 或关窗退出） |
| `--compare=A,B` | 只比较两份结果文件，不开窗 |
| `--help` | 用法 |

兼容旧用法：位置参数写数字=帧数，写场景名=只跑该场景（如 `perf_demo 300 all`）。

## 一次 sweep 覆盖的用例

| 用例 | 内容 | 规模 | repeat |
|---|---|---|---|
| `empty` | 什么都不画（零绘制基准线） | 0 | 1 |
| `textmeasure` | 只做 `DT_CALCRECT` 布局，不栅格化字形 | 200 次 `DrawTextA` | 1 |
| `fill` | 彩色矩形带（模拟控件背景/皮肤） | 1200 个 `FillRect` | 1, 4 |
| `text` | 短字符串，每帧内容不同 | 200 次 `DrawTextA` | 1, 4 |
| `blit` | 离屏位图 1:1 贴图 | 24 次 `BitBlt` | 1, 4 |
| `lines` | 斜线描边 | 128 次 `MoveToEx`/`LineTo` | 1, 4 |
| `all` | 以上全部，且按类别分别计时 | — | 1, 4 |

合计 12 个用例；`--frames=100` 时 Wayland 约 25s、X11 约 45s（X11 每帧更慢）。

## 输出

```text
case         rep frames  paint_avg      p50      p95      min      max  frame_ms paint%
empty          1    100      0.041    0.030    0.060    0.020    0.310      0.10   41.1
textmeasure    1    100      1.240    1.230    1.300    1.210    1.600      1.24   99.3
fill           1    100      2.100    2.090    2.180    2.050    2.400      2.10   99.9
fill           4    100      8.300    8.290    8.400    8.200    8.700      8.30   99.9
text           1    100     15.100   15.090   15.300   14.900   15.800     15.10   99.9
text           4    100     60.100   60.000   60.600   59.500   61.900     60.10   99.9
blit           1    100      0.520    0.510    0.560    0.500    0.700      0.52   99.9
blit           4    100      2.080    2.070    2.140    2.030    2.300      2.08   99.9
lines          1    100      3.200    3.190    3.300    3.100    3.500      3.20   99.9
lines          4    100     12.800   12.790   13.100   12.600   13.400     12.80   99.9
all            1    100     21.070   21.051   21.651   20.412   23.002     21.33   98.8
all            4    100     82.900   82.800   83.600   82.100   84.900     82.90   99.9

breakdown 'all' repeat=1 (avg per frame, share of paint):
  fill      2.100 ms   10.0%      1.75 us/call  (1200 rects)
  text     14.800 ms   70.2%     74.00 us/call  (200 texts)
  blit      0.500 ms    2.4%     20.83 us/call  (24 blits)
  lines     3.200 ms   15.2%     25.00 us/call  (128 lines)
  other     0.470 ms    2.2%   BeginPaint/clip + EndPaint/commit

derived (ms/frame unless noted):
  fixed               0.041   BeginPaint/clip + 端到端提交 + pump
  layout              1.199    6.00 us/run   (textmeasure - fixed)
  glyph_raster       13.860   69.30 us/run   (text - textmeasure)
  fill_net            2.059     1.72 us/call  (paint - fixed)
  fill_per_pass       2.067     1.72 us/call  ((r4 - r1) / 3)
  text_net           15.059    75.30 us/call  (paint - fixed)
  text_per_pass      15.000    75.00 us/call  ((r4 - r1) / 3)
  ...
```

（以上为格式示例，数值随机器浮动。）

- `paint_avg`：`BeginPaint` → 绘制 → `EndPaint` 的耗时，包含后端提交（cairo
  flush / surface commit），不含合成器呈现等待。`p50/p95/min/max` 给离散度。
- `frame_ms` / `paint%`：每帧墙钟（含消息泵）与 paint 占帧周期的比例。
  **paint% 明显小于 100 说明瓶颈不在绘制，而在 pump / 事件处理 / 合成器节流。**
- `breakdown`：`all` 场景下每类图元的耗时、占 paint 比例、单次调用成本；`other`
  是 paint 里除图元外的固定开销（BeginPaint/裁剪 + EndPaint/提交）。

## 推导指标

| 指标 | 算法 | 含义 |
|---|---|---|
| `fixed` | `paint(empty r1)` | 每帧固定成本：BeginPaint/clip + 端到端提交 + pump |
| `layout` | `paint(textmeasure r1) - fixed` | 纯文本布局（shaping / 度量），不产生像素 |
| `glyph_raster` | `paint(text r1) - paint(textmeasure r1)` | 字形栅格化 + 合成 |
| `X_net` | `paint(X r1) - fixed` | 该场景图元的净成本 |
| `X_per_pass` | `(paint(X r4) - paint(X r1)) / 3` | 单趟图元净成本（把每帧固定成本扣干净） |
| `us/call` | `per_pass / 调用次数` | 每次 API 调用的后端成本，跨后端比较的关键列 |

两条用途：

1. `X_net` 与 `X_per_pass` 应该接近；若差得远，说明该场景受缓存/工作集影响
   （`all` 在 `repeat=4` 时尤甚），以 `per_pass` 为准。
2. `us/call` 是判断「per-op 派发成本 vs 像素量」的判据：两个后端共享同一份
   `gdi/cairo`，所以建路径 / shaping / clip 求交这类客户端成本在 A/B 里**相互抵消**；
   不会抵消的只有「每次调用走一遍后端 compositor」——xcb 只编一个协议请求，
   image surface 则要跑 pixman。`us/call` 的差值就是这部分。

## 比较输出

```text
==== perf_demo compare ====
A : x11       ../build-x11/perf_x11.txt
B : wayland   ./perf_wayland.txt

-- 关键指标 --
metric                                 A         B       B-A      B/A
all.r1.paint                       6.950    21.070   +14.120    3.03x
all.r1.p95                         9.661    21.651   +11.990    2.24x
empty.r1.paint                     0.120     0.130    +0.010    1.08x
derived.layout                     1.900     1.199    -0.701    0.63x
derived.glyph_raster               3.100    13.860   +10.760    4.47x
derived.fill.per_pass              1.500     2.067    +0.567    1.38x
...
```

先看「关键指标」，再看「全量指标」（含 `p50/min/max/frame/paint_pct` 与每个
`part_*`）。`B/A` 是倍率，`B-A` 是绝对差；`-` 表示该侧没有这个键（例如一侧没跑
某个用例）。两边窗口尺寸不同会单独告警。

## 怎么读（重要）

1. **不要直接横向比 `paint` 的绝对值**：X11 的窗口是 `cairo_xcb_surface`，绘制只把
   请求写进 xcb 输出缓冲，真正的光栅化在 X server（会话里是 XWayland）异步做；
   Wayland 的 `cairo_image_surface` 是进程内同步光栅化。X11 的 `paint` 天然偏小，
   那 20~30ms 落在了 `frame_ms - paint`（pump，含 `SendExposeEvent` 的 16ms 合帧
   计时器与 `flush()` 背压）。
2. 想回答「Wayland 的绘制成本花在哪」，看 `derived.*.per_pass` 和 `us/call`；
   想回答「是不是被提交/上屏拖住」，看 `paint%` 与 `fixed`。
3. 同一台机器、同一会话（GNOME Wayland 会话下 X11 版走 XWayland，属正常）。
4. `perf_demo` 每帧全窗 invalidate，是压力上限；真实 SOUI 走脏区重绘，看**相对
   占比**比看绝对值有意义。
5. 若 Wayland 明显偏慢，先确认没走软件渲染回退（`SWINX_WL_TRACE=1` 看有没有
   `commit-canvas DROP`），再看 `derived.glyph_raster` / `us/call` 定位到具体图元。
