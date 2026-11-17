# 13 Demo Goal 最终验证记录 — 2026-10-12

## 结论与范围

已使用 `soui-fluent-demo`，以当前 `dpidemo` 的源码与技能参考为基准完成指定 **13 个目录 / 14 个 GUI target**（IPC 含 client/server）的 Fluent 改造和本机适用验收。默认浅色、运行时明暗往返，不以重建控件树掩盖刷新问题。保留原 name/id、事件、数据/adapter、编辑/选择、原单位、语言入口及已有演示动画；不新增运行时依赖，不修改公共 API 或共享系统皮肤。

本工作区原先已有大量未提交改动，本轮增量接续，不把整个工作区 diff 都归因于本 goal。没有创建 commit、PR 或发布。本文及新 manifest 是 **2026-10-12 最新结论**；旧 2026-10-11 与本日 earlier/final/signoff 文件保留为历史，不能替代本次 `closure` 验收。

## 实现与简化

- 13 目录私有、同名的明暗 skin/token 域；共用 `Theme` 属性记录与重放，并覆盖隐藏页、虚拟行、表头、后创建的 Combo。保留已有 factory/delegate 的业务职责，不按旧 RGB 全树替色。
- 各示例原信息架构不重做：demo 属性表/托盘/换肤 caption、wx 会话与媒体 group、dock 两态、grid 编辑器与 option 箭头、chart 有界说明、MC/prop 标题 hitbox、SCI 编辑器、IPC 结果/错误前景、Wallpaper 分类文字留白已补齐。照片/emoji 不被主题染色。
- 仅既有 layered 主窗增加 `WindowChrome`：9 目录共 10 主窗。外加左/上/右/下 **18/12/18/24dp** gutter，按真实 `SLayoutSize` 保留原 px/dp padding、正文与 minimum。重复 Bind 不增长；最大化撤除、还原恢复，旧 native handler 链与析构保持。
- demo2/grid/lightbutton/sci 保留原 NC/non-layered 主窗；Wallpaper 桌面承载层不加阴影。不强改窗口模式来伪造阴影。
- 44 份离线 PNG（18 window / 26 popup）复用当前 dpidemo renderer，成对按私有 palette 填色，使用九宫格；不逐帧模糊，不调用 AI 画精确边框。
- 已迁移的 MenuEx/Combo/search 弹层移除旧 `ncSkin`/uniform-alpha 路径，保留 body 宽高并将 gutter 外加；工作区几何、anchor 和 target scale 分别处理。demo tray、demo2 C bridge、wx BOTTOMALIGN emoji、MC 真实 buddy 接线保留原事件；搜索键盘路径即时显示。
- shared owned enter 为 **150ms / 显式 4dp / cubic-bezier(0.23,1,0.32,1)**。只清自己的动画，已有动画优先，键盘与减少动态效果分支跳过。原主窗动画保留，关闭/命令即时，不承诺额外 exit fade 或逐项 hover 动效。
- 资源收尾发现真实 builder 的 `tmResource` 未计入仅 idx 变化，11 目录的 PNG 已进 RC2 却未进 header。**没有豁免 Wallpaper 检查器**：`build_resources.py` 改为在新临时输出路径调用真实 builder，读取全部产物成功后才同步有差异的字节。失败/缺输出不发布；不是手工生成 ID。13 份完整 `_R` 定义文本（换行正规化后）不变；26 产物重复生成的原始字节 SHA-256 相同。
- 保留先测后修链：两项固定宽 MenuEx 内容/旧 NC 问题、IPC 日志 8dp inset、资源缓存 3 项回归均有 RED→GREEN；最终资源入口 4 项（含缺输出/无工具 mock-only 路径）通过。新 popup fixture 的初始化/编辑基线修正是测试假设修复，不当作产品 RED→GREEN。
- 基础阶段的字体 fixture 在 Windows 严格使用 YaHei UI，暴露 Skia glyph lookup 的 glyphID/source-context 碰撞；内部键修复的范围及已存在核心改动单独保留，13 项文字回归已实际复跑，不外推到所有 fallback 淘汰路径。

## 最新实际验收

下表日志均在仓库 `output/`；manifest 收录完整绝对路径、命令、输入/二进制/截图哈希。

