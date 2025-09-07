#include "PathScanner.h"
#include "FileSys.h"
#include "SevenString.h"

namespace SevenZip {
namespace intl {

#ifdef _WIN32
	const wchar_t PATH_SEP = L'\\';
#else
	const char PATH_SEP = '/';
#endif

	void PathScanner::Scan(const TString& root, Callback& cb) {
		Scan(root, _T("*"), cb);
	}

	void PathScanner::Scan(const TString& root, const TString& searchPattern, Callback& cb) {
		std::deque<TString> directories;
		directories.push_back(root);

		cb.BeginScan();

		bool scanAborted = false;
		while (!directories.empty() && !scanAborted) {
			TString dir = directories.front();
			directories.pop_front();

			bool exitFlag = false;
			bool dirProcessed = ProcessDirectory(dir, searchPattern, cb, directories, exitFlag);

			if (exitFlag) {
				scanAborted = true;
				break;
			}
		}

		cb.EndScan();
	}

	bool PathScanner::ProcessDirectory(const TString& directory,
		const TString& pattern,
		Callback& cb,
		std::deque<TString>& directories,
		bool& exitFlag) {
		cb.EnterDirectory(directory);

		WIN32_FIND_DATA fdata;
		TString findPath = JoinPath(directory, _T("*"));
		HANDLE hFind = FindFirstFile(findPath.c_str(), &fdata);

		if (hFind == INVALID_HANDLE_VALUE) {
			cb.LeaveDirectory(directory);
			return false;
		}

		do {
			if (IsSpecialFileName(fdata.cFileName)) continue;

			TString fullPath = JoinPath(directory, fdata.cFileName);
			bool isDir = (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
			FilePathInfo info = GetFileInfo(fullPath, isDir);

			if (isDir) {
				if (cb.ShouldDescend(info)) {
					directories.push_back(fullPath);
				}
			}
			else {
				if (MatchesPattern(info.FileName, pattern)) {
					cb.ExamineFile(info, exitFlag);
					if (exitFlag) break;
				}
			}
		} while (FindNextFile(hFind, &fdata));

		FindClose(hFind);

		cb.LeaveDirectory(directory);
		return exitFlag;
}

bool PathScanner::IsSpecialFileName(const TString& fileName) {
#ifdef _UNICODE
    return fileName == L"." || fileName == L"..";
#else
    return fileName == "." || fileName == "..";
#endif
}

bool PathScanner::MatchesPattern(const TString& name, const TString& pattern) {
	return PathMatchSpec(name.c_str(), pattern.c_str()) == TRUE;
}

TString PathScanner::JoinPath(const TString& dir, const TString& name) {
    if (dir.empty()) return name;
    
#ifdef _WIN32
    if (dir[dir.size()-1] == _T('\\') || dir[dir.size()-1] == _T('/'))
        return dir + name;
    return dir + _T('\\') + name;
#else
    if (dir[dir.size()-1] == '/')
        return dir + name;
    return dir + _T("/") + name;
#endif
}

FilePathInfo PathScanner::GetFileInfo(const TString& fullPath, bool isDirectory) {
    FilePathInfo info;
    
#ifdef _WIN32
    size_t pos = fullPath.find_last_of(L"\\/");
#else
    size_t pos = fullPath.find_last_of('/');
#endif
    info.FileName = (pos != TString::npos) ? fullPath.substr(pos+1) : fullPath;
    info.FilePath = fullPath;
    info.IsDirectory = isDirectory;

    return info;
}

}
}
