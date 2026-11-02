# wasm-demo 诊断工具

线路 C（canvas2d 后端）调通过程中攒下来的取证脚本。绝大多数只用 Node 内置模块；
Playwright 系（`shot_dpr.cjs` / `probe_tank.cjs`）额外用 `playwright-core` 驱动本机
Edge 无头跑，`npm i` 一次即可（`node_modules/` 已 ignore）。

前提：`../../demos/wasm/out/` 里有构建产物（`soui-wasm-demo.{html,js,wasm,data}`），
下面命令都在本目录执行。

## node_shim.cjs —— 在 Node 里跑 demo，模拟 canvas 2D

```bash
node tools/diag/node_shim.cjs            # 泵若干帧后打统计
SHIM_TRACE=1 node tools/diag/node_shim.cjs   # 每次 canvas 调用都打一行
```

用 `Proxy` 兜住全部 2D API 并统计调用直方图，末尾报告帧缓冲非零像素数和前若干条
非 ASCII 文本。**浏览器里主线程被独占、CDP 全部超时时，这是唯一能拿到"最后一个 GDI
调用"的手段**——同时它不做 `this` 绑定校验，所以 `val::operator()` 丢接收者这类错误
在这里看不出来，必须再回浏览器验一遍。

脚本内部有泵帧定时器，打完 `done` 不会自己退出，可以用 `timeout 120` 包一层。

## shoot.cjs —— CDP 重载页面并截图全画布

```bash
node tools/diag/shoot.cjs [等待毫秒]
```

需要页面上有个 `canvas#screen`，且 CDP 端点在本机 9222。

两个必须遵守的点：

* 不能用 `--virtual-time-budget`——demo 的主循环每帧真实忙等，虚拟时间推不到预算；
* 必须 `Network.setCacheDisabled`——`.html` 能用 `?t=` 破缓存，但 `.wasm`/`.data` 是
  页面自己 fetch 的，URL 不带时间戳，不关缓存就会一直跑旧模块（症状：截图字节数一模一样）。

## shot_dpr.cjs —— HiDPI 档位验收（自带服务器 + 无头 Edge）

```bash
node shot_dpr.cjs <产物目录> <dsf> [输出 png]
node shot_dpr.cjs ../../demos/wasm/out 1.125 /tmp/dpi_1125.png
```

在指定 `deviceScaleFactor` 下打开页面，打印 `dpr / canvas backing / CSS 尺寸 / 帧缓冲`
以及 wasm `main()` 打的 `[soui-wasm] dpi dpr=... -> scale=...%` 那一行（档位是**向上贴**
内置档位后的结果），可选出图。

档位规则（`soui_main.cpp` 的 `main()`）：取 100/125/150/200/250/300 里**不小于** dpr 的
最小一档 —— 帧缓冲 ≥ 真实设备像素，浏览器只会把画布缩下来（超采样，更锐），永远不会
拉伸发虚；倍率超过 1.15 就退回 dpr 的精确百分点（那也已 1:1）。**不能**用
`SDpiScale::NormalizeScale`：它是向下贴，浏览器缩放 110%/120% 会被压成 100%，
帧缓冲只有 800x600 却被按 1.1 倍物理像素放大 ⇒ 标题栏 sc:100 + 整页发虚。

验收基线（`dpr → 档位 / fb`）：

| dpr | 1.0 | 1.125 | 1.25 | 1.5 | 1.6 | 2.0 |
|---|---|---|---|---|---|---|
| sc | 100 | 125 | 125 | 150 | 200 | 200 |
| fb | 800x600 | 1000x750 | 1000x750 | 1200x900 | 1600x1200 | 1600x1200 |

时序坑：`onRuntimeInitialized` 早于 wasm `main()`，页面侧 fb 首次按 800x600 建，
`ensureSurface` 靠「C++ 侧尺寸变了就重建」自愈——若 canvas 是 800x600 而 fb 是 1200x900，
先查这里。

`shoot_dpi.cjs` 是 CDP 版（要求手工起服务器、浏览器开 9222）：`node shoot_dpi.cjs <dsf> [等待毫秒] [输出名]`，
用 `Emulation.setDeviceMetricsOverride` 模拟 dpr，产物在 `build/dpi/<名>.png`。
`interact.cjs` 同样支持 `DSF=1.5` 环境变量模拟 HiDPI（click 坐标仍按逻辑 800x600 给，
由页面 canvasPoint 映射到物理）。

