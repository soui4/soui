#pragma once

#include <SApp.h>
#include "FluentCompat.h"
#include "FluentWindowChrome.h"
#include "FluentPopup.h"
#include <control/SListView.h>
#include <control/SMCListView.h>
#include <control/STileView.h>
#include <control/SouiCtrls.h>
#include <core/SWnd.h>
#include <res.mgr/SUiDef.h>
#include <map>
#include <set>
#include <vector>

namespace SNS
{
namespace FluentDemo
{

// Only demo executables use this helper; the framework and system resources stay unchanged.
class WindowAccess : public SWindow {
  public:
    static ISkinObj *Background(SWindow *window)
    {
        SAutoRefPtr<ISkinObj> SWindow::*member = &WindowAccess::m_pBgSkin;
        return window->*member;
    }

    static void ReplayColorize(SWindow *window)
    {
        COLORREF SWindow::*member = &WindowAccess::m_crColorize;
        const COLORREF color = window->*member;
        if (color != 0)
        {
            // OnAttrSkin replaces the object but core OnColorize skips an
            // unchanged tint. Reapply only this demo window's existing tint.
            window->*member = 0;
            window->SSendMessage(UM_SETCOLORIZE, color);
        }
    }

    static void MarkDirty(SWindow *window)
    {
        void (SWindow::*markDirty)(bool) = &WindowAccess::MarkCacheDirty;
        (window->*markDirty)(true);
    }
};

// SOUI attribute lookup is case-insensitive. Preserve the authored spelling
// for delegate replay while matching insert/erase/read with the same contract.
struct AttributeNameLess
{
    bool operator()(const SStringW &left, const SStringW &right) const
    {
        return left.CompareNoCase(right) < 0;
    }
};

struct AttributeSnapshot
{
    SAutoRefPtr<IWindow> owner;
    std::map<SStringW, SStringW, AttributeNameLess> references;
};

// Shared only by this Theme's recorder/factory and its created controls. A
// destroyed/unbound native host clears the pointer before its tree disappears.
struct PopupOwner
{
    PopupOwner()
        : refs(1)
        , host(NULL)
    {
    }
    void AddRef()
    {
        ++refs;
    }
    void Release()
    {
        if (--refs == 0)
            delete this;
    }

    long refs;
    SHostWnd *host;
};

typedef SharedPtr<PopupOwner> PopupOwnerPtr;

class Attributes : public TObjRefImpl<IAttrStorage> {
  public:
    Attributes(IWindow *owner, IAttrStorage *delegate, PopupOwnerPtr popupOwner = PopupOwnerPtr())
        : m_owner(owner)
        , m_delegate(delegate)
        , m_focusOwner(NULL)
        , m_comboOwner(NULL)
        , m_popupOwner(popupOwner)
        , m_hasBackground(FALSE)
        , m_hasBorder(FALSE)
    {
        Instances().insert(this);
    }

    ~Attributes()
    {
        // SWindow destroys its attribute storage before its still-live event set.
        if (m_focusOwner)
        {
            MemberFunctionSlot<Attributes, IEvtArgs> focus = Subscriber(&Attributes::OnEditFocus, this);
            m_focusOwner->UnsubscribeEvent(EventSetFocus::EventID, &focus);
            m_focusOwner->UnsubscribeEvent(EventKillFocus::EventID, &focus);
        }
        if (m_comboOwner)
        {
            MemberFunctionSlot<Attributes, IEvtArgs> popup = Subscriber(&Attributes::OnComboPopup, this);
            m_comboOwner->UnsubscribeEvent(EventCBDropdown::EventID, &popup);
        }
        Instances().erase(this);
    }

    void STDMETHODCALLTYPE OnSetAttribute(const IStringW *name, const IStringW *value, BOOL handled)
    {
        if (m_delegate)
            m_delegate->OnSetAttribute(name, value, handled);
        const SStringW attribute(name);
        const SStringW text(value);
        if (handled && attribute.CompareNoCase(L"colorBkgnd") == 0)
            m_hasBackground = TRUE;
        if (handled && attribute.CompareNoCase(L"colorBorder") == 0)
            m_hasBorder = TRUE;
        if (text.Left(7) == L"@color/" || attribute.Right(4).CompareNoCase(L"skin") == 0)
            m_references[attribute] = text;
        else
            m_references.erase(attribute);
        if (handled && attribute.Right(4).CompareNoCase(L"skin") == 0)
        {
            if (SWindow *window = sobj_cast<SWindow>(m_owner))
                WindowAccess::ReplayColorize(window);
        }
        // Attribute storage also sees Combo editors created after Bind (grid
        // OPTION cells and virtual rows), unlike a one-off ordinary-tree walk.
        if (handled && m_popupOwner && !m_comboOwner)
        {
            if (SComboBase *combo = sobj_cast<SComboBase>(m_owner))
            {
                m_comboOwner = combo;
                MemberFunctionSlot<Attributes, IEvtArgs> popup = Subscriber(&Attributes::OnComboPopup, this);
                combo->SubscribeEvent(EventCBDropdown::EventID, &popup);
            }
        }
        // Focus events do not bubble through the parent tree. Bind to each
        // actual form edit, including ones created later in virtual panels.
        if (!m_focusOwner && attribute.CompareNoCase(L"skin") == 0 && (text == L"fluent_edit" || text == L"fluent_edit_focus"))
        {
            if (SEdit *edit = sobj_cast<SEdit>(m_owner))
            {
                m_focusOwner = edit;
                MemberFunctionSlot<Attributes, IEvtArgs> focus = Subscriber(&Attributes::OnEditFocus, this);
                edit->SubscribeEvent(EventSetFocus::EventID, &focus);
                edit->SubscribeEvent(EventKillFocus::EventID, &focus);
            }
        }
    }

