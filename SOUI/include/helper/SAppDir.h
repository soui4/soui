#pragma once

SNSBEGIN
class SOUI_EXP SAppDir {
  public:
    SAppDir(HINSTANCE hInst);

    const SStringT &AppDir();

  protected:
    SStringT m_strAppPath;
};

SNSEND
