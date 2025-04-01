#ifndef __SMENUEX_I__H__
#define __SMENUEX_I__H__

#include <interface/obj-ref-i.h>
#include <interface/sxml-i.h>
#include <interface/SSkinobj-i.h>
#include <soui_exp.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IMenuEx
DECLARE_INTERFACE_(IMenuEx, IObjRef)
{

    //! 添加引用
    /*!
     * @return long - 当前引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //! 释放引用
    /*!
     * @return long - 当前引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //! 释放对象
    /*!
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    //=================================================================

    /**
     * @brief 从资源ID加载菜单
     * @param resId - 资源ID
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, LoadMenu)(THIS_ LPCTSTR resId) PURE;

    /**
     * @brief 从资源ID加载菜单（UTF-8编码）
     * @param resId - 资源ID
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, LoadMenuU8)(THIS_ LPCSTR resId) PURE;

    /**
     * @brief 从IXmlNode对象加载菜单
     * @param xmlMenu - IXmlNode对象
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, LoadMenu2)(THIS_ IXmlNode *xmlMenu) PURE;

    /**
     * @brief 插入一个菜单项
     * @param uPosition - 插入位置
     * @param uFlags - 标志，MF_BYCOMMAND或MF_BYPOSITION
     * @param nIDNewItem - 项目ID
     * @param strText - 文本
     * @param iIcon - 图标索引
     * @param hIcon - 图标句柄
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     * @remark 如果hIcon不为空，则iIcon无效。菜单退出后将通过DestroyIcon删除hIcon。
     */
    STDMETHOD_(BOOL, InsertMenu)(THIS_ UINT uPosition, UINT uFlags, int id, LPCTSTR strText, int iIcon DEF_VAL(-1)) PURE;

    /**
     * @brief 追加一个菜单项
     * @param uFlags - 标志，MF_BYCOMMAND或MF_BYPOSITION
     * @param uIDNewItem - 新项ID
     * @param strText - 文本
     * @param iIcon - 图标索引
     * @param hIcon - 图标句柄
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     * @remark 如果hIcon不为空，则iIcon无效。菜单退出后将通过DestroyIcon删除hIcon。
     */
    STDMETHOD_(BOOL, AppendMenu)(THIS_ UINT uFlags, int id, LPCTSTR lpNewItem, int iIcon DEF_VAL(-1)) PURE;

    /**
     * @brief 设置菜单项的检查标志
     * @param uIdCheckItem - 项目ID或索引
     * @param uCheck - MF_BYCOMMAND/MF_BYPOSITION | MF_CHECKED/MF_UNCHECKED
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, CheckMenuItem)(THIS_ UINT uIdCheckItem, UINT uCheck) PURE;

    /**
     * @brief 检查菜单单选项
     * @param idFirst - 第一个ID
     * @param idLast - 最后一个ID
     * @param uCheck - 检查的ID
     * @param uFlags - 标志，指定idFirst、idLast和idCheck的含义。如果此参数为MF_BYCOMMAND，则其他参数指定菜单项标识符。如果为MF_BYPOSITION，则其他参数指定菜单项位置。
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, CheckMenuRadioItem)(THIS_ UINT idFirst, UINT idLast, UINT idCheck, UINT uFlags) PURE;

    /**
     * @brief 删除菜单项
     * @param uPosition - 位置
     * @param uFlags - 标志，MF_BYCOMMAND或MF_BYPOSITION
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, DeleteMenu)(THIS_ UINT uPosition, UINT uFlags) PURE;

    /**
     * @brief 弹出菜单
     * @param uFlags - 标志
     * @param x - X坐标
     * @param y - Y坐标
     * @param hWnd - 窗口句柄
     * @param nScale - 缩放比例，默认值为100
     * @return UINT - 选择的菜单项ID
     */
    STDMETHOD_(UINT, TrackPopupMenu)(THIS_ UINT uFlags, int x, int y, HWND hWnd, int nScale DEF_VAL(100)) PURE;

    /**
     * @brief 销毁菜单
     * @return void
     */
    STDMETHOD_(void, DestroyMenu)(THIS) PURE;

    /**
     * @brief 修改菜单项文本
     * @param uPosition - 位置
     * @param uFlags - 标志
     * @param lpItemString - 项目文本
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, ModifyMenuString)(THIS_ UINT uPosition, UINT uFlags, LPCTSTR lpItemString) PURE;

    /**
     * @brief 设置菜单项用户数据
     * @param uPosition - 位置
     * @param uFlags - 标志
     * @param ulUserData - 用户数据
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, SetMenuUserData)(THIS_ UINT uPosition, UINT uFlags, ULONG_PTR ulUserData) PURE;

    /**
     * @brief 获取菜单项用户数据
     * @param uPosition - 位置
     * @param uFlags - 标志
     * @return ULONG_PTR - 用户数据
     */
    STDMETHOD_(ULONG_PTR, GetMenuUserData)(THIS_ UINT uPosition, UINT uFlags) PURE;

    /**
     * @brief 获取上下文帮助ID
     * @return DWORD - 上下文帮助ID
     */
    STDMETHOD_(DWORD, GetContextHelpId)(CTHIS) SCONST PURE;

    /**
     * @brief 设置上下文帮助ID
     * @param dwId - 上下文帮助ID
     * @return void
     */
    STDMETHOD_(void, SetContextHelpId)(THIS_ DWORD dwId) PURE;

    /**
     * @brief 获取子菜单
     * @param nPos - 子菜单索引
     * @return IMenuEx* - 子菜单指针
     */
    STDMETHOD_(IMenuEx *, GetSubMenu)(THIS_ int nPos) PURE;

    /**
     * @brief 获取菜单项文本
     * @param uPosition - 位置
     * @param uFlags - 标志
     * @param lpItemString - 项目文本
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, GetMenuString)(THIS_ UINT uPosition, UINT uFlags, IStringT *lpItemString) PURE;

    /**
     * @brief 获取宿主窗口指针
     * @return IHostWnd* - 宿主窗口指针
     */
    STDMETHOD_(IHostWnd *, GetHostWnd)(THIS) PURE;
};

EXTERN_C void SOUI_EXP EndMenuEx(int nCmdId);

SNSEND
#endif // __SMENUEX_I__H__
