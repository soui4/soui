#pragma once
#include <core/SSingleton2.h>
#include <interface/obj-ref-i.h>
#include <interface/SResProvider-i.h>

#include <res.mgr/SFontPool.h>
#include <res.mgr/SSkinPool.h>
#include <res.mgr/SStylePool.h>

#include <res.mgr/SObjDefAttr.h>
#include <res.mgr/SNamedValue.h>

SNSBEGIN

struct IUiDefInfo : IObjRef
{
    virtual SSkinPool *GetSkinPool() = 0;
    virtual SStylePool *GetStylePool() = 0;
    virtual STemplatePool *GetTemplatePool() = 0;

    virtual SNamedColor &GetNamedColor() = 0;
    virtual SNamedString &GetNamedString() = 0;
    virtual SNamedDimension &GetNamedDimension() = 0;
	virtual SNamedFont &GetNamedFont() = 0;

	//global settings.
	virtual SStringW GetDefFontInfo() = 0;
    virtual SXmlNode GetCaretInfo() = 0;
	virtual SObjDefAttr *GetObjDefAttr() = 0;
};

class SOUI_EXP SUiDefInfo : public TObjRefImpl<IUiDefInfo> {
public:
	enum{
		UDI_GLOBAL = 1<<0,
		UDI_SKIN = 1<<1,
		UDI_STYLE = 1<<2,
		UDI_TEMPLATE = 1<<3,
		UDI_COLOR   = 1<<4,
		UDI_STRING  = 1<<5,
		UDI_DIMENSION = 1<<6,
		UDI_FONT	  = 1<<7,
	};
public:
	SUiDefInfo()
	{
	}

	UINT Init(IResProvider *pResProvide, LPCTSTR pszUidef);
	UINT Init(IXmlNode *pNode,BOOL bGlobalDomain,IResProvider *pResProvider=NULL);

	virtual SSkinPool *GetSkinPool() override;
	virtual SStylePool *GetStylePool() override;
	virtual STemplatePool *GetTemplatePool() override;
	virtual SObjDefAttr *GetObjDefAttr() override;
	virtual SNamedColor &GetNamedColor() override;
	virtual SNamedString &GetNamedString() override;
	virtual SNamedDimension &GetNamedDimension() override;
	virtual SNamedFont &GetNamedFont() override;
	virtual SStringW GetDefFontInfo() override;
	virtual SXmlNode GetCaretInfo() override;
protected:
	SAutoRefPtr<SSkinPool> pSkinPool;
	SAutoRefPtr<SStylePool> pStylePool;
	SAutoRefPtr<SObjDefAttr> objDefAttr;
	SAutoRefPtr<STemplatePool> templatePool;

	SNamedColor namedColor;
	SNamedString namedString;
	SNamedDimension namedDim;
	SNamedFont namedFont;

	SStringW defFontInfo;
	SXmlDoc xmlCaret;
};

#define GETUIDEF SOUI::SUiDef::getSingletonPtr()

#define GETSTYLE(p) GETUIDEF->GetStyle(p)
#define GETSKIN(p1, scale)         GETUIDEF->GetSkin(p1, scale)
#define GETBUILTINSKIN(p1)         GETUIDEF->GetBuiltinSkin(p1, 100)
#define GETBUILTINSKIN2(p1, scale) GETUIDEF->GetBuiltinSkin(p1, scale)
#define GETCOLOR(x)					GETUIDEF->GetColor(x)
#define GETSTRING(x)				GETUIDEF->GetString(x)
#define GETLAYOUTSIZE(x)			GETUIDEF->GetLayoutSize(x)

class SOUI_EXP SUiDef : public SSingleton2<SUiDef> {
    SINGLETON2_TYPE(SINGLETON_UIDEF)
  public:
    SUiDef(void);
    ~SUiDef(void);

public:
	/**
     * InitDefUiDef
     * @brief    初始化默认UiDef
     * @param    IResProvider *pResProvider --  定义uidef的资源包
	 * @param    LPCTSTR pszUiDef --  定义uidef的资源ID
     * @return   BOOL, TRUE--成功
     * Describe
     */
	BOOL InitDefUiDef(IResProvider *pResProvider, LPCTSTR pszUiDef);

	/**
     * GetUiDef
     * @brief    获取默认UiDef
     * @return   IUiDefInfo * --默认uidef
     * Describe
     */
    IUiDefInfo *GetUiDef();

