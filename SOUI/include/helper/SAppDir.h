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

SNSEND

#endif // __SAPPDIR__H__