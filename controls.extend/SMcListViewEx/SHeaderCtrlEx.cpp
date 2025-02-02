#include "stdafx.h"
#include "SHeaderCtrlEx.h"
#include <commctrl.h>

namespace SOUI
{


    SHeaderCtrlEx::SHeaderCtrlEx(void)
		:m_bMatchParent(FALSE), m_bNewSize(true), m_crLine(RGBA(188, 188, 188, 128)), m_bDrawSplitLine(TRUE), m_Offset(4,4)
    {

    }

	bool SHeaderCtrlEx::isViewWidthMatchParent() const
	{
		return TRUE == m_bMatchParent;
	}

	//第二个参数表明是否拉伸

	void SHeaderCtrlEx::Move(LPCRECT prect, bool _newSize)
	{
		TestMainThread();
		if (prect)
		{
			m_bNewSize = _newSize;
			m_bFloat = TRUE;//使用Move后，程序不再自动计算窗口坐标
			OnRelayout(prect);
			m_bNewSize = false;
		}
		else if (GetParent())
		{
			//恢复自动计算位置
			m_bFloat = FALSE;
			//重新计算自己及兄弟窗口的坐标
			RequestRelayout();
		}
	}

	void SHeaderCtrlEx::UpdataWidth()
	{
		//更新不同步
		SASSERT(m_widItems.GetCount() == m_arrItems.GetCount());
		int totalWidth = 0;
		for (int i = 0; i < m_widItems.GetCount(); i++)
		{
			totalWidth += m_widItems[i];
		}
		int parentViewWid = GetWindowRect().Width();
		int remainingWid = parentViewWid;
		int nozeroPos = 0;//找到第一个不为0的项
		for (int i = 0; i < m_arrItems.GetCount(); i++)
		{
			//跳过为0的项
			if (m_widItems[i] == 0)
				continue;
			nozeroPos = i;
			break;
		}
		for (int i = m_arrItems.GetCount() - 1; i > nozeroPos; i--)
		{
			//跳过为0的项
			if (m_widItems[i] == 0)
				continue;
			int itemwid = m_widItems[i] * parentViewWid / totalWidth;
			m_arrItems[i].cx = itemwid;
			remainingWid -= itemwid;
		}
		//因为除法不一定能整除，把最后余下的全给第一个不为0的子项
		m_arrItems[nozeroPos].cx = remainingWid;
	}

	void SHeaderCtrlEx::OnSize(UINT nType, CSize size)
	{
		if (isViewWidthMatchParent()&&m_bNewSize)
			UpdataWidth();
	}

	BOOL SHeaderCtrlEx::CreateChildren(SXmlNode xmlNode)
	{
		SXmlNode xmlItems = xmlNode.child(L"items");
		if (xmlItems)
			xmlItems.set_userdata(1);
		SWindow::CreateChildren(xmlNode);
		if (!xmlItems) return FALSE;
		SXmlNode xmlItem = xmlItems.child(L"item");
		LockUpdate();

		while (xmlItem)
		{
			SStringW strText = xmlItem.Text();
			strText.TrimBlank();
			strText = tr(GETSTRING(strText));
			SStringT strText2 = S_CW2T(strText);
			int cx = xmlItem.attribute(L"width").as_int(50);
			if (isViewWidthMatchParent())
			{
				m_widItems.InsertAt(m_arrItems.GetCount(), cx);
			}
			LPARAM lParam = xmlItem.attribute(L"userData").as_uint(0);
			UINT fmt = 0;
			SStringW strSort = xmlItem.attribute(L"sortFlag").as_string();
			strSort.MakeLower();
			if(strSort==L"down")
				fmt |= HDF_SORTDOWN;
			else if(strSort == L"up")
				fmt |= HDF_SORTUP;
			SStringW strAlign = xmlItem.attribute(L"align").as_string();
			strAlign.MakeLower();
			if(strAlign == L"left")
				fmt |= HDF_LEFT;
			else if(strAlign == L"center")
				fmt |= HDF_CENTER;
			else if(strAlign == L"right")
				fmt |= HDF_RIGHT;

			InsertItem(-1, strText2, cx, fmt, lParam);

			xmlItem = xmlItem.next_sibling(L"item");
		}
		UnlockUpdate();
		return TRUE;
	}
	BOOL SHeaderCtrlEx::OnSetCursor(const CPoint & pt)
	{
		if (m_bFixWidth) return FALSE;
		DWORD dwHit = HitTest(pt);
		if(dwHit==-1) return FALSE;
		if (IsItemHover(dwHit))
			return SWindow::OnSetCursor(pt);
		HCURSOR hCursor = GETRESPROVIDER->LoadCursor(IDC_SIZEWE);
		SetCursor(hCursor);
		return TRUE;
	}
	void SHeaderCtrlEx::OnPaint(IRenderTarget* pRT)
	{
		SPainter painter;
		BeforePaint(pRT, painter);
		CRect rcClient;
		GetClientRect(&rcClient);
		CRect rcItem(rcClient.left, rcClient.top, rcClient.left, rcClient.bottom);
		bool bFirstItem = true;
		for (UINT i = 0; i < m_arrItems.GetCount(); i++)
		{
			if (!m_arrItems[i].bVisible) continue;
			rcItem.left = rcItem.right;
			rcItem.right = rcItem.left + m_arrItems[i].cx;
			DrawItem(pRT, rcItem, m_arrItems.GetData() + i);
			if ((!bFirstItem) && m_bDrawSplitLine)
			{
				SAutoRefPtr<IPenS> curPen, oldPen;
				pRT->CreatePen(PS_SOLID, m_crLine, 1, &curPen);
				pRT->SelectObject(curPen, (IRenderObj * *)& oldPen);
				CPoint pts[2];
				pts[0] = rcItem.TopLeft();
				pts[1].x = pts[0].x;
				pts[0].y += m_Offset.x;
				pts[1].y = rcItem.bottom - m_Offset.y;
				pRT->DrawLines(pts, 2);
				pRT->SelectObject(oldPen,NULL);
			}
			if (bFirstItem)
				bFirstItem = false;
			if (rcItem.right >= rcClient.right) break;
		}
		if (rcItem.right < rcClient.right)
		{
			rcItem.left = rcItem.right;
			rcItem.right = rcClient.right;
			if (m_pSkinItem) m_pSkinItem->DrawByIndex(pRT, rcItem, 3);
		}
		AfterPaint(pRT, painter);
	}
}//end of namespace SOUI
