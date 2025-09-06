#include "stdafx.h"
#include "DlgResMgr.h"
#include "CDebug.h"

#include "DlgInput.h"
#include "ResManger.h"
#include "helpapi.h"


namespace SOUI
{
	class CResAdapter : public SAdapterBase
	{
	public:
		struct ResInfo{
			SStringT strName;
			SStringT strPath;
		};

	public:
		CResAdapter()
		{

		}
		
		void Init(pugi::xml_node xmlNode,bool bClearFilter)
		{
			m_arrFiles.RemoveAll();
			m_arrFilters.RemoveAll();
			xmlNode = xmlNode.first_child();
			while (xmlNode)
			{
				if (xmlNode.type() != pugi::node_element)
				{
					xmlNode = xmlNode.next_sibling();
					continue;
				}
				ResInfo info;
				info.strName = S_CW2T(xmlNode.attribute(L"name").as_string());
				info.strPath = S_CW2T(xmlNode.attribute(L"path").as_string());
				m_arrFiles.Add(info);
				xmlNode = xmlNode.next_sibling();
			}
			if(bClearFilter)
				SetFilterString(SStringT());
			else
				SetFilterString(m_strFilter);
		}

		void SetFilterString(SStringT strFilter)
		{
			m_arrFilters.RemoveAll();
			strFilter.MakeLower();
			m_strFilter = strFilter;
			for(int i=0;i<m_arrFiles.GetCount();i++)
			{
				if(m_strFilter.IsEmpty())
				{
					m_arrFilters.Add(i);
					continue;
				}
				SStringT strTmp = m_arrFiles[i].strName;
				strTmp.MakeLower();
				if(strTmp.Find(strFilter) != -1)
				{
					m_arrFilters.Add(i);
					continue;
				}
				strTmp = m_arrFiles[i].strPath;
				strTmp.MakeLower();
				if(strTmp.Find(strFilter) != -1)
				{
					m_arrFilters.Add(i);
					continue;
				}
			}
			notifyDataSetChanged();
		}

		int getFiles() const{return m_arrFiles.GetCount();}

		SStringT getFilePath(UINT iFile) const{
			if(m_arrFilters.GetCount()<=iFile)
				return SStringT();
			return m_arrFiles[m_arrFilters[iFile]].strPath;
		}

		int getPathIndex(SStringT strPath) const{
			for(int i=0;i<m_arrFilters.GetCount();i++)
			{
				if(m_arrFiles[m_arrFilters[i]].strPath == strPath)
					return i;
			}
			return -1;
		}
	protected:
		virtual int WINAPI getCount()
		{
			return m_arrFilters.GetCount();
		}

		virtual void WINAPI getView(int position, SItemPanel * pItem, SXmlNode xmlTemplate)
		{
			if(pItem->GetChildrenCount()==0)
			{
				pItem->InitFromXml(&xmlTemplate);
			}
			SStatic *pTxtName = pItem->FindChildByName2<SStatic>(L"txt_name");
			if(pTxtName) pTxtName->SetWindowText(m_arrFiles[m_arrFilters[position]].strName);
			SWindow *pTxtPath = pItem->FindChildByName(L"txt_path");
			if(pTxtPath) pTxtPath->SetWindowText(m_arrFiles[m_arrFilters[position]].strPath);
		}

	private:
		SArray<ResInfo> m_arrFiles;
		SArray<int>		m_arrFilters;
		SStringT	    m_strFilter;
	};


	SResMgrDlg::SResMgrDlg(SStringT strPath,ResManger* pResFileManger) 
		:SHostDialog(_T("layout:xml_res_mgr"))
		,m_pResFileManger(pResFileManger)
		,m_pResAdapter(NULL)
	{
		m_strProPath = strPath.Mid(0, strPath.ReverseFind(TPATH_SLASH));
	}
	
	SResMgrDlg::~SResMgrDlg(void)
	{
	}

	BOOL SResMgrDlg::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
	{
		m_lbResType = (SListBox *)FindChildByName(L"NAME_UIDESIGNER_LB_ZYLX");

		m_pEdit = (SEdit *)FindChildByName(L"NAME_UIDESIGNER_edit_SEARCH");

		m_imgView = (SImgCanvas *)FindChildByName(L"NAME_UIDESIGNER_IMG_VIEW");
		m_txtImageSize = (SStatic *)FindChildByName(L"sel_image_size");

		LRESULT lr = m_pEdit->SSendMessage(EM_SETEVENTMASK, 0, ENM_CHANGE);
		m_pEdit->GetEventSet()->subscribeEvent(EventRENotify::EventID, Subscriber(&SResMgrDlg::OnReNotify, this));


		m_lbResType->GetEventSet()->subscribeEvent(EVT_LB_SELCHANGED, Subscriber(&SResMgrDlg::OnLbResTypeSelChanged, this));

		m_lvRes = FindChildByID2<SListView>(R.id.lv_res_path);
		m_pResAdapter.Attach(new CResAdapter());
		m_lvRes->SetAdapter(m_pResAdapter);
		m_lvRes->GetEventSet()->subscribeEvent(EventLVSelChanged::EventID,Subscriber(&SResMgrDlg::OnLvResSelChanged,this));

		InitResType();

		return TRUE;
	}

