#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <tchar.h>
#include <mutex>
#include <map>
#include <thread>
#include <memory>
#include <atomic>
#include <chrono>

#pragma comment(lib, "winmm.lib")

// 播放状态结构体
struct PlayStatus {
    std::atomic<bool> bStop;
    std::atomic<bool> bDone;
    UINT deviceId; // MCI设备ID
    TCHAR fileName[MAX_PATH]; // 音频文件名
};

class AudioPlayer {
private:
    std::mutex m_mutex;
    // 使用map存储线程ID到播放状态的映射
    std::map<std::thread::id, std::shared_ptr<PlayStatus>> m_threads;
    
    AudioPlayer() {}
    
    ~AudioPlayer() {
        stopAll();
    }
    
    // 停止所有音频播放
    void stopAll() {
        // 将线程列表移动到本地，避免持有互斥锁时等待线程
        std::map<std::thread::id, std::shared_ptr<PlayStatus>> localThreads;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (auto &pair : m_threads) {
                pair.second->bStop.store(true);
            }
            // 转移所有权
            localThreads.swap(m_threads);
        }
        
        // 关闭所有MCI设备
        for (auto &pair : localThreads) {
            if (pair.second->deviceId != 0) {
                mciSendCommand(pair.second->deviceId, MCI_CLOSE, 0, 0);
            }
        }
    }
    
    // 清理已完成的线程
    void cleanupThreads() {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto it = m_threads.begin(); it != m_threads.end();) {
            if (it->second->bDone.load()) {
                // 线程已完成，从map中移除
                it = m_threads.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    // 音频播放线程函数
    static void playThreadFunc(std::shared_ptr<PlayStatus> status) {
        // 打开MCI设备
        MCI_OPEN_PARMS mciOpenParms = { 0 };
        mciOpenParms.lpstrDeviceType = _T("waveaudio");
        mciOpenParms.lpstrElementName = status->fileName;
        
        MCIERROR error = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD_PTR)&mciOpenParms);
        
        if (error == 0) {
            status->deviceId = mciOpenParms.wDeviceID;
            
            // 播放音频
            MCI_PLAY_PARMS mciPlayParms = { 0 };
            error = mciSendCommand(status->deviceId, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)&mciPlayParms);
            
            if (error == 0) {
                // 等待播放完成或停止信号
                MCI_STATUS_PARMS mciStatusParms = { 0 };
                mciStatusParms.dwItem = MCI_STATUS_MODE;
                
                while (!status->bStop.load()) {
                    // 检查播放状态
                    mciSendCommand(status->deviceId, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&mciStatusParms);
                    if (mciStatusParms.dwReturn == MCI_MODE_STOP) {
                        break;
                    }
                    // 短暂睡眠，避免CPU占用过高
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
            
            // 关闭设备
            mciSendCommand(status->deviceId, MCI_CLOSE, 0, 0);
        }
        
        // 设置播放完成标志
        status->bDone.store(true);
    }
    
public:
    BOOL play(LPCSTR pszSound, BOOL bPurge) {
        // 如果是停止播放请求
        if (pszSound == NULL || pszSound[0] == '\0') {
            stopAll();
            return TRUE;
        }
        
        // 如果需要清除当前播放
        if (bPurge) {
            stopAll();
        }
        
        // 清理已完成的线程
        cleanupThreads();
        
        // 创建播放状态
        auto status = std::make_shared<PlayStatus>();
        status->bStop.store(false);
        status->bDone.store(false);
        status->deviceId = 0;
        
        // 转换文件名到TCHAR
        #ifdef _UNICODE
        MultiByteToWideChar(CP_ACP, 0, pszSound, -1, status->fileName, MAX_PATH);
        #else
        strcpy(status->fileName, pszSound);
        #endif
        
        // 创建播放线程
        std::thread playThread(playThreadFunc, status);
        std::thread::id threadId = playThread.get_id();
        
        // 分离线程，使其自动清理
        playThread.detach();
        
        // 将线程添加到map中
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_threads[threadId] = status;
        }
        
        return TRUE;
    }
    
    static AudioPlayer *getInstance() {
        static AudioPlayer instance;
        return &instance;
    }
};

// 增强版PlaySound函数，按照Linux版本的线程模型实现
BOOL WINAPI Win32PlaySound(LPCTSTR pszSound, HMODULE hmod, DWORD fdwSound) {
    // 检查是否是系统声音别名或资源
    if (pszSound != NULL && !(fdwSound & SND_FILENAME)) {
        // 回退到原始PlaySound处理系统声音或资源
        return PlaySound(pszSound, hmod, fdwSound);
    }
    
    // 转换LPCTSTR到LPCSTR
    LPCSTR soundStr = NULL;
    char soundBuf[MAX_PATH];
    
    if (pszSound != NULL) {
        #ifdef _UNICODE
        WideCharToMultiByte(CP_ACP, 0, pszSound, -1, soundBuf, MAX_PATH, NULL, NULL);
        soundStr = soundBuf;
        #else
        soundStr = pszSound;
        #endif
    }
    
    // 调用AudioPlayer的play方法
    return AudioPlayer::getInstance()->play(soundStr, fdwSound & SND_PURGE);
}
