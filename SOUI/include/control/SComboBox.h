/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserved.
 *
 * @file       SCmnCtrl.h
 * @brief      通用控件
 * @version    v1.0
 * @author     soui
 * @date       2014-06-26
 *
 * Describe    ComboBox控件
 */
#pragma once
#include "SComboBase.h"
#include "Slistbox.h"
#include "SCmnCtrl.h"

SNSBEGIN

/**
 * @class      SComboBox
 * @brief      可输入CommboBox
 *
 * Describe    可输入下拉列表
 */
class SOUI_EXP SComboBox : public TComboBaseProxy<IComboBox> {
    DEF_SOBJECT(SComboBase, L"combobox")
  public:
    /**
     * SComboBox::SComboBox
     * @brief    构造函数
     *
     * Describe  构造函数
     */
    SComboBox();

    /**
     * SComboBox::~SComboBox
     * @brief    析构函数
     *
     * Describe  析构函数
     */
    virtual ~SComboBox();

  public:
    STDMETHOD_(BOOL, SetCurSel)(THIS_ int iSel) OVERRIDE;

    STDMETHOD_(int, GetCurSel)(THIS) SCONST OVERRIDE;

    STDMETHOD_(int, GetCount)(THIS) SCONST OVERRIDE;

    STDMETHOD_(LPARAM, GetItemData)(THIS_ UINT iItem) SCONST OVERRIDE;

    STDMETHOD_(BOOL, SetItemData)(THIS_ UINT iItem, LPARAM lParam) OVERRIDE;

    STDMETHOD_(int, InsertItem)(THIS_ int iPos, LPCTSTR pszText, int iIcon, LPARAM lParam) OVERRIDE;
	STDMETHOD_(int, InsertItemA)(THIS_ int iPos, LPCSTR pszText, int iIcon, LPARAM lParam) OVERRIDE{
		SStringT str=S_CA2T(pszText,CP_UTF8);
		return InsertItem(iPos,str,iIcon,lParam);
	}

    STDMETHOD_(BOOL, DeleteString)(THIS_ int iPos) OVERRIDE;

    STDMETHOD_(void, ResetContent)(THIS) OVERRIDE;

    STDMETHOD_(BOOL, GetItemText)(int iItem, BOOL bRawText, IStringT *str) SCONST OVERRIDE;

	STDMETHOD_(BOOL, GetItemTextA)(int iItem, BOOL bRawText, IStringA *str) SCONST OVERRIDE{
		SStringT strBuf;
		BOOL bRet = GetItemText(iItem,bRawText,&strBuf);
		SStringA strBufA=S_CT2A(strBuf,CP_UTF8);
		str->Copy(&strBufA);
		return bRet;
	}

    STDMETHOD_(IListBox *, GetIListBox)(THIS) OVERRIDE;

  public:
    /**
     * SComboBox::GetListBox
     * @brief    获取下拉列表指针
     * @param    返回SListBox *
     *
     * Describe  获取下拉列表指针
     */
    SListBox *GetListBox()
    {
        return m_pListBox;
    }

  protected:
    /**
     * SComboBox::FireEvent
     * @brief    通知消息
     * @param    EventArgs &evt -- 事件对象
     *
     * Describe  此函数是消息响应函数
     */
    STDMETHOD_(BOOL, FireEvent)(THIS_ IEvtArgs *evt) OVERRIDE;

    virtual void OnScaleChanged(int nScale);

    virtual HRESULT OnLanguageChanged();

    /**
     * SComboBox::CreateListBox
     * @brief    创建下拉列表
     * @param    返回BOOL TRUE -- 成功 FALSE -- 失败
     *
     * Describe  创建下拉列表
     */
    virtual BOOL CreateListBox(SXmlNode xmlNode);

    /**
     * SComboBox::GetListBoxHeight
     * @brief    获取下拉列表高度
     * @param    返回int
     *
     * Describe  获取下拉列表高度
     */
    virtual int GetListBoxHeight();

    /**
     * SComboBox::OnCreateDropDown
     * @brief    下拉列表创建事件
     * @param    SDropDownWnd *pDropDown -- 下拉列表指针
     *
     * Describe  下拉列表事件
     */
    virtual void OnCreateDropDown(SDropDownWnd *pDropDown);

    /**
     * SComboBox::OnDestroyDropDown
     * @brief    下拉列表销毁事件
     * @param    SDropDownWnd *pDropDown -- 下拉列表指针
     *
     * Describe  获取下拉列表指针
     */
    virtual void OnDestroyDropDown(SDropDownWnd *pDropDown);

    /**
     * SComboBox::OnSelChanged
     * @brief    下拉列表selected事件
     *
     * Describe  下拉列表selected事件
     */
    virtual void OnSelChanged();

  protected:
    SListBox *m_pListBox; /**< SListBox指针 */
};

SNSEND
