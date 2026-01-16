# 基于SOUI框架的军旗游戏棋子移动动画实现：如何用少量代码打造流畅视觉体验

## 概述

SOUI是一套完全开源的跨平台客户端界面开发框架，已持续开发维护14年，核心编译后体积约1MB。该框架借鉴了WTL、Android、QT和Flash等主流框架的优点，适用于高性能桌面应用开发。代码完全开源，托管于Gitee平台：https://gitee.com/setoutsoft/soui4

本文档详细介绍了如何使用SOUI的属性动画结合锚点布局实现军旗游戏中棋子的移动效果。该方案重点解决了两个关键技术问题：
1. 通过SetKeyFrameWeights方法确保不同距离棋格间的棋子移动具有相同的速度
2. 根据棋格和己方位置的相对关系动态设置棋子旋转方向

特别值得一提的是，实现这些复杂而流畅的动画效果只需要少量的代码，大约50-60行代码即可完成整个动画系统的核心逻辑。

## 技术架构

### 核心组件
- **属性动画系统**: 使用IPropertyAnimator实现平滑移动效果
- **锚点布局**: 利用SAnchorLayout实现精准定位
- **动画插值器**: 使用AnchorPos类型的关键帧控制移动轨迹

## 关键技术实现

### 1. 相同移动速度的实现机制

#### 核心原理
通过计算棋子移动路径上每段距离的权重，使用SetKeyFrameWeights方法设置动画关键帧权重，确保棋子按照实际移动距离的比例分配动画时间。

#### 代码实现分析

```cpp
std::vector<POINT> lstCell;
m_layout.GetMoveCMPath(bSapper, pt, ptTarget, &lstCell);
if(lstCell.empty())
    return FALSE;

std::vector<AnchorPos> lstPos;
std::vector<float> lstRotate;
lstPos.resize(lstCell.size());
lstRotate.resize(lstCell.size());
std::vector<float> lstWeight;
lstWeight.resize(lstCell.size()-1);
float fTotalWeight = 0;

for(int i=0;i<lstCell.size();i++)
{
    lstPos[i].type = 10;
    lstPos[i].x.fSize = lstCell[i].x;
    lstPos[i].y.fSize = lstCell[i].y;
    lstPos[i].fOffsetX = -0.5f;
    lstPos[i].fOffsetY = -0.5f;
    lstRotate[i] = pPiece->CalcRotate(lstCell[i], m_iSelfIndex);
    if(i>0){
        lstWeight[i-1] = distance(lstCell[i], lstCell[i-1]);
        fTotalWeight += lstWeight[i-1];
    }
}

IPropertyValuesHolder *pHolderPos = SPropertyValuesHolder::ofPosition(LayoutProperty::POSITION, lstPos.data(), lstPos.size(), sizeof(AnchorPos));
pHolderPos->SetKeyFrameWeights(lstWeight.data(), lstWeight.size());
IPropertyValuesHolder *pHolderRotate = SPropertyValuesHolder::ofFloat(WindowProperty::ROTATE, lstRotate.data(), lstRotate.size());
pHolderRotate->SetKeyFrameWeights(lstWeight.data(), lstWeight.size());

IPropertyValuesHolder *holders[] = { pHolderPos, pHolderRotate };
SAutoRefPtr<IPropertyAnimator> pAnimator(SPropertyAnimator::ofPropertyValuesHolder(pPiece, holders, 2), FALSE);
pHolderPos->Release();
pHolderRotate->Release();

pAnimator->setDuration(100*fTotalWeight);
```

#### 重要说明
- `distance`函数计算相邻棋格之间的欧几里得距离
- `lstWeight`数组存储每段移动的距离权重
- SetKeyFrameWeights将距离权重应用于动画关键帧，确保速度恒定
- 动画总时长与总移动距离成正比(`100*fTotalWeight`)

### 2. 旋转方向的动态计算

#### 计算原理
棋子的旋转角度根据其在棋盘上的位置以及玩家视角进行动态计算，确保每个玩家都能看到棋子以正确的方向移动。

#### 旋转角度计算

```cpp
float fRotate = pPiece->CalcRotate(lstCell[i], m_iSelfIndex);
```

棋子的旋转角度计算方法考虑了以下几个因素：
- 当前棋子的位置(lstCell[i])
- 当前玩家的视角(m_iSelfIndex)
- 棋子相对于玩家的朝向

