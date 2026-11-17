# 标题栏资源对齐（2026-11-15）

本次局部任务将 Gallery、demo 两个辅助对话框及 fun_test 标题栏对齐到 dpidemo：48dp 标题栏、46×40dp 系统按钮、40×32dp 辅助按钮；有图标时图标为16dp、起点8dp、标题起点29dp，无图标标题起点8dp。保留 ID、名称和文字。

字体补充修正：七个布局的 caption 及其中所有 text 显式设置 `size:14dp,bold:1`，包括 Gallery 的 Fluent Design 副标题和 fun_test 的QQ群文字，避免继承默认字体或 `meta` 类的13dp覆盖。Gallery native 原18dp、普通对话框原12dp均改为14dp；正文文字保持原样。三个本地回归文件先新增严格字体断言、确认失败后再修正资源。

为保留原内容空间，固定宿主高度增加8：demo `dlg_skinset` 500→508、`dlg_formatmsg` 300→308；Gallery `dialog` 270dp→278dp、`gallery_native` 300dp→308dp。fun_test `wnd_soui` 使用自动高度，随标题栏自然增长8dp。原为48dp的两个主窗高度不变。

Gallery native 标题栏从根布局内边距中独立出来，正文移入带内边距的容器：正文首项由72dp移至80dp，原生窗口仍为120dp高，正文可用尺寸保持不变。格式消息输入框沿用前一兄弟控件下方5单位的定位，随标题栏下移。

验证命令与本次结果：

- `python demos/demo/test_secondary_titlebars.py`：2项通过。
- `python demos/SOUIWidgetsGalleryDemo/test_titlebar_geometry.py`：2项通过。
- `python demos/fun_test/test_titlebar_resources.py`：3项通过；临时目录实际生成并解密核对90个ZIP条目，验证重复SVG刷新无变化、非标题栏文件不变、归档失败保留旧包。
- `python -m unittest discover -s demos/fun_test -p 'test_fluent_*.py' -v`：7项通过。
- `python demos/SOUIWidgetsGalleryDemo/build_catalog.py`：按补充授权重新生成 `GalleryCatalog.h`；`--check` 检查通过，覆盖矩阵无变化。
- `python -m unittest discover -s demos/SOUIWidgetsGalleryDemo -p 'test_*.py'`：字体修正及重新生成后23项全部通过，之前生成头文件过期的失败已解决。

fun_test 生成器新增 `--titlebars-only`，只刷新十个SVG，再走原有已验证加密归档流程。仓库 `uires.zip` 未在此局部任务中写入；发布资源时运行 `python demos/fun_test/generate_fluent_resources.py --titlebars-only`。

本记录仅为静态资源与离线回归证据。未运行C++构建或GUI验收，不能据此证明实际字体、DPI、渲染或交互无裁剪。除获授权重新生成的 `GalleryCatalog.h` 外，未修改现有C++/CMake；未修改 Gallery SVG、共享测试或 `.spec`；未提交。

## GUI 后续缺陷修复

实际 PrintWindow 证据及限制见 `output/titlebar-alignment/gallery-demo-gui-report.json`，其结论不由上面的静态测试替代。

- 最大化图标重叠根因：Gallery 的 max/restore 共享46×40dp容器，但两个按钮均缺少 `pos`。`SOUI/src/layout/SouiLayout.cpp` 的默认横向参考为 `PIT_PREV_NEAR`（前一兄弟右边）；隐藏 max 后仍保留布局占位，restore 被排到容器右侧，与 close 重叠。`OnSize` 的可见性切换与两个系统命令处理保持不变。最小修复为两个按钮都设置 `pos="0,0"`，与 dpidemo 相同。
- 新增资源级回归先确认缺少原点会失败，再验证两个按钮共享父容器、原点、尺寸、初始可见性及相邻关闭槽。Gallery现有及新增测试24项全部通过。
- demo `dlg_formatmsg` 标题由 `cls_txt_red` 改用已有 `fluent_caption` 主题色；正文 `<color value="#ff0000">` 样例不变。局部3项测试通过，新增用例先失败后通过。
- `cmake --build build --config Debug --target SOUIWidgetsGalleryDemo demo --parallel 4` 成功。日志 `output/titlebar-alignment/gallery-demo-fix-build.log`。catalog重新生成并通过回归；未修改Gallery C++业务逻辑或共享tests。
- GUI复核等待主线UIEditor验证结束，当前不将本次修复声明为GUI通过。
- 原清理脚本同时向所有同PID顶层窗口（含隐藏辅助窗口）发 `WM_CLOSE`，且未等模态窗口结束就关闭主窗，可能污染此前退出异常。output专用脚本改为已知可见业务弹窗先关闭并等待销毁，再关闭已恢复可用的主窗；仅用异步PostMessage。清理顺序本地单测1项通过，退出码对比仍待GUI时段验证，不据此修改业务析构代码。
- demo深色换肤页底部按钮对比不足仍作为正文范围旧问题记录，不在本次修复中处理。

### 独占 GUI 复核完成（03:16–03:19）

UIEditor释放后仅执行约定两项，证据为 `output/titlebar-alignment/gallery-demo-fix-gui-acceptance.json`，其中链接完整命令、PID、PrintWindow截图和窗口状态。6张验收截图均通过 `view_image` 实际查看。

- Gallery PID57464：浅/深两轮通过标题栏按钮最大化和还原；最大化的还原/关闭图标已分离，点击还原后 `maximized=false`，窗口及客户区矩形与初始值一致。未使用系统还原命令代替按钮验收。进程退出码1，本轮未出现访问异常。
- demo PID58180：格式消息浅色标题为深色、深色标题为浅色，正文红色XML示例保留；输入区、重复次数、OK按钮无遮挡，明暗两次X关闭均有效。
- 有序清理只向已无模态子窗的demo主窗异步发一次 `WM_CLOSE`，仍得到 `0xC00000FD`。这不能仅归因于旧脚本的全窗口关闭顺序；保留未归因风险，不扩展业务修复。
- 两个自启进程已退出，桌面已释放给主线GUI106；不再启动GUI。本结论限定上述两项，不代表全DPI矩阵或桌面合成像素验收。
