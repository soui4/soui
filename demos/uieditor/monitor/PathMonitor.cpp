#include "stdafx.h"
#include "PathMonitor.h"
#include <vector>
#include <algorithm>
// 获取文件夹中所有文件的列表
void CPathMonitor::GetFilesInDirectory(const tstring& directory,std::vector<CPathMonitor::_FileInfo> & files)
{
    tstring search_path = directory + _T("/*.*");
    
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(search_path.c_str(), &fd);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            // 只关注文件，忽略目录
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				if(_tcscmp(fd.cFileName,_T("."))!=0 && _tcscmp(fd.cFileName,_T(".."))!=0)
				{
					GetFilesInDirectory(directory + _T("/") + fd.cFileName, files);
				}
			}
			else
			{
				_FileInfo fi;
				fi.path = directory + _T("/") + fd.cFileName;
				fi.tLastModify = fd.ftLastWriteTime;
                files.push_back(fi);
            }
        } while (FindNextFile(hFind, &fd));
        FindClose(hFind);
    }
}

static BOOL IsFileTimeEqual(const FILETIME & ft1, const FILETIME & ft2)
{
	return ft1.dwHighDateTime == ft2.dwHighDateTime && ft1.dwLowDateTime == ft2.dwLowDateTime; 
}
std::vector<CPathMonitor::_FileChange> CPathMonitor::FindModifiedFiles(
    const FileInfoList& old_files, 
    const FileInfoList& new_files)
{
    std::vector<_FileChange> modified_files;
    
    // 找出新增的文件（在新列表但不在旧列表中）
    for (FileInfoList::const_iterator new_file = new_files.begin(); new_file != new_files.end(); ++new_file) {
        
		FileInfoList::const_iterator it = old_files.begin();
		for(;it!=old_files.end();++it)
		{
			if(it->path == new_file->path)
				break;
		}
		if(it == old_files.end()) {
            modified_files.push_back(_FileChange(new_file->path, FILE_CREATED));
        }else if(!IsFileTimeEqual(new_file->tLastModify, it->tLastModify)) {
			modified_files.push_back(_FileChange(new_file->path, FILE_MODIFIED));
		}
    }
	for (FileInfoList::const_iterator old_file = old_files.begin(); old_file != old_files.end(); ++old_file) {
		FileInfoList::const_iterator it = new_files.begin();
		for(;it!=new_files.end();++it)
		{
			if(it->path == old_file->path)
				break;
		}
        if (it == new_files.end()) {
            modified_files.push_back(_FileChange(old_file->path, FILE_DELETED));
        }
    }
    return modified_files;
}
CPathMonitor::CPathMonitor(IListener *pListener):m_pListener(pListener)
{
}

CPathMonitor::~CPathMonitor(void)
{
}

UINT CPathMonitor::Run()
{
	HANDLE h = FindFirstChangeNotification(m_strPath.c_str(), TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME);
	if (h != INVALID_HANDLE_VALUE)
	{
		HANDLE hs[2] = { m_evtStop, h };

		for (;;)
		{
			DWORD dwRet = WaitForMultipleObjects(2, hs, FALSE, INFINITE);
			if (dwRet == WAIT_OBJECT_0)
			{
				break;
			}
			FileInfoList files;
			GetFilesInDirectory(m_strPath.c_str(),files);
			FileChangeList modified_files = FindModifiedFiles(m_files, files);
			for (FileChangeList::iterator it= modified_files.begin();it!=modified_files.end();++it)
			{
				m_pListener->OnFileChanged(it->path.c_str(),it->nFlag);
			} 
			m_files = files;
			
			if (!FindNextChangeNotification(h))
			{
				break;
			}
		}
		FindCloseChangeNotification(h);
	}
	return 0;
}
void CPathMonitor::SetPath(LPCTSTR pszPath)
{
	if(IsRunning())
		EndThread();
	m_strPath = pszPath;
	m_files.clear();
	GetFilesInDirectory(pszPath,m_files);
	BeginThread();
}

void CPathMonitor::Stop()
{
	if(IsRunning())
		EndThread();
	m_strPath.clear();
	m_files.clear();
}
