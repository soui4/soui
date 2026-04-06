#ifndef _TOOLBARMANAGER_H_
#define _TOOLBARMANAGER_H_

#include <control/SToolBar.h>
#include <souicoll.h>

class ResManger;

SNSBEGIN
class CXmlEditor;
class SSkinPool;

class CToolbarManager
{
public:
    CToolbarManager(SSkinPool* pSkinPool);
    ~CToolbarManager();

    // 工具栏初始化
    void InitWidgetToolbar(SWindow* pParent, CXmlEditor* pXmlEditor);
    void InitSkinToolbar(SWindow* pParent);
    void UpdateEditorToolbar(int editXmlType);

    // 工具栏事件处理
    void OnTbWidgetClick(IEvtArgs *e, CXmlEditor* pXmlEditor);
    void OnTbSkinClick(IEvtArgs *e, ResManger* pResManger, CXmlEditor* pXmlEditor);

    // 获取工具栏
    SToolBar* GetWidgetToolbar() const { return m_tbWidget; }
    SToolBar* GetSkinToolbar() const { return m_tbSkin; }

  private:
    struct IconInfo
    {
        int iIcon;
        SStringW strTxt;
        SStringW strTip;
    };

    SSkinPool* m_pSkinPool;
    SToolBar* m_tbWidget;
    SToolBar* m_tbSkin;
    SMap<SStringW, SStringW> m_mapWidget;

    // 辅助函数
    void LoadToolbarIcons(SXmlNode xmlIconSkin, ISkinObj*& pSkin);
};
SNSEND
#endif // _TOOLBARMANAGER_H_