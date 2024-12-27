/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserved.
 * 
 * @file       SCalendar.h
 * @brief      日历时间控件
 * @version    v1.0      
 * @author     soui      
 * @date       2014-05-25
 * 
 * Describe    此类完成日历控件
 */

#pragma once
#include "core/SWnd.h"

namespace SOUI
{
    /**
     * @class      SCalendar
     * @brief      日历类
     * 
     * Describe    此类是日历的核心类 大部分函数都是静态函数
     */
    class SCalendar2 : public SWindow
    {
    public:
        DEF_SOBJECT(SWindow, L"calendar2")
        

        /**
         * SCalendar::SCalendar
         * @brief    构造函数
         *
         * Describe  构造函数
         */
        SCalendar2();

    protected:

        /**
         * SCalendar::Load
         * @brief    加载xml
         * @param    SXmlNode xmlNode -- xml节点    
         *
         * Describe  通过加载xml来构造窗口
         */
        virtual BOOL WINAPI InitFromXml(IXmlNode * pNode);
    protected:


        /**
         * SCalendar::OnTodayClick
         * @brief    在日期---天的单击事件
         *
         * Describe  在日期---天的单击事件
         */            
        BOOL OnTodayClick(IEvtArgs *pArg);
        BOOL OnMonthClick(IEvtArgs *pArg);
        BOOL OnBtnNextMonth(IEvtArgs *pArg);
        BOOL OnBtnPrevMonth(IEvtArgs *pArg);
        BOOL OnBtnNext10Year(IEvtArgs *pArg);
        BOOL OnBtnPrev10Year(IEvtArgs *pArg);
        BOOL OnBtnPickMonthFinish(IEvtArgs *pArg);
        BOOL OnBtnPickMonthCancel(IEvtArgs *pArg);
        
        BOOL OnSetCalendarDate(IEvtArgs *pArg);
        void initYearButtons();
    protected:
        SCalendar   * m_pCalendar;
		SWindow     * m_pMonthPickerPanel;
		SWindow     * m_pCalenderPanel;
		SWindow     * m_pBtnPrevMonth,* m_pBtnNextMonth;
		SWindow     * m_pTxtMonth;
		
		int           m_iBeginYear;		
    };
}//end of namespace

