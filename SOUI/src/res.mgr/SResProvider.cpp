//////////////////////////////////////////////////////////////////////////
//   File Name: sresprovider.cpp
// Description: Resource Provider
//////////////////////////////////////////////////////////////////////////
#include "souistd.h"
#include "res.mgr/SResProvider.h"
#ifdef _WIN32
#include <io.h>
#else
#include <sys/stat.h>
#include <dirent.h>
#endif
SNSBEGIN

//定义3种系统资源类型
const TCHAR KTypeBitmap[] = _T("BITMAP");
const TCHAR KTypeCursor[] = _T("CURSOR");
const TCHAR KTypeIcon[] = _T("ICON");
const TCHAR KTypeHtml[] = _T("HTML");

//////////////////////////////////////////////////////////////////////////
IBitmapS *SResLoadFromMemory::LoadImage(LPVOID pBuf, size_t size)
{
    IBitmapS *pImg = NULL;
    GETRENDERFACTORY->CreateBitmap(&pImg);
    if (!pImg)
        return NULL;
    HRESULT hr = pImg->LoadFromMemory((LPBYTE)pBuf, size);

    if (!SUCCEEDED(hr))
    {
        pImg->Release();
        pImg = NULL;
    }

    return pImg;
}

IImgX *SResLoadFromMemory::LoadImgX(LPVOID pBuf, size_t size)
{
    IImgX *pImgX = NULL;
    GETRENDERFACTORY->GetImgDecoderFactory()->CreateImgX(&pImgX);
    if (!pImgX)
        return NULL;
    if (0 == pImgX->LoadFromMemory(pBuf, size))
    {
        pImgX->Release();
        pImgX = NULL;
    }
    return pImgX;
}

#ifdef _WIN32
//////////////////////////////////////////////////////////////////////////
SResProviderPE::SResProviderPE()
    : m_hResInst(0)
    , m_bOwner(FALSE)
{
}

SResProviderPE::~SResProviderPE()
{
    if (m_bOwner)
    {
        FreeLibrary(m_hResInst);
    }
}

BOOL SResProviderPE::Init(WPARAM wParam, LPARAM lParam)
{
    if (lParam == 0)
    {
        m_hResInst = (HINSTANCE)wParam;
        m_bOwner = FALSE;
        return TRUE;
    }
    else
    {
        LPCTSTR pszPath = (LPCTSTR)wParam;
        m_hResInst = LoadLibrary(pszPath);
        if (!m_hResInst)
            return FALSE;
        m_bOwner = TRUE;
        return TRUE;
    }
}

HBITMAP SResProviderPE::LoadBitmap(LPCTSTR pszResName)
{
    return ::LoadBitmap(m_hResInst, pszResName);
}

HICON SResProviderPE::LoadIcon(LPCTSTR pszResName, int cx /*=0*/, int cy /*=0*/)
{
    return (HICON)::LoadImage(m_hResInst, pszResName, IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);
}

HCURSOR SResProviderPE::LoadCursor(LPCTSTR pszResName)
{
    HRSRC hRes = ::FindResource(m_hResInst, pszResName, RT_ANICURSOR);
    if (hRes)
    {
        DWORD dwSize = SizeofResource(m_hResInst, hRes);
        HGLOBAL hGlob = LoadResource(m_hResInst, hRes);
        LPBYTE pBytes = (LPBYTE)LockResource(hGlob);
        return (HCURSOR)CreateIconFromResource(pBytes, dwSize, FALSE, 0x00030000);
    }
    else
    {
        return ::LoadCursor(m_hResInst, pszResName);
    }
}

IBitmapS *SResProviderPE::LoadImage(LPCTSTR strType, LPCTSTR pszResName)
{
    if (!HasResource(strType, pszResName))
        return NULL;
    size_t szImgBuf = GetRawBufferSize(strType, pszResName);
    if (szImgBuf == 0)
        return NULL;
    LPVOID pBuf = GetRawBufferPtr(strType, pszResName);
    return SResLoadFromMemory::LoadImage(pBuf, szImgBuf);
}

