#include "stdafx.h"
#include "DlgInsertXmlElement.h"
#include "propgrid/propitem/SPropertyItem-Option.h"
#include <vector>
#ifdef _WIN32
#include "souidlgs.h"
#endif//_WIN32
#include "AutoCompleteAdapter.h"

namespace SOUI{
	DlgInsertXmlElement::DlgInsertXmlElement(ResManger * resMgr,SXmlNode xmlInitProp,SStringW strNodeName)
		:SHostDialog(_T("layout:UIDESIGNER_XML_INSERT_ELEMENT"))
		,m_xmlInitProp(xmlInitProp)
		,m_strNodeName(strNodeName)
,		 m_resMgr(resMgr)
	{
	}

	DlgInsertXmlElement::~DlgInsertXmlElement(void)
	{
	}

	BOOL DlgInsertXmlElement::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
	{
		m_tbProp = FindChildByID2<SToolBar>(R.id.tb_prop);
		m_propgrid = FindChildByName2<SPropertyGrid>(L"propgrid_element");
		InitPropGrid(m_strNodeName,L"");
		SRealWnd *pReal = FindChildByName2<SRealWnd>(L"xml_editor");
		pReal->GetRealHwnd();
		m_xmlEditor = (CScintillaWnd*)pReal->GetUserData();
		SXmlNode xmlContent = m_xmlInitProp.child(m_strNodeName).child(m_strNodeName);
		if(xmlContent)
		{
			m_xmlDoc.root().append_copy(xmlContent);
		}else
		{
			m_xmlDoc.root().append_child(m_strNodeName);
			m_propgrid->EnumProp(&DlgInsertXmlElement::OnEnumPropItem,this);
		}
		SStringW strXml;
		m_xmlDoc.root().ToString(&strXml);
		SStringA buf = S_CW2A(strXml,CP_UTF8);
		m_xmlEditor->InsertText(0,buf.c_str());

		m_bChanged = FALSE;
		SetTimer(100, 500);
		return TRUE;
	}

	BOOL DlgInsertXmlElement::OnEnumPropItem(IPropertyItem *pItem, void* opaque)
	{
		DlgInsertXmlElement * _this = (DlgInsertXmlElement*)opaque;
		if(pItem->HasValue() && pItem->GetType()!=PT_GROUP)
		{
			_this->OnGetValue(pItem,FALSE);
		}
		return TRUE;
	}

	void DlgInsertXmlElement::OnPropValueChanged(IEvtArgs *e)
	{
		EventPropGridValueChanged *e2=sobj_cast<EventPropGridValueChanged>(e);
		OnGetValue(e2->pItem,TRUE);
	}

	void DlgInsertXmlElement::OnTimer(UINT_PTR id)
	{
		if (id == 100)
		{
			if (m_bChanged)
			{				
				SStringA buf = m_xmlEditor->GetWindowText();
				BOOL bValidXml = m_xmlDoc.load_buffer(buf.c_str(), buf.GetLength(), xml_parse_default, enc_utf8);
				m_propgrid->EnableWindow(bValidXml, TRUE);
				if(bValidXml){
					SApplication *pApp = SApplication::getSingletonPtr();
					pApp->AddResProvider(m_resMgr->m_pResProvider,NULL);
					//init skin from xml
					SXmlNode xmlNode = m_xmlDoc.root().first_child();
					ISkinObj *pSkin = pApp->CreateSkinByName(xmlNode.name());
					if(pSkin){
						pSkin->InitFromXml(&xmlNode);
						m_skin = pSkin;
						pSkin->Release();
						SImageWnd * pImgWnd = FindChildByID2<SImageWnd>(R.id.img_preview);
						pImgWnd->SetSkin(m_skin);
					}
					pApp->RemoveResProvider(m_resMgr->m_pResProvider);
				}
				m_bChanged = FALSE;
			}
		}
		else
		{
			SetMsgHandled(FALSE);
		}
		
	}

	void DlgInsertXmlElement::OnGetValue(IPropertyItem *pItem,BOOL bInsertXml)
	{
		if(pItem->GetType() != PT_GROUP)
		{
			SXmlNode root = m_xmlDoc.root().first_child();
			SStringW name = pItem->GetName2();
			if(pItem->HasValue())
			{
				SStringW value = S_CT2W(pItem->GetValue());
				if (pItem->GetType() == PT_OPTION)
				{
					SPropertyItemOption* pOption = ( SPropertyItemOption* )pItem;
					value = S_CT2W(pOption->Option2Value(pItem->GetValue()));
				}
				root.attribute2(name).set_value(value);
			}else
			{
				root.remove_attribute(name);
			}
		}

		if(bInsertXml)
		{
			SStringW strXml;
			m_xmlDoc.root().ToString(&strXml);
			SStringA buf = S_CW2A(strXml,CP_UTF8);
			m_xmlEditor->SetSel(0,-1);
			m_xmlEditor->ReplaseSel(buf.c_str());
		}
	}

