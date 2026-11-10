/*
 * Index.ets
 *
 * 基金持仓助手（OHOS）主页面。
 *
 * 放置 SouiScreen 组件，传入 screenId 和 layoutName。
 * SouiScreen 在首次尺寸就绪后自动调用 SouiPlatformBridge.registerScreen
 * → C++ 层 ScreenStartup → 创建 CMainDlg（layout:dlg_main）并显示。
 */

import { SouiScreen } from '@soui/ohos';
import { hilog } from '@kit.PerformanceAnalysisKit';

const DOMAIN = 0x5350;
const TAG = 'ohos_fundholdings';

@Entry
@Component
struct Index {
  @State private souiReady: boolean = false;

  aboutToAppear(): void {
    this.checkReady();
  }

  private checkReady(): void {
    // SouiPlatformBridge.startSouiApp 已在 EntryAbility 中调用
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
          Text('正在初始化基金持仓...')
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
    .backgroundColor('#0B1018')
  }
}
