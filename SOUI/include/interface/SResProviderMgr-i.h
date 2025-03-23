#ifndef __SRESPROVIDERMGR_I__H__
#define __SRESPROVIDERMGR_I__H__
#include <interface/SResProvider-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IResProviderMgr
DECLARE_INTERFACE(IResProviderMgr)
{
    /**
     * @brief 添加资源提供者
     * @param pResProvider IResProvider* -- 资源提供者对象
     * @param pszUidef LPCTSTR -- UI定义，默认值为 "uidef:xml_init"
     * @return void
     */
    STDMETHOD_(void, AddResProvider)(THIS_ IResProvider * pResProvider, LPCTSTR pszUidef DEF_VAL(_T("uidef:xml_init"))) PURE;

    /**
     * @brief 添加资源提供者
     * @param pResProvider IResProvider* -- 资源提供者对象
     * @param pszUidef LPCSTR -- UI定义，默认值为 "uidef:xml_init"
     * @return void
     */
    STDMETHOD_(void, AddResProviderA)(THIS_ IResProvider * pResProvider, LPCSTR pszUidef DEF_VAL("uidef:xml_init")) PURE;

    /**
     * @brief 移除资源提供者
     * @param pResProvider IResProvider* -- 资源提供者对象
     * @return void
     */
    STDMETHOD_(void, RemoveResProvider)(THIS_ IResProvider * pResProvider) PURE;

    /**
     * @brief 移除所有资源提供者
     * @return void
     */
    STDMETHOD_(void, RemoveAll)(THIS) PURE;

    /**
     * @brief 获取头部资源提供者
     * @return IResProvider* -- 头部资源提供者对象
     */
    STDMETHOD_(IResProvider *, GetHeadResProvider)(CTHIS) SCONST PURE;

    /**
     * @brief 获取尾部资源提供者
     * @return IResProvider* -- 尾部资源提供者对象
     */
    STDMETHOD_(IResProvider *, GetTailResProvider)(CTHIS) SCONST PURE;

    /**
     * @brief 设置文件前缀
     * @param pszFilePrefix LPCTSTR -- 文件前缀
     * @return void
     */
    STDMETHOD_(void, SetFilePrefix)(THIS_ LPCTSTR pszFilePrefix) PURE;

    //////////////////////////////////////////////////////////////////////////

    /**
     * @brief 查询资源是否存在
     * @param pszType LPCTSTR -- 资源类型
     * @param pszResName LPCTSTR -- 资源名称
     * @return BOOL -- TRUE: 存在，FALSE: 不存在
     */
    STDMETHOD_(BOOL, HasResource)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) PURE;

    /**
     * @brief 从资源中加载ICON
     * @param pszResName LPCTSTR -- ICON名称
     * @param cx int -- ICON宽度，默认值为0
     * @param cy int -- ICON高度，默认值为0
     * @param bFromFile BOOL -- 是否从文件加载，默认值为FALSE
     * @return HICON -- 成功返回ICON的句柄，失败返回0
     */
    STDMETHOD_(HICON, LoadIcon)(THIS_ LPCTSTR pszResName, int cx DEF_VAL(0), int cy DEF_VAL(0), BOOL bFromFile DEF_VAL(FALSE)) PURE;

    /**
     * @brief 从资源中加载光标
     * @param pszResName LPCTSTR -- 光标名称
     * @param bFromFile BOOL -- 是否从文件加载，默认值为FALSE
     * @return HCURSOR -- 成功返回光标的句柄，失败返回0
     */
    STDMETHOD_(HCURSOR, LoadCursor)(THIS_ LPCTSTR pszResName, BOOL bFromFile DEF_VAL(FALSE)) PURE;

    /**
     * @brief 从资源中加载HBITMAP
     * @param pszResName LPCTSTR -- BITMAP名称
     * @param bFromFile BOOL -- 是否从文件加载，默认值为FALSE
     * @return HBITMAP -- 成功返回BITMAP的句柄，失败返回0
     */
    STDMETHOD_(HBITMAP, LoadBitmap)(THIS_ LPCTSTR pszResName, BOOL bFromFile DEF_VAL(FALSE)) PURE;

    /**
     * @brief 从资源加载一个IBitmap对象
     * @param pszType LPCTSTR -- 图片类型
     * @param pszResName LPCTSTR -- 图片名称
     * @return IBitmapS* -- 成功返回一个IBitmap对象，失败返回0
     */
    STDMETHOD_(IBitmapS *, LoadImage)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) PURE;

    /**
     * @brief 从资源中创建一个IImgX对象
     * @param pszType LPCTSTR -- 图片类型
     * @param pszResName LPCTSTR -- 图片名称
     * @return IImgX* -- 成功返回一个IImgX对象，失败返回0
     */
    STDMETHOD_(IImgX *, LoadImgX)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) PURE;

    /**
     * @brief 获得资源数据大小
     * @param pszType LPCTSTR -- 资源类型
     * @param pszResName LPCTSTR -- 资源名称
     * @return size_t -- 资源大小（byte)，失败返回0
     */
    STDMETHOD_(size_t, GetRawBufferSize)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) PURE;

    /**
     * @brief 获得资源内存块
     * @param pszType LPCTSTR -- 资源类型
     * @param pszResName LPCTSTR -- 资源名称
     * @param pBuf LPVOID -- 输出内存块
     * @param size size_t -- 内存大小
     * @return BOOL -- TRUE: 成功
     * @remark 应该先用GetRawBufferSize查询资源大小再分配足够空间
     */
    STDMETHOD_(BOOL, GetRawBuffer)(THIS_ LPCTSTR pszType, LPCTSTR pszResName, LPVOID pBuf, size_t size) PURE;
};

SNSEND
#endif // __SRESPROVIDERMGR_I__H__