	void DlgInsertXmlElement::OnDestroy()
	{
		m_strXml = m_xmlEditor->GetWindowText();
		m_strXml += "\0x0a\0x0d";
		SHostDialog::OnDestroy();
	}

	SStringA DlgInsertXmlElement::GetXml() const
	{
		return m_strXml;
	}

	void DlgInsertXmlElement::OnCommand(UINT uNotifyCode, int nID, HWND wndCtl)
	{
		if(wndCtl == m_xmlEditor->m_hWnd && uNotifyCode == EN_CHANGE)
		{
			m_bChanged = TRUE;
		}else
		{
			SetMsgHandled(FALSE);
		}
	}

	void DlgInsertXmlElement::OnPropItemActive(IEvtArgs *e)
	{
		EventPropGridItemActive *e2=sobj_cast<EventPropGridItemActive>(e);
		FindChildByID(R.id.txt_prop_title)->SetWindowText(e2->pItem->GetTitle());
		FindChildByID(R.id.txt_prop_desc)->SetWindowText(e2->pItem->GetDescription());
	}

	void DlgInsertXmlElement::InitPropGrid(const SStringW & strNodeName,SStringW strParents)
	{
		SXmlNode xmlNode = m_xmlInitProp.child(strNodeName);
		if(xmlNode)
		{
			SStringW strParent = xmlNode.attribute(L"parent").as_string();
			if(!strParent.IsEmpty())
			{
				SStringWList lstParent;
				SplitString(strParent,L'|',lstParent);
				for(int i=0;i<lstParent.GetCount();i++)
				{
					if(strParents.Find(lstParent[i])==-1)
					{//use strParents to avoid round parent defined.
						strParents.Append(L",");
						strParents.Append(lstParent[i]);
						InitPropGrid(lstParent[i],strParents);
					}
				}
			}
		}
		m_propgrid->LoadFromXml(xmlNode.child(L"groups"));
	}

	void DlgInsertXmlElement::OnPropToolbarCmd(IEvtArgs *e)
	{
		EventToolBarCmd *e2=sobj_cast<EventToolBarCmd>(e);
		if(e2->nCmdId == 0)
		{
			m_propgrid->SetOrderType(SPropertyGrid::OT_GROUP);
			m_tbProp->SetItemCheck(0,TRUE);
			m_tbProp->SetItemCheck(1,FALSE);
		}else
		{
			m_propgrid->SetOrderType(SPropertyGrid::OT_NAME);
			m_tbProp->SetItemCheck(1,TRUE);
			m_tbProp->SetItemCheck(0,FALSE);
		}
	}

	void DlgInsertXmlElement::OnPropItemButtonClick(IEvtArgs *e)
	{
		EventPropGridItemButtonClick *e2 = sobj_cast<EventPropGridItemButtonClick>(e);
		SStringW strExType = e2->pItem->GetExtendType();

		if(strExType == L"font")
		{
			LOGFONT lf={0};
			#ifdef _WIN32
			CFontDialog fontDlg(&lf, CF_SCREENFONTS|CF_NOVERTFONTS);
			if(fontDlg.DoModal()== IDOK)
			{
				lf = fontDlg.m_lf;
				FontInfo fi;
				fi.strFaceName = S_CT2W(lf.lfFaceName);
				fi.style.attr.nSize = abs(lf.lfHeight);
				fi.style.attr.byWeight = lf.lfWeight/4;
				fi.style.attr.byCharset = lf.lfCharSet;
				fi.style.attr.fItalic = lf.lfItalic;
				fi.style.attr.fUnderline = lf.lfUnderline;
				fi.style.attr.fStrike = lf.lfStrikeOut;
				if(lf.lfWeight == FW_BOLD)
				{
					fi.style.attr.fBold = 1;
					fi.style.attr.byWeight = 0;
				}
				SStringW strFontDesc = SFontPool::FontInfoToString(fi);
				e2->pItem->SetValue(S_CW2T(strFontDesc));
			}
			#else
			//todo:hjx
			#endif//_WIN32
		}
	}