	void SResMgrDlg::InitResType()
	{
		pugi::xml_node xmlNode = m_pResFileManger->m_xmlNodeUiRes.child(L"resource").first_child();

		while (xmlNode)
		{
			if (xmlNode.type() != pugi::node_element)
			{
				xmlNode = xmlNode.next_sibling();
				continue;
			}

			m_lbResType->AddString(S_CW2T(xmlNode.name()));

			xmlNode = xmlNode.next_sibling();
		}
	}

	BOOL SResMgrDlg::OnLbResTypeSelChanged(IEvtArgs *pEvtBase)
	{
		EventLBSelChanged *pEvt = (EventLBSelChanged*)pEvtBase;
		SListBox *listbox = (SListBox*)pEvt->sender;
		{
			SStringT strText = listbox->GetText(pEvt->nNewSel);
			CResAdapter *pAdapter = (CResAdapter*)m_lvRes->GetAdapter();
			pAdapter->Init(m_pResFileManger->m_xmlNodeUiRes.child(L"resource").child(S_CT2W(strText)),true);
			m_lvRes->SetSel(-1);
			m_pEdit->GetEventSet()->setMutedState(true);
			m_pEdit->SetWindowText(_T(""));
			m_pEdit->GetEventSet()->setMutedState(false);
		}

		return true;
	}

	BOOL SResMgrDlg::OnLvResSelChanged(IEvtArgs *pEvtBase)
	{
		m_imgView->Clear();
		m_txtImageSize->SetWindowText(_T(""));

		EventLVSelChanged *pEvt = (EventLVSelChanged*)pEvtBase;
		pugi::xml_node xmlNode;
		if (pEvt->iNewSel == -1)
		{
			return false;
		}
		SStringT strPath = m_pResAdapter->getFilePath(pEvt->iNewSel);

		ShowImage(strPath);
		return true;
	}

	void SResMgrDlg::OnOK()
	{
		m_pResFileManger->SaveRes();
		EndDialog(IDOK);
	}


	void SResMgrDlg::OnNewResType()
	{
		SDlgInput dlg;
		if (IDOK != dlg.DoModal(m_hWnd))
		{
			return;
		}

		SStringW strName = S_CT2W(dlg.m_strValue);
		pugi::xml_node xmlNode = m_pResFileManger->m_xmlNodeUiRes.child(L"resource");
		if (xmlNode.child(strName))
		{
			CDebug::Debug(_T("该资源类型已存在！"));
			return;
		}

		xmlNode = xmlNode.append_child(strName);
		int n = m_lbResType->AddString(dlg.m_strValue);

		m_lbResType->SetCurSel(n,TRUE);
		m_lbResType->EnsureVisible(n);
	}

	void SResMgrDlg::OnDelResType()
	{
		int iSel = m_lbResType->GetCurSel();

		if (iSel < 0)
		{
			CDebug::Debug(_T("请选择资源类型"));
			return;
		}

		if (iSel == 0)
		{
			CDebug::Debug(_T("不能选择内置皮肤类型"));
			return;
		}

		if (m_pResAdapter->getFiles() > 0)
		{
			CDebug::Debug(_T("请先删除资源"));
			return;
		}

		pugi::xml_node xmlNode = m_pResFileManger->m_xmlNodeUiRes.child(L"resource").child(S_CT2W(ListBox_GetCurSelText(m_lbResType)));
		if (xmlNode)
		{
			xmlNode.parent().remove_child(xmlNode);
			m_lbResType->DeleteString(iSel);
		}
	}
	