## shot_tabs.cjs —— 只截标签条（放大 2 倍）

```bash
node tools/diag/shot_tabs.cjs
```

用来放大看九宫格皮肤有没有拼接缝、色块错位这类一像素级瑕疵。

## tab_xs.cjs —— 从像素里量出标签文字的分段坐标

```bash
node tools/diag/tab_xs.cjs
```

对标签条那一行做 `getImageData`，按"墨迹列"切段输出每段中心 x。
点某个标签时如果不想靠猜坐标，先跑这个。

## click_tab.cjs —— 点某个标签页并截图

```bash
node tools/diag/click_tab.cjs <TABX 下标> <输出文件名>
TAB_X=367 node tools/diag/click_tab.cjs 0 shot_grid.png   # 直接给 canvas 内 x
```

## symbolize.cjs / wasm_funcs.cjs —— wasm trap 栈符号化

```bash
# 1. 带符号链接：configure 时加 -DSOUI_WASM_LINK_DEBUG=ON，产出 out/soui-wasm-demo.map
# 2. 把运行时打印的 "wasm-function[N]:0xPC" 翻成符号
node tools/diag/symbolize.cjs [map 路径]
node tools/diag/wasm_funcs.cjs     # 手解 wasm 段，PC 落在哪个函数索引
```

`symbolize.cjs` 里的 `DELTA = 5` 是 link map 偏移与 wasm 文件偏移的固定差，前提是链接
时带了 `-O0`（跳过 binaryen 的函数重排）；带 `-O0` 之外的其他优化级别这个差值就对不上了。

## probe_fill.cjs —— 每条 fill 的路径段数 / 填充色 / 变换

```bash
node tools/diag/probe_fill.cjs 12     # 参数是标签下标（需先跑 cmp_shot 生成 tabs.json）
```

钩住 `beginPath`/各路径段函数/`fill`，回读每次填充时的**路径段数**、`fillStyle`（渐变画刷
显示成 `[grad]`）、变换矩阵与 `globalAlpha`。用来区分三种"画面空白"：

* 根本没收到底层 `fill` ⇒ 上层没派发绘制（皮肤/资源/图层问题）；
* 段数为 0 ⇒ 画的时候路径已经空掉（`BeginPath`/`EndPath` 的记录链路断了）；
* 段数和颜色都正常却没有像素 ⇒ 查变换（`SetWorldTransform`/视图原点）、裁剪、`globalAlpha`。

## 逐标签对照 cairo 版（cmp_shot / cmp_make / img / crop）

这一组专门回答"现有 canvas2d 版和 cairo 参考版差在哪一页的哪个像素"。参考版是预编译
包（`D:\soui.com.cn\wasm`），起个静态服务器指过去即可，两侧都不用改代码。

```bash
# 0) 两个版本各起一个静态服务器（参考版 8124，当前版 8125；8125 指 demos/wasm-demo/out）
python -m http.server 8124 -d /d/soui.com.cn/wasm
python -m http.server 8125 -d demos/wasm-demo/out

# 1) 逐标签截图（自动认标签位置，两边字体不同也各自对得准）
node tools/diag/cmp_shot.cjs http://127.0.0.1:8124/soui-wasm-demo.html cairo
node tools/diag/cmp_shot.cjs http://127.0.0.1:8125/soui-wasm-demo.html canvas2d

# 2) 生成并排图 pair_NN.png / pair_NN_diff.png，并打一张差异百分比表
node tools/diag/cmp_make.cjs
```

`cmp_shot.cjs` 的标签定位是双路的：canvas2d 版文本走 `fillText`，直接从调用记录里取；
cairo 版把整帧 `putImageData`、**根本不调 `fillText`**，只能回到像素——在标签带做"墨迹
列"投影、按间隙切段。两侧各自写一份 `build/cmp/<tag>/tabs.json`。

`cmp_make.cjs` 只做合成与统计，解码/编码/缩放/逐像素 diff 全在 `img.cjs`（纯 Node，
本机没有 PIL/numpy）。

```bash
# 3) 局部取证：裁剪 + 3× 放大；给两张图就是并排 + 区域内差异直方图
node tools/diag/crop.cjs build/cmp/canvas2d/12.png 430 200 300 280
node tools/diag/crop.cjs build/cmp/cairo/12.png build/cmp/canvas2d/12.png 160 310 90 45
```

