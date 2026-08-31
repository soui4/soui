//
// NativeEditViewView.ets
//
// NativeEditView 的 ArkUI 视图组件，对应 SouiSurfaceView 的角色。
// 使用 @ObjectLink 引用 @Observed NativeEditView 实例，当 NativeEditView
// 的视觉属性（x/y/width/height/visible/enabled/opacity/padLeft/padRight）
// 变化时自动触发 build() 重新执行，从而更新 TextInput 的实际渲染属性。
//
// 对应 Android 端：View 体系下 EditText 自身的 setTranslationX/
// setLayoutParams/setVisibility 等方法直接触发 View 重新布局/重绘；
// ArkUI 中等价地通过 @Observed + @ObjectLink 实现。
//
// 设计：
//   - @Component + @ObjectLink，绑定 NativeEditView 实例
//   - build() 直接声明 TextInput，所有视觉属性通过 @ObjectLink 绑定
//   - 事件回调（onChange/onFocus/onBlur/onTextSelectionChange）委托给
//     NativeEditView 的公共方法 onTextInputChange/onTextInputFocus 等

import { NativeEditView } from './NativeEditView';

@Component
export struct NativeEditViewView {
  // @ObjectLink 引用 @Observed NativeEditView，属性变化自动触发 build() 重新执行 
  @ObjectLink edit: NativeEditView;

  build() {
    TextInput({ text: this.edit.mText, controller: this.edit.mController })
      // 用 .id() 而非 .key()：focusControl.requestFocus 接受的是 .id() 设置的值。
      .id(this.edit.focusKey)
      .width(this.edit.width > 0 ? this.edit.width : '100%')
      .height(this.edit.height > 0 ? this.edit.height : 48)
      .position({ x: this.edit.x, y: this.edit.y })
      .opacity(this.edit.opacity)
      .enabled(this.edit.enabled && !this.edit.mReadOnly)
      .visibility(this.edit.visible ? Visibility.Visible : Visibility.None)
      .padding({ left: this.edit.padLeft, right: this.edit.padRight })
      .type(this.edit.mPasswordChar !== 0 ? InputType.Password : InputType.Normal)
      // 获焦时自动拉起软键盘：部分 SDK 版本 TextInput 默认不自动弹 IME，显式设置更稳妥。
      .enableKeyboardOnFocus(true)
      .onChange((value: string) => {
        // 委托给 NativeEditView.onTextInputChange 处理 EN_UPDATE/EN_CHANGE 等通知
        this.edit.onTextInputChange(value);
      })
      .onTextSelectionChange((selectionStart: number, selectionEnd: number) => {
        this.edit.onTextInputSelection(selectionStart, selectionEnd);
      })
      .onFocus(() => {
        this.edit.onTextInputFocus();
      })
      .onBlur(() => {
        this.edit.onTextInputBlur();
      });
  }
}
