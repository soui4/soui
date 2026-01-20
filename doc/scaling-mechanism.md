# SOUI开心象棋demo中的创新缩放机制

## 1. 引言

在棋牌游戏开发中，窗口拉伸调整是一个常见的用户需求。如何在不同屏幕尺寸和分辨率下保持良好的用户体验，是开发者面临的一个重要挑战。本文将详细介绍SOUI开心象棋demo中实现的一种创新缩放机制，该机制巧妙地解决了传统缩放方案的局限性。

## 2. 传统缩放方案分析

### 2.1 固定尺寸切换方案

**代表产品**：QQ游戏象棋

**实现方式**：提供两套或几套固定尺寸配置，根据用户的屏幕分辨率选择合适的尺寸，不支持自由拉伸。

**优点**：
- 功能按钮保持合适的比例，方便用户操作
- 开发简单，无需复杂的缩放逻辑

**缺点**：
- 无法充分利用屏幕空间
- 缺乏灵活性，用户体验受限

### 2.2 全屏幕等比例缩放方案

**代表产品**：JJ象棋

**实现方式**：整个游戏界面所有元素等比例缩放，包括棋盘、按钮、头像等。

**优点**：
- 充分利用屏幕空间
- 实现相对简单

**缺点**：
- 功能按钮可能会过大或过小，影响操作体验
- 在极端分辨率下，UI元素可能会失真

## 3. SOUI开心象棋的创新方案

### 3.1 设计思路

SOUI开心象棋demo实现了一种新的缩放机制，结合了上述两种方案的优点：

- **棋盘部分**：根据棋盘窗口的大小按比例缩放
- **UI元素（功能按钮、头像等）**：仍然根据屏幕DPI进行缩放，保持常规比例

### 3.2 核心技术架构

#### 3.2.1 锚点布局（SAnchorLayout）的巧妙运用

需要明确的是，**锚点布局并不是这套缩放机制的基础**，而是我们巧妙利用了锚点布局提供的`AnchorPos.type`参数来区分不同类型的元素。SOUI默认的锚点类型是0-8（9个参考点），用于窗口位置定位。

在棋牌游戏中，棋盘中的元素（如棋子）需要参考棋盘坐标系，而不是窗口坐标系。为此，我们采用了**业务层自定义的锚点解析机制**来处理棋子坐标，并为这些棋盘元素指定了`type >= 10`的特殊标识，与常规UI元素区分开来：

| 元素类型 | `AnchorPos.type` | 缩放策略 |
|---------|-----------------|---------|
| 棋盘元素（棋子、棋盘格等） | ≥ 10 | 根据棋盘窗口大小计算的缩放比例进行缩放 |
| UI元素（按钮、头像等） | < 10 | 根据屏幕DPI进行缩放 |

#### 3.2.2 缩放消息处理机制

核心实现位于`CChessBoard::SDispatchMessage`方法中，该方法专门处理`UM_SETSCALE`消息，根据元素类型选择性地传递缩放消息：

```cpp
void CChessBoard::SDispatchMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == UM_SETSCALE)
    {
        if(lParam!=0)
        {
            m_nChsScale = (int)wParam;
        }else{
            SSendMessage(message, wParam, lParam);
        }
        // 遍历所有子元素，根据类型选择是否传递缩放消息
        SWindow *pChild = GetWindow(GSW_FIRSTCHILD);
        while (pChild)
        {
            SAnchorLayoutParam *pParam = (SAnchorLayoutParam *)pChild->GetLayoutParam();
            SAnchorLayoutParamStruct *pParamStruct = (SAnchorLayoutParamStruct *)pParam->GetRawData();
            BOOL bChessAnchor = pParamStruct->pos.type >= 10;
            if (bChessAnchor && lParam != 0) // 棋盘元素且是用户指定的缩放
            {
                pChild->SDispatchMessage(message, wParam, lParam);
            }
            else if (!bChessAnchor && lParam == 0) // UI元素且是DPI配置的缩放
            {
                pChild->SDispatchMessage(message, wParam, lParam);
            }
            pChild = pChild->GetWindow(GSW_NEXTSIBLING);
        }
    }
    else
    {
        __baseCls::SDispatchMessage(message, wParam, lParam);
    }
}
```

#### 3.2.3 缩放类型区分

该方法通过`lParam`参数区分两种不同的缩放类型：

- **`lParam == 0`**：表示这是窗口DPI配置的缩放比例，仅应用于常规UI元素
- **`lParam != 0`**：表示这是用户指定的缩放比例，仅应用于棋盘元素

