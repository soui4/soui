#include <windows.h>
#include <mmsystem.h>
#include <process.h>
#include <stdio.h>
#include <tchar.h>
#include <souicoll.h>
#include <helper/slog.h>
#define kLogTag "SPlaySound"
#pragma comment(lib, "winmm.lib")

SNSBEGIN

// 播放状态结构体
struct PlayStatus {
    HANDLE hStop;    // 手动重置事件:置位=请求停止本次播放
    HANDLE hDone;    // 手动重置事件:置位=播放线程已退出
    HANDLE hThread;  // 播放线程句柄(由 AudioPlayer 持有,回收时 WaitForSingleObject + CloseHandle)
    DWORD  deviceId; // MCI设备ID(仅播放线程读写)
    TCHAR  fileName[MAX_PATH]; // 音频文件名

    PlayStatus() {
        hStop = NULL;
        hDone = NULL;
        hThread = NULL;
        deviceId = 0;
        fileName[0] = _T('\0');
    }
};

class AudioPlayer {
private:
    CRITICAL_SECTION m_cs;      // 保护 m_items
    SList<PlayStatus *> m_items; // 尚未回收的播放项

    AudioPlayer() {
        ::InitializeCriticalSection(&m_cs);
    }

    ~AudioPlayer() {
        stopAll();
        ::DeleteCriticalSection(&m_cs);
    }

    // 回收线程:bAll=TRUE 时请求全部停止并等待线程退出,否则只回收已退出的。
    // 等待放在临界区之外,避免与播放线程互相阻塞。
    void reapThreads(BOOL bAll) {
        SList<PlayStatus *> localItems;

        ::EnterCriticalSection(&m_cs);
        SPOSITION pos = m_items.GetHeadPosition();
        while (pos) {
            // 先记住当前节点,GetNext 会把 pos 推进到下一个节点
            SPOSITION posCur = pos;
            PlayStatus *p = m_items.GetNext(pos);
            if (bAll || ::WaitForSingleObject(p->hDone, 0) == WAIT_OBJECT_0) {
                // 先置停止事件再摘除,线程随后会自行关闭MCI设备并置位 hDone
                ::SetEvent(p->hStop);
                localItems.AddTail(p);
                m_items.RemoveAt(posCur);
            }
        }
        ::LeaveCriticalSection(&m_cs);

        // 等线程真正退出后才释放句柄与状态,避免线程使用已回收的对象
        SPOSITION posWait = localItems.GetHeadPosition();
        while (posWait) {
            PlayStatus *p = localItems.GetNext(posWait);
            ::WaitForSingleObject(p->hThread, INFINITE);
            ::CloseHandle(p->hThread);
            ::CloseHandle(p->hStop);
            ::CloseHandle(p->hDone);
            delete p;
        }
    }

    // 停止所有音频播放(并等待播放线程退出)
    void stopAll() {
        reapThreads(TRUE);
    }

    // 清理已结束的线程
    void cleanupThreads() {
        reapThreads(FALSE);
    }

    // 音频播放线程函数(_beginthreadex 要求 __stdcall,返回 unsigned)
    static unsigned __stdcall playThreadProc(void *pArg) {
        PlayStatus *status = (PlayStatus *)pArg;

        // 若已收到停止请求(极端顺序下可能发生),无需再打开设备
        if (::WaitForSingleObject(status->hStop, 0) == WAIT_OBJECT_0) {
            ::SetEvent(status->hDone);
            return 0;
        }

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
			//SLOGI() << "SPlaySound: Playing " << status->fileName << ", deviceId=" << status->deviceId << ", error=" << error;
            if (error == 0) {
                // 等待播放完成或停止事件:超时 10ms 轮询一次播放状态,避免 CPU 空转
                MCI_STATUS_PARMS mciStatusParms = { 0 };
                mciStatusParms.dwItem = MCI_STATUS_MODE;

                while (::WaitForSingleObject(status->hStop, 10) == WAIT_TIMEOUT) {
                    // 检查播放状态
                    mciSendCommand(status->deviceId, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&mciStatusParms);
                    if (mciStatusParms.dwReturn == MCI_MODE_STOP) {
                        break;
                    }
                }
            }

            // 关闭设备:唯一的关闭点,只由打开设备的线程执行
            mciSendCommand(status->deviceId, MCI_CLOSE, MCI_WAIT, 0);
			//SLOGI() << "SPlaySound: Closed deviceId=" << status->deviceId;
        }

        // 通知回收方:本次播放已结束
        ::SetEvent(status->hDone);
        return 0;
    }

public:
    BOOL play(LPCTSTR pszSound, BOOL bPurge) {
        // 如果是停止播放请求
        if (pszSound == NULL || pszSound[0] == '\0') {
            stopAll();
            return TRUE;
        }

        // 如果需要清除当前播放
        if (bPurge) {
            stopAll();
        } else {
            // 清理已结束的线程
            cleanupThreads();
        }

        // 创建播放状态与同步用的内核对象
        PlayStatus *status = new PlayStatus;
        status->hStop = ::CreateEvent(NULL, TRUE, FALSE, NULL);
        status->hDone = ::CreateEvent(NULL, TRUE, FALSE, NULL);
        if (status->hStop == NULL || status->hDone == NULL) {
            if (status->hStop) ::CloseHandle(status->hStop);
            if (status->hDone) ::CloseHandle(status->hDone);
            delete status;
            return FALSE;
        }
        _tcsncpy(status->fileName, pszSound, MAX_PATH - 1);

        // 先登记再启动线程(两步都在临界区内完成):这样 stopAll()/cleanupThreads()
        // 在任意时刻都一定能看到这次播放并把它回收掉,不会漏掉线程与设备。
        ::EnterCriticalSection(&m_cs);
        status->hThread = (HANDLE)_beginthreadex(NULL, 0, playThreadProc, status, 0, NULL);
        if (status->hThread != NULL) {
            m_items.AddTail(status);
        }
        ::LeaveCriticalSection(&m_cs);

        if (status->hThread == NULL) {
            ::CloseHandle(status->hStop);
            ::CloseHandle(status->hDone);
            delete status;
            return FALSE;
        }

        return TRUE;
    }

    static AudioPlayer *getInstance() {
        static AudioPlayer instance;
        return &instance;
    }
};

SNSEND

// 增强版PlaySound函数，按照Linux版本的线程模型实现
UTILITIES_API BOOL SPlaySound(LPCTSTR pszSound, HMODULE hmod, DWORD fdwSound) {
    // 检查是否是系统声音别名或资源
    if (pszSound != NULL && !(fdwSound & SND_FILENAME)) {
        // 回退到原始PlaySound处理系统声音或资源
        return PlaySound(pszSound, hmod, fdwSound);
    }
    // 调用AudioPlayer的play方法
    return SNS::AudioPlayer::getInstance()->play(pszSound, fdwSound & SND_PURGE);
}