IImgX *SResProviderPE::LoadImgX(LPCTSTR strType, LPCTSTR pszResName)
{
    if (!HasResource(strType, pszResName))
        return NULL;
    size_t szImgBuf = GetRawBufferSize(strType, pszResName);
    if (szImgBuf == 0)
        return NULL;
    LPVOID pBuf = GetRawBufferPtr(strType, pszResName);
    return SResLoadFromMemory::LoadImgX(pBuf, szImgBuf);
}

size_t SResProviderPE::GetRawBufferSize(LPCTSTR strType, LPCTSTR pszResName)
{
    HRSRC hRsrc = MyFindResource(strType, pszResName);

    if (NULL == hRsrc)
        return 0;

    return ::SizeofResource(m_hResInst, hRsrc);
}

BOOL SResProviderPE::GetRawBuffer(LPCTSTR strType, LPCTSTR pszResName, LPVOID pBuf, size_t size)
{
    SASSERT(strType);
    HRSRC hRsrc = MyFindResource(strType, pszResName);

    if (NULL == hRsrc)
        return FALSE;

    size_t dwSize = ::SizeofResource(m_hResInst, hRsrc);
    if (0 == dwSize)
        return FALSE;

    if (size < dwSize)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
    HGLOBAL hGlobal = ::LoadResource(m_hResInst, hRsrc);
    if (NULL == hGlobal)
        return FALSE;

    LPVOID pBuffer = ::LockResource(hGlobal);
    if (NULL == pBuffer)
        return FALSE;

    memcpy(pBuf, pBuffer, dwSize);

    ::FreeResource(hGlobal);

    return TRUE;
}

LPVOID SResProviderPE::GetRawBufferPtr(LPCTSTR strType, LPCTSTR pszResName)
{
    SASSERT(strType);
    HRSRC hRsrc = MyFindResource(strType, pszResName);

    if (NULL == hRsrc)
        return NULL;

    size_t dwSize = ::SizeofResource(m_hResInst, hRsrc);
    if (0 == dwSize)
        return NULL;

    HGLOBAL hGlobal = ::LoadResource(m_hResInst, hRsrc);
    if (NULL == hGlobal)
        return NULL;

    LPVOID pBuffer = ::LockResource(hGlobal);

    ::FreeResource(hGlobal);

    return pBuffer;
}

BOOL SResProviderPE::HasResource(LPCTSTR strType, LPCTSTR pszResName)
{
    if (!strType)
        return FALSE;
    return MyFindResource(strType, pszResName) != NULL;
}

HRSRC SResProviderPE::MyFindResource(LPCTSTR strType, LPCTSTR pszResName)
{
    if (_tcsicmp(strType, KTypeBitmap) == 0)
        strType = RT_BITMAP;
    else if (_tcsicmp(strType, KTypeIcon) == 0)
        strType = RT_GROUP_ICON;
    else if (_tcsicmp(strType, KTypeCursor) == 0)
        strType = RT_GROUP_CURSOR;
    else if (_tcsicmp(strType, KTypeHtml) == 0)
        strType = RT_HTML;

    HRSRC hRet = ::FindResource(m_hResInst, pszResName, strType);
    if (!hRet && strType == RT_GROUP_CURSOR)
        hRet = ::FindResource(m_hResInst, pszResName, RT_ANICURSOR);
    return hRet;
}

struct EnumResParam
{
    EnumResCallback fun;
    LPARAM lParam;
};

static BOOL CALLBACK EnumResNameProc(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam)
{
    EnumResParam *enumParam = (EnumResParam *)lParam;
    return enumParam->fun(lpszType, lpszName, enumParam->lParam);
}

