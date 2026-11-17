# demos 阴影统一（2026-10-14）

## 目标与边界

以当前 `demos/dpidemo` 已交付的 PNG 为参考，而不是历史 Fluent 文档中的旧参数。
统一其余 16 个桌面 demo 资源目录（IPC 含两个可执行程序）的自绘窗口、菜单及下拉阴影。
保留各目录的面板填充与描边、资源名称/索引、菜单项、事件、DPI、语言和已有窗口模式。
`dpidemo` 本身不改。Android/OHOS 各有 APK assets/HAP rawfile 资源副本，但未发现独立桌面阴影或自绘菜单入口；本轮不替换移动端系统资源、模态遮罩或创建桌面阴影窗口。

## 可观察验收

- 所有已接线 shadow PNG（含参考共 58 张、浅/深色）alpha 与当前 dpidemo 对应主题完全一致；面板填充仍来自各 demo 的原 token。
- 保持 100×100 RGBA、面板 (18,12,64,64)、圆角 8、九宫格 26/20/26/32 和阴影留白 18/12/18/24dp；透明外缘与四边渐变逐像素检查。
- 原 layered 主窗保持内容尺寸、重复绑定不累加，最大化/还原、DPI、菜单定位/键盘与销毁沿用现有 GUI 回归。
- demo2/gridctrl_demo/lightbutton_demo/sci_demo、fun_test 与 uieditor 仍保留原 NC/non-layered 主窗；原生菜单由系统绘制，不额外叠加栅格阴影。uieditor 的 child/停靠浮窗也保持原模式。fun_test 有 window PNG，但原主窗 `translucent="0"`，不能把素材对齐宣称为主窗已新增可见阴影。
- 仅生成阴影，不重跑全量 palette/icon 模板；fun_test 的加密 ZIP 必须同步并逐文件校验。

## 实施顺序与验证计划

1. 先添加跨目录参考像素回归，记录现状失败；保存修改前后对比图。
2. 共享 Chrome、fun_test 与 uieditor 的 popup 统一使用 dpidemo 当前 `window=True` 配方：contact 3dp/0dp，ambient 5dp/2dp，全留白 smoothstep；alpha 浅色 30%/8%、深色 34%/10%。窗口和菜单面板色仍独立。
3. 仅刷新受控阴影 PNG；复用真实资源 builder（若索引未变则生成物不应变化），更新 fun_test ZIP。
4. 跑 Python 资源/生成器回归、静态 contract、Windows x64 Debug 所有桌面 demo 构建、headless CTest 和 GUI Chrome/Popup 测试。
5. 运行实际窗口/菜单烟测并审图；记录通过证据以及无法验证的平台/画面场景，不以离线 PNG 检查代替 native 合成像素验证。

## 初始证据

- `python -B demos/fluent/generate_chrome_resources.py --check`：26 个 popup PNG 与旧生成入口期望不一致。
- 原 shared Chrome Python 回归：22 tests，26 个资源 subtest 失败，1 个旧 AST 断言错误（参考 now 使用 starred tuple）；主窗 PNG 已使用紧凑配方。
- `python -B -m unittest discover -s demos/dpidemo -p test_popup_shadow.py -v`：1 test 通过，参考菜单/window alpha 一致。

## 本次执行结果

### 修改范围

- 共享13目录 + SoTool + fun_test + uieditor：32张 popup 明暗 PNG 换成参考的紧凑配方；其余 window PNG 已一致、不重写。
- 3个生成入口同步配方；fun_test 新增 `--shadows-only`，避免重建私有 palette/icon。
- 加密 `fun_test/uires.zip` 同步，90条加密条目与 FILE 资源逐字节一致。
- 1856份资源哈希审计：只有预期32张 PNG 改变，原 palette/index、其它图、dpidemo、共享系统资源未改；13目录真实资源 builder 输出也未产生 diff。
- 新增跨17资源目录/58张 PNG 回归和 fun_test 增量写范围、幂等、坏颜色/缺文件防半写回归；旧 shared AST 断言改为读取当前参考 THEMES，不再误解析 starred tuple。

### 已运行验证

