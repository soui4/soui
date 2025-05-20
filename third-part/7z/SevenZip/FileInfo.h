#pragma once
#include "SevenString.h"
#include "../CPP/Common/MyWindows.h"

namespace SevenZip
{
namespace intl
{
	struct FileInfo
	{
		TString		FileName;
		time_t		LastWriteTime;
		time_t		CreationTime;
		time_t		LastAccessTime;
		ULONGLONG	Size;
		UINT		Attributes;
		bool		IsDirectory;
	};

	struct FilePathInfo : public FileInfo
	{
		TString		FilePath;
	};
}
}
