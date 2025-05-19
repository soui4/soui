#include "FileSys.h"
#include "PathScanner.h"
#include <ctime>
#include "../CPP/Common/MyCom.h"
#include "../CPP/Common/MyWindows.h"

#ifdef _WIN32
#include <ShlObj.h>
#include <Shlwapi.h>
#endif

namespace SevenZip
{
namespace intl
{

class ScannerCallback : public PathScanner::Callback
{
private:

	std::vector< FilePathInfo > m_files;
	bool m_recursive;
	bool m_onlyFirst;

public:

	ScannerCallback( bool recursive, bool onlyFirst = false ) : m_recursive( recursive ), m_onlyFirst( onlyFirst ) {}
	const std::vector< FilePathInfo >& GetFiles() { return m_files; }

	virtual bool ShouldDescend( const FilePathInfo& /*directory*/ ) { return m_recursive; }
	virtual void ExamineFile( const FilePathInfo& file, bool& exit )
	{
		m_files.push_back( file );
		if ( m_onlyFirst )
		{
			exit = true;
		}
	}
};


class IsEmptyCallback : public PathScanner::Callback
{
private:

	bool m_isEmpty;

public:

	IsEmptyCallback() : m_isEmpty( true ) {}
	bool IsEmpty() const { return m_isEmpty; }

