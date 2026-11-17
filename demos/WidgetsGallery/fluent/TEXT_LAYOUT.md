# 公共文字测量与布局修复（2026-10-12）

## 目标与边界

用户发现多个界面文字贴边、顶部裁剪，要求找到共性根因，而不是逐页增加高度。
本次在此前 Fluent 改造之上修复公共文本路径，保留已验证的导航密度和首页局部修复。
不改变控件 name/id、adapter、事件、固定尺寸、公共 API、平台字体分支或子模块指针；不新增依赖。

**两个问题必须分开：**字形完整性由公共测量/绘制契约保障；行文字的设计留白由显式 Fluent 样式保障。
给所有 `<text>` 强加 padding 会改变绝对定位、居中列和布局演示，不能作为根因修复。

## 已复现的根因

1. **Skia 测量框与实际字形不一致。**旧高度依赖主字体 metrics 与固定 4px 补偿，无法可靠覆盖回退字体、斜体 sidebearing 和实际混合 run；浮点截断还可能丢失边缘像素。`CALCRECT` 原来受 viewport/输入高度影响。现在 `MeasureText`、`DrawText` 与 `TextOut` 共用绘制属性，采用主字体推荐行高与实际 run ink bounds，向外取整，统一 baseline、width 和 height。
2. **冷缓存的回退字形使用了错误字体升级 metrics。**先求 advance、后求完整 metrics 时，缓存原来用主字体 scaler 解读回退字体的 glyph ID。真实离屏复现：`基础控件` 的 ink 宽约 48px，旧测量框仅 26px；修复后冷测量框为 51px，四字完整。必须在新进程中先测后画；先画后测会热缓存并掩盖缺陷。两个 `getUnicharMetrics` overload 现在都使用字形记录的 source context。
3. **多行文本的预算与绘制不一致。**有界测量未扣除自身 margin/padding，且人为给宽度至少 10px；多行行高用 `A` 代表所有文字，输入 bottom 又限制了自然高度。现在预算与 `GetTextRect` 一致，按完整行测高度，空行才使用 `A`；`CALCRECT` 计算全部自然高度。center/bottom 对齐只移动 top，保留原 bottom，避免漏行。
4. **加权线性布局将已分配 px 再当 dp 缩放。**二次测量时，125%–200% 下约束宽度被放大，文字按错误宽度算行数。现在只用 px 内容预算，扣除主轴 extend，并完整恢复原布局参数。
5. **负字号增量误用了布局哨兵值。**`adding:-1/-2/-3` 被解释成 wrap/match/invalid，而不是有符号字号差。字体池现在单独换算 signed delta，保留原 `floor(x + 0.5)` 舍入、缓存和继承规则。
6. **Fluent 行文字留白入口不一致。**对用户列出的 13 个 demo 的 183 个 XML 做了审计。确认 demo2 的 `txt_test` 是没有已有 inset 的左对齐裸行标签；wallpaper 的分类列表写 `text-x="8"`，在其 px 默认单位下不会跟随 DPI。显式 `fluent_row_label` 仅左右 8dp；只用于该行标签。wallpaper 的 local listbox 入口使用 `8dp`。已有图标偏移、居中列、绝对定位示例不叠加 padding。

## 裁剪契约与审查修正

自然测量变大不能让刻意设置的小框溢出。`SStatic` 多行绘制在非 `CALCRECT` / 非 `NOCLIP` 时，使用垂直对齐前的原调用矩形做 `RGN_AND`，绘制后恢复已有 RT clip。
`NOCLIP` 不因输入高度停止后续显式换行，但仍受调用者原有 clip 限制。独立审查指出的这两个边界先新增像素/行提交回归，修前 3/3 失败，修后通过。

Skia 独立审查指出的旋转省略号约束和长段落重复扫描也已修正：旋转绘制沿用原宽度；段落边界缓存；省略点先按 advance 定位，再核对实际 ink。
长文本测试断言结果，不用脆弱的毫秒阈值作为通过条件。

## 主要文件

