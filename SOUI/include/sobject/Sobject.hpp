/**
* Copyright (C) 2014-2050
* All rights reserved.
*
* @file       Sobject.hpp
* @brief
* @version    v1.0
* @author     SOUI group
* @date       2014/08/01
*
* Describe    the base class used in SOUI, which provides type identify of class in runtime
*             and implement attributes dispatcher described in XML.
*/

#pragma once

#include <interface/sobject-i.h>
#include <string/strcpcvt.h>
#include <xml/SXml.h>

SNSBEGIN

#ifndef ENABLE_SOBJ_XML
#ifdef _DEBUG
#define ENABLE_SOBJ_XML 1
#else
#define ENABLE_SOBJ_XML 0
#endif
#endif

template<class T>
class SObjectImpl : public T
{
public:
	static void MarkAttributeHandled(SXmlAttr xmlAttr, bool bHandled)
	{
		xmlAttr.set_userdata(bHandled?1:0);
	}

	static bool IsAttributeHandled(SXmlAttr xmlAttr)
	{
		return xmlAttr.get_userdata()==1?true:false;
	}
	SObjectImpl():m_attrHandler(NULL){}

public:
	STDMETHOD_(BOOL,InitFromXml)(THIS_ IXmlNode * pXmlNode ) OVERRIDE
	{
		SXmlNode xmlNode(pXmlNode);

		if (!xmlNode) return FALSE;
#if ENABLE_SOBJ_XML
		{
			pXmlNode->ToString(&m_strXml);
		}
#endif

		//设置当前对象的属性

		for (SXmlAttr attr = xmlNode.first_attribute(); attr; attr = attr.next_attribute())
		{
			if (IsAttributeHandled(attr)) continue;   //忽略已经被预处理的属性
			SetAttribute(attr.name(), attr.value(), TRUE);
		}
		//调用初始化完成接口
		OnInitFinished(pXmlNode);
		return TRUE;
	}

	STDMETHOD_(LPCWSTR,GetName)(THIS) SCONST OVERRIDE
	{
		return NULL;
	}

	STDMETHOD_(LPCSTR,GetNameA)(THIS) SCONST OVERRIDE
	{
		static SStringA str;
		LPCWSTR pszName=GetName();
		if(pszName)
			str=S_CW2A(pszName);
		else
			str.Empty();
		return str.c_str();
	}

	STDMETHOD_(void,SetName)(THIS_ LPCWSTR pszName) OVERRIDE
	{
	}

	
	STDMETHOD_(int,GetID)(THIS) SCONST OVERRIDE
	{
		return 0;
	}

	STDMETHOD_(void,SetID)(THIS_ int nID) OVERRIDE
	{
	}

	STDMETHOD_(HRESULT,AfterAttribute)(THIS_ LPCWSTR strAttribName,LPCWSTR strValue, BOOL bLoading, HRESULT hr) OVERRIDE
	{
		UNREFERENCED_PARAMETER(strAttribName);
		UNREFERENCED_PARAMETER(strValue);
		UNREFERENCED_PARAMETER(bLoading);
		return hr;
	}

	STDMETHOD_(HRESULT,SetAttributeA)(THIS_ LPCSTR  pszAttr, LPCSTR  pszValue, BOOL bLoading) OVERRIDE
	{
		return SetAttribute(S_CA2W(pszAttr,CP_UTF8),S_CA2W(pszValue,CP_UTF8),bLoading);
	}

	STDMETHOD_(HRESULT,SetAttribute)(THIS_ LPCWSTR  pszAttr, LPCWSTR  pszValue, BOOL bLoading) OVERRIDE
	{
		SStringW strName(pszAttr),strValue(pszValue);
		if(m_attrHandler){
			HRESULT hr = m_attrHandler(this,&strName,&strValue,bLoading);
			if(SUCCEEDED(hr))
				return hr;
		}

		return SetAttribute(strName,strValue,bLoading);
	}

	STDMETHOD_(HRESULT,ISetAttribute)(THIS_ const IStringW *  strAttr, const IStringW *  strValue, BOOL bLoading) OVERRIDE
	{
		if(m_attrHandler){
			HRESULT hr = m_attrHandler(this,strAttr,strValue,bLoading);
			if(SUCCEEDED(hr))
				return hr;
		}

		return SetAttribute(SStringW(strAttr),SStringW(strValue),bLoading);
	}

	STDMETHOD_(LPCWSTR,GetObjectClass)(THIS_) SCONST OVERRIDE
	{
		return NULL;
	}

	STDMETHOD_(int,GetObjectType)(THIS)  SCONST OVERRIDE
	{
		return 0;
	}


	STDMETHOD_(BOOL,IsClass)(THIS_ LPCWSTR lpszName) SCONST OVERRIDE
	{
		UNREFERENCED_PARAMETER(lpszName);
		return FALSE;
	}

	STDMETHOD_(BOOL,GetAttribute)(THIS_ LPCWSTR strAttr, IStringW * pValue) SCONST OVERRIDE
	{
		UNREFERENCED_PARAMETER(strAttr);
		UNREFERENCED_PARAMETER(pValue);
		return FALSE;
	}

	/**
	* OnInitFinished
	* @brief    属性初始化完成处理接口
	* @param    IXmlNode * xmlNode --  属性节点
	* @return   void
	* Describe
	*/ 
	STDMETHOD_(void,OnInitFinished)(THIS_ IXmlNode * xmlNode) OVERRIDE
	{
		UNREFERENCED_PARAMETER(xmlNode);
	}

	STDMETHOD_(void, SetAttrHandler)(THIS_ FunAttrHandler attrHandler) OVERRIDE{
		m_attrHandler = attrHandler;
	}

public:
	virtual HRESULT SetAttribute(const SStringW &strAttr,const SStringW & strValue,BOOL bLoading)
	{
		return DefAttributeProc(strAttr, strValue, bLoading);
	}

	virtual HRESULT DefAttributeProc(const SStringW &strAttr,const SStringW & strValue,BOOL bLoading)
	{
		return E_FAIL;
	}


#if ENABLE_SOBJ_XML
public:
	SStringW m_strXml;  //<** XML字符串，用于在调试时观察对象*/
#endif//ENABLE_SOBJ_XML

protected:
	FunAttrHandler m_attrHandler;
};


typedef SObjectImpl<IObject> SObject;

SNSEND
