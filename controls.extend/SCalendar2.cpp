/**
* Copyright (C) 2014-2050 SOUI团队
* All rights reserved.
*
* @file       SCalendar.cpp
* @brief      SCalendarCore以及SCalendar类源文件
* @version    v1.0
* @author     soui
* @date       2014-05-25
*
* Describe  时间控件相关函数实现
*/
#include "stdafx.h"
#include <control/SCalendar.h>
#include <helper/STime.h>
#include "SCalendar2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCalendar

namespace SOUI{


//子控件的名字
static const wchar_t * KName_Btn_Today          =    L"btn_today";
static const wchar_t * KName_Panel_Calendar     =    L"panel_calendar";
static const wchar_t * KName_Panel_Month_Picker =    L"panel_month_picker";
static const wchar_t * KName_Txt_Month          =    L"txt_month";
static const wchar_t * KName_Btn_Next_Month     =    L"btn_next_month";
static const wchar_t * KName_Btn_Prev_Month     =    L"btn_prev_month";
static const wchar_t * KName_Btn_Next_Year      =    L"btn_year_next";
static const wchar_t * KName_Btn_Prev_Year      =    L"btn_year_prev";
static const wchar_t * KName_Btn_Pick_Month_Finish =    L"btn_pick_month_finish";
static const wchar_t * KName_Btn_Pick_Month_Cancel =    L"btn_pick_month_cancel";
static const wchar_t * KName_Group_Years        =    L"group_years";
static const wchar_t * KName_Group_Months       =    L"group_months";

static const int KID_Month_Begin          = 1000;
static const int KID_Year_Begin           = 2000;

	//////////////////////////////////////////////////////////////////////////
	//    CCalendar
	//////////////////////////////////////////////////////////////////////////

	SCalendar2::SCalendar2()
	{
	}



	BOOL SCalendar2::InitFromXml(IXmlNode * pNode )
	{
		BOOL bLoad=__baseCls::InitFromXml(pNode);
		if(bLoad)
		{
			SWindow *pBtnToday=FindChildByName(KName_Btn_Today);
			if(pBtnToday)
			{
				pBtnToday->SetID(100);
				pBtnToday->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SCalendar2::OnTodayClick,this));
			}
			m_pCalenderPanel = FindChildByName(KName_Panel_Calendar);
            SASSERT(m_pCalenderPanel);
            m_pCalendar = m_pCalenderPanel->FindChildByName2<SCalendar>(L"wnd_calendar");
            SASSERT(m_pCalendar);            
            m_pCalendar->GetEventSet()->subscribeEvent(EventCalendarSetDate::EventID,Subscriber(&SCalendar2::OnSetCalendarDate,this));
			m_pMonthPickerPanel = FindChildByName(KName_Panel_Month_Picker);
			SASSERT(m_pMonthPickerPanel);
			m_pMonthPickerPanel->SetVisible(FALSE);
			