`crop.cjs` 在并排模式下会把该区域按"原色 → 新色"聚合并按像素数排序，用来区分
"整体色差"（同一对颜色刷满整片）与"结构缺失"（颜色对很碎）。产物写在**第一张图所在
目录**（并排时是 `crop_pair.png`）。

## probe_hit.cjs —— 探针：视口、DPR 与命中的对不上时先跑它

```bash
node tools/diag/probe_hit.cjs
```

打印 canvas 的 `getBoundingClientRect`、DPR、视口尺寸，再派发一次点击并回报命中。
**headless 的 `--window-size` 不生效**（实测视口只有 500×320），canvas 常落在视口外，
症状是"所有标签截图字节数完全一样"——必须 `Emulation.setDeviceMetricsOverride`。

## hook_draw.cjs —— 记录 fillRect/strokeRect/fillText/fill/stroke 的参数

```bash
node tools/diag/hook_draw.cjs 2500
```

每条记录带：坐标是否分数（亚像素 ⇒ 抗锯齿边缘）、`globalAlpha`、变换矩阵的 `e/f`
（视图原点/世界变换有没有落下）、`fillStyle`/`strokeStyle`。定位"颜色不对/整体偏移/
半透明丢失"这一类问题的最快入口。

## probe_text.cjs —— 文本/矩形/位图拷贝 + 变换 + alpha 全量记录

```bash
node tools/diag/probe_text.cjs <url> <tabIndex|-> <filter> [clickX,clickY] [delayMs]
node tools/diag/probe_text.cjs http://127.0.0.1:8125/soui-wasm-demo.html 11 alpha 60,199 250
DUMP=build/rec.json EXPR='...' node tools/diag/probe_text.cjs ... # 导出全量 / 收尾再求值
```

钩 `fillText`/`fillRect`/`putImageData`/`drawImage`，每条记录：
`[kind, what, a,b,c,d,e,f, globalAlpha, fillStyle, canvasWxH, canvasId]`。

- **`canvasId`** 给每个 canvas 分配稳定编号（`window.__byid[id]`），用来区分主画布与离屏
  RT —— 分层窗口 / 位图 DC 的内容都先画在离屏 canvas 上，只有被 `drawImage` 合到主画布
  才会出现在屏幕上。
- **`DUMP=<path>`** 把全量记录写成 JSON，方便用 Node 做统计（例如"源矩形是否落在位图外"）。
- **`EXPR='js'`** 在延时结束后于页面里求值，可直接读某块离屏 canvas 的像素：
  `EXPR='(function(){var c=window.__byid[69];var d=c.getContext("2d").getImageData(10,10,1,1).data;return d[0]+","+d[1]+","+d[2];})()'`
- 注意 `drawImage` 那条记录的 `what` 是 `<-宽x高#ID`，**参数本身没进记录**；要查源矩形是否
  越界得配合 `DUMP` + 代码里的 `srcRectDevice`，或直接读离屏 canvas 的像素确认内容在不在。
- **`NOCLEAR=1`**：默认工具在切标签后才清空记录（只看后续交互），而静态页"哪些行没画出来"
  必须抓**切标签那一刻**的绘制 —— 加 `NOCLEAR=1` 保留全程记录（含首帧）。

## interact.cjs —— 交互式探针：切标签 → 点击 → 按时刻连拍

静态截图查不出动画类缺陷（补间 / 值动画 / 模态遮罩都是时间序列），这时用它。

```bash
node tools/diag/interact.cjs <url> <tabIndex|-> [step ...]
# 动画页点第一个按钮，连拍三帧：
node tools/diag/interact.cjs http://127.0.0.1:8125/soui-wasm-demo.html 11 \
  click:400,150 wait:60 shot:a0 wait:250 shot:a1 wait:250 shot:a2
```

step 形式：

| step | 说明 |
|---|---|
| `click:X,Y` | 在**画布逻辑坐标**（800×600）点击，脚本自己换算成页面坐标 |
| `down:X,Y` / `move:X,Y` / `up:X,Y` | 按下 / 拖动 / 抬起（测滑块、拖拽） |
| `key:COMBO` | 组合键，`+` 连接：`key:Ctrl+A` / `key:Shift+End` / `key:a`。页面侧按 `e.code` 翻 VK，所以修饰键发 `ControlLeft` 等 code；richedit/scintilla 的选中态就是 `click:X,Y wait:200 key:Ctrl+A` |
| `wait:MS` | 等待 |
| `shot:NAME` | 截一帧到 `build/cmp/live/NAME.png` |
| `eval:JS` | 页面内求值并打印（读 `window.__x` 之类） |