    BOOL STDMETHODCALLTYPE OnGetAttribute(const IStringW *name, IStringW *value) const
    {
        // Preserve the original factory's attribute/read-priority contract.
        if (m_delegate && m_delegate->OnGetAttribute(name, value))
            return TRUE;
        std::map<SStringW, SStringW, AttributeNameLess>::const_iterator found = m_references.find(SStringW(name));
        if (found == m_references.end())
            return FALSE;
        value->Copy(&found->second);
        return TRUE;
    }

    static void Refresh()
    {
        std::vector<AttributeSnapshot> snapshots;
        for (std::set<Attributes *>::const_iterator it = Instances().begin(); it != Instances().end(); ++it)
        {
            Attributes *attributes = *it;
            AttributeSnapshot snapshot = { attributes->m_owner, attributes->m_references };
            if (SListBox *list = sobj_cast<SListBox>(attributes->m_owner))
            {
                if (SComboBox *combo = sobj_cast<SComboBox>(list->GetOwner()))
                {
                    // The framework's no-liststyle branch assigns these two
                    // colors directly, bypassing attribute storage. Replay only
                    // unassigned defaults; authored liststyle/literal colors win.
                    if (combo->GetListBox() == list)
                    {
                        if (!attributes->m_hasBackground)
                            snapshot.references[L"colorBkgnd"] = L"@color/thm_color";
                        if (!attributes->m_hasBorder)
                            snapshot.references[L"colorBorder"] = L"@color/thm_border";
                    }
                }
            }
            snapshots.push_back(snapshot);
        }
        // Keep owners alive and replay a snapshot: attribute callbacks may mutate the registry.
        for (std::vector<AttributeSnapshot>::const_iterator it = snapshots.begin(); it != snapshots.end(); ++it)
        {
            const AttributeSnapshot &snapshot = *it;
            for (std::map<SStringW, SStringW, AttributeNameLess>::const_iterator reference = snapshot.references.begin(); reference != snapshot.references.end(); ++reference)
                snapshot.owner->SetAttribute(reference->first, reference->second, FALSE);
            if (SWindow *window = sobj_cast<SWindow>(snapshot.owner))
                WindowAccess::MarkDirty(window);
            snapshot.owner->Invalidate();
        }
    }

  private:
    BOOL OnComboPopup(IEvtArgs *event)
    {
        SHostWnd *host = m_popupOwner ? m_popupOwner->host : NULL;
        if (!host || !m_comboOwner || !m_comboOwner->GetContainer() || m_comboOwner->GetContainer()->GetHostHwnd() != host->m_hWnd)
            return FALSE;
        return OnComboDropdown(event, host);
    }

    BOOL OnEditFocus(IEvtArgs *event)
    {
        if (event->Sender() == m_focusOwner)
        {
            SEdit *edit = m_focusOwner;
            // Borderless messaging/content edits and custom skins keep their
            // own focus behavior; only Fluent form fields use this ring.
            ISkinObj *skin = WindowAccess::Background(edit);
            if (!skin || (skin != GETSKIN(L"fluent_edit", edit->GetScale()) && skin != GETSKIN(L"fluent_edit_focus", edit->GetScale())))
                return FALSE;
            edit->SetAttribute(L"skin", event->GetID() == EventSetFocus::EventID ? L"fluent_edit_focus" : L"fluent_edit");
            WindowAccess::MarkDirty(edit);
            edit->Invalidate();
        }
        return FALSE;
    }

    static std::set<Attributes *> &Instances()
    {
        // Attribute callbacks and refresh execute on the owning UI thread.
        static std::set<Attributes *> instances;
        return instances;
    }

