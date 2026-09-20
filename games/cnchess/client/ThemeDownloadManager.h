// ThemeDownloadManager.h: 客户端主题资源下载管理器
// 负责从服务器下载主题zip、MD5校验、解压到本地缓存目录
//
//////////////////////////////////////////////////////////////////////

#ifndef __THEMEDOWNLOADMANAGER_H__
#define __THEMEDOWNLOADMANAGER_H__

#include <helper/obj-ref-impl.hpp>
#include <string/tstring.h>
#include <string/strcpcvt.h>
#include <interface/SMsgLoop-i.h>
#include <interface/STaskLoop-i.h>
#include <atomic>
#include <vector>
#include <list>
#include <protocol.h>

class WebSocketClient;

/**
 * @brief 主题下载管理器
 *
 * 管理从服务器下载主题资源包的完整流程：
 * 1. 计算本地缓存主题zip的MD5
 * 2. 通过WebSocket向服务器发送下载请求
 * 3. 接收服务器分块数据并重组
 * 4. MD5校验下载结果
 * 5. 在工作线程（ITaskLoop）中解压到本地缓存目录，避免阻塞UI线程
 */
class ThemeDownloadManager
{
public:
    /**
     * @brief 下载监听器接口
     */
    struct IListener
    {
        /**
         * @brief 主题已就绪（可能是新下载的，也可能是本地缓存匹配的）
         * @param strThemeDir 主题目录路径
         * @param bUpdated 是否为本次新下载的
         */
        virtual void OnThemeReady(const SStringT& strThemeDir, bool bUpdated) = 0;
        /**
         * @brief 下载进度更新（在UI线程回调）
         * @param nPercent 0-100
         */
        virtual void OnThemeProgress(int nPercent) {}
        /**
         * @brief 解压进度更新（在UI线程回调）
         * @param nPercent 0-100
         */
        virtual void OnThemeExtractProgress(int nPercent) {}
        /**
         * @brief 下载失败
         * @param strErr 错误描述
         */
        virtual void OnThemeError(const SStringA& strErr) = 0;
    };

    enum State
    {
        STATE_IDLE = 0,       ///< 空闲
        STATE_WAITING_ACK,    ///< 已发送请求，等待应答
        STATE_DOWNLOADING,    ///< 正在接收数据
        STATE_EXTRACTING,     ///< 正在解压
        STATE_DONE,           ///< 完成
        STATE_ERROR,          ///< 错误
    };

public:
    ThemeDownloadManager();
    ~ThemeDownloadManager();

    /**
     * @brief 初始化缓存路径
     * @param strCacheDir 缓存根目录（zip和md5存放位置）
     * @return 是否成功
     */
    bool Init(const SStringT& strCacheDir);

    /**
     * @brief 获取主题解压目录路径
     */
    SStringT GetThemeDir() const { return m_strThemeDir; }

    /**
     * @brief 获取本地缓存zip的MD5
     * @param md5 输出MD5（16字节）
     * @return 是否存在本地缓存
     */
    bool GetLocalMD5(unsigned char md5[16]) const;

    /**
     * @brief 检查主题是否已就绪（解压目录存在且包含config.xml）
     */
    bool IsThemeReady() const;

    /**
     * @brief 发起主题下载请求
     * @param pWs WebSocket客户端
     * @return 是否成功发送请求
     */
    bool RequestTheme(WebSocketClient* pWs);

    /**
     * @brief 异步解压缓存的zip（m_strZipPath）到主题目录（m_strThemeDir）
     *
     * 解压在工作线程（ITaskLoop）中执行，期间通过 IListener::OnThemeExtractProgress 上报进度，
     * 结束后通过 IListener::OnThemeReady 或 OnThemeError 在UI线程回调，避免阻塞UI线程。
     *
     * @param bUpdated 是否为本次新下载的（透传给 OnThemeReady）
     * @return 是否成功提交解压任务
     */
    bool ExtractZipAsync(bool bUpdated);

    /**
     * @brief 处理WebSocket消息（由上层转发主题相关消息）
     * @param dwType 消息类型
     * @param pData 消息数据
     * @param dwSize 数据大小
     * @return 是否由本管理器处理
     */
    bool HandleMessage(DWORD dwType, const BYTE* pData, DWORD dwSize);