#### 旋转实现细节
在棋子移动过程中，不仅位置发生变化，旋转角度也同时变化，使得棋子看起来像是沿着路径转动前进。

### 3. 动画过程中的路径指示

#### 移动方向指示
在棋子移动过程中，系统会实时显示移动路径：

```cpp
void CJunqiGame::onAnimationUpdate(IValueAnimator *pAnimator)
{
    IPropertyAnimator *pPropAnimator = sobj_cast<IPropertyAnimator>(pAnimator);
    if(pPropAnimator->GetID() == ANI_MOVE)
    {
        float fraction = pAnimator->getAnimatedFraction();
        IPropertyValuesHolder *pHolder = pPropAnimator->GetPropertyValuesHolderByIndex(0);
        int index = pHolder->Fraction2FrameIndex(fraction);
        int oldIndex = (int)(UINT_PTR)pAnimator->GetUserData();

        if(index != oldIndex)
        {    
            // ... 创建路径指示图标 ...
            for(int i = oldIndex+1; i <= index; i++)
            {
                // ... 创建方向图标 ...
                int nDir = CalcMoveDir(pt1, pt2);
                pDir->SetIcon(nDir);
            }
        }
    }
}
```

#### 方向计算方法

```cpp
int CJunqiGame::CalcMoveDir(POINT ptFrom, POINT ptTo){
    int nDir = 0;
    int nDiffX = ptTo.x - ptFrom.x;
    int nDiffY = ptTo.y - ptFrom.y;
    if(nDiffX==0){
        nDir = nDiffY>0?0:4;
    }else if(nDiffY==0){ 
        nDir = nDiffX>0?2:6;
    }else{
        if(nDiffX>0 && nDiffY>0){
            nDir = 1;
        }else if(nDiffX>0 && nDiffY<0){
            nDir = 3;
        }else if(nDiffX<0 && nDiffY>0){
            nDir = 7;
        }else if(nDiffX<0 && nDiffY<0){
            nDir = 5;
        }
    }
    return nDir;
}
```

## 与其他军旗游戏的对比

### QQ游戏四国军旗
QQ游戏的四国军旗在棋子移动方面采用了较为简单的处理方式：
- 没有真正的棋子移动动画，只是在移动路径的格子中定时显示一个运动方向标志
- 这种设计虽然看起来有点像移动动画效果，但实际上并没有真正移动棋子
- 通过这种方式巧妙地规避了棋子旋转的问题，减少了视觉上的复杂度
- 效果相对简单，但实现起来较容易，对性能要求较低

### 其他军旗游戏
- 有些军旗游戏确实实现了棋子移动动画，但使用的是正方形棋子
- 正方形棋子的设计可以规避棋子旋转带来的视觉方向问题
- 因为正方形无论怎么旋转看起来都一样，所以不需要处理复杂的旋转逻辑

### 我们的解决方案优势
相比之下，我们的实现方案具有以下优点：
- 提供真正的棋子移动动画，增强游戏体验的真实感
- 通过动态旋转处理，解决了棋子在不同玩家视角下的方向问题
- 使用加权关键帧技术，确保棋子移动速度的一致性
- 路径指示功能增强了游戏的交互性和策略性
- 实现这些高级功能仅需要少量代码，体现了SOUI框架的强大能力

## 性能优化考虑

### 动画帧率控制
- 使用SetKeyFrameWeights避免了复杂的插值计算
- 在回放模式下禁用动画以提高性能

### 内存管理
- 使用智能指针管理动画资源
- 动画结束后及时清理相关UI元素

## 扩展性设计

### 动画类型支持
当前实现支持：
- 棋子移动动画(`ANI_MOVE`)
- 棋子交换动画(`ANI_SWAP`)
- 提示消息动画(`ANI_TIP`)

### 配置化支持
- 可通过主题系统调整动画参数
- 支持开关动画效果

## 总结

该实现方案通过巧妙运用SOUI的属性动画系统和锚点布局，实现了流畅自然的棋子移动效果。通过SetKeyFrameWeights方法确保了棋子移动速度的一致性，通过动态计算旋转角度保证了不同玩家视角下的正确显示，同时提供了路径指示功能增强用户体验。这种设计既保证了动画效果的自然流畅，又具备良好的性能表现和扩展性。

最值得注意的是，这些复杂而精美的动画效果仅需少量代码（约50-60行核心代码）即可实现，充分展现了SOUI框架在动画处理方面的强大能力和简洁性。