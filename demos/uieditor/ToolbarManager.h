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
    SStringW GetWidgetContent(const SStringT &strText) const
    {
        const SMap<SStringT, SStringW>::CPair *pair = m_mapWidget.Lookup(strText);
        if (pair)
        {
            return pair->m_value;
        }
        else
        {
            return SStringW();
        }
    }

  private:
    struct IconInfo
    {
        int iIcon;
        SStringT strTxt;
        SStringT strTip;
    };

    SSkinPool* m_pSkinPool;
    SToolBar* m_tbWidget;
    SToolBar* m_tbSkin;
    SMap<SStringT, SStringW> m_mapWidget;

    // 辅助函数
    void LoadToolbarIcons(SXmlNode xmlIconSkin, ISkinObj*& pSkin);
};
SNSEND
#endif // _TOOLBARMANAGER_H_