			m_pTxtMonth = FindChildByName(KName_Txt_Month);
			m_pBtnNextMonth=FindChildByName(KName_Btn_Next_Month);
			m_pBtnPrevMonth=FindChildByName(KName_Btn_Prev_Month);
			SASSERT(m_pTxtMonth && m_pBtnNextMonth && m_pBtnPrevMonth);
			m_pBtnNextMonth->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SCalendar2::OnBtnNextMonth,this));
			m_pBtnPrevMonth->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SCalendar2::OnBtnPrevMonth,this));
			m_pTxtMonth->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SCalendar2::OnMonthClick,this));
			SStringT strMonth = SCalendarCore::FormatYear(m_pCalendar->GetYear()) +_T("|")+ SCalendarCore::FormatMonth(m_pCalendar->GetMonth());
			m_pTxtMonth->SetWindowText(strMonth);
			
			SWindow *pBtnNext10Year = m_pMonthPickerPanel->FindChildByName(KName_Btn_Next_Year);
			pBtnNext10Year->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SCalendar2::OnBtnNext10Year,this));
            SWindow *pBtnPrev10Year = m_pMonthPickerPanel->FindChildByName(KName_Btn_Prev_Year);
            pBtnPrev10Year->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SCalendar2::OnBtnPrev10Year,this));
            SWindow *pBtnPickMonthFinish = m_pMonthPickerPanel->FindChildByName(KName_Btn_Pick_Month_Finish);
            SASSERT(pBtnPickMonthFinish);
            pBtnPickMonthFinish->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SCalendar2::OnBtnPickMonthFinish,this));
            SWindow *pBtnPickMonthCancel = m_pMonthPickerPanel->FindChildByName(KName_Btn_Pick_Month_Cancel);
            SASSERT(pBtnPickMonthCancel);
            pBtnPickMonthCancel->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SCalendar2::OnBtnPickMonthCancel,this));
            
		}
		return bLoad;
	}

	BOOL SCalendar2::OnTodayClick( IEvtArgs *pArg)
	{
		STime today=STime::GetCurrentTime();
		m_pCalendar->SetDate(today.GetYear(),today.GetMonth(),today.GetDay());
		return true;
	}

    BOOL SCalendar2::OnMonthClick(IEvtArgs *pArg)
    {
        m_pCalenderPanel->SetVisible(FALSE);
        m_pMonthPickerPanel->SetVisible(TRUE,TRUE);
        
        m_iBeginYear = m_pCalendar->GetYear()/10*10;
        initYearButtons();
        m_pMonthPickerPanel->FindChildByName(KName_Group_Years)->FindChildByID(KID_Year_Begin+m_pCalendar->GetYear()-m_iBeginYear)->SetCheck(TRUE);
        m_pMonthPickerPanel->FindChildByName(KName_Group_Months)->FindChildByID(KID_Month_Begin+m_pCalendar->GetMonth()-1)->SetCheck(TRUE);
        return true;
    }

    BOOL SCalendar2::OnBtnNextMonth(IEvtArgs *pArg)
    {
        int iYear = m_pCalendar->GetYear();
        int iMonth = m_pCalendar->GetMonth();
        iMonth ++;
        if(iMonth>12) iMonth = 1,iYear++;
        m_pCalendar->SetDate(iYear,iMonth,0);
        return true;
    }

    BOOL SCalendar2::OnBtnPrevMonth(IEvtArgs *pArg)
    {
        int iYear = m_pCalendar->GetYear();
        int iMonth = m_pCalendar->GetMonth();
        iMonth --;
        if(iMonth==0) iMonth = 12,iYear--;
        m_pCalendar->SetDate(iYear,iMonth,0);
        return true;
    }

    BOOL SCalendar2::OnBtnNext10Year(IEvtArgs *pArg)
    {
        m_iBeginYear += 10;
        initYearButtons();
        return true;
    }

    BOOL SCalendar2::OnBtnPrev10Year(IEvtArgs *pArg)
    {
        m_iBeginYear -= 10;
        initYearButtons();
        return true;
    }

    void SCalendar2::initYearButtons()
    {
        for(int i=0;i<10;i++)
        {
            SStringT strYear = SStringT().Format(_T("%d"),m_iBeginYear+i);
            m_pMonthPickerPanel->FindChildByID(KID_Year_Begin+i)->SetWindowText(strYear);
        }
    }

    BOOL SCalendar2::OnBtnPickMonthFinish(IEvtArgs *pArg)
    {
        m_pMonthPickerPanel->SetVisible(FALSE,TRUE);
        m_pCalenderPanel->SetVisible(TRUE);
        int iYear = m_iBeginYear + m_pMonthPickerPanel->FindChildByName(KName_Group_Years)->GetSelectedChildInGroup()->GetID()-KID_Year_Begin;
        int iMonth = m_pMonthPickerPanel->FindChildByName(KName_Group_Months)->GetSelectedChildInGroup()->GetID()-KID_Month_Begin+1;
        m_pCalendar->SetDate(iYear,iMonth,1);
        return true;
    }

    BOOL SCalendar2::OnBtnPickMonthCancel(IEvtArgs *pArg)
    {
        m_pMonthPickerPanel->SetVisible(FALSE,TRUE);
        m_pCalenderPanel->SetVisible(TRUE);
        return true;
    }

    BOOL SCalendar2::OnSetCalendarDate(IEvtArgs *pArg)
    {
        SStringT strMonth = SCalendarCore::FormatYear(m_pCalendar->GetYear()) +_T("|")+ SCalendarCore::FormatMonth(m_pCalendar->GetMonth());
        m_pTxtMonth->SetWindowText(strMonth);
        return true;
    }

}//end of namespace