#### 3.2.4 棋盘大小变化处理

**`CChessGame::OnGameBoardSizeChanged`函数是这套机制的关键触发点**，当棋盘的宿主窗口大小变化时，该函数被调用，负责计算合适的缩放比例并发送缩放消息：

```cpp
void CChessGame::OnGameBoardSizeChanged(IEvtArgs *e)
{
    EventSwndSize *evt = sobj_cast<EventSwndSize>(e);
    SSkinBoard * pSkin = sobj_cast<SSkinBoard>(m_pChessBoard->GetSkin());
    CSize szBoard = m_pChessBoard->GetSkin()->GetSkinSize();
    CRect rcGameGoard = m_pGameBoard->GetClientRect();
    CSize szGameBoard = evt->szWnd;
    float fRatio1 = (float)szGameBoard.cx / szGameBoard.cy;
    float fRatio2 = (float)szBoard.cx / szBoard.cy;
    CRect rcBoard =  rcGameGoard;
    float scale = 1.0f;
    
    // 根据窗口宽高比和棋盘宽高比计算合适的缩放比例
    if(fRatio1 > fRatio2)
    {
        int nWid = (int)(szGameBoard.cy * fRatio2);
        rcBoard.DeflateRect((szGameBoard.cx - nWid) / 2, 0);
        scale = (float)nWid / szBoard.cx;
        m_pChessBoard->Move(rcBoard);
    }
    else
    {
        int nHei = (int)(szGameBoard.cx / fRatio2);
        rcBoard.DeflateRect(0, (szGameBoard.cy - nHei) / 2);
        scale = (float)nHei / szBoard.cy;
        m_pChessBoard->Move(rcBoard);
    }
    
    // 计算棋盘单元格大小
    CRect rcMargin = pSkin->GetMargin();
    CSize szCellAll = szBoard - CSize(rcMargin.left + rcMargin.right,rcMargin.top+rcMargin.bottom);
    m_cellWidth = szCellAll.cx * scale / 8;
    m_cellHeight = szCellAll.cy * scale / 9;
    
    // 调整棋盘边距
    rcMargin.left *= scale;
    rcMargin.top *= scale;
    rcMargin.right *= scale;
    rcMargin.bottom *= scale;
    rcBoard.DeflateRect(rcMargin);
    m_ptBoardOrigin.x = rcBoard.left;
    m_ptBoardOrigin.y = rcBoard.bottom;
    
    // 发送缩放消息，lParam=1表示这是用户指定的缩放比例
    m_pGameBoard->SDispatchMessage(UM_SETSCALE, scale*100, 1);
}
```

## 4. 实现细节与代码分析

### 4.1 锚点布局参数的利用

虽然锚点布局不是这套机制的基础，但我们巧妙地利用了锚点布局的参数结构来传递棋盘元素的特殊标识。锚点布局的核心参数定义在`SAnchorLayoutParamStruct.h`中：

```cpp
struct AnchorPos
{
    int type;                  // 锚点类型，>=10表示棋盘元素
    SLayoutSize x, y;         // 坐标值
    float fOffsetX, fOffsetY;  // 窗口坐标偏移量
};

struct SAnchorLayoutParamStruct
{
    AnchorPos pos;             // 锚点位置
    SLayoutSize width;         // 宽度
    SLayoutSize height;        // 高度
};
```

### 4.2 棋盘元素标识设置

在游戏初始化或元素创建时，为棋盘元素设置`type >= 10`的特殊标识：

```cpp
// 为棋子设置锚点类型示例
SAnchorLayoutParam *pParam = (SAnchorLayoutParam*)pPiece->GetLayoutParam();
pParam->pos.type = 10;  // 设置为棋盘元素
pParam->pos.x.fSize = x;
pParam->pos.y.fSize = y;
```

### 4.3 缩放消息的触发与发送

缩放消息主要由`CChessGame::OnGameBoardSizeChanged`函数触发，该函数在棋盘宿主窗口大小变化时被调用：

```cpp
// 发送用户指定的缩放比例，仅影响棋盘元素
m_pGameBoard->SDispatchMessage(UM_SETSCALE, scaleValue, 1);
```

此外，当系统DPI变化时，SOUI框架会自动发送`UM_SETSCALE`消息，`lParam`参数为0：

```cpp
// 系统DPI变化时自动发送，仅影响UI元素
// m_pGameBoard->SDispatchMessage(UM_SETSCALE, dpiScale, 0);
```

