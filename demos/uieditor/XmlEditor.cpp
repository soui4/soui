#include "stdafx.h"
#include "XmlEditor.h"
#include "core/SWnd.h"
#include "MainDlg.h"
#include "SysdataMgr.h"
#include "helpapi.h"
#include <propgrid/propitem/SPropertyItem-Option.h>
#include <propgrid/propitem/SPropertyItem-Color.h>
#include <helper/SAppDir.h>
#include <helper/SFunctor.hpp>
#include <control/SRealWnd.h>
#include <SouiFactory.h>
#ifdef _WIN32
#include <shlwapi.h>
#endif//_WIN32
#include "AutoCompleteAdapter.h"

//编辑界面时XML窗口只显示选择控件的XML文本
//#define  ONLYSHOWSELXML

extern CSysDataMgr g_SysDataMgr;

#define  MARGIN 20
#define kLogTag "xml_editor"

SNSBEGIN

//-----------------------------------------------------------------------------------------------
CXmlEditor::CXmlEditor(CMainDlg *pMainDlg):m_bValidXml(TRUE),m_bUpdateDesigner(TRUE)
{
	m_pMainDlg = pMainDlg;
	m_pResManger = &pMainDlg->m_UIResFileMgr;
}


CXmlEditor::~CXmlEditor()
{

}
void CXmlEditor::SetProjectPath(const SStringT & strProjPath)
{
	if(!m_strProPath.IsEmpty())
		return;
	int pos = strProjPath.ReverseFind(SLASH);
	if(pos!=-1)
		m_strProPath = strProjPath.Left(pos);
	else
		m_strProPath = strProjPath;
}

BOOL CXmlEditor::CloseProject()
{
	m_strXmlFile =
	m_strProPath = SStringT();
	m_bValidXml = true;

	m_pScintillaWnd->SendEditor(SCI_CLEARALL);
	m_pScintillaWnd->SetDirty(false);
	m_treeXmlStruct->RemoveAllItems();
	m_pPropGrid->RemoveAllItems();
	m_vecSelectOrder.clear();
	m_pDesignWnd->Close();
	return TRUE;
}

void CXmlEditor::StartPreviewProcess()
{
	m_pPropGrid->RemoveAllItems();
	m_vecSelectOrder.clear();
	SApplication::getSingletonPtr()->AddResProvider(m_pResManger->m_pResProvider,NULL);
	SAutoRefPtr<IUiDefInfo> pUiDef = GETUIDEF->GetUiDef();
	GETUIDEF->SetUiDef(m_pResManger->m_pUiDef,false);
	m_pDesignWnd->Init(m_strLayoutName,m_pMainDlg->m_hWnd);
	GETUIDEF->SetUiDef(pUiDef,false);
	SApplication::getSingletonPtr()->RemoveResProvider(m_pResManger->m_pResProvider);
}
	
BOOL CXmlEditor::LoadXml(SStringT strFileName, SStringT layoutName)
{
	m_bChanged = FALSE;
	m_strXmlFile = strFileName;
	m_strXmlFile.ReplaceChar(_T('\\'),_T('/'));
	m_strLayoutName = layoutName;
	m_pScintillaWnd->SendMessage(SCI_SETREADONLY, 0, 0);
	SStringT strPath = m_strProPath + TPATH_SLASH + m_strXmlFile;
	m_pScintillaWnd->OpenFile(strPath);
	m_nCaretPos = m_pScintillaWnd->SendEditor(SCI_GETCURRENTPOS);
    m_bSetCaretPos = FALSE;
	m_treeXmlStruct->RemoveAllItems();
	if(!layoutName.IsEmpty())
	{
		m_xml_editor->ShowPane(0);
		StartPreviewProcess();
	}else
	{
		m_xml_editor->HidePane(0);
	}

	return TRUE;
}