static BOOL CALLBACK EnumResTypeProc(HMODULE hModule, LPTSTR lpszType, LONG_PTR lParam)
{
    return EnumResourceNames(hModule, lpszType, EnumResNameProc, lParam);
}

void SResProviderPE::EnumResource(EnumResCallback funEnumCB, LPARAM lp)
{
    EnumResParam param = { funEnumCB, lp };
    EnumResourceTypes(m_hResInst, EnumResTypeProc, (LONG_PTR)&param);
}

struct EnumFileParam
{
    EnumFileCallback fun;
    LPARAM lParam;
};
static BOOL CALLBACK EnumResFileProc(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam)
{
    EnumFileParam *enumParam = (EnumFileParam *)lParam;
    SStringT strPath = SStringT().Format(_T("%s:%s"), lpszType, lpszName);
    return enumParam->fun(strPath.c_str(), enumParam->lParam);
}

static BOOL CALLBACK EnumResTypeProc2(HMODULE hModule, LPTSTR lpszType, LONG_PTR lParam)
{
    return EnumResourceNames(hModule, lpszType, EnumResFileProc, lParam);
}
void SResProviderPE::EnumFile(THIS_ EnumFileCallback funEnumCB, LPARAM lp)
{
    EnumFileParam param = { funEnumCB, lp };
    EnumResourceTypes(m_hResInst, EnumResTypeProc2, (LONG_PTR)&param);
}

#endif //_WIN32
//////////////////////////////////////////////////////////////////////////
//

HBITMAP SResLoadFromFile::LoadBitmap(LPCTSTR strPath)
{
    return (HBITMAP)::LoadImage(NULL, strPath, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
}

HICON SResLoadFromFile::LoadIcon(LPCTSTR strPath, int cx /*=0*/, int cy /*=0*/)
{
    return (HICON)::LoadImage(NULL, strPath, IMAGE_ICON, cx, cy, LR_LOADFROMFILE);
}

HCURSOR SResLoadFromFile::LoadCursor(LPCTSTR strPath)
{
    return (HCURSOR)::LoadImage(NULL, strPath, IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE);
}

IBitmapS *SResLoadFromFile::LoadImage(LPCTSTR strPath)
{
    IBitmapS *pImg = NULL;
    GETRENDERFACTORY->CreateBitmap(&pImg);

    HRESULT hr = pImg->LoadFromFile(strPath);
    if (!SUCCEEDED(hr))
    {
        pImg->Release();
        pImg = NULL;
    }
    return pImg;
}

IImgX *SResLoadFromFile::LoadImgX(LPCTSTR strPath)
{
    IImgX *pImgX = NULL;
    GETRENDERFACTORY->GetImgDecoderFactory()->CreateImgX(&pImgX);
    if (!pImgX)
        return NULL;
#ifdef _UNICODE
    if (0 == pImgX->LoadFromFileW(strPath))
#else
    if (0 == pImgX->LoadFromFileA(strPath))
#endif
    {
        pImgX->Release();
        pImgX = NULL;
    }
    return pImgX;
}

size_t SResLoadFromFile::GetRawBufferSize(LPCTSTR strPath)
{
    return file_length(strPath);
}

BOOL SResLoadFromFile::GetRawBuffer(LPCTSTR strPath, LPVOID pBuf, size_t size)
{
    FILE *f = _tfopen(strPath, _T("rb"));
    if (!f)
        return FALSE;
#ifdef _WIN32
    size_t len = _filelength(_fileno(f));
#else
    fseek(f, 0, SEEK_END);
    size_t len = ftell(f);
    fseek(f, 0, SEEK_SET);
#endif
    if (len > size)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        fclose(f);
        return FALSE;
    }
    BOOL bRet = (len == fread(pBuf, 1, len, f));

    fclose(f);
    return bRet;
}

//////////////////////////////////////////////////////////////////////////
// SResProviderFiles
#ifdef _WIN32
#define kPath_Slash _T("\\")
#else
#define kPath_Slash _T("/")
#endif //_WIN32