`tabIndex` 从 `build/cmp/canvas2d/tabs.json` 取标签中心 x（需先跑一次 `cmp_shot.cjs`），
传 `-` 表示不切标签。`TAB_Y` 覆盖标签行 y（默认 72），`LIVE_DIR` 覆盖输出目录。

## 本轮定位到的三条判据（矩阵变换类缺陷）

1. **`drawImage` 的源矩形落在离屏位图外** ⇒ 位图拷贝传的是**源 DC 逻辑坐标**，没按
   `ptViewport`/`xfWorld` 换算成像素。症状：补间动画一点按钮色块整块消失、模态表单整片空白。
2. **离屏 RT 尺寸远小于传入的裁剪矩形坐标范围** ⇒ `clipRectDevice` 在单位矩阵下裁逻辑坐标，
   整片裁剪落在图外。症状：属性动画的色块变成纯灰底。
3. **渐变主体只剩最内圈色标** ⇒ 半径单位用错（`gradRadius` 是**像素**，不是 0..1 比例）。
   症状：消消乐水果"像蒙了一层半透明白纱"。

> 挂钩子只认 **`CanvasRenderingContext2D.prototype`**（以及 `OffscreenCanvasRenderingContext2D`）。
> 若打出来全是 0，先确认**点击真的落到标签上了**——点击不生效时页面根本不重绘，钩子
> 自然一条都收不到（不是钩子失灵）。派发事件用 `Input.dispatchMouseEvent` 的
> `mousePressed`/`mouseReleased`，`pointerdown`/`pointerup` 在 swinx 的宿主里不产生
> SOUI 鼠标消息。

## probe_tank.cjs —— Lua 游戏（坦克大战）四阶段帧率取证

```bash
node probe_tank.cjs            # 进 Lua 游戏页 → 坦克大战 → 开始，跑 idle/moveUp/moveLeft/fire
```

**判据是 rAF 间隔**，不是 Canvas 方法计数：cairo 后端一帧只有 1 次 `putImageData`，
按方法数看两边"差不多"，但帧节奏差一倍。指标：

| 指标 | 含义 |
|---|---|
| `fps` / `rafMed` / `rafP95` | 相邻 rAF 间隔的中位/95 分位 —— 后端无关的流畅度 |
| `opsMed` | 一帧的 Canvas 调用数（≈ 跨语言边界次数） |
| `spanMed` / `inJsMed` | 本帧首个→末个调用的墙钟跨度 / 其中真正花在浏览器里的时间 |
| `presents` | 上屏次数（帧边界 = `putImageData` 或 `this===window.__swxDisplay` 的 `drawImage`） |
| `slowTicks` | 最慢的 6 个 rAF 间隔及期间上屏次数 `dp`（`dp:0` = 空转帧，`dp>1` = 掉帧补画） |

2026-11 基线（headless Edge，软件渲染）：idle 57.7fps / moveUp 56.9 / moveLeft 56 / fire 57.3，
rafP95 16.8ms，ops/帧 ~2300。改造前 idle 21.7fps、rafP95 100ms、ops 11412。

> `fps` 的分母是**这一段真实经过的时间**（`snap()` 里的 `ms` 字段），不是固定 3000ms：
> `fire` 段只按 6 次空格（约 1.1s），早先按 3000ms 当分母会算出 24fps，而同时 rafP95 只有
> 16.8ms —— 看着像掉帧，其实是分母错。判断有没有掉帧看 `rafP95` 与 `slowTicks`，`fps` 只看趋势。

`probe_profile.cjs` 是同一场景的 CPU 采样（CDP `Profiler`，200µs 间隔，按 self time 排序
并打印热点调用链）：`node probe_profile.cjs`。用它把"慢在绘制调用"与"慢在别的 JS
（`getImageData` / `__localtime_js`）"分开——`probe_tank` 只能告诉你"一帧多久"。

## probe_sfx.cjs —— 坦克大战音效链路（lua → swinx → 宿主）

```bash
node probe_sfx.cjs            # 进坦克页 → 开始 → 连射 → 等敌人开炮，按 wav 名计数
```

