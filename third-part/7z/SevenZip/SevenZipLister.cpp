#include "SevenZipLister.h"
#include "GUIDs.h"
#include "FileSys.h"
#include "ArchiveOpenCallback.h"
#include "ArchiveExtractCallback.h"
#include "InStreamWrapper.h"
#include "PropVariant2.h"
#include "UsefulFunctions.h"


namespace SevenZip
{

	using namespace intl;

	SevenZipLister::SevenZipLister()
		: SevenZipArchive()
	{
	}

	SevenZipLister::~SevenZipLister()
	{
	}

    bool SevenZipLister::ListArchive(ListCallback* callback)
	{
#if  defined(_UNICODE)
		FILE* fileStream = _wfopen(m_archivePath.c_str(), L"rb");
#else
		FILE* fileStream = fopen(m_archivePath.c_str(), "rb");
#endif
		if (fileStream == NULL) {
			return false;
		}

		return ListArchive(fileStream, callback);
	}

	bool SevenZipLister::ListArchive(FILE* archiveStream, ListCallback* callback)
	{
		CMyComPtr< IInArchive > archive = UsefulFunctions::GetArchiveReader(m_compressionFormat);
		CMyComPtr< InStreamWrapper > inFile = new InStreamWrapper(archiveStream);
		CMyComPtr< ArchiveOpenCallback > openCallback = new ArchiveOpenCallback();

		HRESULT hr = archive->Open(inFile, 0, openCallback);
		if (hr != S_OK)
		{
			return false;
		}

		// List command
		UInt32 numItems = 0;
		archive->GetNumberOfItems(&numItems);
		for (UInt32 i = 0; i < numItems; i++)
		{
			{
				// Get uncompressed size of file
				CPropVariant prop;
				archive->GetProperty(i, kpidSize, &prop);

				int size = prop.intVal;

				// Get name of file
				archive->GetProperty(i, kpidPath, &prop);

				//valid string? pass back the found value and call the callback function if set
				if (prop.vt == VT_BSTR) {
					WCHAR* path = prop.bstrVal;
					if (callback) {
						callback->OnFileFound(path, size);
					}
				}
			}
		}
		CPropVariant prop;
		archive->GetArchiveProperty(kpidPath,&prop);
		archive->Close();

		if (prop.vt == VT_BSTR) {
			WCHAR* path = prop.bstrVal;
			if (callback) {
				callback->OnListingDone(path);
			}
		}
		return true;
	}
}