- `components/render-skia/drawtext-skia.{cpp,h}`、`render-skia.{cpp,h}`：统一字形尺寸、baseline、CALCRECT、NOCLIP、换行与旋转约束。
- `third-part/skia/src/core/SkGlyphCache.cpp`：回退字形冷缓存 metrics 升级。这是主仓库 tracked 文件，不是子模块变更。
- `SOUI/src/core/Swnd.cpp`、`SOUI/src/control/SCmnCtrl.cpp`：预算、自然行高、垂直对齐和原框裁剪。
- `SOUI/src/layout/SLinearLayout.cpp`、`SOUI/src/res.mgr/SFontPool.cpp`：px 二次测量与 signed 字号增量。
- `demos/fluent/controls.xml`、`sync_row_text.py`：显式行样式及局部、幂等同步入口。
- `demos/demo2/uires/{xml/dlg_main.xml,uidef/init.xml}`、`demos/wallpaper/uires/{xml/dlg_main.xml,uidef/init.xml}`：仅经审计确认的留白入口。
- `demos/fun_test/test_render_text.cpp`、`test_soui_text_layout.cpp`、`test_soui_font_delta.cpp`、`test_soui_linear_dpi.cpp`、`test_e2e_fluent_controls.cpp`：真实 renderer/core/XML 回归。

## 复现命令

```powershell
# 明确选择已支持的资源入口，不全量覆盖各 demo 的定制 UIDEF/palette。
python -B demos/fluent/sync_row_text.py --demo demo2 --demo wallpaper --check
python -B demos/fluent/build_resources.py --demo demo2 --demo wallpaper
python -B demos/fluent/check_contracts.py
python -B -m unittest discover -s demos/fluent -p 'tests_*.py'

cmake -S . -B build
cmake --build build --config Debug --target fun_test --parallel 4
& build/bin/Debug/fun_test.exe '--gtest_filter=soui_render_text.*:soui_font_delta.*:soui_linear_dpi.*:soui_text_layout.*'
# 冷缓存：独立进程，不先 TextOut 中文。
& build/bin/Debug/fun_test.exe '--gtest_filter=soui_render_text.skia_multiline_tight_box_preserves_each_line_ink'
ctest --test-dir build -C Debug -L '^soui-headless$' --output-on-failure
& build/bin/Debug/fun_test.exe '--gtest_filter=-window.*'
```

本地主机用已有 VS2022/x64/Debug 构建树，仅通过 shell 环境补入已安装的 14.16.27023 ATL include/lib；未改变工程/toolchain。详细命令及 GUI owned-PID 用法见 `README.md` 与本次证据目录 `output/text-root-cause/`。

## 最终验证记录

2026-10-12 收尾验证已完成。公共文本修复与 13 Demo Fluent 改造在同一最终工作区构建，Windows VS2022 x64 Debug 的 14 个 GUI target 与 fun_test 构建通过；基础 CTest 注册 521 项，520 项启用并执行通过，1 项既有 Disabled。完整 Goal 结果及未覆盖范围见 `GOAL_VERIFICATION_20261012.md`，这里不把组合工作区的通过当作单独重放前置提交的证明。

提交前再次实际运行 `soui_render_text.*:soui_font_delta.*:soui_linear_dpi.*:soui_text_layout.*`，四组 **46/46** 通过；其中渲染回归 **13/13** 包含冷缓存 source-context 升级、同 glyph ID 在不同字体上下文中不互相污染。最新日志为 `output/goal-fluent-20261012-commit-text.log`。最终源码与构建锁中的 1120 项 SHA-256 一致。

此前公共文本路径的 9 个实现文件及 4 个配套回归文件按前置修复单独提交；13 Demo 的资源、接线及演示回归另行提交，不将核心改动藏在换肤提交中。技能安装、OMX 状态、截图、构建输出和 Python 缓存不属于交付源码。

未运行 Linux/macOS/移动端或 Windows Release；不覆盖全部字库、复杂 shaping、所有 fallback 淘汰路径或每页逐像素验收。GUI 命令通过不等于 native composed-backdrop 像素证据通过。

## 不应误报为本次缺陷的情况

- 明确固定的过小文本框仍保持固定尺寸，不会偷偷放大。资源作者须给予足够空间或有意选择省略/裁剪。
- 滚动视口顶部/底部只显示半行是正常 viewport clip，与完整行框里的字形顶部丢失不同。
- Windows 上的真实像素/DPI 证据不能泛化成 Linux/macOS/Android/OHOS 通过；未覆盖复杂 shaping、所有字库和每一页的逐像素验收。
- 明暗往返截图证明代表场景的外观，不替代每个业务交互或真实跨屏 DPI 输入验证。
