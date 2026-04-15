#ifndef __SAPPDIR__H__
#define __SAPPDIR__H__

SNSBEGIN

/**
 * @class SAppDir
 * @brief 应用程序目录管理类
 */
class SOUI_EXP SAppDir {
  public:
    /**
     * @brief 构造函数
     * @param hInst 应用程序实例句柄
     */
    SAppDir(HINSTANCE hInst);

    /**
     * @brief 获取应用程序目录路径
     * @return 应用程序目录路径字符串
     */
    const SStringT &AppDir();

  protected:
    SStringT m_strAppPath; ///< 应用程序目录路径字符串
};

/**
 * @brief 选择文件夹对话框
 * @param szPath 存储选择的文件夹路径
 * @param pszTitle 对话框标题
 * @param initPath 初始路径
 * @return TRUE 选择成功，FALSE 选择失败
 */
BOOL SOUI_EXP SSelectFolder(TCHAR szPath[MAX_PATH],LPCTSTR pszTitle, LPCTSTR initPath);

SNSEND

#endif // __SAPPDIR__H__