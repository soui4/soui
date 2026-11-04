#pragma once

// 跨平台的临界区：VS2008 没有 <mutex>/std::mutex/std::lock_guard，
// 这里用 pimpl 包一层（Windows 下是 CRITICAL_SECTION，其它平台是 pthread_mutex_t），
// 好处是不用把 windows.h / pthread.h 带进这个头文件，避免和 min/max 宏打架。
class CLogLock {
  public:
    CLogLock();
    ~CLogLock();

    void Enter();
    void Leave();

  private:
    CLogLock(const CLogLock &);             // 不允许拷贝
    CLogLock &operator=(const CLogLock &);  // 不允许赋值

    struct Impl;
    Impl *m_impl;
};

// RAII 加锁（等价于 std::lock_guard）
class CLogAutoLock {
  public:
    explicit CLogAutoLock(CLogLock &lock) : m_lock(lock) { m_lock.Enter(); }
    ~CLogAutoLock() { m_lock.Leave(); }

  private:
    CLogAutoLock(const CLogAutoLock &);
    CLogAutoLock &operator=(const CLogAutoLock &);

    CLogLock &m_lock;
};

// 界面「请求日志」的数据来源。
//
// 旧做法是去读 log4z 落盘的 log/*.log：要在 UI 线程做文件 IO（文件一大就卡界面），
// 而且只能看到已经 flush 到磁盘的那部分。现在改为挂 SLog 的回调：
//   SAppCfg::SetLog(TRUE) 会把 SLog 回调接到 log4z 的 SouiLog_Callback，
//   Install() 在此基础上再串一层 —— 先把日志塞进内存环形队列给界面用，
//   再原样转交给之前的回调，log4z 的落盘行为完全不变。
//
// 回调可能来自任意工作线程，因此内部用互斥量保护，界面侧只做「按序号取增量」，
// 全程不涉及文件，也不会阻塞 UI。
class CLogSink {
  public:
    // 进程内单例
    static CLogSink &Instance();

    // 安装（并链接原有）日志回调。必须在 SAppCfg::DoConfig 之后调用，
    // 那时 gs_logCallback 已经是 log4z 的回调。
    void Install();

    // 卸载并恢复原回调（进程退出前调用，避免回调继续指向已销毁的对象）
    void Uninstall();

    // 累计产生过的日志条数（单调递增）。界面用它判断是否有新日志。
    fund_uint64 Count() const;

    // 取 [from, from+maxN) 区间的日志行；from 过旧时自动裁剪到仍保留的最早一行。
    // 返回的是宽字符串，已按 UTF-8 / 本地代码页自动解码。
    std::vector<std::wstring> Snapshot(fund_uint64 from, size_t maxN) const;

    // 清空内存队列（不影响 log4z 已落盘的文件）
    void Clear();

  private:
    CLogSink();
    ~CLogSink();

    static void OnLog(const char *tag, const char *logStr, int level, const char *file, int line,
                      const char *fun, void *retAddr);

    void AppendLine(const std::wstring &line);

    enum { kMaxLines = 4000 };  // 内存里最多保留的行数

    mutable CLogLock m_lock;
    std::deque<std::wstring> m_lines;  // 只保留最近 kMaxLines 行
    fund_uint64 m_total;               // 累计条数（单调递增，不因裁剪而回退）
    SNS::LogCallback m_prev;           // 安装前的回调（log4z），用于链式转交
};