| 检查 | 实际结果 | 最新证据 |
|---|---|---|
| Windows VS2022 x64 Debug 构建 | 14 GUI targets + fun_test PASS | `goal-fluent-20261012-signoff-build.log`；最终 review 后再构建 `goal-fluent-20261012-closure-build.log` |
| 构建输入锁定 | 1120 source files；26 generated files；前后零变化；15 EXE SHA 与 closure GUI 所用相同 | `goal-fluent-20261012-closure-source-before-build.json` / `closure-source-after-build.json` |
| 基础 headless CTest | **521 注册；520 启用并执行全部 PASS；1 既有 Disabled** | `goal-fluent-20261012-closure-registered.log` / `closure-headless.log` |
| Fluent 定向 | **55/55 PASS**，17 单元 + 38 E2E | `goal-fluent-20261012-closure-targeted-complete.log` |
| 同进程随机顺序 | **55 项 × 2 轮 PASS** | `goal-fluent-20261012-closure-single-process-complete.log` |
| 文字渲染定向 | **13/13 PASS** | `goal-fluent-20261012-closure-fonts.log` |
| GUI CTest | **13/13 命令 PASS**；其中 composed-pixel 守卫 unavailable，不计作像素通过 | `goal-fluent-20261012-closure-gui-tests.log` |
| 同进程 native Chrome | 10 项命令 PASS（含 keyboard/reduced/destroy/late Combo 等） | `goal-fluent-20261012-closure-native.log` / `.xml` |
| Python 工具回归 | shared **97 PASS**（GUI57 + row14 + Chrome22 + builder4），Wallpaper **11 PASS**；合计 **108** | `goal-fluent-20261012-closure-python-shared.log` / `signoff-wallpaper-static-green.log` |
| 静态与原契约 | 13 strict audits **0 error / 0 warning**；57 布局/菜单契约 **0 error**；Chrome generator `--check` PASS | `goal-fluent-20261012-closure-audit-*.log` / `closure-contracts.log` / `closure-chrome-check.log` |
| 资源一致性 | 真实 builder 两遍 26 生成物 SHA 相同；13 完整 `_R` 定义文本不变；严格 Wallpaper 校验 PASS | `goal-fluent-20261012-signoff-resource-sync.log` / `signoff-resource-repeat.log` / `closure-wallpaper-verify.log` |
| 实际窗口 | 12 单窗 light→dark→light_return + IPC 双窗 32 captures；owned PID/HWND、尺寸、非黑图与主题差异检查通过 | `goal-fluent-20261012-closure-gui.json` / `closure-extra-gui.json` |
| 代表性弹层 | demo2 MenuEx、wx emoji、MC 搜索各三相；尺寸不增长，return bit-exact；wx final-bin 再重复一轮 | 上述 extra 索引及各 run 独立 `visual_review.json` |
| Diff 空白检查 | CRLF-aware `git diff --check` PASS | `goal-fluent-20261012-closure-diff-check.log` |

55 项定向过滤必须包含 `FluentPopupChromeE2E`。本轮曾用旧过滤只执行 50 项，已保留 narrow 日志并用 complete 日志实际补齐；不能把 50 项命令标成 55。

## GUI 与视觉证据

主线程实际使用 `functions.view_image` 查看最新 3 张主窗 contact sheets、3 组 popup 三相以及 IPC 全部 32 张构成的 4 张 contact sheets。`visual-verdict` skill 未安装，未声称调用；独立记录见 `output/goal-fluent-20261012-closure-visual-review.json`。

- demo2 popup **250×358**、wx emoji **342×302**、MC search **246×136** 三次相同；light-return 原图字节相同。MC 查询 `sb` 的三项 suggestion 与真实 buddy 对齐，主窗 query 保留；caption/close 分离。
- IPC 浅色与还原相加 **300**、拼接 **100200**；原 Sum 把 n>10 夹到10，结果 **55**，不把错误的5050预期改入产品。暗色调用保留参数10，相加210/拼接10200正确。两进程独立切主题不掉连接，显式断开/重连、服务端观察 client exit、scoped cleanup 正确；“日志:”有 inset。
- Wallpaper 使用原异步管线的离线合成 fixture；`SOUI_WALLPAPER_SMOKE_DIR` 仅传给测试子进程，未恢复到真实 HTTP，也未关闭图片 ROI 验证。此结果不证明远程服务可用。
- 一次 wx 短等待的异步 popup return capture 未发现可见 popup，原失败保留；增加有界等待后旧阶段与最终二进制的多轮重开均通过。根因未严格证明，不包装成产品修复；modal 调度仍需保留该自动化时序边界。
- basic smoke 的 demo/sci 用了 owned-PID termination fallback；其他正常返回值1/2不是工具失败。cleanup 无 error、无残留，不等同于所有应用优雅退出路径通过。