把 `Module.swinxAudioPlay` 包一层按文件名计数。判据是"请求有没有提交"，不是"真的听见"：
无头环境里 `AudioContext` 常年 suspended，解码/播放根本不发生，而 lua 该不该在某事件上
喊这一声、swinx 有没有把字节递到页面，正是这条链路的责任边界。

```
[afterStart] tank_start.wav=1 tank_spawn.wav=4 tank_efire.wav=2
[afterFire]  tank_fire.wav=4 tank_brick.wav=6 tank_steel.wav=1 ...
```

`tank_efire` 走 `SND_NOSTOP`（已有音效在播就丢弃），被丢弃的会单独打一行
`被 NOSTOP 丢弃` —— 这行常年为空是正常的：玩家炮声与爆炸用的是 `nostop=false`，
只有敌坦克齐射才让路。基线：`tank_start/fire/brick` 必现，`tank_steel` 取决于
是否打到钢墙（向上直射第 2 关才有）。

## 性能取证三件套（线路 C 的绘制开销）

`measure_span.cjs` / `bench_repaint.cjs` / `snap_tabs.cjs`（另加 `probe_present.cjs`、
`dump_surface.cjs`）是查"canvas2d 后端为什么比 cairo 慢"的固定套路，都是 monkey patch
`CanvasRenderingContext2D.prototype`，不改 wasm。

```bash
# 一次整窗重绘里到底调了多少 Canvas API、各方法占比、以及"绘制跨度"
node measure_span.cjs ../../demos/wasm/out "" "" 4
node measure_span.cjs ../../demos/wasm/out "?wiggle=1" "" 4       # 点标签栏制造重绘

# 跨版本可比的确定性基准：在两个页签间来回点，只统计整窗重绘帧
node bench_repaint.cjs ../../demos/wasm/out 12 140,200

# 逐页签像素对比（跨版本回归）：先跑一次存档，切版本后再跑，比 md5
node snap_tabs.cjs ../../demos/wasm/out /tmp/base_tabs 140,200,260,320,380,440,500,560

# 离屏窗口画布 vs 显示画布（判断错位出在绘制还是上屏）
node dump_surface.cjs ../../demos/wasm/out 140 /tmp/cur
```

指标含义：
* **ops** —— 一帧里的 Canvas API 调用次数（≈ 跨 wasm→JS 边界的次数）；
* **span** —— 本帧第一个调用到最后一个调用的墙钟跨度 ≈ 整帧绘制耗时；
* **inJs** —— 这些调用体内部真正花掉的时间（浏览器光栅化）；`span - inJs` 就是
  跨边界 + 参数封送的开销。
* **sameOps / sameN / sameSpanMed** —— 一轮点击偶尔会多出或少掉一两帧（按下、抬起、
  切页各可能来一帧），`bigOps` 均值就被这些重帧带偏。所以取出现次数最多的 ops 档
  （mode），**只在 ops 完全相同的帧之间**比 span —— 跨版本比这个才可比。

2026-11 实测（800x600，headless Edge 软件渲染）。**同机、同 harness、交替跑**的 A/B：

| 版本 | mode 帧 ops | mode 帧 span | mode 帧 inJs | 全部大帧 span 中位 |
| --- | --- | --- | --- | --- |
| 改造前（全 embind + 逐像素换序上屏） | 1949 | 13.7~14.0ms | 7.2~8.1ms | 14.4~14.6ms |
| 改造后 | 1773 | **11.2~12.4ms** | 5.2~5.3ms | 8.2~8.8ms |

⇒ 整窗重绘帧约 1.2 倍，ops 少 9%，帧内 JS 时间少 30%。

**别用"跨环境比出来的倍数"下结论**：早先记过一组 39.9ms→8.1ms（4.9 倍），事后复测
发现那组的"改造前"是在别的时间点/构建配置下量的，同机交替跑复现不出来。要看倍数
就当场 A/B（把两版产物各放一个目录，`bench_repaint.cjs` 交替跑），别跨天比。

`check_imports.cjs`（不在"三件套"里，但同类问题先跑它）：比对 `.wasm` 用
`WebAssembly.Module.imports()` 声明要的 import 与 `.js` 胶水实际提供的名字，
用于快速判定"wasm 与胶水不是同一次构建"。

```bash
node check_imports.cjs ../../demos/wasm/out
```

注意 `bench_repaint.cjs` 的 `frames` 也是判据：上屏路径若对"非整行宽的脏矩形"逐行
`putImageData`，12 次切页的上屏次数会从 24 次暴涨到上万次。
