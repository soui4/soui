#ifndef _SGIFPLAYER_H_
#define _SGIFPLAYER_H_

#include <core/SWnd.h>
#include "SSkinAni.h"

SNSBEGIN

#ifndef EVT_GIF_BEGIN
#define EVT_GIF_BEGIN (EVT_EXTERNAL_BEGIN + 600)
#endif // EVT_GIF_BEGIN

enum
{
    EVT_GIF_PLAY_OVER = EVT_GIF_BEGIN,
    EVT_GIF_LOOP_OVER,
    ETV_GIF_PLAY_FRAME,
};

DEF_EVT_EXT(EventGifPlayOver, EVT_GIF_PLAY_OVER, {int dummy;})
DEF_EVT_EXT(EventGifLoopOver, EVT_GIF_LOOP_OVER, {int iLoop;int nLoop;})
DEF_EVT_EXT(EventGifPlayFrame, ETV_GIF_PLAY_FRAME, {int iFrame;})

/**
 * @class     SGifPlayer
 * @brief     GIF图片显示控件
 *
 * Describe
 */
class SGifPlayer
    : public SWindow
    , public ITimelineHandler {
    DEF_SOBJECT(SWindow, L"gifplayer") // 定义GIF控件在XM加的标签
  public:
    SGifPlayer();
    ~SGifPlayer();

    /**
     * PlayGifFile
     * @brief    在控件中播放一个GIF图片文件
     * @param    LPCTSTR pszFileName --  文件名
     * @return   BOOL -- true:成功
     * Describe
     */
    BOOL PlayGifFile(LPCTSTR pszFileName);

    /**
     * PlayAPNGFile
     * @brief    在控件中播放一个APNG图片文件
     * @param    LPCTSTR pszFileName --  文件名
     * @return   BOOL -- true:成功
     * Describe
     */
    BOOL PlayAPNGFile(LPCTSTR pszFileName);

    /**
     * LoadFromMemory
     * @brief    从内存加载APNG or GIF图片
     * @param    LPVOID pBits --  内存地址
     * @param    size_t szData --  内存数据长度
     * @return   BOOL -- true:成功
     * Describe
     */
    BOOL LoadFromMemory(LPVOID pBits, size_t szData);

    /**
     * SetAniSkin
     * @brief    设置SSkinAni对象
     * @param    SSkinAni *pSkin --  SSkinAni对象
     * @return   void
     * Describe
     */
    void SetAniSkin(SSkinAni *pSkin);

    /**
     * Start
     * @brief    开始播放
     * @param    int iFrame --  开始播放的帧数,-1:从当前帧开始播放
     * @return   void
     * Describe
     */
    void Start(int iFrame = -1);

    /**
     * Stop
     * @brief    停止播放
     * @return   void
     * Describe
     */
    void Stop();

    /**
     * SetLoop
     * @brief    设置循环播放的次数
     * @param    int nLoop --  循环播放的次数
     * @return   void
     * Describe
     */
    void SetLoop(int nLoop);

    int GetFrameCount() const;
    void SetCurFrame(int iFrame);
    int GetCurFrame() const;
  protected: // SWindow的虚函数
    SIZE MeasureContent(int nParentWid, int nParentHei) override;
    void OnContainerChanged(ISwndContainer *pOldContainer, ISwndContainer *pNewContainer) override;
    void OnScaleChanged(int scale) override;

  protected: // ITimerLineHander
    STDMETHOD_(void, OnNextFrame)(THIS_) OVERRIDE;
  public: // 属性处理
    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"skin", OnAttrSkin) // 为控件提供一个skin属性，用来接收SSkinObj对象的name
        ATTR_INT(L"scale", m_nScale, FALSE)
        ATTR_BOOL(L"enableScale", m_bEnableScale, TRUE)
        ATTR_BOOL(L"tile", m_bTile, TRUE)
        ATTR_INT(L"loopCount", m_nLoop, TRUE)
        ATTR_BOOL(L"autoPlay", m_bAutoPlay, TRUE)
    SOUI_ATTRS_END()
  protected:
    HRESULT OnAttrSkin(const SStringW &strValue, BOOL bLoading);

  protected:
    BOOL _PlayFile(LPCTSTR pszFileName);
  protected:
    void OnPaint(IRenderTarget *pRT);
    void OnShowWindow(BOOL bShow, UINT nStatus);
    void OnDestroy();

    // SOUI控件消息映射表
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)      
        MSG_WM_SHOWWINDOW(OnShowWindow)
        MSG_WM_DESTROY(OnDestroy)
    SOUI_MSG_MAP_END()

  protected:
    SAutoRefPtr<SSkinAni> m_aniSkin;
    int m_iCurFrame;
    int m_nNextInterval;
    int m_nScale;
    BOOL m_bEnableScale;
    BOOL m_bTile;
    int  m_nLoop;
    int m_iLoop;
    BOOL m_bAutoPlay;
    BOOL m_bPlaying;
};

SNSEND

#endif //_SGIFPLAYER_H_