    IWindow *m_owner;
    SAutoRefPtr<IAttrStorage> m_delegate;
    std::map<SStringW, SStringW, AttributeNameLess> m_references;
    SEdit *m_focusOwner;
    SComboBase *m_comboOwner;
    PopupOwnerPtr m_popupOwner;
    BOOL m_hasBackground;
    BOOL m_hasBorder;
};

class AttributeFactory : public TObjRefImpl<IAttrStorageFactory> {
  public:
    explicit AttributeFactory(IAttrStorageFactory *delegate, PopupOwnerPtr popupOwner = PopupOwnerPtr())
        : m_delegate(delegate)
        , m_popupOwner(popupOwner)
    {
    }

    HRESULT STDMETHODCALLTYPE CreateAttrStorage(IWindow *owner, IAttrStorage **storage) const
    {
        if (!storage)
            return E_INVALIDARG;
        *storage = NULL;
        if (!owner)
            return E_INVALIDARG;
        SAutoRefPtr<IAttrStorage> delegate;
        if (m_delegate)
        {
            const HRESULT result = m_delegate->CreateAttrStorage(owner, &delegate);
            if (FAILED(result))
                return result;
        }
        *storage = new Attributes(owner, delegate, m_popupOwner);
        return S_OK;
    }

  private:
    SAutoRefPtr<IAttrStorageFactory> m_delegate;
    PopupOwnerPtr m_popupOwner;
};

class Theme {
  public:
    typedef void (*ChangedHandler)(void *context);

    Theme()
        : m_popupOwner(new PopupOwner())
        , m_root(NULL)
        , m_dark(FALSE)
        , m_loaded(FALSE)
        , m_changed(NULL)
        , m_changedContext(NULL)
    {
    }

    ~Theme()
    {
        Unbind();
        if (m_loaded)
        {
            GETUIDEF->PopSkinPool(m_skins[m_dark ? 1 : 0]);
            GETUIDEF->PopUiDefInfo(m_colors[m_dark ? 1 : 0]);
            SApplication &app = SApplication::getSingleton();
            if (app.GetAttrStorageFactory() == m_factory)
                app.SetAttrStorageFactory(m_previousFactory);
        }
    }

    BOOL Load()
    {
        if (m_loaded)
            return TRUE;
        SAutoRefPtr<IUiDefInfo> colors[2];
        SAutoRefPtr<ISkinPool> skins[2];
        SApplication &app = SApplication::getSingleton();
        for (int index = 0; index < 2; ++index)
        {
            SXmlDoc colorDocument, skinDocument;
            if (!app.LoadXmlDocment(colorDocument, index ? _T("values:theme_color_dark") : _T("values:theme_color_light")) || !app.LoadXmlDocment(skinDocument, index ? _T("values:fluent_dark") : _T("values:fluent_light")))
                return FALSE;
            SXmlNode colorRoot = colorDocument.root().child(L"UIDEF");
            SXmlNode skinRoot = skinDocument.root().child(L"skin");
            if (!colorRoot.child(L"color") || !skinRoot)
                return FALSE;
            colors[index].Attach(SUiDef::CreateUiDefInfo());
            if (!(colors[index]->Init2(&colorRoot, FALSE) & UDI_COLOR))
                return FALSE;
            skins[index].Attach(GETUIDEF->CreateSkinPool(TRUE));
            if (!skins[index]->LoadSkins(&skinRoot))
                return FALSE;
        }
        for (int index = 0; index < 2; ++index)
        {
            m_colors[index] = colors[index];
            m_skins[index] = skins[index];
        }
        m_previousFactory = app.GetAttrStorageFactory();
        m_factory.Attach(new AttributeFactory(m_previousFactory, m_popupOwner));
        app.SetAttrStorageFactory(m_factory);
        GETUIDEF->PushUiDefInfo(m_colors[0]);
        GETUIDEF->PushSkinPool(m_skins[0]);
        m_dark = FALSE;
        m_loaded = TRUE;
        return TRUE;
    }

    // Load before Create; bind after Create and before entering the application's message loop.
    BOOL Bind(SWindow *root)
    {
        if (!root || !m_loaded || !root->FindChildByName(L"fluent_theme_toggle"))
            return FALSE;
        if (m_root == root)
            return TRUE;
        Unbind();
        m_root = root;
        MemberFunctionSlot<Theme, IEvtArgs> toggle = Subscriber(&Theme::OnToggle, this);
        root->FindChildByName(L"fluent_theme_toggle")->SubscribeEvent(EventCmd::EventID, &toggle);
        MemberFunctionSlot<Theme, IEvtArgs> destroy = Subscriber(&Theme::OnDestroyed, this);
        root->SubscribeEvent(EventSwndDestroy::EventID, &destroy);
        return TRUE;
    }

    // Preserve XML content dimensions; decorate only already-layered hosts.
    BOOL Bind(SHostWnd &host)
    {
        if (!Bind(host.GetRoot()))
            return FALSE;
        m_popupOwner->host = &host;
        return m_chrome.Attach(host);
    }

