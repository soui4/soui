#pragma once

SNSBEGIN
class SOUI_EXP SDpiAwareFont {
  public:
    SDpiAwareFont(void);
    ~SDpiAwareFont(void);

    IFontS *GetFontPtr();

    void SetFontDesc(const SStringW &strDesc, int nScale);

    void UpdateFont(int nScale);

  protected:
    IFontS *OnGetFont(const SStringW &strDesc, int nScale);

    SStringW m_strDesc;
    SAutoRefPtr<IFontS> m_object;
};

SNSEND
