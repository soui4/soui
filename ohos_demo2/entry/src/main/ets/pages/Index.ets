import souidemo from 'libsouidemo.so';
import swinx from 'libswinx.so';
import { window } from '@kit.ArkUI';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { inputMethod } from '@kit.IMEKit';
import pointer from '@ohos.multimodalInput.pointer';

const LOG_DOMAIN = 0x5350;
const LOG_TAG = 'SOUI_ARK';

function registerMainWindowMoveHandler(): void {
  try {
    const mainWindow = AppStorage.get<window.Window>('souiMainWindow');
    if (mainWindow === undefined) {
      return;
    }
    swinx.setMainWindowMoveCallback((dx: number, dy: number): void => {
      try {
        const rect = mainWindow.getWindowProperties().windowRect;
        mainWindow.moveWindowTo(rect.left + dx, rect.top + dy);
      } catch (_) {
      }
    });
    swinx.setMainWindowResizeCallback((x: number, y: number, width: number, height: number): void => {
      try {
        mainWindow.moveWindowTo(x, y);
        mainWindow.resize(width, height);
      } catch (_) {
      }
    });
  } catch (_) {
  }
}

@Entry
@Component
struct Index {
  @State private resourcesReady: boolean = false;
  @State private startupError: string = '';
  @State private imeProxyText: string = '';
  private imeActive: boolean = false;

  private updateCursor(cursorId: number): void {
    try {
      const cursorController = this.getUIContext().getCursorController();
      switch (cursorId) {
        case 32513:
          cursorController.setCursor(pointer.PointerStyle.TEXT_CURSOR);
          break;
        case 32515:
          cursorController.setCursor(pointer.PointerStyle.CROSS);
          break;
        case 32640:
        case 32646:
          cursorController.setCursor(pointer.PointerStyle.MOVE);
          break;
        case 32642:
          cursorController.setCursor(pointer.PointerStyle.NORTH_WEST_SOUTH_EAST);
          break;
        case 32643:
          cursorController.setCursor(pointer.PointerStyle.NORTH_EAST_SOUTH_WEST);
          break;
        case 32644:
          cursorController.setCursor(pointer.PointerStyle.RESIZE_LEFT_RIGHT);
          break;
        case 32645:
          cursorController.setCursor(pointer.PointerStyle.RESIZE_UP_DOWN);
          break;
        case 32649:
          cursorController.setCursor(pointer.PointerStyle.HAND_POINTING);
          break;
        default:
          cursorController.restoreDefault();
          break;
      }
    } catch (error) {
      hilog.error(LOG_DOMAIN, LOG_TAG, 'set cursor failed: %{public}s', JSON.stringify(error));
    }
  }

  private finishStartup(renderMode: number): void {
    const modeOk = souidemo.setRenderMode(renderMode);
    if (!modeOk) {
      this.resourcesReady = false;
      this.startupError = 'SOUI native render init failed';
      return;
    }

    const ok = souidemo.setResourceRoot();
    this.resourcesReady = ok;
    this.startupError = ok ? '' : 'SOUI native resource init failed';
    if (ok) {
      registerMainWindowMoveHandler();
      swinx.setCursorCallback((cursorId: number): void => {
        this.updateCursor(cursorId);
      });
      const imeCallbackOk = souidemo.setImeRequestCallback((show: boolean): void => {
        hilog.info(LOG_DOMAIN, LOG_TAG, 'native requested IME show=%{public}s', JSON.stringify(show));
        if (show) {
          this.requestImeFocus();
        } else {
          this.hideIme();
        }
      });
      hilog.info(LOG_DOMAIN, LOG_TAG, 'set IME callback result=%{public}s', JSON.stringify(imeCallbackOk));
    }
  }

  private selectRenderMode(): void {
    AlertDialog.show({
      title: 'select a render',
      message: '选择渲染类型',
      primaryButton: {
        value: 'Skia',
        action: (): void => {
          this.finishStartup(1);
        }
      },
      secondaryButton: {
        value: 'GDI',
        action: (): void => {
          this.finishStartup(0);
        }
      },
      cancel: (): void => {
        this.startupError = 'Render selection cancelled';
      }
    });
  }

