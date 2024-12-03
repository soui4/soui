// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#define ENV_SOUI4  _T("SOUI4PATH")
#define ENV_INSTALL_32 _T("SOUI4_INSTALL_32")
#define ENV_INSTALL_64 _T("SOUI4_INSTALL_64")
#define DIR_DST _T("soui5")
#define WIZARD_EXE _T("Soui5Wizard.vsz")
#define WIZARD_DLL _T("Soui5DllWizard.vsz")

const LPCTSTR kEntryFiles[]={
	_T("Soui5DllWizard.ico"),
	_T("Soui5DllWizard.vsdir"),
	_T("Soui5Wizard.ico"),
	_T("Soui5Wizard.vsdir"),
};

class CMainDlg : public CDialogImpl<CMainDlg>
{
	struct VSENVCFG
	{
		CString strName;
		CString strVsDir;
		CString strDataTarget;
		CString strEntrySrc;
		CString strEntryTarget;
		CString strScriptSrc;
	};


 
public:
	enum { IDD = IDD_MAINDLG };

	BEGIN_MSG_MAP_EX(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_BROWSE2, OnBrowseSouiDir)
		COMMAND_ID_HANDLER(IDC_BROWSE_DIR32, OnBrowseInstallDir32)
		COMMAND_ID_HANDLER(IDC_BROWSE_DIR64, OnBrowseInstallDir64)
		COMMAND_ID_HANDLER(IDC_INSTALL, OnInstall)
		COMMAND_ID_HANDLER(IDC_UNINSTALL, OnUninstall)
		COMMAND_ID_HANDLER(IDC_HOMESITE, OnHomeSite)
		END_MSG_MAP()

	CString m_strWizardDir;//数据目录
	CString m_strInstall32, m_strInstall64;

	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process;

	BOOL IsWow64()
	{
		BOOL bIsWow64 = FALSE;

		//IsWow64Process is not available on all supported versions of Windows.
		//Use GetModuleHandle to get a handle to the DLL that contains the function
		//and GetProcAddress to get a pointer to the function if available.

		fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
			GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

		if (NULL != fnIsWow64Process)
		{
			if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
			{
				//handle error
			}
		}
		return bIsWow64;
	}

	CString GetVSDir(LPCTSTR pszEnvName)
	{
        static const LPCTSTR kVsVer[3]
            = { _T("[15.0,16.0]"), _T("[16.0,17.0]"), _T("[17.0,18.0]") };

		if (_tcscmp(_T("VS141COMNTOOLS"), pszEnvName) == 0)
			return GetVs2017OrLaterDir(kVsVer[0]);

		if (_tcscmp(_T("VS142COMNTOOLS"), pszEnvName) == 0)
			return GetVs2017OrLaterDir(kVsVer[1]);
        if (_tcscmp(_T("VS143COMNTOOLS"), pszEnvName) == 0)
            return GetVs2017OrLaterDir(kVsVer[2]);
		CString strRet;
		strRet.GetEnvironmentVariable(pszEnvName);
		if (!strRet.IsEmpty()) strRet = strRet.Left(strRet.GetLength() - 14);//14=length("Common7\Tools\")
		return strRet;
	}

	bool FolderExists(LPCTSTR pszDir)
	{
		DWORD dwCode = GetFileAttributes(pszDir);
		if (dwCode == INVALID_FILE_ATTRIBUTES)
			return false;
		
		if (dwCode& FILE_ATTRIBUTE_DIRECTORY)
		{
			return true;
		}
		return false;
	}