	/**
     * SetUiDef
     * @brief    设置默认UiDef
	 * @param    IUiDefInfo * --默认uidef
	 * @param    bool bUpdateDefFont --更新默认字体标志
     * @return   void
     * Describe
     */
    void SetUiDef(IUiDefInfo *pUiDefInfo, bool bUpdateDefFont);
public:
    /**
     * PushUiDefInfo
     * @brief    Push一个UiDef对象
	 * @param    IUiDefInfo * --新的uidef
     * @return   void
     * Describe soui的uidef对象是一个列表,每一个界面可以有自己的uidef对象
     */
	void PushUiDefInfo(IUiDefInfo *pUiDefInfo);

	/** PushUiDefInfo
	* @brief    Pop一个UiDef对象
	* @param    IUiDefInfo * --新的uidef
	* @return   BOOL, TRUE--成功
	* Describe soui的uidef对象是一个列表,每一个界面可以有自己的uidef对象
	*/
	BOOL PopUiDefInfo(IUiDefInfo *pUiDefInfo);

	/**
     * PushSkinPool
     * @brief    向列表中增加一个新的SSkinPool对象
     * @param    SSkinPool * pSkinPool --    SSkinPool对象
     * @return   void
     * Describe
     */
    void PushSkinPool(SSkinPool *pSkinPool);

    /**
     * PopSkinPool
     * @brief    弹出一个SSkinPool对象
     * @param    SSkinPool * pSkinPool --   准备弹出的SSkinPool对象
     * @return   BOOL
     * Describe  内建SkinPool不用调用PopSkinPool
     */
    BOOL PopSkinPool(SSkinPool *pSkinPool);
public:

	 /**
     * GetSkin
     * @brief    获得与指定name匹配的SkinObj
     * @param    LPCWSTR strSkinName --    Name of Skin Object
     * @return   ISkinObj*  -- 找到的Skin Object
     * Describe
     */
    ISkinObj *GetSkin(const SStringW &strSkinName, int nScale);

    /**
     * GetBuiltinSkin
     * @brief    获得SOUI系统内建的命名SkinObj
     * @param    SYS_SKIN uID --  内建SKIN的ID
     * @return   ISkinObj * 与SKINID对应的ISkinObj
     * Describe  可能返回失败
     */
    ISkinObj *GetBuiltinSkin(SYS_SKIN uID, int nScale);

    /**
     * GetBuiltinSkinPool
     * @brief    获得管理内建SkinPool对象
     * @return   SSkinPool * -- 内建SkinPool指针
     * Describe
     */
    SSkinPool *GetBuiltinSkinPool();

    /**
     * GetStyle
     * @brief    Get style object from pool by name
     * @param    const SStringW & strName --  name of style
     * @return   SXmlNode -- style xml
     * Describe
     */
    SXmlNode GetStyle(const SStringW & strName);

	/**
     * GetTemplateString
     * @brief    Get template string from pool by name
     * @param    const SStringW & strName --  name of template
     * @return   SStringW -- template content
     * Describe
     */
    SStringW GetTemplateString(const SStringW &strName);

	//从字符串返回颜色值，字符串可以是：@color/red
	//(red是在资源包中的颜色表定义的颜色名)，也可以是rgba(r,g,b,a)，也可以是rgb(r,g,b)，还可以是#ff0000(ff)这样的格式
	COLORREF GetColor(const SStringW &strColor);
	//获取资源包中的颜色表中固定索引号的颜色，只支持从默认资源包中查询
	COLORREF GetColor(int idx);

	//从字符串名返回字符串值，字符串可以是：@string/title
	//(title是在资源包中的字符串表定义的命名字符串)
	SStringW GetString(const SStringW &strString);
	//获取资源包中的字符串表中固定索引号的字符串，只支持从默认资源包中查询
	SStringW GetString(int idx);

	//从字符串名返回LayoutSize，字符串可以是：@dim/something
	//(something是在资源包中的Dim表定义的命名字符串)
	SLayoutSize GetLayoutSize(const SStringW &strSize);
	SLayoutSize GetLayoutSize(int idx);

	//从字符串名返回字体描述，字符串可以是：@font/something
	//(something是在资源包中的font表定义的命名字符串)
    SStringW GetFontDesc(const SStringW &strFont);
	SStringW GetFontDesc(int idx);
protected:
	SAutoRefPtr<IUiDefInfo> m_defUiDefInfo;
	SList<IUiDefInfo *> m_lstUiDefInfo;
    SList<SSkinPool *> m_lstSkinPools;
	SAutoRefPtr<SSkinPool> m_bulitinSkinPool;
	SCriticalSection	m_cs;
};

SNSEND