    /**
     * @brief 设置监听器
     */
    void SetListener(IListener* pListener) { m_pListener = pListener; }

    /**
     * @brief 设置UI线程的消息循环
     *
     * 解压工作线程需要通过它把进度与结果回调切回UI线程。未设置时会尝试
     * 从 SApplication 按当前线程查找，建议由窗口在初始化时显式设置。
     *
     * @param pMsgLoop UI线程消息循环（弱引用，生命周期由调用方保证）
     */
    void SetMsgLoop(IMessageLoop* pMsgLoop) { m_pMsgLoop = pMsgLoop; }

    /**
     * @brief 获取当前状态
     */
    State GetState() const { return m_state; }

    /**
     * @brief 重置状态到空闲（取消下载）
     */
    void Reset();

private:
    void OnThemeAck(const BYTE* pData, DWORD dwSize);
    void OnThemeData(const BYTE* pData, DWORD dwSize);
    bool SaveDownloadedZip();
    bool WriteMD5File(const unsigned char md5[16]);
    void NotifyError(const SStringA& strErr);
    void NotifyReady(bool bUpdated);

    /**
     * @brief 工作线程中执行的解压实现
     * @param bUpdated 是否为本次新下载的
     * @return 解压是否成功（结果会投递回UI线程回调监听器）
     */
    bool DoExtractZip(bool bUpdated);

    /**
     * @brief 惰性创建并启动解压工作线程（只创建一次，随本对象析构而停止）
     * @return 工作线程是否可用
     */
    bool EnsureExtractLoop();

    /**
     * @brief EnumFile 回调：统计待解压内容（第一遍）或提取单个文件（第二遍）
     * @param pszFileName zip内文件名
     * @param lp ExtractContext* 上下文
     * @return 是否继续枚举
     */
    static BOOL CALLBACK EnumZipFileCallback(LPCTSTR pszFileName, LPARAM lp);

    /**
     * @brief 上报解压进度（在工作线程调用，内部切换到UI线程通知监听器）
     * @param nPercent 0-100
     */
    void NotifyExtractProgress(int nPercent);

    /**
     * @brief 解压进度回调（在UI线程执行）
     */
    void OnExtractProgress(int nPercent);

    /**
     * @brief 解压结束回调（在UI线程执行）
     * @param bSuccess 解压是否成功
     * @param bUpdated 是否为本次新下载的
     */
    void OnExtractFinished(bool bSuccess, bool bUpdated);

    /**
     * @brief 把解压结果投递到UI线程（在工作线程调用）
     */
    void PostExtractFinished(bool bSuccess, bool bUpdated);

private:
    struct Chunk {
        DWORD dwOffset;            ///< 分块在完整zip中的偏移
        std::vector<BYTE> data;    ///< 分块数据
        Chunk(DWORD offset, size_t size) : dwOffset(offset), data(size) {}
    };

    IListener* m_pListener;          ///< 监听器
    State m_state;                    ///< 当前状态

    SStringT m_strCacheDir;           ///< 缓存根目录
    SStringT m_strZipPath;            ///< 下载的zip文件路径
    SStringT m_strMD5Path;            ///< MD5文件路径
    SStringT m_strThemeDir;           ///< 解压后的主题目录

    // 下载状态
    unsigned char m_serverMD5[16];    ///< 服务器返回的MD5
    DWORD m_dwTotalSize;             ///< 服务器返回的zip总大小
    DWORD m_dwReceivedBytes;         ///< 已接收字节数
    std::list<Chunk> m_zipChunks;     ///< 分块接收缓冲区（按偏移分段，避免一次性分配完整内存）

    WebSocketClient* m_pWsClient;     ///< WebSocket客户端（弱引用）

    // 解压状态（工作线程）
    SAutoRefPtr<ITaskLoop> m_pExtractLoop;   ///< 解压工作线程（惰性创建，析构时停止）
    IMessageLoop* m_pMsgLoop;                ///< UI线程消息循环（弱引用，用于工作线程回调切回UI线程）
    std::atomic<bool> m_bExtractAbort;       ///< 解压中止标志（对象析构或Reset时置位）
    std::atomic<int> m_nExtractPercent;      ///< 最近一次上报的解压进度，避免重复投递
};

#endif // __THEMEDOWNLOADMANAGER_H__
