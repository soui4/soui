#ifndef __SAPPDIR__H__
#define __SAPPDIR__H__

SNSBEGIN
class SOUI_EXP SAppDir {
  public:
    SAppDir(HINSTANCE hInst);

    const SStringT &AppDir();

  protected:
    SStringT m_strAppPath;
};

SNSEND

#endif // __SAPPDIR__H__