void CXmlEditor::UpdateXmlData()
{
	SStringA strUtf8=m_pScintillaWnd->GetWindowText();
	m_bValidXml=m_xmlParser.loadUtf8(strUtf8,strUtf8.GetLength());
	m_treeXmlStruct->RemoveAllItems();

	spugi::xml_node root = m_xmlParser.first_child();
	UpdateXmlStruct(root, STVI_ROOT);
	m_treeXmlStruct->Invalidate();
	if(!m_bValidXml)
	{
		SLOGI()<<"error xml";
		return;
	}
    if (!m_strLayoutName.IsEmpty() && m_bUpdateDesigner)
    {//reload layout
        SApplication::getSingletonPtr()->AddResProvider(m_pResManger->m_pResProvider, NULL);
        SAutoRefPtr<IUiDefInfo> pUiDef = GETUIDEF->GetUiDef();
        GETUIDEF->SetUiDef(m_pMainDlg->m_UIResFileMgr.m_pUiDef, false);
        m_pDesignWnd->UpdateLayoutBuf(strUtf8);
        GETUIDEF->SetUiDef(pUiDef, false);
        SApplication::getSingletonPtr()->RemoveResProvider(m_pResManger->m_pResProvider);
    }
}

//保存当前打开的布局文件
bool CXmlEditor::SaveFile()
{
	if (m_strXmlFile.IsEmpty())
	{
		return false;
	}
	
	return m_pScintillaWnd->SaveFile(m_strProPath + TPATH_SLASH+ m_strXmlFile);
}

bool CXmlEditor::UpdateXmlStruct(spugi::xml_node xmlNode, HSTREEITEM item,int iSib)
{
	if (!xmlNode)
	{
		return false;
	}
	int count = 0;

	if(xmlNode.type() != spugi::node_element)
		return false;
    SStringA strName = xmlNode.name();
    if (xmlNode.attribute("name"))
    {
        strName += SStringA().Format(".%s", xmlNode.attribute("name").value());
    }
    else if (xmlNode.attribute("id"))
    {
        strName += SStringA().Format(".%s", xmlNode.attribute("id").value());
	}
    HSTREEITEM itemNew = m_treeXmlStruct->InsertItem(S_CA2T(strName), item);
	m_treeXmlStruct->SetItemData(itemNew,iSib);

	int iChild = 0;
	spugi::xml_node nodeChild = xmlNode.first_child();
	while (nodeChild)
	{
		bool bOK = UpdateXmlStruct(nodeChild,itemNew,iChild);
		if(bOK) iChild++;

		nodeChild = nodeChild.next_sibling();
	}
	return true;
}

// 响应窗口结构中点击选中界面元素
void CXmlEditor::OnTCSelChanged(IEvtArgs *pEvt)
{
	EventTCSelChanged *evt = (EventTCSelChanged*)pEvt;
	HSTREEITEM item = evt->hNewSel;
	int nLevel = 0;
	while(item)
	{
		nLevel++;
		item = m_treeXmlStruct->GetParentItem(item);
	}
	
	int *idx = new int[nLevel];
	item = evt->hNewSel;
	int iLevel = nLevel-1;
	while(item)
	{
		int iSib = m_treeXmlStruct->GetItemData(item);
		idx[iLevel] = iSib;
		item = m_treeXmlStruct->GetParentItem(item);
		iLevel--;
	}
	SelectCtrlByOrder(idx,nLevel,TRUE);
	delete []idx;
}

