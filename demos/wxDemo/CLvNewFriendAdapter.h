#pragma once

#include <helper/SAdapterBase.h>
#include <vector>


class CLvNewFriendAdapter : public SAdapterBase
{
public:
    struct ItemData
    {
        SStringW m_sstrID;
        SStringW m_sstrAvatar;
        SStringW m_sstrName;
        SStringW m_sstrMessage;
        int m_nState;
    };

    struct IListen
    {
        //virtual void OnMessageItemClick(int &nIndex) = 0;
    };

public:
    CLvNewFriendAdapter(SListView *pOwner, IListen *pListen)
    {
        m_pOwner = pOwner;
        SASSERT(m_pOwner);
        //m_pOwner->GetEventSet()->subscribeEvent(&CLvNewFriendAdapter::OnEventLvSelChangeing, this);

        m_pListen = pListen;
    }

    virtual int WINAPI getCount()
    {
        return static_cast<int>(m_vecItemDatas.size());
    }

    virtual void WINAPI getView(int position, SItemPanel *pItem, SXmlNode xmlTemplate)
    {
        if (0 == pItem->GetChildrenCount())
        {
            pItem->InitFromXml(&xmlTemplate);
            //pItem->GetEventSet()->subscribeEvent(EventItemPanelClick::EventID, Subscriber(&CLvNewFriendAdapter::OnEventItemPanelClick, this));
        }

        size_t sPos = static_cast<size_t>(position);
        if (sPos >= m_vecItemDatas.size())
            return;

        ItemData *pItemData = m_vecItemDatas[sPos];
        if (NULL == pItemData)
            return;

        // avatar
        SImageWnd *pAvatar = pItem->FindChildByName2<SImageWnd>(L"item_avatar");
        SASSERT(pAvatar);
        if (pItemData->m_sstrAvatar == L"")
        {
            // use default avatar
            pAvatar->SetAttribute(L"skin", L"skin_default_personal32");
        }
        else
        {
            // use default avatar
        }

        // name
         SStatic *pName = pItem->FindChildByName2<SStatic>(L"item_name");
         SASSERT(pName);
         pName->SetWindowText(S_CW2T(pItemData->m_sstrName));

        // message
         SStatic *pContent = pItem->FindChildByName2<SStatic>(L"item_message");
         SASSERT(pContent);
         pContent->SetWindowText(S_CW2T(pItemData->m_sstrMessage));

        //state
         SStatic *pTextState = pItem->FindChildByName2<SStatic>(L"text_state");
         SASSERT(pTextState);
         SImageButton *pBtnAccept = pItem->FindChildByName2<SImageButton>(L"btn_accept");
         SASSERT(pBtnAccept);
         switch (pItemData->m_nState)
         {
         case 0:
         {
             //接收到好友请求
             pTextState->SetAttribute(L"show", L"0");
             pBtnAccept->SetAttribute(L"show", L"1");
         }
         break;
         case 1:
         {
             //已添加
             pTextState->SetAttribute(L"show", L"1");
             pBtnAccept->SetAttribute(L"show", L"0");
             pTextState->SetWindowText(_T("已添加"));
         }
         break;
         case 2:
         {
             //己方发送的好友请求，等待验证
             pTextState->SetAttribute(L"show", L"1");
             pBtnAccept->SetAttribute(L"show", L"0");
             pTextState->SetWindowText(_T("等待验证"));
         }
         break;
         default:
             break;
         }
    }

    void AddItem(SStringW &sstrID, SStringW &sstrAvatar, SStringW &sstrName, SStringW &sstrMessage, int& nState)
    {
        ItemData *data = new ItemData;
        data->m_sstrID = sstrID;
        data->m_sstrAvatar = sstrAvatar;
        data->m_sstrName = sstrName;
        data->m_sstrMessage = sstrMessage;
        data->m_nState = nState;
        m_vecItemDatas.push_back(data);

        notifyDataSetChanged();
    }

    void DeleteAllItem()
    {
        m_vecItemDatas.clear();
        notifyDataSetChanged();
    }

    ItemData *GetItemData(int &nIndex)
    {
        return m_vecItemDatas[nIndex];
    }

    void EnsureVisible(int &nIndex)
    {
        m_pOwner->EnsureVisible(nIndex);
        notifyDataSetChanged();
    }

  private:
    std::vector<ItemData *> m_vecItemDatas; // 数据

    SListView *m_pOwner;
    IListen *m_pListen;
};
