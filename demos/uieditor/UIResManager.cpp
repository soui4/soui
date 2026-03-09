#include "stdafx.h"
#include "UIResManager.h"
#include <shlwapi.h>
#include <helper/slog.h>

#define kLogTag "UIResManager"

CUIResManager::CUIResManager(ResManger* pResManger)
    : m_pResManger(pResManger)
{
}

CUIResManager::~CUIResManager()
{
}

void CUIResManager::AddUires(HSTREEITEM hItem, STreeView* pTreeView, CFileTreeAdapter* pAdapter)
{
    BOOL bUpdated = FALSE;
    int selCount = pTreeView->GetSelItemCount();
    if (selCount)
    {
        HSTREEITEM *pItems = new HSTREEITEM[selCount];
        pTreeView->GetSelItems(pItems, selCount);
        // support multi select including sub directory
        for (int i = 0; i < selCount; i++)
        {
            const FileItemData &itemData = pAdapter->GetItemData(pItems[i]);

            if (itemData.bIsDir)
            {
                // 如果是目录，递归添加目录中的所有文件
                AddFilesInDirectoryToUIRes(itemData.strPath);
                bUpdated = TRUE;
            }
            else
            {
                // 如果是文件，直接添加到UIRes
                if (m_pResManger->AddToUIRes(itemData.strPath))
                    bUpdated = TRUE;
            }
        }
        delete[] pItems;
    }
    else
    {
        const FileItemData &itemData = pAdapter->GetItemData(hItem);
        bUpdated = m_pResManger->AddToUIRes(itemData.strPath);
    }
    
    // 这里可以添加UI更新逻辑，例如刷新XML编辑器
}