	CString ExeCmd(CString pszCmd)
	{
		// 创建匿名管道
		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
		HANDLE hRead, hWrite;
		if (!CreatePipe(&hRead, &hWrite, &sa, 0))
		{
			return TEXT(" ");
		}

		// 设置命令行进程启动信息(以隐藏方式启动命令并定位其输出到hWrite
		STARTUPINFO si = { sizeof(STARTUPINFO) };
		GetStartupInfo(&si);
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.wShowWindow = SW_HIDE;
		si.hStdError = hWrite;
		si.hStdOutput = hWrite;

		// 启动命令行
		PROCESS_INFORMATION pi;
		if (!CreateProcess(NULL, pszCmd.GetBuffer(), NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
		{
			return TEXT("Cannot create process");
		}

		// 立即关闭hWrite
		CloseHandle(hWrite);

		// 读取命令行返回值
		CStringA strRetTmp;
		char buff[1024] = { 0 };
		DWORD dwRead = 0;
		strRetTmp = buff;
		while (ReadFile(hRead, buff, 1024, &dwRead, NULL))
		{
			strRetTmp += buff;
		}
		CloseHandle(hRead);

		LPCSTR pszSrc = strRetTmp;
		int nLen = MultiByteToWideChar(CP_ACP, 0, buff, -1, NULL, 0);
		if (nLen == 0)
			return _T("");

		wchar_t* pwszDst = new wchar_t[nLen];
		if (!pwszDst)
			return _T("");

		MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, pwszDst, nLen);
		CString strRet(pwszDst);
		delete[] pwszDst;
		pwszDst = NULL;

		strRet.TrimRight(_T("\r\n"));

		if(FolderExists(strRet))
			return strRet+_T("\\Common7\\IDE\\");
		return CString();
	}

	CString GetVs2017OrLaterDir(LPCTSTR ver)
	{
		//C:\Program Files(x86)\Microsoft Visual Studio\Installer

		LPCTSTR strProgFileRegKey = _T("Software\\Microsoft\\Windows\\CurrentVersion");

		LPCTSTR strProgFile = _T("ProgramFilesDir");
		LPCTSTR strxProgFileX86 = _T("ProgramFilesDir (x86)");

		CString strRet;
		HKEY hKey;
		LSTATUS ec = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			strProgFileRegKey,
			0,
			KEY_READ, &hKey);
		if (ec == ERROR_SUCCESS)
		{
			DWORD dwType = REG_SZ;
			DWORD dwSize = MAX_PATH;
			wchar_t data[MAX_PATH] = { 0 };
			if (ERROR_SUCCESS == RegQueryValueEx(hKey, strxProgFileX86, 0, &dwType, (LPBYTE)data, &dwSize))
			{
				strRet = data;
			}
			else if(ERROR_SUCCESS == RegQueryValueEx(hKey, strProgFile, 0, &dwType, (LPBYTE)data, &dwSize))
			{
				strRet = data;
			}
			RegCloseKey(hKey);
		}

		if (strRet.IsEmpty())
			return CString();
		strRet += _T("\\Microsoft Visual Studio\\Installer\\vswhere.exe");
		if (GetFileAttributes(strRet) == INVALID_FILE_ATTRIBUTES)
		{			
			return CString();
		}

		CString CMD = _T("\"") + strRet + _T("\"") + _T(" -nologo -version "+ver+" -prerelease -property installationPath -format value");

		return ExeCmd(CMD);
	}
			
	CString GetVS2017Dir(LPCTSTR pszEnvName)
	{
		const WCHAR *wowkey[2] = { L"(SOFTWARE\\WOW6432Node\\Microsoft\\VisualStudio\\SxS\\VS7)",
			L"(SOFTWARE\\Microsoft\\VisualStudio\\SxS\\VS7)" };

		CString strRet;
		HKEY hKey;
		LSTATUS ec = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			IsWow64() ? wowkey[0] : wowkey[1],
			0,
			KEY_READ, &hKey);
		if (ec == ERROR_SUCCESS)
		{
			DWORD dwType = REG_SZ;
			DWORD dwSize = MAX_PATH;
			wchar_t data[MAX_PATH] = { 0 };
			if (ERROR_SUCCESS == RegQueryValueEx(hKey, L"15.0", 0, &dwType, (LPBYTE)data, &dwSize))
			{
				strRet = data;
				strRet += L"(Common7\\IDE\\)";
			}
			RegCloseKey(hKey);
		}

		return strRet;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CListViewCtrl vslist;
		vslist.Attach(GetDlgItem(IDC_VSLIST));
		for (int i = 0; i < vslist.GetItemCount(); i++)
		{
			delete (VSENVCFG*)vslist.GetItemData(i);
		}
		return 0;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();

		// set icons
		HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
		SetIcon(hIconSmall, FALSE);

		CListViewCtrl vslist;
		vslist.Attach(GetDlgItem(IDC_VSLIST));
		ListView_SetExtendedListViewStyleEx(vslist.m_hWnd, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);

