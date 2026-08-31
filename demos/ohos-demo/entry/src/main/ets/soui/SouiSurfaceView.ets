//
// SouiSurfaceView.ets
//
// SouiSurface 的 ArkUI 视图组件。使用 @ObjectLink 引用 @Observed SouiSurface 实例，
// 当 SouiSurface 的视觉属性（x/y/width/height/visible/enabled）变化时自动触发重新渲染。
//
// 对应 Android 端 View 体系：View.setVisibility/setTranslationX/setLayoutParams 等方法
// 直接触发 View 重新布局/重绘；ArkUI 中等价地通过 @Observed + @ObjectLink 实现。
//
// 设计：
//   - @Component + @ObjectLink，绑定 SouiSurface 实例
//   - build() 中声明 Canvas 组件，绑定响应式属性
//   - 事件回调委托给 SouiSurface 的 onTouchEvent/onKeyEvent 等方法
//   - Canvas 生命周期（onReady/onAreaChange）回调 SouiSurface

import { SouiSurface } from './SouiSurface';

@Component
export struct SouiSurfaceView {
  // @ObjectLink 引用 @Observed SouiSurface，属性变化自动触发 build() 重新执行 
  @ObjectLink surface: SouiSurface;

  build() {
    // 尺寸策略：
    //   - width/height > 0：精确像素尺寸（SOUI 已定位）
    //   - 子窗口且 0 尺寸：填满父容器（对齐 Android WRAP/MATCH fallback，保留旧行为）
    //   - 顶层 0 尺寸窗口（如 0x0 的 tooltip host）：保持 0 尺寸，避免被展开成
    //     全屏 Canvas 抢占渲染/触摸层 —— 它只是不可见的消息窗口
    Canvas(this.surface.mCtx)
      // 用 .id() 而非 .key()：focusControl.requestFocus 接受的是 .id() 设置的值。
      // 早期用 .key() 导致 requestFocus 找不到组件，Canvas 不获焦，IME attach 后
      // insertText 事件不触发（IME 未绑定到获焦的可编辑组件）→ 字符传不到 SOUI edit。
      .id(this.surface.focusKey)
      .width(this.surface.width > 0 ? this.surface.width : (this.surface.isChild ? '100%' : 0))
      .height(this.surface.height > 0 ? this.surface.height : (this.surface.isChild ? '100%' : 0))
      .position({ x: this.surface.x, y: this.surface.y })
      .visibility(this.surface.visible ? Visibility.Visible : Visibility.Hidden)
      .enabled(this.surface.enabled)
      // focusable + focusOnTouch：让 Canvas 可获焦并在触摸时夺焦，配合 focusKey
      // 实现 focusControl.requestFocus 定位。Canvas 获焦后配合 imc.attach 绑定
      // 自绘组件 IME，使 SOUI 自绘 edit 控件也能接收输入法文本。
      .focusable(true)
      .focusOnTouch(true)
      .onReady(() => {
        this.surface.onCanvasReady();
      })
      .onAreaChange((_old, newArea) => {
        const w = Math.floor(newArea.width as number);
        const h = Math.floor(newArea.height as number);
        this.surface.onAreaChanged(w, h);
      })
      .onTouch((event: TouchEvent) => {
        this.surface.onTouchEvent(event);
      })
      .onKeyEvent((event: KeyEvent) => {
        return this.surface.onKeyEvent(event);
      });
  }
}