  private setImeProxyActive(active: boolean): void {
    const ok = souidemo.setImeProxyActive(active);
    hilog.info(LOG_DOMAIN, LOG_TAG, 'set IME proxy active=%{public}s result=%{public}s',
      JSON.stringify(active), JSON.stringify(ok));
  }

  private releaseImeProxySoon(): void {
    setTimeout((): void => {
      this.setImeProxyActive(false);
    }, 80);
  }

  private requestImeFocus(): void {
    try {
      this.setImeProxyActive(true);
      this.imeActive = true;
      const focused = focusControl.requestFocus('soui-ime-proxy');
      hilog.info(LOG_DOMAIN, LOG_TAG, 'request IME focus result=%{public}s', JSON.stringify(focused));
      setTimeout((): void => {
        inputMethod.getController()
          .showTextInput(inputMethod.RequestKeyboardReason.MOUSE)
          .then((): void => {
            hilog.info(LOG_DOMAIN, LOG_TAG, 'showTextInput ok');
          })
          .catch((error: Error): void => {
            hilog.error(LOG_DOMAIN, LOG_TAG, 'showTextInput failed: %{public}s', JSON.stringify(error));
            this.releaseImeProxySoon();
          });
      }, 40);
    } catch (error) {
      hilog.error(LOG_DOMAIN, LOG_TAG, 'request IME focus failed: %{public}s', JSON.stringify(error));
      this.releaseImeProxySoon();
    }
  }

  private hideIme(): void {
    this.imeActive = false;
    this.imeProxyText = '';
    this.setImeProxyActive(true);
    try {
      this.getUIContext().getFocusController().clearFocus();
    } catch (error) {
      hilog.error(LOG_DOMAIN, LOG_TAG, 'clear proxy focus failed: %{public}s', JSON.stringify(error));
    }
    inputMethod.getController()
      .hideTextInput()
      .then((): void => {
        hilog.info(LOG_DOMAIN, LOG_TAG, 'hideTextInput ok');
        this.releaseImeProxySoon();
      })
      .catch((error: Error): void => {
        hilog.error(LOG_DOMAIN, LOG_TAG, 'hideTextInput failed: %{public}s', JSON.stringify(error));
        this.releaseImeProxySoon();
      });
  }

  private dispatchImeText(text: string, source: string): void {
    if (text.length === 0) {
      return;
    }
    hilog.info(LOG_DOMAIN, LOG_TAG, 'dispatch IME text source=%{public}s length=%{public}d', source, text.length);
    souidemo.dispatchTextInput(text);
    this.imeProxyText = '';
  }

  private handleImeChange(value: string): void {
    hilog.info(LOG_DOMAIN, LOG_TAG, 'proxy onChange length=%{public}d active=%{public}s', value.length, JSON.stringify(this.imeActive));
    if (!this.imeActive) {
      this.imeProxyText = '';
      return;
    }
    this.dispatchImeText(value, 'change');
  }

  private handleImeDelete(): void {
    if (!this.imeActive) {
      return;
    }
    hilog.info(LOG_DOMAIN, LOG_TAG, 'dispatch IME backspace');
    souidemo.dispatchKeyEvent(2055, 0, 0, '');
    souidemo.dispatchKeyEvent(2055, 1, 0, '');
  }

  private getKeyboardModifierMask(event: KeyEvent): number {
    let mask = 0;
    try {
      if (event.getModifierKeyState !== undefined) {
        if (event.getModifierKeyState(['ctrl'])) {
          mask |= 1;
        }
        if (event.getModifierKeyState(['alt'])) {
          mask |= 2;
        }
        if (event.getModifierKeyState(['shift'])) {
          mask |= 4;
        }
      }
    } catch (_) {
    }
    return mask;
  }

  private dispatchKeyEvent(event: KeyEvent): boolean {
    const unicode = event.unicode === undefined ? 0 : event.unicode;
    return souidemo.dispatchKeyEvent(event.keyCode, event.type, unicode, event.keyText,
      this.getKeyboardModifierMask(event));
  }

  aboutToAppear(): void {
    try {
      this.selectRenderMode();
    } catch (error) {
      const err = error as Error;
      this.resourcesReady = false;
      this.startupError = err.message;
    }
  }