		TCHAR szDir[1000];
		GetCurrentDirectory(1000, szDir);
		m_strWizardDir = szDir;

		CString szVsList = m_strWizardDir += _T("\\vslist.ini");
		int i = 0;
		for (;;)
		{
			CString entry;
			entry.Format(_T("vs_%d"), ++i);
			TCHAR szBuf[1000];
			if (0 == GetPrivateProfileString(entry, _T("name"), NULL, szBuf, 1000, szVsList))
				break;

			VSENVCFG *pEnvCfg = new VSENVCFG;
			pEnvCfg->strName = szBuf;
			GetPrivateProfileString(entry, _T("envname"), NULL, szBuf, 1000, szVsList);
			pEnvCfg->strVsDir = GetVSDir(szBuf);
			if (pEnvCfg->strVsDir.IsEmpty())
			{
				delete pEnvCfg;
				continue;
			}
			GetPrivateProfileString(entry, _T("entryfilesrc"), NULL, szBuf, 1000, szVsList);
			pEnvCfg->strEntrySrc = szBuf;
			GetPrivateProfileString(entry, _T("entryfiletarget"), NULL, szBuf, 1000, szVsList);
			pEnvCfg->strEntryTarget = szBuf;
			GetPrivateProfileString(entry, _T("wizarddatatarget"), NULL, szBuf, 1000, szVsList);
			pEnvCfg->strDataTarget = szBuf;
			//vs 2019
			CString dataTarget = pEnvCfg->strVsDir + pEnvCfg->strEntryTarget;
            ::SendMessage(GetDlgItem(IDC_LOG), LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)dataTarget);
			if(FolderExists(dataTarget))
			if(CreateDirectory(dataTarget, 0))
			{
				MessageBox(_T("无法创建数据目标文件夹！"), _T("错误"), MB_OK | MB_ICONSTOP);
				delete pEnvCfg;
				continue;
			}

			GetPrivateProfileString(entry, _T("scriptsrc"), NULL, szBuf, 1000, szVsList);
			pEnvCfg->strScriptSrc = szBuf;