	void DlgInsertXmlElement::OnPropItemInplaceInit(IEvtArgs *e)
	{
		EventPropGridItemInplaceInit *e2 = sobj_cast<EventPropGridItemInplaceInit>(e);
		if(e2->pItem->IsClass(SPropertyItemOption::GetClassName()))
		{
			SComboBox *pCombox = (SComboBox*)e2->pInplaceWnd;
			if(e2->pItem->GetExtendType() == L"skin")
			{
				SStringA skinAuto = m_resMgr->GetSkinAutos(_T(""));
				SStringT strAuto = S_CA2T(skinAuto,CP_UTF8);
				SStringTList lstSkin;
				SplitString(strAuto,' ',lstSkin);
				pCombox->InsertItem(0,_T(""),0,0);
				for(int i=0;i<lstSkin.GetCount();i++)
				{
					pCombox->InsertItem(i+1,lstSkin[i],0,0);
				}
			}else if(e2->pItem->GetExtendType() == L"class")
			{
				SStringA strAutoA = m_resMgr->GetStyleAutos(_T(""));
				SStringT strAuto = S_CA2T(strAutoA,CP_UTF8);
				SStringTList lstStyle;
				SplitString(strAuto,' ',lstStyle);
				pCombox->InsertItem(0,_T(""),0,0);
				for(int i=0;i<lstStyle.GetCount();i++)
				{
					pCombox->InsertItem(i+1,lstStyle[i],0,0);
				}
			}
		}
	}

    void DlgInsertXmlElement::OnPropGridItemAutoCompleteFill(IEvtArgs *e)
    {
		EventPropGridItemAutoCompleteFill *e2 = sobj_cast<EventPropGridItemAutoCompleteFill>(e);
		SStringW strType = e2->pItem->GetExtendType();
		EventDropdownListFill *e3 = sobj_cast<EventDropdownListFill>(e2->pOriginEvt);
		SStringT strPrefix(e3->strKey);
		if(strType.CompareNoCase(L"imageSrc")==0)
		{
			SAutoCompleteImageSrcAdapter *pAdapter = new SAutoCompleteImageSrcAdapter;
			std::vector<std::pair<SStringT,SStringT> > vecSrc;
			m_resMgr->GetImageList(vecSrc);
			for(int i=0;i<vecSrc.size();i++){
				if(strPrefix.IsEmpty() || vecSrc[i].first.Find(strPrefix)!=-1){
					pAdapter->addItem(vecSrc[i].first,vecSrc[i].second);
				}
			}
			e3->pListView->SetAdapter(pAdapter);
			e3->bPopup = pAdapter->getCount()>0;
			pAdapter->Release();
		}
		else if(strType.CompareNoCase(L"color")==0)
		{
			CAutoCompleteColorAdapter *pAdapter = new CAutoCompleteColorAdapter;
			std::vector<SStringT> vecColor;
			m_resMgr->GetColorList(vecColor);
			for(int i=0;i<vecColor.size();i++){
				SStringT strColor = SStringT(_T("@color/"))+vecColor[i];
				if(strPrefix.IsEmpty() || strColor.Find(strPrefix)!=-1){
					COLORREF cr = m_resMgr->GetColor(strColor);
					pAdapter->AddColor(strColor,cr);
				}
			}
			e3->pListView->SetAdapter(pAdapter);
			e3->bPopup = pAdapter->getCount()>0;
			pAdapter->Release();
		}
		else if (strType.CompareNoCase(L"dim") == 0)
		{
			CAutoCompleteStringAdapter *pAdapter = new CAutoCompleteStringAdapter;
			std::vector<SStringT> vecDims;
			m_resMgr->GetDimensionList(vecDims);
			for(int i=0;i<vecDims.size();i++){
				SStringT strDim = SStringT(_T("@dim/"))+vecDims[i];
				if (strPrefix.IsEmpty() || strDim.Find(strPrefix) != -1) {
					pAdapter->AddString(strDim,S_CW2T(m_resMgr->GetDimension(vecDims[i])));
				}
			}
			e3->pListView->SetAdapter(pAdapter);
			e3->bPopup = pAdapter->getCount()>0;
			pAdapter->Release();
		}
		else{
			e3->bPopup = FALSE;
		}
    }
    void DlgInsertXmlElement::OnPropGridItemAutoCompleteSelected(IEvtArgs *e)
    {
		EventPropGridItemAutoCompleteSelected *e2 = sobj_cast<EventPropGridItemAutoCompleteSelected>(e);
		EventDropdownListSelected *e3 = sobj_cast<EventDropdownListSelected>(e2->pOriginEvt);
		CAutoCompleteAdapter *pAdapter = (CAutoCompleteAdapter*)(e3->pListView->GetAdapter());
		if (e3->nValue != -1)
		{
			SStringT strSelected = pAdapter->getItemText(e3->nValue);
			e2->pItem->SetValue(strSelected);
		}   
    }

    } // namespace SOUI