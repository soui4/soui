#ifndef __SPropertyItemBase_H__
#define __SPropertyItemBase_H__


#include "SPropertyItem-i.h"
#include <sobject/Sobject.hpp>
#include <helper/obj-ref-impl.hpp>

SNSBEGIN

    class SPropertyItemBase : public TObjRefImpl<IPropertyItem>
    {
        DEF_SOBJECT(TObjRefImpl<IPropertyItem>,L"propitembase")
    public:
        virtual ~SPropertyItemBase();

	public:
		STDMETHOD_(int,GetID)(THIS) SCONST OVERRIDE
		{
			return m_nID;
		}

		STDMETHOD_(void,SetID)(THIS_ int nID) OVERRIDE
		{
			m_nID = nID;
		}

		STDMETHOD_(LPCWSTR,GetName)(THIS) SCONST OVERRIDE
		{
			return m_strName.c_str();
		}
		STDMETHOD_(void,SetName)(THIS_ LPCWSTR pszName) OVERRIDE
		{
			m_strName = pszName;
		}

	public:
        virtual BOOL HasButton() const  OVERRIDE{return FALSE;}
        virtual int  GetLevel() const  OVERRIDE;
        virtual BOOL IsExpand() const  OVERRIDE;
        virtual void Expand(BOOL bExpend)  OVERRIDE;

        virtual IPropertyItem * GetParent() const  OVERRIDE;
        virtual void SetParent(IPropertyItem * pParent) OVERRIDE;
        virtual IPropertyItem * GetItem(PROPITEMTYPE type) const  OVERRIDE;
		virtual IPropertyItem * GetChildById(int nID) const OVERRIDE;

        virtual SPropertyGrid * GetOwner() const  OVERRIDE;
        virtual BOOL InsertChild(IPropertyItem * pChild,IPropertyItem * pInsertAfter=IC_LAST) OVERRIDE;
        virtual BOOL RemoveChild(IPropertyItem * pChild) OVERRIDE;
        virtual int ChildrenCount() const OVERRIDE;

		virtual SStringT GetTitle() const OVERRIDE{return m_strTitle.IsEmpty()?S_CW2T(m_strName):m_strTitle;}
        virtual void SetTitle(const SStringT & strName) OVERRIDE{m_strTitle=strName;}
		virtual SStringW GetName2() const  OVERRIDE{return m_strName;}
        virtual SStringT GetDescription() const  OVERRIDE{return m_strDescription;}
        virtual void SetDescription(const SStringT & strDescription) OVERRIDE{m_strDescription =strDescription;}
        virtual SStringT GetValue() const  OVERRIDE{return _T("");}
        virtual void SetValue(const SStringT & strValue)  OVERRIDE{}

        virtual void AdjustInplaceActiveWndRect(CRect & rc) OVERRIDE{}
        virtual void DrawItem(IRenderTarget *pRT,CRect rc) OVERRIDE{}
        virtual BOOL IsInplaceActive() const  OVERRIDE{return m_bInplaceActive;}
        virtual void OnInplaceActive(BOOL bActive) OVERRIDE{ m_bInplaceActive = bActive;}
        virtual BOOL OnButtonClick() OVERRIDE{ return FALSE;}
        virtual void OnValueChanged() OVERRIDE;
        virtual void OnChildValueChanged( IPropertyItem *pChild ) OVERRIDE{};
		virtual SList<IPropertyItem*>* GetItemList(){return &m_childs;};
		
		virtual BOOL IsReadOnly() const OVERRIDE{return m_bReadOnly;}
		virtual void SetReadOnly(BOOL bReadOnly) OVERRIDE;
		virtual SStringW GetExtendType() const OVERRIDE;

		IPropertyItem * FindChildByName(LPCWSTR pszName) const OVERRIDE;
		IPropertyItem * FindChildById(int nID) OVERRIDE;
		
        SOUI_ATTRS_BEGIN()
            ATTR_STRINGT(L"title",m_strTitle,TRUE)
			ATTR_STRINGW(L"name",m_strName,FALSE)
			ATTR_INT(L"id",m_nID,FALSE)
            ATTR_STRINGT(L"description",m_strDescription,FALSE)
            ATTR_INT(L"readOnly",m_bReadOnly,FALSE)
            ATTR_CUSTOM(L"expanded",OnAttrExpanded)
			ATTR_STRINGW(L"extendType",m_strExType,FALSE)
        SOUI_ATTRS_END()

        STDMETHOD_(BOOL,InitFromXml)(THIS_ IXmlNode * xmlNode) OVERRIDE;
    protected:
        HRESULT OnAttrExpanded(const SStringW &  strValue,BOOL bLoading);

        SStringT        m_strTitle;
		SStringW		m_strName;
		int				m_nID;
        SStringT        m_strDescription;
		SStringW		m_strExType;

        SPropertyGrid * m_pOwner;
        IPropertyItem * m_pParent;

        typedef IPropertyItem* IPropertyItemPtr;
        typedef SList<IPropertyItemPtr> PropItemList;
        PropItemList    m_childs;

		BOOL            m_bExpanded;  //折叠或展开
		BOOL            m_bReadOnly;
		BOOL            m_bInplaceActive;
    protected:
        SPropertyItemBase(SPropertyGrid * pOwner)
            :m_pOwner(pOwner)
            ,m_pParent(NULL)
            ,m_bExpanded(TRUE)
            ,m_nID(0)
			,m_bReadOnly(FALSE)
            ,m_bInplaceActive(FALSE)
        {
            SASSERT(pOwner);
        }
    };

SNSEND

#endif//_SPropertyItemBase_H_