SResProviderFiles::SResProviderFiles()
{
}

SStringT SResProviderFiles::GetRes(LPCTSTR strType, LPCTSTR pszResName)
{
    if (!strType)
    {
        // pszResName is relative path
        SStringT strRet = m_strPath + kPath_Slash + pszResName;
        DWORD dwAttr = GetFileAttributes(strRet);
        if (dwAttr == INVALID_FILE_ATTRIBUTES || (dwAttr & FILE_ATTRIBUTE_ARCHIVE) == 0)
            strRet = _T("");
        return strRet;
    }
    SResID resID(strType, pszResName);
    SMap<SResID, SStringT>::CPair *p = m_mapFiles.Lookup(resID);
    if (!p)
        return _T("");

    SStringT strRet = m_strPath + kPath_Slash + p->m_value;
    return strRet;
}

HBITMAP SResProviderFiles::LoadBitmap(LPCTSTR pszResName)
{
    SStringT strPath = GetRes(KTypeBitmap, pszResName);
    if (strPath.IsEmpty())
        return NULL;
    return SResLoadFromFile::LoadBitmap(strPath);
}

HICON SResProviderFiles::LoadIcon(LPCTSTR pszResName, int cx /*=0*/, int cy /*=0*/)
{
    SStringT strPath = GetRes(KTypeIcon, pszResName);
    if (strPath.IsEmpty())
        return NULL;
    return SResLoadFromFile::LoadIcon(strPath, cx, cy);
}

HCURSOR SResProviderFiles::LoadCursor(LPCTSTR pszResName)
{
    SStringT strPath = GetRes(KTypeCursor, pszResName);
    if (strPath.IsEmpty())
        return NULL;
    return SResLoadFromFile::LoadCursor(strPath);
}

IBitmapS *SResProviderFiles::LoadImage(LPCTSTR strType, LPCTSTR pszResName)
{
    SStringT strPath = GetRes(strType, pszResName);
    if (strPath.IsEmpty())
        return NULL;
    return SResLoadFromFile::LoadImage(strPath);
}

IImgX *SResProviderFiles::LoadImgX(LPCTSTR strType, LPCTSTR pszResName)
{
    SStringT strPath = GetRes(strType, pszResName);
    if (strPath.IsEmpty())
        return NULL;
    return SResLoadFromFile::LoadImgX(strPath);
}

size_t SResProviderFiles::GetRawBufferSize(LPCTSTR strType, LPCTSTR pszResName)
{
    SStringT strPath = GetRes(strType, pszResName);
    if (strPath.IsEmpty())
        return 0;
    return SResLoadFromFile::GetRawBufferSize(strPath);
}

BOOL SResProviderFiles::GetRawBuffer(LPCTSTR strType, LPCTSTR pszResName, LPVOID pBuf, size_t size)
{
    SStringT strPath = GetRes(strType, pszResName);
    if (strPath.IsEmpty())
        return FALSE;
    return SResLoadFromFile::GetRawBuffer(strPath, pBuf, size);
}