void CUIResManager::AddSkin(HSTREEITEM hItem, STreeView* pTreeView, CFileTreeAdapter* pAdapter)
{
    BOOL bUpdated = FALSE;
    int selCount = pTreeView->GetSelItemCount();
    std::vector<FileItemData> selectedItems;

    if (selCount)
    {
        HSTREEITEM *pItems = new HSTREEITEM[selCount];
        pTreeView->GetSelItems(pItems, selCount);
        for (int i = 0; i < selCount; i++)
        {
            const FileItemData &itemData = pAdapter->GetItemData(pItems[i]);
            if (!itemData.bIsDir && m_pResManger->GetFileType(itemData.strPath) == FT_IMAGE)
            {
                selectedItems.push_back(itemData);
            }
        }
        delete[] pItems;
    }
    else
    {
        const FileItemData &itemData = pAdapter->GetItemData(hItem);
        if (!itemData.bIsDir && m_pResManger->GetFileType(itemData.strPath) == FT_IMAGE)
        {
            selectedItems.push_back(itemData);
        }
    }

    if (!selectedItems.empty())
    {
        // 加载skin.xml文件
        pugi::xml_document &xmlDocSkin = m_pResManger->m_xmlDocSkin;
        pugi::xml_node xmlSkinRoot = xmlDocSkin.child(L"skins");
        if (!xmlSkinRoot)
        {
            xmlSkinRoot = xmlDocSkin.append_child(L"skins");
        }

        // 遍历选中的图片文件
        for (std::vector<FileItemData>::iterator it = selectedItems.begin(); it != selectedItems.end(); ++it)
        {
            FileItemData &itemData = *it;
            wchar_t szName[MAX_PATH], szExt[50];
            SStringW strPath = S_CT2W(itemData.strPath);
            _wsplitpath(strPath.c_str(), NULL, NULL, szName, szExt);

            SStringW strName = szName;

            // 解析文件名中的配置信息
            int nStates = 1, left = -1, top = -1, right = -1, bottom = -1;
            int nValues = 0;
            int nColorize = 1, nAutoFit = 1, nTile = 0, nVertical = 0, nFilter = 0;

            wchar_t* p = wcsrchr(szName, L'[');
            if (p)
            {
                *p = 0;
                strName = szName;
                nValues = swscanf(p + 1, L"%d{%d,%d,%d,%d}]", &nStates, &left, &top, &right, &bottom);

                const wchar_t* pszFind = wcsstr(p + 1, L"{ec=");
                if (pszFind) nColorize = _wtoi(pszFind + 4);
                pszFind = wcsstr(p + 1, L"{fit=");
                if (pszFind) nAutoFit = _wtoi(pszFind + 5);
                pszFind = wcsstr(p + 1, L"{filter=");
                if (pszFind) nFilter = _wtoi(pszFind + 8);
                pszFind = wcsstr(p + 1, L"{tile=");
                if (pszFind) nTile = _wtoi(pszFind + 6);
                pszFind = wcsstr(p + 1, L"{vert=");
                if (pszFind) nVertical = _wtoi(pszFind + 6);
            }

            // 从uires.idx中查询src路径
            SStringW strSrc;
            pugi::xml_node xmlNodeUiRes = m_pResManger->m_xmlDocUiRes.child(L"uires");
            if (xmlNodeUiRes)
            {
                pugi::xml_node fileNode = xmlNodeUiRes.child(strName.c_str());
                if (fileNode)
                {
                    SStringW strPath = fileNode.attribute(L"path").as_string();
                    wchar_t szPathName[MAX_PATH], szPathExt[50];
                    _wsplitpath(strPath, NULL, NULL, szPathName, szPathExt);
                    SStringW strPathName = szPathName;
                    // 构建src路径，格式为：section:name
                    pugi::xml_node parentNode = fileNode.parent();
                    if (parentNode && wcscmp(parentNode.name(), L"uires") != 0)
                    {
                        strSrc = parentNode.name();
                        strSrc += L":";
                        strSrc += strName;
                    }
                    else
                    {
                        strSrc = L"";
                        strSrc += L":";
                        strSrc += strName;
                    }
                }
            }

            if (!strSrc.IsEmpty())
            {
                // 检查是否已存在同名皮肤
                pugi::xml_node existingSkin = xmlSkinRoot.child(strName.c_str());
                if (existingSkin)
                {
                    xmlSkinRoot.remove_child(existingSkin);
                }

                // 根据解析结果添加相应的皮肤配置
                pugi::xml_node newSkin;
                if (strPath.EndsWith(L".9.png"))
                {
                    // Android .9.png 格式
                    newSkin = xmlSkinRoot.append_child(L"imgframe2");
                }
                else if (nValues == 0 || nValues == 1)
                {
                    // imglist
                    newSkin = xmlSkinRoot.append_child(L"imglist");
                }
                else if (nValues == 3 || nValues == 5)
                {
                    // imgframe
                    newSkin = xmlSkinRoot.append_child(L"imgframe");
                    SStringW strMargin;
                    strMargin.Format(L"%d,%d,%d,%d", left, top, right == -1 ? left : right, bottom == -1 ? top : bottom);
                    newSkin.append_attribute(L"margin") = strMargin;
                }

                if (newSkin)
                {
                    newSkin.append_attribute(L"name") = strName;
                    newSkin.append_attribute(L"src") = strSrc;
                    newSkin.append_attribute(L"states") = nStates;

                    // 设置各种可选属性
                    if (nColorize == 0) newSkin.append_attribute(L"enableColorize") = 0;
                    if (nAutoFit == 0) newSkin.append_attribute(L"autoFit") = 0;
                    if (nTile != 0) newSkin.append_attribute(L"tile") = 1;
                    if (nVertical != 0) newSkin.append_attribute(L"vertical") = 1;
                    switch (nFilter)
                    {
                    case 1: newSkin.append_attribute(L"filterLevel") = L"low"; break;
                    case 2: newSkin.append_attribute(L"filterLevel") = L"midium"; break;
                    case 3: newSkin.append_attribute(L"filterLevel") = L"high"; break;
                    }

                    bUpdated = TRUE;
                }
            }
        }

        if (bUpdated)
        {
            // 保存skin.xml文件
            xmlDocSkin.save_file(m_pResManger->m_strSkinFile, L"\t", pugi::format_default, pugi::encoding_utf8);
        }
    }
}

void CUIResManager::AddFilesInDirectoryToUIRes(const SStringT& dirPath)
{
    WIN32_FIND_DATA findData;
    HANDLE hFind;
    SStringT searchPath = dirPath;
    searchPath += _T("/*.*");

    hFind = FindFirstFile(searchPath, &findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // 是子目录，跳过"."和".."目录
                if (_tcscmp(findData.cFileName, _T(".")) != 0 &&
                    _tcscmp(findData.cFileName, _T("..")) != 0)
                {
                    SStringT subDirPath = dirPath;
                    subDirPath += _T("/");
                    subDirPath += findData.cFileName;
                    AddFilesInDirectoryToUIRes(subDirPath); // 递归处理子目录
                }
            }
            else
            {
                // 是文件，添加到UIRes
                SStringT filePath = dirPath;
                filePath += _T("/");
                filePath += findData.cFileName;
                AddFileToUIRes(filePath);
            }
        } while (FindNextFile(hFind, &findData));

        FindClose(hFind);
    }
}

void CUIResManager::AddFileToUIRes(const SStringT& filePath)
{
    m_pResManger->AddToUIRes(filePath);
}