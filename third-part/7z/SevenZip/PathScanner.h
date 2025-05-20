#pragma once
#include <string>
#include <deque>
#include <sys/stat.h>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#include <Shlwapi.h>
#else
#include <dirent.h>
#include <fnmatch.h>
#include <unistd.h>
#include <cstring>
#endif

#include "FileInfo.h"
#include "SevenString.h"

namespace SevenZip {
namespace intl {

class PathScanner {
public:
	class Callback
	{
	public:

		virtual ~Callback() {}

		virtual void BeginScan() {}
		virtual void EndScan() {}
		virtual bool ShouldDescend(const FilePathInfo& directory) = 0;
		virtual void EnterDirectory(const TString& /*path*/) {}
		virtual void LeaveDirectory(const TString& /*path*/) {}
		virtual void ExamineFile(const FilePathInfo& file, bool& exit) = 0;
	};

    static void Scan(const TString& root, Callback& cb);
    static void Scan(const TString& root, const TString& searchPattern, Callback& cb);

private:
    static bool ProcessDirectory(const TString& directory, 
                                 const TString& pattern,
                                 Callback& cb,
                                 std::deque<TString>& directories,
                                 bool& exitFlag);

    static bool IsSpecialFileName(const TString& fileName);
    static bool MatchesPattern(const TString& name, const TString& pattern);
    static TString JoinPath(const TString& dir, const TString& name);
    static FilePathInfo GetFileInfo(const TString& fullPath, bool isDirectory);
};

}
}