			int iItem = vslist.InsertItem(vslist.GetItemCount(), pEnvCfg->strName);
			vslist.SetItemData(iItem, (DWORD_PTR)pEnvCfg);
		}

		TCHAR szPath[MAX_PATH];
		if (GetEnvironmentVariable(ENV_SOUI4, szPath, MAX_PATH))
		{
			SetDlgItemText(IDC_SOUIDIR, szPath);
		}
		else
		{
			GetCurrentDirectory(MAX_PATH, szPath);
			TCHAR *pUp = _tcsrchr(szPath, _T('\\'));
			if (pUp)
			{
				_tcscpy(pUp, _T("\\SOUI"));
				if (GetFileAttributes(szPath) != INVALID_FILE_ATTRIBUTES)
				{
					*pUp = 0;
					SetDlgItemText(IDC_SOUIDIR, szPath);
				}
			}
		}
		if (GetEnvironmentVariable(ENV_INSTALL_32, szPath, MAX_PATH))
		{
			SetDlgItemText(IDC_INSTALL_DIR32, szPath);
		}
		if (GetEnvironmentVariable(ENV_INSTALL_64, szPath, MAX_PATH))
		{
			SetDlgItemText(IDC_INSTALL_DIR64, szPath);
		}
		return TRUE;
	}

	LRESULT OnHomeSite(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ShellExecute(0, _T("open"), _T("https://github.com/soui4/soui"), NULL, NULL, SW_SHOWNORMAL);
		return 0;
	}

	LRESULT OnBrowseSouiDir(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CFolderDialog folderDlg;
		if (folderDlg.DoModal() == IDOK)
		{
			SetDlgItemText(IDC_SOUIDIR, folderDlg.GetFolderPath());
		}
		return 0;
	}

	LRESULT OnBrowseInstallDir32(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CFolderDialog folderDlg;
		if (folderDlg.DoModal() == IDOK)
		{
			SetDlgItemText(IDC_INSTALL_DIR32, folderDlg.GetFolderPath());
		}
		return 0;
	}

	LRESULT OnBrowseInstallDir64(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CFolderDialog folderDlg;
		if (folderDlg.DoModal() == IDOK)
		{
			SetDlgItemText(IDC_INSTALL_DIR64, folderDlg.GetFolderPath());
		}
		return 0;
	}

	LRESULT OnInstall(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		SetCurrentDirectory(m_strWizardDir);

		CListViewCtrl vslist;
		vslist.Attach(GetDlgItem(IDC_VSLIST));

		if (GetFileAttributes(_T("SouiWizard")) == INVALID_FILE_ATTRIBUTES
			|| GetFileAttributes(_T("SouiDllWizard")) == INVALID_FILE_ATTRIBUTES)
		{
			MessageBox(_T("当前目录下没有找到SOUI的向导数据"), _T("错误"), MB_OK | MB_ICONSTOP);
			return 0;
		}
		TCHAR szSouiDir[MAX_PATH] = { 0 }, szSourCore[MAX_PATH];
		int nLen = GetDlgItemText(IDC_SOUIDIR, szSouiDir, MAX_PATH);
		if (szSouiDir[nLen - 1] == _T('\\')) szSouiDir[--nLen] = 0;

		_tcscpy(szSourCore, szSouiDir);
		_tcscat(szSourCore, _T("\\SOUI"));
		if (GetFileAttributes(szSourCore) == INVALID_FILE_ATTRIBUTES)
		{
			MessageBox(_T("当前目录下没有找到SOUI的源代码"), _T("错误"), MB_OK | MB_ICONSTOP);
			return 0;
		}

		CString strDir32,strDir64;
		{
			TCHAR szDir[MAX_PATH]={0};
			GetDlgItemText(IDC_INSTALL_DIR32, szDir, MAX_PATH);
			strDir32 = szDir;
		}
		{
			TCHAR szDir[MAX_PATH]={0};
			GetDlgItemText(IDC_INSTALL_DIR64, szDir, MAX_PATH);
			strDir64=szDir;
		}
		if(!strDir32.IsEmpty() && GetFileAttributes(strDir32)==INVALID_FILE_ATTRIBUTES){
			strDir32.Empty();
		}
		if(!strDir64.IsEmpty() && GetFileAttributes(strDir64)==INVALID_FILE_ATTRIBUTES){
			strDir64.Empty();
		}
		if(strDir32.IsEmpty() && strDir64.IsEmpty()){
			MessageBox(_T("没有找到32位或者64位soui安装目录"), _T("错误"), MB_OK | MB_ICONSTOP);
			return 0;
		}

		//设置环境变量

		CRegKey reg;
		if (ERROR_SUCCESS == reg.Open(HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Control\\Session Manager\\Environment"), KEY_SET_VALUE | KEY_QUERY_VALUE))
		{
			//set 3 Environmental values
			reg.SetStringValue(ENV_SOUI4, szSouiDir);
			reg.SetStringValue(ENV_INSTALL_32, strDir32);
			reg.SetStringValue(ENV_INSTALL_64, strDir64);			
			reg.Close();
			DWORD_PTR msgResult = 0;
			//广播环境变量修改消息
			SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, LPARAM(_T("Environment")), SMTO_ABORTIFHUNG, 5000, &msgResult);
		}
		else
		{
			MessageBox(_T("添加环境变量失败"), _T("错误"), MB_OK | MB_ICONSTOP);
			return 0;
		}

		//准备复制文件
		TCHAR szFrom[1024] = { 0 };
		TCHAR szTo[1024] = { 0 };
		SHFILEOPSTRUCT shfo;
		shfo.pFrom = szFrom;
		shfo.pTo = szTo;

		for (int i = 0; i < vslist.GetItemCount(); i++)
		{
			if (!vslist.GetCheckState(i)) continue;

			VSENVCFG *pCfg = (VSENVCFG*)vslist.GetItemData(i);
			//复制入口数据
			BOOL bOK = TRUE;
			if (bOK)
			{
				shfo.wFunc = FO_COPY;
				shfo.fFlags = FOF_NOCONFIRMMKDIR | FOF_NOCONFIRMATION;
				memset(szFrom, 0, sizeof(szFrom));
				memset(szTo, 0, sizeof(szTo));
				_tcscpy(szFrom, _T("entry\\*.*"));
				_stprintf(szTo,_T("%s%s\\%s"),pCfg->strVsDir,pCfg->strDataTarget,DIR_DST);
				bOK = 0 == SHFileOperation(&shfo);
			}
			//改写SouiWizard.vsz
			if (bOK)
			{
				_stprintf(szFrom,_T("%s\\%s"),pCfg->strEntrySrc,WIZARD_EXE);
				_stprintf(szTo,_T("%s%s\\%s\\%s"),pCfg->strVsDir, pCfg->strEntryTarget,DIR_DST, WIZARD_EXE);
				CopyFile(szFrom, szTo, FALSE);

				FILE *f = _tfopen(szTo, _T("r"));
				if (f)
				{
					char szBuf[4096];
					int nReaded = fread(szBuf, 1, 4096, f);
					szBuf[nReaded] = 0;
					fclose(f);

					f = _tfopen(szTo, _T("w"));
					if (f)
					{//清空原数据再重新写入新数据
						CStringA str = szBuf;
						str.Replace("%SOUI4PATH%", CT2A(szSouiDir));
						fwrite((LPCSTR)str, 1, str.GetLength(), f);
						fclose(f);
					}
				}
			}

			//改写SouiDllWizard.vsz
			{
				_stprintf(szFrom,_T("%s\\%s"),pCfg->strEntrySrc,WIZARD_DLL);
				_stprintf(szTo,_T("%s%s\\%s\\%s"),pCfg->strVsDir, pCfg->strEntryTarget,DIR_DST, WIZARD_DLL);
				CopyFile(szFrom, szTo, FALSE);

				FILE *f = _tfopen(szTo, _T("r"));
				if (f)
				{
					char szBuf[4096];
					int nReaded = fread(szBuf, 1, 4096, f);
					szBuf[nReaded] = 0;
					fclose(f);

					f = _tfopen(szTo, _T("w"));
					if (f)
					{//清空原数据再重新写入新数据
						CStringA str = szBuf;
						str.Replace("%SOUI4PATH%", CT2A(szSouiDir));
						fwrite((LPCSTR)str, 1, str.GetLength(), f);
						fclose(f);
					}
				}
			}

			CString strMsg;
			strMsg.Format(_T("为%s安装SOUI5向导:%s"), pCfg->strName, bOK ? _T("成功") : _T("失败"));
			::SendMessage(GetDlgItem(IDC_LOG), LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)strMsg);
		}

		return 0;
	}

	LRESULT OnUninstall(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CListViewCtrl vslist;
		vslist.Attach(GetDlgItem(IDC_VSLIST));

		SHFILEOPSTRUCT shfo = { 0 };
		shfo.pTo = NULL;
		shfo.wFunc = FO_DELETE;
		shfo.fFlags = FOF_NOCONFIRMMKDIR | FOF_NOCONFIRMATION | FOF_SILENT;

		for (int i = 0; i < vslist.GetItemCount(); i++)
		{
			if (!vslist.GetCheckState(i)) continue;

			VSENVCFG *pCfg = (VSENVCFG*)vslist.GetItemData(i);
			//remove entry files
			BOOL bOK = TRUE;
			for(int j=0;j<ARRAYSIZE(kEntryFiles) && bOK;j++){
				CString strSource;
				strSource.Format(_T("%s%s\\%s\\%s"),pCfg->strVsDir, pCfg->strEntryTarget, DIR_DST,kEntryFiles[j]);
				::SendMessage(GetDlgItem(IDC_LOG), LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)strSource);
				bOK = DeleteFile(strSource);
			}

			// 删除Soui目录
			if (bOK)
			{
				CString strSource;
				strSource.Format(_T("%s%s\\%s"),pCfg->strVsDir, pCfg->strEntryTarget, DIR_DST);
                ::SendMessage(GetDlgItem(IDC_LOG), LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)strSource);
				bOK = RemoveDirectory(strSource);
			}

			CString strMsg;
			strMsg.Format(_T("从%s中卸载SOUI5向导%s"), pCfg->strName, bOK ? _T("成功") : _T("失败"));
			::SendMessage(GetDlgItem(IDC_LOG), LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)strMsg);

		}
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

};
