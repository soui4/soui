/*
 * EntryAbility.ets
 *
 * 基金持仓助手（OHOS）入口 UIAbility。
 *
 * 职责：
 *   1. 加载 N-API 模块（libfundholdings.so / libsoui4ohos.so）
 *   2. 初始化 SouiPlatformBridge 并启动 SOUI 应用
 *      （C++ 侧通过 OH_ResourceManager_InitNativeResourceManager 直读 rawfile 资源）
 */

import { UIAbility, AbilityConstant, Want, common } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { window } from '@kit.ArkUI';

import fundholdings from 'libfundholdings.so';
import { SouiPlatformBridge } from '@soui/ohos';

const DOMAIN = 0x5350;
const TAG = 'fundholdings';

export default class EntryAbility extends UIAbility {
  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    hilog.info(DOMAIN, TAG, 'EntryAbility onCreate');

    // 1. 触发业务层加载（Soui4OhosEntry 注册）
    const demoInited: boolean = fundholdings.init();
    hilog.info(DOMAIN, TAG, 'fundholdings.init() = %{public}s', JSON.stringify(demoInited));

    // 2. 初始化 SouiPlatformBridge（内部直接 import libsoui4ohos.so 使用 N-API 模块）
    SouiPlatformBridge.instance().init(this.context);

    // 3. 直接启动 SOUI（资源从 rawfile 直读，无需复制）
    this.startSouiApp();
  }

  private startSouiApp(): void {
    try {
      const ok = SouiPlatformBridge.instance().startSouiApp();
      hilog.info(DOMAIN, TAG, 'SouiPlatformBridge.startSouiApp() = %{public}s', JSON.stringify(ok));
    } catch (e) {
      hilog.error(DOMAIN, TAG, 'startSouiApp failed: %{public}s', JSON.stringify(e));
    }
  }

  onDestroy(): void {
    hilog.info(DOMAIN, TAG, 'EntryAbility onDestroy');
    SouiPlatformBridge.instance().stopSouiApp();
    SouiPlatformBridge.instance().uninit();
  }

  onWindowStageCreate(windowStage: window.WindowStage): void {
    hilog.info(DOMAIN, TAG, 'EntryAbility onWindowStageCreate');
    windowStage.loadContent('pages/Index', (err) => {
      if (err.code) {
        hilog.error(DOMAIN, TAG, 'Failed to load content: %{public}s', JSON.stringify(err));
        return;
      }
      hilog.info(DOMAIN, TAG, 'Succeeded in loading content.');
    });
  }

  onWindowStageDestroy(): void {
    hilog.info(DOMAIN, TAG, 'EntryAbility onWindowStageDestroy');
  }

  onForeground(): void {
    hilog.info(DOMAIN, TAG, 'EntryAbility onForeground');
  }

  onBackground(): void {
    hilog.info(DOMAIN, TAG, 'EntryAbility onBackground');
  }
}
