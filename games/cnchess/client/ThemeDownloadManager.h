// ThemeDownloadManager.h: 客户端主题资源下载管理器
// 负责从服务器下载主题zip、MD5校验、解压到本地缓存目录
//
//////////////////////////////////////////////////////////////////////

#ifndef __THEMEDOWNLOADMANAGER_H__
#define __THEMEDOWNLOADMANAGER_H__

#include <helper/obj-ref-impl.hpp>
#include <string/tstring.h>
#include <string/strcpcvt.h>
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
 * 5. 解压到本地缓存目录
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
         * @brief 下载进度更新
         * @param nPercent 0-100
         */
        virtual void OnThemeProgress(int nPercent) {}
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
    bool ExtractZip(const SStringT& strZipPath, const SStringT& strDestDir);
    bool WriteMD5File(const unsigned char md5[16]);
    void NotifyError(const SStringA& strErr);
    void NotifyReady(bool bUpdated);

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
};

#endif // __THEMEDOWNLOADMANAGER_H__
