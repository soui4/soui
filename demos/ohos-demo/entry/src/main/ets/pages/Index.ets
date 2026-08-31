/*
 * Index.ets
 *
 * SOUI for OHOS Demo 主页面。
 *
 * 放置 SouiScreen 组件，传入 screenId 和 layoutName。
 * SouiScreen 在首次尺寸就绪后自动调用 SouiPlatformBridge.registerScreen
 * → C++ 层 ScreenStartup → 创建 SHostWnd（CMainDlg）并显示。
 */

import { SouiScreen } from '../soui/SouiScreen';
import { hilog } from '@kit.PerformanceAnalysisKit';

const DOMAIN = 0x5350;
const TAG = 'SOUI_Demo';

@Entry
@Component
struct Index {
  @State private souiReady: boolean = false;

  aboutToAppear(): void {
    // 检查 SOUI 是否已启动（资源复制完成后由 EntryAbility 触发）
    // 这里通过定时轮询等待，实际项目中可用回调
    this.checkReady();
  }

  private checkReady(): void {
    // SouiPlatformBridge.startSouiApp 已在 EntryAbility 中调用
    // 这里简单等待一帧后显示
    setTimeout(() => {
      this.souiReady = true;
      hilog.info(DOMAIN, TAG, 'Index souiReady=true');
    }, 100);
  }

  build() {
    Column() {
      if (this.souiReady) {
        SouiScreen({ screenId: 1, layoutName: 'layout:dlg_main' })
          .width('100%')
          .height('100%')
      } else {
        Column({ space: 12 }) {
          Text('正在初始化 SOUI...')
            .fontSize(18)
            .fontColor('#334155')
          LoadingProgress()
            .width(48)
            .height(48)
        }
        .width('100%')
        .height('100%')
        .justifyContent(FlexAlign.Center)
      }
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#F5F7FA')
  }
}