## 5. 与传统方案的对比优势

| 特性 | QQ游戏象棋（固定尺寸） | JJ象棋（全屏幕缩放） | SOUI开心象棋（创新方案） |
|------|---------------------|---------------------|--------------------------|
| 功能按钮比例 | ✅ 合适 | ❌ 可能过大/过小 | ✅ 始终合适 |
| 屏幕空间利用率 | ❌ 有限 | ✅ 充分 | ✅ 充分 |
| 支持自由拉伸 | ❌ 不支持 | ✅ 支持 | ✅ 支持 |
| 高分屏适配 | ❌ 较差 | ✅ 支持 | ✅ 完美支持 |
| 操作体验 | ✅ 良好 | ❌ 可能不佳 | ✅ 优秀 |
| 开发复杂度 | ✅ 简单 | ✅ 简单 | ⚠️ 中等 |

## 6. 技术优势深度解析

### 6.1 精细化的元素控制

通过`AnchorPos.type`属性，可以对界面中的每个元素进行精细化的缩放控制，不仅限于棋盘/UI的简单二分。开发者可以根据需要，为不同类型的元素指定不同的缩放策略。

### 6.2 良好的扩展性

这套机制基于SOUI的锚点布局，具有良好的扩展性。开发者可以轻松地将其应用到其他类型的游戏或应用中(如soui提供的另一个四国军旗游戏)，只需为需要特殊缩放的元素设置相应的`type`值即可。

### 6.3 优秀的用户体验

该机制确保了功能按钮始终保持在适合点击的大小，同时允许棋盘充分利用屏幕空间，为用户提供了最佳的视觉效果和操作体验。

### 6.4 支持高分屏适配

结合SOUI的高分屏机制，这套缩放方案能够完美适应不同DPI的屏幕，在各种设备上都能提供清晰、舒适的显示效果。

## 7. 应用场景与推广价值

### 7.1 适用场景

这套缩放机制特别适合以下场景：

- **棋牌类游戏**：棋盘需要自适应，UI按钮需要保持合适大小
- **地图应用**：地图区域需要自适应，控制面板需要固定大小
- **视频播放器**：视频区域需要自适应，控制栏需要固定大小
- **图片浏览器**：图片区域需要自适应，工具栏需要固定大小

### 7.2 推广价值

该设计理念可以推广到其他UI框架中，为开发者提供一种新的UI缩放解决方案。其核心思想——根据元素功能区分缩放策略——具有普遍的参考价值。


## 8. 总结

SOUI开心象棋demo中实现的创新缩放机制，通过结合锚点布局和自定义缩放消息处理，成功解决了传统缩放方案的局限性。该机制能够同时满足"功能按钮保持合适比例"和"棋盘充分利用屏幕空间"的需求，为用户提供了优秀的视觉效果和操作体验。

这套设计理念不仅适用于棋牌游戏，也可以推广到其他需要复杂缩放策略的应用场景中。其核心思想——根据元素功能区分缩放策略——为UI设计提供了一种新的思路，值得开发者借鉴和学习。

## 9. 关于SOUI

SOUI是一个开源的C++轻量级UI框架，专注于跨平台的桌面应用开发。其主要开源信息如下：

- **开源协议**：自定义协议,商用收费
- **项目地址**：https://gitee.com/setoutsoft/soui4
- **最新版本**：当前文档基于SOUI 5.x版本
- **维护状态**：活跃维护中，定期更新和修复bug
- **社区支持**：拥有活跃的开发者社区，提供技术支持和交流

SOUI框架以其轻量、高效、易于扩展的特点，被广泛应用于各种Windows桌面应用开发，包括棋牌游戏、工具软件、工业控制界面等。开心象棋demo是SOUI框架的一个典型应用案例，展示了其强大的UI定制能力和灵活的布局机制。

### 9.1  关于开心象棋

开心象棋demo作为SOUI框架的官方示例项目，遵循SOUI开源协议，其主要开源信息如下：

- **项目位置**：包含在SOUI框架的`games/cnchess`目录下
- **代码结构**：包含完整的客户端和服务器实现
- **可定制性**：开发者可以基于该demo进行二次开发和定制
- **学习价值**：展示了SOUI框架在实际项目中的最佳实践

作为开源项目，开心象棋demo不仅是一个完整的棋牌游戏实现，也是学习SOUI框架的重要资源，开发者可以通过研究其源代码，深入理解SOUI的各种特性和最佳实践。
