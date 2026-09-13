#ifndef __SAPPDIR__H__
#define __SAPPDIR__H__

SNSBEGIN

/**
 * @class SAppDir
 * @brief Application directory management class
 */
class SOUI_EXP SAppDir {
  public:
    /**
     * @brief Constructor
     * @param hInst application instance handle
     */
    SAppDir(HINSTANCE hInst);

    /**
     * @brief Get the application directory path
     * @return application directory path string
     */
    const SStringT &AppDir();

  protected:
    SStringT m_strAppPath; /**<  Application directory path string */
};

/**
 * @brief Folder selection dialog
 * @param szPath stores the selected folder path
 * @param pszTitle dialog title
 * @param initPath initial path
 * @return TRUE if selection succeeded, FALSE if failed
 */
BOOL SOUI_EXP SSelectFolder(TCHAR szPath[MAX_PATH], LPCTSTR pszTitle, LPCTSTR initPath);

SNSEND

#endif /**< __SAPPDIR__H__ */