	virtual bool ShouldDescend( const FilePathInfo& /*directory*/ ) { return true; }
	virtual void ExamineFile( const FilePathInfo& /*file*/, bool& exit ) { m_isEmpty = false; exit = true; }
};


TString FileSys::GetPath( const TString& filePath )
{
	// Find the last "\" or "/" in the string and return it and everything before it.
	size_t index  = filePath.rfind( _T( '\\' ) );
	size_t index2 = filePath.rfind( _T( '/' ) );

	if ( index2 != std::string::npos && index2 > index )
	{
		index = index2;
	}

	if ( index == std::string::npos )
	{
		// No path sep.
		return TString();
	}
	else if ( index + 1 >= filePath.size() )
	{
		// Last char is path sep, the whole thing is a path.
		return filePath;
	}
	else
	{
		return filePath.substr( 0, index + 1 );
	}
}

TString FileSys::GetFileName( const TString& filePathOrName )
{
	// Find the last "\" or "/" in the string and return everything after it.
	size_t index  = filePathOrName.rfind( _T( '\\' ) );
	size_t index2 = filePathOrName.rfind( _T( '/' ) );

	if ( index2 != std::string::npos && index2 > index )
	{
		index = index2;
	}

	if ( index == std::string::npos )
	{
		// No path sep, return the whole thing.
		return filePathOrName;
	}
	else if ( index + 1 >= filePathOrName.size() )
	{
		// Last char is path sep, no filename.
		return TString();
	}
	else
	{
		return filePathOrName.substr( index + 1, filePathOrName.size() - ( index + 1 ) );
	}
}

TString FileSys::AppendPath( const TString& left, const TString& right )
{
	if ( left.empty() )
	{
		return right;
	}

	TCHAR lastChar = left[ left.size() - 1 ];
	if ( lastChar == _T( '\\' ) || lastChar == _T( '/' ) )
	{
		return left + right;
	}
	else
	{
		return left + _T( "\\" ) + right;
	}
}


TString FileSys::ExtractRelativePath( const TString& basePath, const TString& fullPath )
{
	if ( basePath.size() >= fullPath.size() )
	{
		return TString();
	}

	if ( basePath != fullPath.substr( 0, basePath.size() ) )
	{
		return TString();
	}
	return fullPath.substr( basePath.size(), fullPath.size() - basePath.size() );
}

bool FileSys::DirectoryExists( const TString& path )
{
#ifdef _WIN32
	DWORD attributes = GetFileAttributes(path.c_str());

	if (attributes == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}
	else
	{
		return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}
#else
	struct stat info;
	if (stat(ToPathNativeStr(path).c_str(), &info) != 0) {
		return false;
	}
	return S_ISDIR(info.st_mode);
#endif
}


bool FileSys::FileExists(const TString& path)
{
#ifdef _WIN32
    DWORD attributes = GetFileAttributes(path.c_str());

    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }
    else
    {
        return (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    }
#else 
	struct stat info;
	if (stat(ToPathNativeStr(path).c_str(), &info) != 0) {
		return false;
	}
	return S_ISREG(info.st_mode);
#endif
}


bool FileSys::PathExists(const TString& path)
{
#ifdef _WIN32
    DWORD attributes = GetFileAttributes(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES;
#else
	struct stat info;
	return stat(ToPathNativeStr(path).c_str(), &info) == 0;
#endif
}



bool FileSys::RemovePath(const TString& path)
{
    if (path.empty() || path.length() > MAX_PATH)
    {
        return false;
    }

    //确保文件或者目录存在
    if (!PathExists(path))
        return true;

#ifdef _WIN32

    //目录的路径以2个\0结尾
    TString tmp_path = path;
    tmp_path.resize(tmp_path.size() + 2);

    SHFILEOPSTRUCT FileOp;
    ZeroMemory(&FileOp, sizeof(SHFILEOPSTRUCT));
    FileOp.fFlags |= FOF_SILENT;        /*不显示进度*/
    FileOp.fFlags |= FOF_NOERRORUI;        /*不报告错误信息*/
    FileOp.fFlags |= FOF_NOCONFIRMATION;/*直接删除，不进行确认*/
    FileOp.fFlags &= ~FOF_ALLOWUNDO; /*直接删除，不放入回收站*/
    FileOp.hNameMappings = NULL;
    FileOp.hwnd = NULL;
    FileOp.lpszProgressTitle = NULL;
    FileOp.wFunc = FO_DELETE;
    FileOp.pFrom = &tmp_path[0]; /*要删除的目录，必须以2个\0结尾*/
    FileOp.pTo = NULL;

    /*删除目录*/
    if (0 == SHFileOperation(&FileOp))
    {
        return true;
    }
    else
    {
        return false;
    }
#else
	if (DirectoryExists(path)) {
		std::string strPath = ToPathNativeStr(path);
		DIR* dir = opendir(strPath.c_str());
		if (dir == nullptr) {
			return false;
		}

		struct dirent* entry;
		while ((entry = readdir(dir)) != nullptr) {
			if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..") {
				continue;
			}
			TString subPath = AppendPath(ToWstring(strPath), ToWstring(entry->d_name));
			if (!RemovePath(subPath)) {
				closedir(dir);
				return false;
			}
		}
		closedir(dir);
		return rmdir(strPath.c_str()) == 0;
}
	else {
		return unlink(ToPathNativeStr(path).c_str()) == 0;
	}
#endif
}


bool FileSys::RenameFile(const TString& oldfile, const TString&newfile)
{
#ifdef _WIN32
    return MoveFileEx(oldfile.c_str(), newfile.c_str(), MOVEFILE_REPLACE_EXISTING) != FALSE;
#else
	return rename(ToPathNativeStr(oldfile).c_str(), ToPathNativeStr(newfile).c_str()) == 0;
#endif
}


bool FileSys::BackupFile(const TString&orignal, const TString&backup)
{
#ifdef _WIN32
    return MoveFileEx(orignal.c_str(), backup.c_str(), MOVEFILE_REPLACE_EXISTING) != FALSE
        || BackupFile(orignal.c_str(), backup.c_str()) != FALSE;
#else
	return rename(ToPathNativeStr(orignal).c_str(), ToPathNativeStr(backup).c_str()) == 0;
#endif
}

static TString path_without_extension(const TString& lpszOriginalPath)
{
    size_t pos_base_name = lpszOriginalPath.find_last_of(_T("/\\"));
    size_t pos_dot = lpszOriginalPath.find(_T('.'), pos_base_name);
    return lpszOriginalPath.substr(0, pos_dot);
}

static TString path_without_extension(const TString& lpszOriginalPath, TString& ext)
{
    size_t pos_base_name = lpszOriginalPath.find_last_of(_T("/\\"));
    size_t pos_dot = lpszOriginalPath.find(_T('.'), pos_base_name);
    ext =
        (pos_dot != TString::npos)
        ? lpszOriginalPath.substr(pos_dot) : _T("");

    return lpszOriginalPath.substr(0, pos_dot);
}

static TString get_date_time_str()
{
	time_t rawtime = time(nullptr);
	struct tm timeinfo;
#if defined(_WIN32)
	localtime_s(&timeinfo, &rawtime);
#else
	localtime_r(&rawtime, &timeinfo);
#endif

#ifdef _WIN32
	wchar_t buffer[80];
	wcsftime(buffer, sizeof(buffer) / sizeof(wchar_t), L"%Y%d%m-%H%M%S", &timeinfo);

#ifdef _UNICODE
	return buffer;
#else
	return ToString(buffer);
#endif
#else
	char buffer[80];
	strftime(buffer, sizeof(buffer), "%Y%d%m-%H%M%S", &timeinfo);
#ifdef _UNICODE
	return ToWstring(buffer);
#else
	return buffer;
#endif
#endif
}

static TString get_unique_path_with_dt(const TString&path)
{
    TString ext;
    TString base_name = path_without_extension(path, ext);

    // 找第一个不存在的路径
	TString save_path = path;
    while (FileSys::PathExists(save_path))
    {
        save_path = base_name + _T('~') + get_date_time_str() + ext;
    }
    return save_path;
}

TString FileSys::GetUniquePath(const TString& path)
{
    TString ext;
    TString base_name = path_without_extension(path, ext);

    // 找第一个不存在的路径
	TString save_path = path;
    while (PathExists(save_path))
    {
        save_path = base_name + _T('~') + get_date_time_str() + ext;
    }
    return save_path;
}

bool FileSys::IsDirectoryEmptyRecursive(const TString& path)
{
	IsEmptyCallback cb;
	PathScanner::Scan( path, cb );
	return cb.IsEmpty();
}

bool FileSys::CreateDirectoryTree( const TString& path )
{
#ifdef _WIN32
	int ret = SHCreateDirectoryEx( NULL, path.c_str(), NULL );
    return ret == ERROR_SUCCESS || ret == ERROR_ALREADY_EXISTS;
#else
	TString tempPath = path;
	size_t pos = 0;
	while ((pos = tempPath.find_first_of(_T('/'), pos + 1)) != std::string::npos) {
		TString subPath = tempPath.substr(0, pos);
		if (!DirectoryExists(subPath)) {
			if (mkdir(ToPathNativeStr(subPath).c_str(), 0755) != 0 && errno != EEXIST) {
				return false;
			}
		}
	}
	if (!DirectoryExists(tempPath)) {
		return mkdir(ToPathNativeStr(tempPath).c_str(), 0755) == 0 || errno == EEXIST;
	}
	return true;
#endif
}

std::vector< FilePathInfo > FileSys::GetFile( const TString& filePathOrName )
{
	TString path = FileSys::GetPath( filePathOrName );
	TString name = FileSys::GetFileName( filePathOrName );

	ScannerCallback cb( false, true );
	PathScanner::Scan( path, name, cb );
	return cb.GetFiles();
}

std::vector< FilePathInfo > FileSys::GetFilesInDirectory( const TString& directory, const TString& searchPattern, bool recursive )
{
	ScannerCallback cb( recursive );
	PathScanner::Scan( directory, searchPattern, cb );
	return cb.GetFiles();
}

}
}
