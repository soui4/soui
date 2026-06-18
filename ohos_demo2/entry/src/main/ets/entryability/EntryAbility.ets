import { AbilityConstant, ConfigurationConstant, UIAbility, Want } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { window } from '@kit.ArkUI';

const DOMAIN = 0x0000;
const MAIN_WINDOW_KEY = 'souiMainWindow';

function applyMainWindowConfig(mainWindow: window.Window): void {
  try {
    mainWindow.setWindowDecorVisible(false);
    hilog.info(DOMAIN, 'testTag', '%{public}s', 'Window decor hidden.');
  } catch (err) {
    hilog.error(DOMAIN, 'testTag', 'Failed to hide window decor. Cause: %{public}s', JSON.stringify(err));
  }

  try {
    mainWindow.setWindowTitleButtonVisible(false, false, false);
  } catch (err) {
    hilog.error(DOMAIN, 'testTag', 'Failed to hide title buttons. Cause: %{public}s', JSON.stringify(err));
  }

  try {
    mainWindow.setWindowTitleMoveEnabled(false);
  } catch (err) {
    hilog.error(DOMAIN, 'testTag', 'Failed to disable system title move. Cause: %{public}s', JSON.stringify(err));
  }

  try {
    mainWindow.setResizeByDragEnabled(false);
  } catch (err) {
    hilog.error(DOMAIN, 'testTag', 'Failed to disable system resize by drag. Cause: %{public}s', JSON.stringify(err));
  }
}

export default class EntryAbility extends UIAbility {
  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    try {
      this.context.getApplicationContext().setColorMode(ConfigurationConstant.ColorMode.COLOR_MODE_NOT_SET);
    } catch (err) {
      hilog.error(DOMAIN, 'testTag', 'Failed to set colorMode. Cause: %{public}s', JSON.stringify(err));
    }
    hilog.info(DOMAIN, 'testTag', '%{public}s', 'Ability onCreate');
  }

  onDestroy(): void {
    hilog.info(DOMAIN, 'testTag', '%{public}s', 'Ability onDestroy');
  }

  onWindowStageCreate(windowStage: window.WindowStage): void {
    // Main window is created, set main page for this ability
    hilog.info(DOMAIN, 'testTag', '%{public}s', 'Ability onWindowStageCreate');

    let mainWindow: window.Window | null = null;
    try {
      mainWindow = windowStage.getMainWindowSync();
      AppStorage.setOrCreate(MAIN_WINDOW_KEY, mainWindow);
      applyMainWindowConfig(mainWindow);
    } catch (err) {
      hilog.error(DOMAIN, 'testTag', 'Failed to get main window. Cause: %{public}s', JSON.stringify(err));
    }

    windowStage.loadContent('pages/Index', (err) => {
      if (err.code) {
        hilog.error(DOMAIN, 'testTag', 'Failed to load the content. Cause: %{public}s', JSON.stringify(err));
        return;
      }
      if (mainWindow !== null) {
        applyMainWindowConfig(mainWindow);
      }
      hilog.info(DOMAIN, 'testTag', 'Succeeded in loading the content.');
    });
  }

  onWindowStageDestroy(): void {
    // Main window is destroyed, release UI related resources
    hilog.info(DOMAIN, 'testTag', '%{public}s', 'Ability onWindowStageDestroy');
  }

  onForeground(): void {
    // Ability has brought to foreground
    const mainWindow = AppStorage.get<window.Window>(MAIN_WINDOW_KEY);
    if (mainWindow !== undefined) {
      applyMainWindowConfig(mainWindow);
    }
    hilog.info(DOMAIN, 'testTag', '%{public}s', 'Ability onForeground');
  }

  onBackground(): void {
    // Ability has back to background
    hilog.info(DOMAIN, 'testTag', '%{public}s', 'Ability onBackground');
  }
}