	void SResMgrDlg::OnAddResource()
	{
		if (m_lbResType->GetCurSel() < 0)
		{
			CDebug::Debug(_T("请选择资源类型"));
			return;
		}

		SStringT strResType = ListBox_GetCurSelText(m_lbResType);
		if (m_lbResType->GetCurSel() <= 3)
		{
			CDebug::Debug(_T("不能选择") + strResType + _T("类型"));
			return;
		}

		CFileDialogEx OpenDlg(TRUE, m_strProPath, NULL, NULL, 6, _T("所有文件 (*.*)\0*.*\0\0"));
		if (IDOK == OpenDlg.DoModal())
		{
			SStringT strFileName = OpenDlg.m_szFileName;
			int n = strFileName.Find(m_strProPath);
			if (n != 0)
			{
				SMessageBox(NULL, _T("请将资源放到uires目录下"), _T("提示"), MB_OK);
				return;
			}

			SStringW strResTypeW = S_CT2W(strResType);
			SStringT strFile = strFileName.Mid(m_strProPath.GetLength() + 1);

			pugi::xml_node xmlNode = m_pResFileManger->m_xmlNodeUiRes.child(L"resource").child(strResTypeW).first_child();
			pugi::xml_node xmlNewNode;

			while (xmlNode)
			{
				if (xmlNode.type() != pugi::node_element)
				{
					xmlNode = xmlNode.next_sibling();
					continue;
				}

				if (strFile.CompareNoCase(S_CW2T(xmlNode.attribute(L"path").value())) == 0)
				{
					break;
				}

				xmlNode = xmlNode.next_sibling();
			}

			if (!xmlNode)
			{
				// 如果该不存在该类型的资源，则添加	
				xmlNewNode = m_pResFileManger->m_xmlNodeUiRes.child(L"resource").child(strResTypeW).append_child(L"file");

				SStringT strResName = GetFilename(strFile);
				strResName.Replace(_T("\\"), _T("_"));
				strResName.Replace(_T("."), _T("_"));

				xmlNewNode.append_attribute(L"name").set_value(strResTypeW);
				xmlNewNode.append_attribute(L"path").set_value(S_CT2W(strFile));
				
				m_pResAdapter->Init(m_pResFileManger->m_xmlNodeUiRes.child(L"resource").child(strResTypeW),false);
			}

			//定位到资源
			{
				int iFile = m_pResAdapter->getPathIndex(strFile);
				if(iFile != -1)
				{
					m_lvRes->EnsureVisible(iFile);
					m_lvRes->SetSel(iFile,TRUE);
				}
			}
		}
	}
	void SResMgrDlg::OnRemoveResource()
	{
		if (m_lbResType->GetCurSel() == 0)
		{
			CDebug::Debug(_T("内置资源不能删除"));
			return;
		}

		SStringT strResType = ListBox_GetCurSelText(m_lbResType);
		if (strResType.IsEmpty())
		{
			return;
		}

		int iFile = m_lvRes->GetSel();
		if(iFile == -1)
		{
			return;
		}
		

		int nResult = SMessageBox(NULL, _T("确定要删除资源吗?"), _T("提示"), MB_OKCANCEL);
		if (nResult != 1)
		{
			return;
		}

		SStringT strPath = m_pResAdapter->getFilePath(iFile);
		pugi::xml_node xmlNode = m_pResFileManger->m_xmlNodeUiRes.child(L"resource").child(S_CT2W(strResType)).first_child();
		while (xmlNode)
		{
			if (xmlNode.type() != pugi::node_element)
			{
				xmlNode = xmlNode.next_sibling();
				continue;
			}
			if (strPath.CompareNoCase(S_CW2T(xmlNode.attribute(L"path").value())) == 0)
			{

				pugi::xml_node xmlNodeP = xmlNode.parent();
				xmlNodeP.remove_child(xmlNode);
				m_pResAdapter->Init(xmlNodeP,false);
				break;
			}
			xmlNode = xmlNode.next_sibling();
		}

	}


	SStringT SResMgrDlg::ListBox_GetCurSelText(SListBox * lb)
	{
		SStringT s(_T(""));
		int n = lb->GetCurSel();

		if (n < 0)
		{
			return s;
		}

		return lb->GetText(n);
	}

	BOOL SResMgrDlg::OnReNotify(IEvtArgs *pEvt)
	{
		EventRENotify *pReEvt = (EventRENotify*)pEvt;
		if (pReEvt->iNotify == EN_CHANGE)
		{
			SStringT strValue = m_pEdit->GetWindowText();
			m_pResAdapter->SetFilterString(strValue);
		}
		return true;
	}

	void SResMgrDlg::ShowImage(const SStringT& strImgname)
	{
		SStringT strImgPath = m_strProPath + TPATH_SLASH + strImgname;

		m_imgView->Clear();
		m_imgView->AddFile(strImgPath);
		CSize imgSize = m_imgView->GetImgInfo();
		SStringT strSize;
		strSize.Format(_T("%d * %d"), imgSize.cx, imgSize.cy);
		m_txtImageSize->SetWindowText(strSize);
	}


}


