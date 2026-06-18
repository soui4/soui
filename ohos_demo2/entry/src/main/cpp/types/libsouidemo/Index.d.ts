declare namespace souidemo {
  function setResourceRoot(resourceDir?: string): boolean;
  function setRenderMode(mode: number): boolean;
  function setImeRequestCallback(callback: (show: boolean) => void): boolean;
  function setImeProxyActive(active: boolean): boolean;
  function dispatchKeyEvent(keyCode: number, keyType: number, unicode: number, keyText: string, modifierMask?: number): boolean;
  function dispatchTextInput(text: string): boolean;
}

export default souidemo;