BOOL SResProviderFiles::Init(WPARAM wParam, LPARAM lParam)
{
    LPCTSTR pszPath = (LPCTSTR)wParam;

    SStringT strPathIndex = pszPath;
    strPathIndex += kPath_Slash;
    strPathIndex += UIRES_INDEX;

    SXmlDoc xmlDoc;
    SStringT strFileName;
    if (!xmlDoc.load_file(strPathIndex, xml_parse_default, enc_auto))
        return FALSE;

    SXmlNode xmlResource = xmlDoc.root().child(L"resource");
    if (!xmlResource)
        return FALSE;
    SXmlNode xmlType = xmlResource.first_child();
    while (xmlType)
    {
        SStringT strType = S_CW2T(xmlType.name());
        SXmlNode xmlFile = xmlType.child(L"file");
        while (xmlFile)
        {
            SResID id(strType, S_CW2T(xmlFile.attribute(L"name").value()));
            SStringT strFile = S_CW2T(xmlFile.attribute(L"path").value());
#if !defined(_WIN32)
            strFile.ReplaceChar(_T('\\'), _T('/'));
#endif
            //再次Init时会因为此行代码导致资源无法加载
            // if(!m_strPath.IsEmpty())
            // strFile.Format(_T("%s\\%s"),(LPCTSTR)m_strPath,(LPCTSTR)strFile);
            m_mapFiles[id] = strFile;
            xmlFile = xmlFile.next_sibling(L"file");
        }
        xmlType = xmlType.next_sibling();
    }
#ifdef _WIN32
    TCHAR szFullPath[1025];
    GetFullPathName(pszPath, 1024, szFullPath, NULL);
    m_strPath = szFullPath;
#else
    m_strPath = pszPath;
#endif //_WIN32
    return TRUE;
}

BOOL SResProviderFiles::HasResource(LPCTSTR strType, LPCTSTR pszResName)
{
    SStringT resPath = GetRes(strType, pszResName);
    return !resPath.IsEmpty();
}

void SResProviderFiles::EnumResource(EnumResCallback funEnumCB, LPARAM lp)
{
    SPOSITION pos = m_mapFiles.GetStartPosition();
    while (pos)
    {
        SResID id = m_mapFiles.GetNextKey(pos);
        if (!funEnumCB(id.szName, id.szType, lp))
            break;
    }
}

void SResProviderFiles::EnumFile(THIS_ EnumFileCallback funEnumCB, LPARAM lp)
{
    _EnumFile(NULL, funEnumCB, lp);
}

void SResProviderFiles::_EnumFile(LPCTSTR pszPath, EnumFileCallback funEnumCB, LPARAM lp)
{
#ifdef _WIN32
    WIN32_FIND_DATA wfd;
    SStringT strFilter;
    if (pszPath)
        strFilter = m_strPath + _T("\\") + pszPath + _T("\\*.*");
    else
        strFilter = m_strPath + _T("\\*.*");
    HANDLE hFind = FindFirstFile(strFilter.c_str(), &wfd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            SStringT strPath;
            if (pszPath == NULL)
                strPath = wfd.cFileName;
            else
                strPath = SStringT().Format(_T("%s\\%s"), pszPath, wfd.cFileName);
            if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (_tcscmp(wfd.cFileName, _T(".")) == 0 || _tcscmp(wfd.cFileName, _T("..")) == 0)
                    continue;
                _EnumFile(strPath.c_str(), funEnumCB, lp);
            }
            else if (wfd.dwFileAttributes & (FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY))
            {
                if (!funEnumCB(strPath.c_str(), lp))
                    break;
            }
        } while (FindNextFile(hFind, &wfd));
        FindClose(hFind);
    }
#else
    DIR *dir;
    struct dirent *entry;

    SStringT strFilter;
    if (pszPath)
        strFilter = m_strPath + _T("/") + pszPath;
    else
        strFilter = m_strPath;

    dir = opendir(pszPath); // 替换为需要枚举的文件夹路径
    if (dir == NULL)
    {
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        SStringT strPath;
        if (pszPath == NULL)
            strPath = entry->d_name;
        else
            strPath = SStringT().Format(_T("%s\\%s"), pszPath, entry->d_name);
        if (entry->d_type & DT_DIR)
        {
            if (_tcscmp(entry->d_name, _T(".")) == 0 || _tcscmp(entry->d_name, _T("..")) == 0)
                continue;
            _EnumFile(strPath.c_str(), funEnumCB, lp);
        }
        else if (entry->d_type & DT_REG)
        {
            if (!funEnumCB(strPath.c_str(), lp))
                break;
        }
    }

    closedir(dir);
#endif //_WIN32
}

SNSEND
