#pragma once
struct musicItem
{
	SStringT songname;
	SStringT artist;
	SStringT album;
	SStringT lasttime;
	SStringT songpath;
};

class SMusicListAdapter :public SMcAdapterBaseEx
{
	SArray<SStringW> m_colNames;
	SArray<musicItem> m_musicList;
	SArray<musicItem> m_musicListBackup;
	int m_id;
	int m_curren_play;
	HWND m_nofitywnd;
public:
	SMusicListAdapter(int id, HWND nofitywnd) :m_id(id), m_nofitywnd(nofitywnd), m_curren_play(0)
	{

		musicItem abc = { _T(""),_T("天天") ,_T("SOUI就是棒") ,_T("三百年前") };
		LPCTSTR pszSonger[] = { _T("刘德华"),_T("张学友"),_T("启程软件") };
		for (int i = 0; i < 1000; i++)
		{
			abc.songname.Format(_T("第%d个卖炭的"), i);
			abc.artist = pszSonger[i % 3];
			m_musicList.Add(abc);
		}
	}

	STDMETHOD_(int, getCount)() OVERRIDE
	{
		//SAutoLock autolock(updatalock);
		return m_musicList.GetCount();
	}
	void add1(musicItem& item)
	{
		m_musicList.Add(item);
		notifyDataSetInvalidated();
	}
	void add2(musicItem& item)
	{
		m_musicList.Add(item);
		notifyDataSetChanged();
	}
	virtual void WINAPI getView(int position, SItemPanel* pItem, SXmlNode xmlTemplate)
	{
		if (pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(&xmlTemplate);
		}
		pItem->FindChildByName(_T("songname"))->SetWindowText(m_musicList[position].songname);
		pItem->FindChildByName(_T("artist"))->SetWindowText(m_musicList[position].artist);
		pItem->FindChildByName(_T("album"))->SetWindowText(m_musicList[position].album);
		if ((pItem->GetState() & WndState_Check) || (pItem->GetState() & WndState_Hover))
		{
			SWindow* pChildItem = pItem->FindChildByName(_T("playbtn"));
			if (pChildItem)
			{
				pChildItem->SetVisible(TRUE, TRUE);
			}
		}
		pItem->GetEventSet()->subscribeEvent(EventSwndStateChanged::EventID, Subscriber(&SMusicListAdapter::OnItemStateChanged, this));
	}

	BOOL OnItemStateChanged(IEvtArgs* pEvt)
	{
		SWindow* pItem = sobj_cast<SWindow>(pEvt->Sender());
		if ((pItem->GetState() & WndState_Check) || (pItem->GetState() & WndState_Hover))
		{
			pItem = pItem->FindChildByName(_T("playbtn"));
			if (pItem)
			{
				pItem->SetVisible(TRUE, TRUE);
			}
		}
		else
		{
			pItem = pItem->FindChildByName(_T("playbtn"));
			if (pItem)
			{
				pItem->SetVisible(FALSE, TRUE);
			}
		}
		return true;
	}

	int GetColCount()
	{
		return m_colNames.GetCount();
	}

	virtual bool SwapItem(long pos, SList<long>& sellist) override
	{
		SAutoLock autolock(updatalock);
		SArray<musicItem> _temp_items;
		SArray<long> _remove_idxs;
		SPOSITION _pos = sellist.GetHeadPosition();
		size_t offsetpos = pos;
		while (_pos)
		{
			long posoffset = 0;
			long idx = sellist.GetNext(_pos);
			for (size_t i = 0; i < _remove_idxs.GetCount(); ++i)
			{
				if (_remove_idxs[i] < idx)
					--posoffset;
			}
			_remove_idxs.Add(idx);
			if (idx < pos)
				--offsetpos;
			idx += posoffset;
			_temp_items.Add(m_musicList[idx]);
			m_musicList.RemoveAt(idx);
		}
		m_musicList.InsertArrayAt(offsetpos, &_temp_items);
		//更新选中索引	
		_pos = sellist.GetHeadPosition();
		while (_pos)
		{
			sellist.SetAt(_pos, offsetpos++);
			sellist.GetNext(_pos);
		}
		return true;
	}

	STDMETHOD_(SStringW, GetColumnName)(int iCol) const {
		return m_colNames[iCol];
	}

	void IniColNames(SXmlNode xmlTemplate)
	{
		for (xmlTemplate = xmlTemplate.first_child(); xmlTemplate; xmlTemplate = xmlTemplate.next_sibling())
		{
			m_colNames.Add(xmlTemplate.attribute(L"name").value());
		}
	}

	virtual void WINAPI InitByTemplate(SXmlNode xmlTemplate)
	{
		IniColNames(xmlTemplate);
	}
	//同步添加方法
	void add(SStringT& filepath)
	{
		//SAutoLock lock(updatalock);
		musicItem item;
		item.songname = item.songpath = filepath;
		// 		libZPlay::TID3InfoExW id3_inf;
		// 		if (SLibZplay::getSingleton().GetFileId3(filepath, id3_inf))
		// 		{
		// 			item.songname = id3_inf.Title;
		// 		}
		m_musicList.Add(item);
		notifyDataSetChanged();
	}
	//异步添加，必须到主线程notifyDataSetChanged
	void addformthread(SStringT& filepath)
	{
		// 		musicItem item;
		// 		item.songpath = filepath;
		// 		libZPlay::TID3InfoExW id3_inf;
		// 		if (SLibZplay::getSingleton().GetFileId3(filepath, id3_inf))
		// 		{
		// 			item.songname = id3_inf.Title;
		// 			item.artist = id3_inf.Artist;
		// 			item.album = id3_inf.Album;
		// 			updatalock.Enter();
		// 			m_musicList.Add(item);
		// 			updatalock.Leave();
		// 		}
				//此处不应该postmessage，因为getview里的异步锁会可能让界面假死,因为这个消息可能会很多，加载时会使WM_TIMER无法执行，所以动画可能会失效。
				//SendMessage(m_nofitywnd, UPDATAMSG, m_id, NULL);
	}
private:
	SCriticalSection updatalock;
};