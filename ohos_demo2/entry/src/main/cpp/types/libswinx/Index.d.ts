declare namespace swinx {
  function registerXComponent(xcomponent: object): boolean;
  function showDemo(): boolean;
  function setMainWindowMoveCallback(callback: (dx: number, dy: number) => void): boolean;
  function setMainWindowResizeCallback(callback: (x: number, y: number, width: number, height: number) => void): boolean;
  function setCursorCallback(callback: (cursorId: number) => void): boolean;
}

export default swinx;
