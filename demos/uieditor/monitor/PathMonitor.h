#ifndef _PATHMONITOR_H_
#define _PATHMONITOR_H_

#include <string>
#include <vector>
#include "ThreadObject.h"

#ifdef _UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif
 
class CPathMonitor : public CThreadObject
{
public:
	enum Flag
	{
		FILE_MODIFIED = 1,
		FILE_DELETED,
		FILE_CREATED 
	};
	struct IListener
	{
		virtual void OnFileChanged(LPCTSTR pszFile, Flag nFlag) = 0;
	};
public:
	CPathMonitor();
	~CPathMonitor(void);

	void AddListener(IListener *pListener);
	void RemoveListener(IListener *pListener);
	void SetPath(LPCTSTR pszPath);
	void Stop();
protected:
	virtual UINT Run() ;

protected:
	tstring m_strPath;
	struct _FileInfo
	{
		tstring path;
		FILETIME tLastModify;
	};
	struct _FileChange
	{
		_FileChange(const tstring &path_,Flag nFlag_):path(path_),nFlag(nFlag_){}
		tstring path;
		Flag nFlag;
	};

	typedef std::vector<_FileInfo> FileInfoList;
	typedef std::vector<_FileChange> FileChangeList;
	typedef std::vector<IListener*> ListenerList;

    static void GetFilesInDirectory(const tstring &directory, std::vector<CPathMonitor::_FileInfo> &files);
    static std::vector<_FileChange> FindModifiedFiles(const FileInfoList &old_files, const FileInfoList &new_files);

    FileInfoList m_files;
	ListenerList m_listeners;
	CRITICAL_SECTION m_csListeners;
};

#endif// _PATHMONITOR_H_