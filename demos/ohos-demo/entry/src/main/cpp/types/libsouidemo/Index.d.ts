declare namespace souidemo {
  /**
   * 初始化 demo 业务层（触发 Soui4OhosEntry 注册）。
   * 调用后 libsoui4ohos.so 的 soui4ohos 模块即可正常使用。
   * @returns 是否成功加载（true 表示静态构造已执行，Soui4OhosEntry 已注册）
   */
  function init(): boolean;
}

export default souidemo;
