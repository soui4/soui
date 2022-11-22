#pragma once

#include "SComboBase.h"
#include "SListView.h"

SNSBEGIN
class SOUI_EXP SComboView : public TComboBaseProxy<IComboView> {
    DEF_SOBJECT(SComboBase, L"comboview")
  public:
    SComboView(void);
    ~SComboView(void);

  public:
    STDMETHOD_(BOOL, SetCurSel)(int iSel) OVERRIDE;

    STDMETHOD_(int, GetCurSel)() SCONST OVERRIDE;

    STDMETHOD_(int, GetCount)() SCONST OVERRIDE;

    STDMETHOD_(BOOL, GetItemText)(int iItem, BOOL bRawText, IStringT *str) SCONST OVERRIDE;

	STDMETHOD_(BOOL, GetItemTextA)(int iItem, BOOL bRawText, IStringA *str) SCONST OVERRIDE{
		SStringT strBuf;
		BOOL bRet = GetItemText(iItem,bRawText,&strBuf);
		SStringA strBufA = S_CT2A(strBuf,CP_UTF8);
		str->Copy(&strBufA);
		return bRet;
	}

    STDMETHOD_(IListView *, GetIListView)(THIS) OVERRIDE;

  public:
    /**
     * SComboView::GetListBox
     * @brief    获取下拉列表指针
     * @param    返回SListBox *
     *
     * Describe  获取下拉列表指针
     */
    SListView *GetListView();

  protected:
    /**
     * SComboView::FireEvent
     * @brief    通知消息
     * @param    EventArgs &evt -- 事件对象
     *
     * Describe  此函数是消息响应函数
     */
    STDMETHOD_(BOOL, FireEvent)(THIS_ IEvtArgs *evt) OVERRIDE;

    virtual void OnScaleChanged(int nScale);

    /**
     * SComboView::CreateListBox
     * @brief    创建下拉列表
     * @param    返回BOOL TRUE -- 成功 FALSE -- 失败
     *
     * Describe  创建下拉列表
     */
    virtual BOOL CreateListBox(SXmlNode xmlNode);

    /**
     * SComboView::GetListBoxHeight
     * @brief    获取下拉列表高度
     * @param    返回int
     *
     * Describe  获取下拉列表高度
     */
    virtual int GetListBoxHeight();

    /**
     * SComboView::OnCreateDropDown
     * @brief    下拉列表事件
     * @param    SDropDownWnd *pDropDown -- 下拉列表指针
     *
     * Describe  下拉列表事件
     */
    virtual void OnCreateDropDown(SDropDownWnd *pDropDown);

    /**
     * SComboView::OnDestroyDropDown
     * @brief    下拉列表关闭事件
     * @param    SDropDownWnd *pDropDown -- 下拉列表指针
     * @param    UINT uCode -- 消息码
     *
     * Describe  获取下拉列表指针
     */
    virtual void OnDestroyDropDown(SDropDownWnd *pDropDown);

    /**
     * SComboView::OnSelChanged
     * @brief    下拉列表selected事件
     *
     * Describe  下拉列表selected事件
     */
    virtual void OnSelChanged();

    virtual HRESULT OnLanguageChanged() override;

  protected:
    SListView *m_pListBox; /**< SListBox指针 */
};

SNSEND
