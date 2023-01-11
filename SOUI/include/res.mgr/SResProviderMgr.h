#pragma once

#include <interface/sresprovidermgr-i.h>
#include <atl.mini/scomcli.h>
#include <helper/SCriticalSection.h>
#include <layout/SLayoutSize.h>

SNSBEGIN

class SOUI_EXP SResProviderMgr : public IResProviderMgr {

  public:
    SResProviderMgr(void);
    ~SResProviderMgr(void);

  public:
    STDMETHOD_(void, AddResProvider)
    (THIS_ IResProvider *pResProvider, LPCTSTR pszUidef DEF_VAL(_T("uidef:xml_init"))) OVERRIDE;

	STDMETHOD_(void, AddResProviderA)
		(THIS_ IResProvider *pResProvider, LPCSTR pszUidef DEF_VAL("uidef:xml_init")) OVERRIDE;

    STDMETHOD_(void, RemoveResProvider)(THIS_ IResProvider *pResProvider) OVERRIDE;

    STDMETHOD_(void, RemoveAll)(THIS) OVERRIDE;

    STDMETHOD_(IResProvider *, GetHeadResProvider)(THIS) SCONST OVERRIDE;
    STDMETHOD_(IResProvider *, GetTailResProvider)(THIS) SCONST OVERRIDE;

    STDMETHOD_(void, SetFilePrefix)(THIS_ LPCTSTR pszFilePrefix) OVERRIDE;

    //////////////////////////////////////////////////////////////////////////
    STDMETHOD_(BOOL, HasResource)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) OVERRIDE;

    STDMETHOD_(HICON, LoadIcon)
    (THIS_ LPCTSTR pszResName, int cx = 0, int cy = 0, BOOL bFromFile = FALSE) OVERRIDE;

    STDMETHOD_(HCURSOR, LoadCursor)(THIS_ LPCTSTR pszResName, BOOL bFromFile = FALSE) OVERRIDE;

    STDMETHOD_(HBITMAP, LoadBitmap)(THIS_ LPCTSTR pszResName, BOOL bFromFile = FALSE) OVERRIDE;

    STDMETHOD_(IBitmapS *, LoadImage)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) OVERRIDE;

    STDMETHOD_(IImgX *, LoadImgX)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) OVERRIDE;

    STDMETHOD_(size_t, GetRawBufferSize)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) OVERRIDE;

    STDMETHOD_(BOOL, GetRawBuffer)
    (THIS_ LPCTSTR pszType, LPCTSTR pszResName, LPVOID pBuf, size_t size) OVERRIDE;

  public: // helper
          // find the match resprovider from tail to head, which contains the specified resource
          // type and name
    IResProvider *GetMatchResProvider(LPCTSTR pszType, LPCTSTR pszResName);

    //使用type:name形式的字符串加载图片
    IBitmapS *LoadImage2(const SStringW &strImgID);

    //使用name:size形式的字符串加载图标，如果没有size,则默认系统图标SIZE
    HICON LoadIcon2(const SStringW &strIconID);

  protected:
#ifdef _DEBUG
    static BOOL CALLBACK CheckUsage(LPCTSTR pszName, LPCTSTR pszType, LPARAM lp);
#endif
    LPCTSTR SysCursorName2ID(LPCTSTR pszCursorName);

    //检查资源类型是否为外部文件
    //使用file:xxx 的形式来引用外部文件资源
    BOOL IsFileType(LPCTSTR pszType);

    SStringT m_strFilePrefix;
    SList<IResProvider *> m_lstResPackage;

    typedef SMap<SStringT, HCURSOR> CURSORMAP;
    CURSORMAP m_mapCachedCursor;

    SCriticalSection m_cs;

#ifdef _DEBUG
    //资源使用计数
    SMap<SStringT, int> m_mapResUsageCount;
#endif
};

SNSEND