  build() {
    Column() {
      if (this.resourcesReady) {
        Stack({ alignContent: Alignment.TopStart }) {
          XComponent({
            id: 'soui-xcomponent',
            type: XComponentType.SURFACE,
            libraryname: 'libsouidemo'
          })
            .onKeyEvent((event: KeyEvent): boolean => {
              return this.dispatchKeyEvent(event);
            })
            .width('100%')
            .height('100%')

          TextInput({ text: this.imeProxyText })
            .id('soui-ime-proxy')
            .type(InputType.Normal)
            .enterKeyType(EnterKeyType.NEW_LINE)
            .enableKeyboardOnFocus(true)
            .enablePreviewText(false)
            .caretColor(Color.Transparent)
            .fontColor(Color.Transparent)
            .backgroundColor(Color.Transparent)
            .borderWidth(0)
            .padding(0)
            .width(240)
            .height(48)
            .opacity(0.02)
            .position({ x: 0, y: 0 })
            .zIndex(1)
            .hitTestBehavior(HitTestMode.Transparent)
            .focusable(true)
            .onFocus((): void => {
              this.imeActive = true;
              this.setImeProxyActive(true);
              hilog.info(LOG_DOMAIN, LOG_TAG, 'proxy focused');
            })
            .onBlur((): void => {
              hilog.info(LOG_DOMAIN, LOG_TAG, 'proxy blurred');
            })
            .onEditChange((isEditing: boolean): void => {
              if (isEditing) {
                this.imeActive = true;
              }
              hilog.info(LOG_DOMAIN, LOG_TAG, 'proxy edit change=%{public}s', JSON.stringify(isEditing));
            })
            .onChange((value: string): void => {
              this.handleImeChange(value);
            })
            .onKeyEvent((event: KeyEvent): boolean => {
              const unicode = event.unicode === undefined ? 0 : event.unicode;
              const modifierMask = this.getKeyboardModifierMask(event);
              if (unicode >= 0x20 && unicode !== 0x7f && modifierMask === 0) {
                return false;
              }
              return souidemo.dispatchKeyEvent(event.keyCode, event.type, unicode, event.keyText, modifierMask);
            })
            .onWillAttachIME((): void => {
              hilog.info(LOG_DOMAIN, LOG_TAG, 'proxy will attach IME');
            })
            .onWillInsert((value: InsertValue): boolean => {
              hilog.info(LOG_DOMAIN, LOG_TAG, 'proxy will insert length=%{public}d', value.insertValue.length);
              this.dispatchImeText(value.insertValue, 'willInsert');
              return false;
            })
            .onDidInsert((value: InsertValue): void => {
              hilog.info(LOG_DOMAIN, LOG_TAG, 'proxy did insert length=%{public}d', value.insertValue.length);
            })
            .onWillDelete((value: DeleteValue): boolean => {
              hilog.info(LOG_DOMAIN, LOG_TAG, 'proxy will delete length=%{public}d direction=%{public}d offset=%{public}d',
                value.deleteValue.length, value.direction, value.deleteOffset);
              if (this.imeActive) {
                this.handleImeDelete();
              }
              return false;
            })
            .onDidDelete((): void => {
              hilog.info(LOG_DOMAIN, LOG_TAG, 'proxy did delete');
            })
            .onSubmit((): void => {
              souidemo.dispatchKeyEvent(2054, 0, 0, '');
              souidemo.dispatchKeyEvent(2054, 1, 0, '');
              this.imeProxyText = '';
              hilog.info(LOG_DOMAIN, LOG_TAG, 'proxy submit');
            })
        }
          .width('100%')
          .height('100%')
      } else {
        Column({ space: 12 }) {
          if (this.startupError.length > 0) {
            Text(this.startupError)
              .fontSize(16)
              .fontColor('#334155')
          }
          Button('Skia')
            .width(160)
            .onClick((): void => {
              this.finishStartup(1);
            })
          Button('GDI')
            .width(160)
            .onClick((): void => {
              this.finishStartup(0);
            })
        }
      }
    }
    .width('100%')
    .height('100%')
    .justifyContent(FlexAlign.Center)
    .backgroundColor('#F5F7FA')
  }
}
