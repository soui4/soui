#ifndef __SMENU_I__H__
#define __SMENU_I__H__

#include <interface/obj-ref-i.h>
#include <interface/sxml-i.h>
#include <interface/SSkinobj-i.h>

SNSBEGIN

#undef INTERFACE
#define INTERFACE IMenu
DECLARE_INTERFACE_(IMenu, IObjRef)
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
     * @brief 关联到hmenu
     * @param hMenu - 菜单句柄
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, Attach)(THIS_ HMENU hMenu) PURE;

    /**
     * @brief 分离并释放其hmenu
     * @return HMENU - 菜单句柄
     */
    STDMETHOD_(HMENU, Detach)(THIS) PURE;

    /**
     * @brief 获取hmenu
     * @return HMENU - 菜单句柄
     */
    STDMETHOD_(HMENU, GetHMenu)(THIS) SCONST PURE;

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
    STDMETHOD_(BOOL, LoadMenu2)(THIS_ IXmlNode * xmlMenu) PURE;

    /**
     * @brief 设置菜单项的图标皮肤
     * @param icons - 包含一组图标的ISkinObj对象
     * @return void
     */
    STDMETHOD_(void, SetIconSkin)(THIS_ ISkinObj * icons) PURE;

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
    STDMETHOD_(BOOL, InsertMenu)(THIS_ UINT uPosition, UINT uFlags, UINT_PTR nIDNewItem, LPCTSTR strText, int iIcon DEF_VAL(-1), HICON hIcon DEF_VAL(0)) PURE;

    /**
     * @brief 追加一个菜单项
     * @param uFlags - 标志
     * @param uIDNewItem - 新项ID
     * @param lpNewItem - 新项文本
     * @param iIcon - 图标索引
     * @param hIcon - 图标句柄
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     * @remark 如果hIcon不为空，则iIcon无效。菜单退出后将通过DestroyIcon删除hIcon。
     */
    STDMETHOD_(BOOL, AppendMenu)(THIS_ UINT uFlags, UINT_PTR uIDNewItem, LPCTSTR strText, int iIcon DEF_VAL(-1), HICON hIcon DEF_VAL(0)) PURE;

    /**
     * @brief 检查菜单单选项
     * @param idFirst - 第一个ID
     * @param idLast - 最后一个ID
     * @param idCheck - 检查的ID
     * @param uFlags - 标志
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, CheckMenuItem)(THIS_ UINT uIdCheckItem, UINT uCheck) PURE;

    /**
     * @brief 检查菜单项
     * @param uIdCheckItem - 检查项ID
     * @param uCheck - 检查标志
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, CheckMenuRadioItem)(THIS_ UINT idFirst, UINT idLast, UINT idCheck, UINT uFlags) PURE;

    /**
     * @brief 删除菜单项
     * @param uPosition - 位置
     * @param uFlags - 标志
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, DeleteMenu)(THIS_ UINT uPosition, UINT uFlags) PURE;

    /**
     * @brief 弹出菜单
     * @param uFlags - 标志
     * @param x - X坐标
     * @param y - Y坐标
     * @param hWnd - 窗口句柄
     * @param nScale - 缩放比例
     * @return UINT - 选择的菜单项ID
     */
    STDMETHOD_(UINT, TrackPopupMenu)(THIS_ UINT uFlags, int x, int y, HWND hWnd, LPCRECT prcRect DEF_VAL(NULL), int nScale DEF_VAL(100)) PURE;

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
    STDMETHOD_(DWORD, GetContextHelpId)(THIS) SCONST PURE;

    /**
     * @brief 设置上下文帮助ID
     * @param dwId - 上下文帮助ID
     * @return void
     */
    STDMETHOD_(void, SetContextHelpId)(THIS_ DWORD dwId) PURE;

    /**
     * @brief 获取子菜单
     * @param nPos - 位置
     * @return hmenu - 子菜单HMENU
     */
    STDMETHOD_(HMENU, GetSubMenu)(THIS_ int nPos) PURE;

    /**
     * @brief 获取菜单项文本
     * @param uPosition - 位置
     * @param uFlags - 标志
     * @param lpItemString - 项目文本
     * @return BOOL - 成功返回TRUE，失败返回FALSE
     */
    STDMETHOD_(BOOL, GetMenuString)(THIS_ UINT uPosition, UINT uFlags, IStringT * lpItemString) PURE;
};

SNSEND
#endif // __SMENU_I__H__
