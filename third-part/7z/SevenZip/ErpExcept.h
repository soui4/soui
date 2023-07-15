#ifndef __ERPEXCEPTION_H
#define __ERPEXCEPTION_H

/********************************************************************
  Class Name: ERPException
  Created Date:20090805
  Version 2.0
  Author: wuding
  Definition file: ErpExcept.cpp
  Last Version Date: 20091105
  Version Date:
  vserion 2.0  加入异常级别处理，方便前后台对异常进行分类
********************************************************************/ 
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <map>    
#include <stdarg.h> 

//
const int SYSTEM_ERROR = 1;

#define MAX_EXCEPTION_LEN  4*1024

enum EGrade { EERROR=0,EWARNING,EHINT};


class ERPException
{
    public:            
        explicit ERPException(EGrade nGrade,const char *pFormat,...);
        explicit ERPException(EGrade nGrade, int nCode, const char *pFormat,...);
        ~ERPException();
    public:
        EGrade  GetErrorGrade() ;
        int   GetErrorCode();
		const char *GetErrorMsg();
        const std::wstring GetErrorMsgW(); 
        const std::string  GetErrorMsgA(); 
    private:
        EGrade mGrade;             //错误等级
        int nErrCode;               //错误编号
        char m_str[MAX_EXCEPTION_LEN];            //错误说明 
};

#endif