void CXmlEditor::OnPropGridItemAutoCompleteFill(IEvtArgs *e)
{
	EventPropGridItemAutoCompleteFill *e2 = sobj_cast<EventPropGridItemAutoCompleteFill>(e);
	SStringW strType = e2->pItem->GetExtendType();
	EventDropdownListFill *e3 = sobj_cast<EventDropdownListFill>(e2->pOriginEvt);
	SStringT strPrefix(e3->strKey);
	if(strType.CompareNoCase(L"skin")==0)
	{
		CAutoCompleteSkinAdapter *pAdapter = new CAutoCompleteSkinAdapter;
		std::vector<SStringT> vecSkin;
		m_pResManger->GetSkinList(vecSkin);
		for(int i=0;i<vecSkin.size();i++){
			if(strPrefix.IsEmpty() || vecSkin[i].Find(strPrefix)!=-1){
				ISkinObj *pSkin = m_pResManger->GetSkin(S_CT2W(vecSkin[i]),100);
				pAdapter->AddSkin(vecSkin[i],pSkin);
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
		m_pResManger->GetColorList(vecColor);
		for(int i=0;i<vecColor.size();i++){
			SStringT strColor = SStringT(_T("@color/"))+vecColor[i];
			if(strPrefix.IsEmpty() || strColor.Find(strPrefix)!=-1){
				COLORREF cr = m_pResManger->GetColor(strColor);
				pAdapter->AddColor(strColor,cr);
			}
		}
		e3->pListView->SetAdapter(pAdapter);
		e3->bPopup = pAdapter->getCount()>0;
		pAdapter->Release();
	}
	else if (strType.CompareNoCase(L"string") == 0)
	{
		CAutoCompleteStringAdapter *pAdapter = new CAutoCompleteStringAdapter;
		std::vector<SStringT> vecString;
		m_pResManger->GetStringList(vecString);
		for(int i=0;i<vecString.size();i++){
			SStringT strString = SStringT(_T("@string/"))+vecString[i];
			if (strPrefix.IsEmpty() || strString.Find(strPrefix) != -1) {
				SStringT strValue = m_pResManger->m_mapStrings[vecString[i]].value;
				pAdapter->AddString(strString,strValue);
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
		m_pResManger->GetDimensionList(vecDims);
		for(int i=0;i<vecDims.size();i++){
			SStringT strDim = SStringT(_T("@dim/"))+vecDims[i];
			if (strPrefix.IsEmpty() || strDim.Find(strPrefix) != -1) {
				pAdapter->AddString(strDim,S_CW2T(m_pResManger->GetDimension(vecDims[i])));
			}
		}
		e3->pListView->SetAdapter(pAdapter);
		e3->bPopup = pAdapter->getCount()>0;
		pAdapter->Release();
	}else if(strType.CompareNoCase(L"font")==0){
		CAutoCompleteStringAdapter *pAdapter = new CAutoCompleteStringAdapter;
		std::vector<SStringT> vecFonts;
		m_pResManger->GetFontList(vecFonts);
		for(int i=0;i<vecFonts.size();i++){
			SStringT strFont = SStringT(_T("@font/"))+vecFonts[i];
			if (strPrefix.IsEmpty() || strFont.Find(strPrefix) != -1) {
				pAdapter->AddString(strFont,S_CW2T(m_pResManger->GetFont(vecFonts[i])));
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

void CXmlEditor::OnPropGridItemAutoCompleteSelected(IEvtArgs *e)
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

void CXmlEditor::OnPropGridConvertColor(IEvtArgs *pEvt)
{
	EventPropGridConvertColor *evt = sobj_cast<EventPropGridConvertColor>(pEvt);
	evt->crValue = m_pResManger->GetColor(evt->strValue);
	evt->bConverted = TRUE;
}

void CXmlEditor::OnPropGridValueChanged(IEvtArgs *e)
{
	if(m_vecSelectOrder.empty())
		return;
	EventPropGridValueChanged *e2 = sobj_cast<EventPropGridValueChanged>(e);
	SStringW strName = e2->pItem->GetName2();
	SStringW strValue = S_CT2W(e2->pItem->GetValue());
	SXmlNode xmlNode = m_xmlEditing.root().first_child();
	BOOL bSetText = FALSE;
	if(strName.CompareNoCase(L"text")==0)
	{
		if(xmlNode.attribute(strName)){
			xmlNode.attribute(strName).set_value(strValue);
		}else{
			xmlNode.SetText(strValue);
			bSetText = TRUE;
		}
	}else{
		xmlNode.attribute2(strName).set_value(strValue);
	}


	NodeRange nr = m_xmlParser.getNodePos(&m_vecSelectOrder[0],m_vecSelectOrder.size());
	if(nr.begin!=-1)
	{
		m_bSetCaretPos = TRUE;
		if(bSetText)
		{
			SStringW strXml;
			xmlNode.ToString(&strXml);
			m_pScintillaWnd->SetSel(nr.begin,nr.end);
			m_pScintillaWnd->ReplaseSel(S_CW2A(strXml,CP_UTF8));
		}else if(nr._break!=-1){
			//update attribute value
			SStringW strXml= SStringW().Format(L"<%s",xmlNode.name());
			for(SXmlAttr attr = xmlNode.first_attribute();attr;attr = attr.next_attribute()){
				strXml.AppendFormat(L" %s=\"%s\"",attr.name(),attr.value());
			}
			strXml.Append(L">");
			m_pScintillaWnd->SetSel(nr.begin,nr._break);
			m_pScintillaWnd->ReplaseSel(S_CW2A(strXml,CP_UTF8));
		}
		m_bSetCaretPos = FALSE;
	}
}
void CXmlEditor::InitPropGrid(const SStringW &strNodeName, SStringW strParents)
{
	SXmlNode xmlInitProp = g_SysDataMgr.getCtrlDefNode().child(L"controls");
    SXmlNode xmlNode = xmlInitProp.child(strNodeName);
    if (xmlNode)
    {
        SStringW strParent = xmlNode.attribute(L"parent").as_string();
        if (!strParent.IsEmpty())
        {
            SStringWList lstParent;
            SplitString(strParent, L'|', lstParent);
            for (int i = 0; i < lstParent.GetCount(); i++)
            {
                if (strParents.Find(lstParent[i]) == -1)
                { // use strParents to avoid round parent defined.
                    strParents.Append(L",");
                    strParents.Append(lstParent[i]);
                    InitPropGrid(lstParent[i], strParents);
                }
            }
        }
		m_pPropGrid->LoadFromXml(xmlNode.child(L"groups"));
    }else
	{
		//check if strNodeName is a window object
		IWindow *pWnd = SApplication::getSingletonPtr()->CreateWindowByName(strNodeName);
		if(pWnd){
			//using default window property
			pWnd->Release();
			InitPropGrid(SWindow::GetClassName(),L"");
		}
	}
}

void CXmlEditor::SelectCtrlByOrder(const int *pOrder, int nLen, BOOL bSelXml)
{
	#ifdef _DEBUG
	SStringA str;
	for(int i=0;i<nLen;i++)
	{
		str += SStringA().Format("%d,",pOrder[i]);
	}
	SLOGI()<<"order:"<<str;
	#endif
	if(m_vecSelectOrder.size()==nLen && memcmp(&m_vecSelectOrder[0],pOrder,nLen*sizeof(int))==0)
		return;
	NodeRange range = m_xmlParser.getNodePos(pOrder,nLen);
	if(range.begin!=-1)
	{
		int begin = range.begin;
		int end = range.end!=-1?range.end:range._break;
		if(end != -1)
		{
            if (bSelXml)
            {
                m_bSetCaretPos = TRUE;
                m_pScintillaWnd->SetSel(begin, end);
                m_bSetCaretPos = FALSE;
            }
			SStringA strXml = m_pScintillaWnd->GetRange(begin,end);
			m_xmlEditing.Reset();
			if(m_xmlEditing.LoadBufferInplace((void*)strXml.c_str(),strXml.GetLength(),xml_parse_default,enc_utf8)){
				//init propgrid
				m_pPropGrid->GetEventSet()->setMutedState(TRUE);
				m_pPropGrid->RemoveAllItems();
				SXmlNode xmlNode = m_xmlEditing.root().first_child();
				SStringW strName = SStringW(xmlNode.name());
				InitPropGrid(strName,L"");
				for(SXmlAttr attr = xmlNode.first_attribute();attr;attr = attr.next_attribute()){
					IPropertyItem *pItem = m_pPropGrid->FindItemByName(attr.name());
					if(pItem){
						pItem->SetValue(S_CW2T(attr.value()));
					}
				}
				IPropertyItem *pItem = m_pPropGrid->FindItemByName(L"text");
				if(pItem){
					SStringW strValue = SStringW(xmlNode.Text());
					if(!strValue.IsEmpty())
						pItem->SetValue(S_CW2T(strValue));
				}
				m_pPropGrid->GetEventSet()->setMutedState(FALSE);
			}
		}
	}
	m_pDesignWnd->SelectCtrlByOrder(pOrder,nLen);
	m_vecSelectOrder.assign(pOrder,pOrder+nLen);
}

void CXmlEditor::OnCommand(UINT uNotifyCode, int nID, HWND wndCtl)
{
	if(wndCtl == m_pScintillaWnd->m_hWnd && uNotifyCode == EN_CHANGE)
	{
		if(!m_bChanged)
		{
			m_bChanged = TRUE;
			::SetTimer(m_pMainDlg->m_hWnd,300,200,NULL);
		}
	}else
	{
		SetMsgHandled(FALSE);
	}
}

void CXmlEditor::Init(SWindow * pRoot,CScintillaWnd::IListener *pListener)
{
	m_treeXmlStruct = pRoot->FindChildByName2<STreeCtrl>(L"uidesigner_wnd_xmltree");
	m_xml_editor = pRoot->FindChildByName2<SSplitWnd>("xml_editor");
	{
		SRealWnd *pRealWnd  = pRoot->FindChildByName2<SRealWnd>(L"uidesigner_wnd_layout");
		m_pDesignWnd = (CDesignWnd*)pRealWnd->GetUserData();
		m_pDesignWnd->SetXmlEditor(this);
	}
	{
		SRealWnd *pRealWnd = pRoot->FindChildByName2<SRealWnd>(L"uidesigner_scintilla");
		m_pScintillaWnd = (CScintillaWnd*)pRealWnd->GetUserData();
		if (m_pScintillaWnd)
		{
			m_pScintillaWnd->SendEditor(SCI_SETMODEVENTMASK,SC_MOD_INSERTTEXT|SC_MOD_DELETETEXT);
			m_pScintillaWnd->SetListener(pListener);
		}
	}
	{
		m_pPropGrid = pRoot->FindChildByName2<SPropertyGrid>(L"propgrid_element");
	}
}
bool CXmlEditor::isDirty() const
{
	return m_pScintillaWnd->isDirty();
}

void CXmlEditor::InsertText(SStringA strText)
{
	m_pScintillaWnd->ReplaseSel(strText.c_str());
}

BOOL CXmlEditor::InsertElement(SStringA xmlElement)
{
	//find a valid insert position for the new xml element
	int pos = m_pScintillaWnd->SendEditor(SCI_GETCURRENTPOS);
	//find the element that contains the pos
	if(m_bChanged){
		UpdateXmlData();
		m_bChanged = false;
	}
	NodeRange range = m_xmlParser.findElementRange(pos);
	if(range.begin==-1)
		return FALSE;
	if(range.begin!=-1)
	{
		if(pos > range.begin && pos < range.end)
		{
			if(range._break!=-1)
			{
				pos = range._break;
			}else{
				pos = range.end;
			}
		}
		m_pScintillaWnd->SendEditor(SCI_GOTOPOS,pos);
		m_pScintillaWnd->InsertText(pos,xmlElement.c_str());
	}
    return TRUE;
}

BOOL CXmlEditor::InsertWidget(SStringA xmlWidget)
{
	BOOL bRet = InsertElement(xmlWidget);
	if(bRet)
	{
		UpdateXmlData();
		m_bChanged=FALSE;
		//auto select the new inserted element
		int pos = m_pScintillaWnd->SendEditor(SCI_GETCURRENTPOS);
		std::vector<int> order;
		int nOrder = m_xmlParser.findElementOrder(pos,order);
		if(nOrder>1)
		{//skip the root element
			SelectCtrlByOrder(&order[1],nOrder-1,TRUE);
		}
	}
	return bRet;
}

LRESULT CXmlEditor::OnNotify(int idCtrl, LPNMHDR pnmh)
{
	if (pnmh->hwndFrom == m_pScintillaWnd->m_hWnd && pnmh->code == SCN_UPDATEUI)
	{
		int nCaretPos = m_pScintillaWnd->SendEditor(SCI_GETCURRENTPOS);
		if (nCaretPos != m_nCaretPos)
		{
			m_nCaretPos = nCaretPos;
			if(m_bSetCaretPos)
				return 0;
            OnTimer(300); // update xml data if needed
			std::vector<int> order;
			m_xmlParser.findElementOrder(nCaretPos,order);
			if(order.size()>1)
			{//skip the root element
				SelectCtrlByOrder(&order[1],order.size()-1,FALSE);
			}
		}
	}else{
		SetMsgHandled(FALSE);
	}
    return 0;
}

void CXmlEditor::OnTimer(UINT_PTR id)
{
	if(id == 300)
	{
		if(m_bChanged){
			m_bChanged = FALSE;
			UpdateXmlData();
		}
		::KillTimer(m_pMainDlg->m_hWnd,id);
	}else
	{
		SetMsgHandled(FALSE);
	}
}

bool CXmlEditor::isValidXml()
{
	if(m_bChanged)
	{
		UpdateXmlData();
		m_bChanged = false;
	}
	return m_bValidXml;
}

void CXmlEditor::OnSelectedCtrl(const int *pData, int nCount)
{
    bool bRootLayout = m_xmlParser.isRootLayout();
    int *pOrder = new int[nCount + 1];
    pOrder[0] = 0;
    memcpy(pOrder + 1, pData, nCount * sizeof(int));
    if (bRootLayout)
        SelectCtrlByOrder(pOrder, nCount + 1,TRUE);
    else
        SelectCtrlByOrder(pOrder + 1, nCount,TRUE);
    delete[] pOrder;
}

void CXmlEditor::OnMoveCtrl()
{
    NodeRange nr = m_xmlParser.getNodePos(&m_vecSelectOrder[0], m_vecSelectOrder.size());
    if (nr.begin != -1)
    {
		m_bSetCaretPos = TRUE;
		SXmlNode xmlNode = m_xmlEditing.root().first_child();
		if(nr._break!=-1)
		{
			SStringW strXml= SStringW().Format(L"<%s ",xmlNode.name());
			for(SXmlAttr attr = xmlNode.first_attribute();attr;attr = attr.next_attribute()){
				strXml.AppendFormat(L" %s=\"%s\"",attr.name(),attr.value());
			}
			strXml.Append(L">");
			m_pScintillaWnd->SetSel(nr.begin, nr._break);
			m_pScintillaWnd->ReplaseSel(S_CW2A(strXml, CP_UTF8));
		}else{
			SStringW strXml;
			xmlNode.ToString(&strXml);
			m_pScintillaWnd->SetSel(nr.begin, nr.end);
			m_pScintillaWnd->ReplaseSel(S_CW2A(strXml, CP_UTF8));
		}
		m_bSetCaretPos = FALSE;
        m_bUpdateDesigner = FALSE;
        OnTimer(300); // reload right now.
        UpdatePropGridLayout();
        m_bUpdateDesigner = TRUE;
    }
}

void CXmlEditor::OnUpdateSize(int nWidth, int nHeight)
{
	SXmlNode xmlNode = m_xmlEditing.root().first_child();
	SXmlAttr xmlWidth = xmlNode.attribute(L"width");
	SXmlAttr xmlHeight = xmlNode.attribute(L"height");
	SXmlAttr xmlSize = xmlNode.attribute(L"size");
	if(xmlWidth){
		xmlWidth.set_value(SStringW().Format(L"%d",nWidth));
	}
	if(xmlHeight){
		xmlHeight.set_value(SStringW().Format(L"%d",nHeight));
	}
	if(xmlSize){
		xmlSize.set_value(SStringW().Format(L"%d,%d",nWidth,nHeight));
	}
	OnMoveCtrl();
}

void CXmlEditor::OnUpdatePos(SStringW strPos)
{
	SXmlNode xmlNode = m_xmlEditing.root().first_child();
	xmlNode.RemoveAttribute(L"size");
	xmlNode.RemoveAttribute(L"width");
	xmlNode.RemoveAttribute(L"height");
	xmlNode.attribute2(L"pos").set_value(strPos);
	OnMoveCtrl();
}

void CXmlEditor::UpdatePropGridLayout()
{
    m_pPropGrid->GetEventSet()->setMutedState(TRUE);
    SXmlNode xmlNode = m_xmlEditing.root().first_child();
	{
		IPropertyItem *pItem = m_pPropGrid->FindItemByName(L"pos");
		if(pItem){
			if (SXmlAttr xmlPos = xmlNode.attribute(L"pos"))
			{
				pItem->SetValue(S_CW2T(xmlPos.value()));
			}else{
				pItem->ClearValue();
			}
		}
	}
	{
		IPropertyItem *pItem = m_pPropGrid->FindItemByName(L"size");
		if(pItem){
			if (SXmlAttr xmlSize = xmlNode.attribute(L"size"))
			{
				pItem->SetValue(S_CW2T(xmlSize.value()));
			}else{
				pItem->ClearValue();
			}
		}
	}
	{
		IPropertyItem *pItem = m_pPropGrid->FindItemByName(L"width");
		if(pItem){
			SXmlAttr xmlWidth = xmlNode.attribute(L"width");
			if(xmlWidth)
			{
				pItem->SetValue(S_CW2T(xmlWidth.value()));
			}else{
				pItem->ClearValue();
			}
		}
	}
	{
		IPropertyItem *pItem = m_pPropGrid->FindItemByName(L"height");
		if(pItem){
			SXmlAttr xmlHeight = xmlNode.attribute(L"height");
			if(xmlHeight)
			{
				pItem->SetValue(S_CW2T(xmlHeight.value()));
			}else{
				pItem->ClearValue();
			}
		}
	}
	{
		IPropertyItem *pItem = m_pPropGrid->FindItemByName(L"offset");
		if(pItem){
			SXmlAttr xmlOffset = xmlNode.attribute(L"offset");
			if(xmlOffset)
			{
				pItem->SetValue(S_CW2T(xmlOffset.value()));
			}else{
				pItem->ClearValue();
			}
		}
	}

    m_pPropGrid->GetEventSet()->setMutedState(FALSE);
}
SNSEND