| 验证 | 实际结果 |
| --- | --- |
| `python -B demos/fluent/test_demo_shadow_style.py` | 2 tests / 58张参考alpha、颜色、几何、透明边缘/四边渐变通过；改前64个 subtest 失败 |
| `python -B demos/fun_test/test_fluent_shadow_resources.py` | 4 tests通过；改前4个 error |
| `python -B -m unittest discover -s demos/fluent -p '*test*.py'` | 123 tests通过 |
| 同上，SoTool/uieditor/fun_test/dpidemo分别用 `test*.py` / 参考用 `test_popup_shadow.py` | 11 + 49 + 9 + 1 tests通过，合计193（含新增用例） |
| `python -B demos/fluent/generate_chrome_resources.py --check` | 0 files differ |
| `python -B demos/fluent/check_contracts.py` | 13目录 / 57份原布局菜单，0 errors |
| `cmake --build build --config Debug --parallel 4 --target demo demo2 wxDemo dpidemo flowlayout_demo framelayout_demo gridctrl_demo hellocharts_demo lightbutton_demo mclistview_demo propanimator_demo sci_demo wallpaper ipc_client ipc_server SoTool uieditor fun_test` | Windows x64 Debug全部18 target成功 |
| `ctest --test-dir build -C Debug -L '^soui-headless$' --output-on-failure` | 530注册，529通过，1原有Disabled |
| `ctest --test-dir build -C Debug -L '^soui-gui$' --output-on-failure` | 18通过；包含Chrome/max-restore/DPI、Popup/keyboard/destruction、私有菜单/浮动重停靠 |
| `gui_smoke.py --demo <17个待改桌面目标> --single-theme --phase shadow-style-main-20261014` | 17/17主窗PID-scoped PrintWindow烟测通过，清理已启动进程 |
| `gui_smoke.py --demo flowlayout_demo --toggle 452 36 --phase shadow-style-themes-20261014` | light→dark→light_return截图比较通过；窗口几何不变 |
| `ruff check <新增2个Python测试>` / `ruff check --select F,E9 <7个变更Python文件>` | 通过；完整规则与HEAD基线同为16个原有问题，新增0个，不做无关清理 |
| `git diff --check` | 通过 |

### Review

独立只读 AI Review 检查7个生成器/回归文件的当前diff：无可定位问题；另外重跑58张资源对照测试2/2通过。Review不替代上述本轮构建和运行验证。

### 视觉证据与已知边界

- `output/shadow-style/before.png` / `after.png`：浅底/深底离线阴影对比，审图已确认菜单与参考同样紧凑、无旧的外扩宽光晕；这不是native屏幕合成证据。
- `output/shadow-style/verification.json` 收录资源、源码、EXE、日志与GUI截图证据；`output/shadows-*.log` 为本轮日志。
- 初次flow主题烟测使用旧坐标543/30点中了最大化，按实际截图改为452/36后往返通过；未修改产品几何。
- 实际flow dropdown输入成功且出现136×142 popup，但分层popup的PrintWindow为黑图，故该截图尝试标为未验证，不以主窗截图或PNG审图代替。
- 额外运行 `window.fluent_popup_native_pixels_follow_backdrop_after_late_translucent_switch`，native输出明确为 `[ NATIVE PIXEL NOT VERIFIED ]`：当前环境未观察到自有纯色backdrop，屏幕合成alpha不可安全归因；虽然该测试进程成功，**nativepixels仍unavailable**。
- 原生SMenu、移动端包内系统资源、非分层/NC主窗、自绘诊断轮廓与设计用户窗口保留原策略；SoTool3份已注册但无消费者的旧MenuEx资源未新增业务入口。
- Linux/macOS/Android/OHOS未运行，不宣称全平台通过。

### 增量复现

```powershell
python -B demos/fluent/generate_chrome_resources.py
# SoTool复用其generate_fluent.py既有chrome调用段，仅调用chrome.generate，不运行main()重建私有素材。
python -B demos/uieditor/generate_fluent_resources.py --popup-only
python -B demos/fun_test/generate_fluent_resources.py --shadows-only
python -B demos/fluent/generate_chrome_resources.py --check
python -B demos/fluent/test_demo_shadow_style.py
```

主窗Chrome/菜单几何和运行期绑定无修改；无需改变原有默认窗口尺寸或GUI脚本坐标。