## Review 收敛

此前独立有界 review 对 WindowChrome 两项 P2 的修复、PopupOwner/factory/delegate/lifecycle、固定 body-width 与三处接线已收敛，无待处理的确定产品问题。最后资源 wrapper 的独立只读 review 发现 mock-only 测试依赖真实 builder 的 P2；已改为临时占位路径，并补“成功却缺 header”不发布测试。review 与实际构建/GUI 分开，不用 reviewer 读日志替代主线程运行。

## 复现命令与环境

Windows x64 Debug，`SOUI_ENABLE_GUI_SMOKE=ON`。本机 v143 ATL 不完整，构建期间仅临时 `_CL_` 加 v141 ATL include、`_LINK_` 加 x64 lib，结束恢复；未修改工具链或 WINVER。头文件、RC2来自真实 `tools/uiresbuilder.exe`，更新后运行：

~~~powershell
python -X utf8 -B demos/fluent/build_resources.py
python -X utf8 -B demos/fluent/generate_chrome_resources.py --check
python -X utf8 -B demos/fluent/check_contracts.py
cmake --build build --config Debug --target demo demo2 wxDemo flowlayout_demo framelayout_demo gridctrl_demo hellocharts_demo lightbutton_demo mclistview_demo propanimator_demo sci_demo wallpaper ipc_client ipc_server fun_test --parallel 4
ctest --test-dir build -C Debug -N -L '^soui-headless$'
ctest --test-dir build -C Debug -L '^soui-headless$' --output-on-failure
ctest --test-dir build -C Debug -R 'soui_(fluent|demo_fluent)|FluentDockStatesE2E|FluentPopupChromeE2E' --output-on-failure
build/bin/Debug/fun_test.exe --gtest_filter=soui_fluent*:soui_demo_fluent*:FluentDockStatesE2E.*:FluentPopupChromeE2E.* --gtest_shuffle --gtest_repeat=2
build/bin/Debug/fun_test.exe --gtest_filter=soui_render_text.*
ctest --test-dir build -C Debug -L '^soui-gui$' --output-on-failure
python -X utf8 -B -m unittest discover -s demos/fluent -p 'tests_*.py' -v
python -X utf8 -B demos/fluent/test_build_resources.py
python -X utf8 -B demos/wallpaper/test_fluent_static.py
python -X utf8 -B output/goal-fluent-20261012-run-closure-gui.py
git -c core.whitespace=blank-at-eol,blank-at-eof,space-before-tab,cr-at-eol diff --check
~~~

实际 shared 97 命令从 `demos/fluent` 执行 `python -X utf8 -B -m unittest -v tests_gui_smoke tests_sync_row_text tests_chrome_resources test_build_resources`。GUI 每项完整命令在索引，不用脚本顶部旧坐标。完整迁移**不要重跑全局 `generate_resources.py` 套模板**，以免覆盖定制 palette。

## 明确的未验证与未实现范围

- Native composed-pixel 取样仍 **unavailable**：安全的 owned-window/backdrop 可见性守卫未成立，不抓全桌面、不放宽安全检查、不以 RT corner alpha 代替 native display。XML `native_pixel_evidence="unavailable"` 必须随命令 PASS 一起读。真实 skin/RT 透明角、尺寸/模式/命令/销毁已分别验证，但不是桌面背景合成像素证明。
- Linux/macOS/移动端、Windows Release、完整 OS-DPI×语言×页面矩阵未实跑。已有代表性应用 scale 与语言入口不等于全矩阵覆盖。
- 150ms 参数与精确曲线、普通 HWND 的 keyboard/reduced 分支及快速销毁已测；没有物理帧/录屏时序、系统设置更改后的端到端证明或所有快速重入组合。native reduced 用受控分支，不修改用户全局系统设置。
- Wallpaper 真实 HTTP、全部业务/错误/超时路径、native 编辑菜单及未迁移 dateTimePicker popup 未完成本次自动化覆盖。exit fade/逐项 hover不是本次实现承诺。
- staged generator 对 builder 失败/缺输出保留原文件；输出写入中途失败不提供双文件事务回滚。
- 人工最终 Review、线上 CI Required Checks、PR、发布未执行。本地通过不宣称已发布或门禁已生效。

新证据索引：`output/remaining-fluent/evidence/manifest-20261012-goal-final.json`。旧 `manifest.json` 不覆盖；当前 goal 完成表示上述实现与本机适用验收闭环完成，不表示这些边界已被消除。