    BOOL Switch(BOOL dark)
    {
        dark = dark != FALSE;
        if (!m_loaded)
            return FALSE;
        if (m_dark == dark)
            return TRUE;
        GETUIDEF->PopSkinPool(m_skins[m_dark ? 1 : 0]);
        GETUIDEF->PopUiDefInfo(m_colors[m_dark ? 1 : 0]);
        m_dark = dark;
        GETUIDEF->PushUiDefInfo(m_colors[m_dark ? 1 : 0]);
        GETUIDEF->PushSkinPool(m_skins[m_dark ? 1 : 0]);
        Refresh();
        if (m_changed)
            m_changed(m_changedContext);
        return TRUE;
    }

    // Replay after a demo updates its own semantic color domain (for example,
    // the main demo's persistent accent/background chooser), without a new tree.
    void Refresh()
    {
        Attributes::Refresh();
        if (m_root)
        {
            m_root->SDispatchMessage(UM_SETSCALE, m_root->GetScale(), 0);
            RefreshDetached(m_root);
            m_chrome.Refresh();
        }
    }

    BOOL IsDark() const
    {
        return m_dark;
    }
    void SetChangedCallback(ChangedHandler callback, void *context = NULL)
    {
        m_changed = callback;
        m_changedContext = context;
    }

  private:
    Theme(const Theme &);
    Theme &operator=(const Theme &);

    void Unbind()
    {
        m_popupOwner->host = NULL;
        m_chrome.Detach();
        if (!m_root)
            return;
        MemberFunctionSlot<Theme, IEvtArgs> destroy = Subscriber(&Theme::OnDestroyed, this);
        m_root->UnsubscribeEvent(EventSwndDestroy::EventID, &destroy);
        if (SWindow *button = m_root->FindChildByName(L"fluent_theme_toggle"))
        {
            MemberFunctionSlot<Theme, IEvtArgs> toggle = Subscriber(&Theme::OnToggle, this);
            button->UnsubscribeEvent(EventCmd::EventID, &toggle);
        }
        m_root = NULL;
    }

    BOOL OnToggle(IEvtArgs *)
    {
        return Switch(!m_dark);
    }

    BOOL OnDestroyed(IEvtArgs *event)
    {
        if (event->Sender() == m_root)
        {
            m_popupOwner->host = NULL;
            m_chrome.Detach(false);
            m_root = NULL;
        }
        return FALSE;
    }

    static void RefreshDetached(SWindow *window)
    {
        if (SComboBox *combo = sobj_cast<SComboBox>(window))
        {
            if (SListBox *list = combo->GetListBox())
                list->SDispatchMessage(UM_SETSCALE, window->GetScale(), 0);
        }
        if (SMCListView *list = sobj_cast<SMCListView>(window))
        {
            if (SHeaderCtrl *header = list->GetHeaderCtrl())
                header->SDispatchMessage(UM_SETSCALE, window->GetScale(), 0);
            if (IMcAdapter *adapter = list->GetAdapter())
            {
                for (int index = 0; index < adapter->getCount(); ++index)
                    if (SItemPanel *panel = list->GetItemPanel(index))
                        RefreshDetached(panel);
            }
        }
        if (SListView *list = sobj_cast<SListView>(window))
        {
            if (ILvAdapter *adapter = list->GetAdapter())
            {
                for (int index = 0; index < adapter->getCount(); ++index)
                    if (SItemPanel *panel = list->GetItemPanel(index))
                        RefreshDetached(panel);
            }
        }
        if (STileView *list = sobj_cast<STileView>(window))
        {
            if (ILvAdapter *adapter = list->GetAdapter())
            {
                for (int index = 0; index < adapter->getCount(); ++index)
                    if (SItemPanel *panel = list->GetItemPanel(index))
                        RefreshDetached(panel);
            }
        }
        for (SWindow *child = window->GetWindow(GSW_FIRSTCHILD); child; child = child->GetWindow(GSW_NEXTSIBLING))
            RefreshDetached(child);
        WindowAccess::ReplayColorize(window);
        WindowAccess::MarkDirty(window);
        window->Invalidate();
    }

    WindowChrome m_chrome;
    PopupOwnerPtr m_popupOwner;
    SWindow *m_root;
    BOOL m_dark;
    BOOL m_loaded;
    SAutoRefPtr<IAttrStorageFactory> m_previousFactory;
    SAutoRefPtr<IAttrStorageFactory> m_factory;
    SAutoRefPtr<IUiDefInfo> m_colors[2];
    SAutoRefPtr<ISkinPool> m_skins[2];
    ChangedHandler m_changed;
    void *m_changedContext;
};

} // namespace FluentDemo
} // namespace SNS
