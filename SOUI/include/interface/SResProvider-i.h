/**
 * Copyright (C) 2014-2050
 * All rights reserved.
 *
 * @file       SResProvider-i.h
 * @brief
 * @version    v1.0
 * @author     SOUI group
 * @date       2014/08/02
 *
 * Describe
 */

 #ifndef _SRESPROVIDER_I_
 #define _SRESPROVIDER_I_
 
 #include <interface/obj-ref-i.h>
 #include <interface/SRender-i.h>
 
 #define UIRES_INDEX _T("uires.idx") //文件夹资源的文件映射表索引表文件名
 
 #ifdef _WIN32
 #define PATH_SLASH '\\'
 #else
 #define PATH_SLASH '/'
 #endif //_WIN32
 
 SNSBEGIN
 
 typedef enum BUILTIN_RESTYPE
 {
     RES_PE = 0,
     RES_FILE,
 } BUILTIN_RESTYPE;
 
 typedef BOOL(CALLBACK *EnumResCallback)(LPCTSTR pszType, LPCTSTR pszName, LPARAM lp);
 typedef BOOL(CALLBACK *EnumFileCallback)(LPCTSTR pszFileName, LPARAM lp);
 
 /**
  * @struct     IResProvider
  * @brief      ResProvider对象
  *
  * Describe  实现各种资源的加载
  */
 #undef INTERFACE
 #define INTERFACE IResProvider
 DECLARE_INTERFACE_(IResProvider, IObjRef)
 {
     /**
      * @brief 添加引用
      * @return long -- 引用计数
      */
     STDMETHOD_(long, AddRef)(THIS) PURE;
 
     /**
      * @brief 释放引用
      * @return long -- 引用计数
      */
     STDMETHOD_(long, Release)(THIS) PURE;
 
     /**
      * @brief 释放对象
      * @return void
      */
     STDMETHOD_(void, OnFinalRelease)(THIS) PURE;
 
     /**
      * @brief 资源初始化函数
      * @param wParam WPARAM -- 参数1
      * @param lParam LPARAM -- 参数2
      * @return BOOL -- TRUE: 成功
      * @remark 每个ResProvider必须实现此接口
      */
     STDMETHOD_(BOOL, Init)(THIS_ WPARAM wParam, LPARAM lParam) PURE;
 
     /**
      * @brief 查询一个资源是否存在
      * @param pszType LPCTSTR -- 资源类型
      * @param pszResName LPCTSTR -- 资源名称
      * @return BOOL -- TRUE: 存在，FALSE: 不存在
      */
     STDMETHOD_(BOOL, HasResource)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) PURE;
 
     /**
      * @brief 从资源中加载ICON
      * @param pszResName LPCTSTR -- ICON名称
      * @param cx int -- ICON宽度
      * @param cy int -- ICON高度
      * @return HICON -- 成功返回ICON的句柄，失败返回0
      */
     STDMETHOD_(HICON, LoadIcon)(THIS_ LPCTSTR pszResName, int cx, int cy) PURE;
 
     /**
      * @brief 从资源中加载HBITMAP
      * @param pszResName LPCTSTR -- BITMAP名称
      * @return HBITMAP -- 成功返回BITMAP的句柄，失败返回0
      */
     STDMETHOD_(HBITMAP, LoadBitmap)(THIS_ LPCTSTR pszResName) PURE;
 
     /**
      * @brief 从资源中加载光标
      * @param pszResName LPCTSTR -- 光标名
      * @return HCURSOR -- 成功返回光标的句柄，失败返回0
      * @remark 支持动画光标
      */
     STDMETHOD_(HCURSOR, LoadCursor)(THIS_ LPCTSTR pszResName) PURE;
 
     /**
      * @brief 从资源加载一个IBitmap对象
      * @param pszType LPCTSTR -- 图片类型
      * @param pszResName LPCTSTR -- 图片名
      * @return IBitmapS* -- 成功返回一个IBitmap对象，失败返回0
      * @remark 如果没有定义strType，则根据name使用FindImageType自动查找匹配的类型
      */
     STDMETHOD_(IBitmapS *, LoadImage)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) PURE;
 
     /**
      * @brief 从资源中创建一个IImgX对象
      * @param pszType LPCTSTR -- 图片类型
      * @param pszResName LPCTSTR -- 图片名
      * @return IImgX* -- 成功返回一个IImgX对象，失败返回0
      */
     STDMETHOD_(IImgX *, LoadImgX)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) PURE;
 
     /**
      * @brief 获得资源数据大小
      * @param pszType LPCTSTR -- 资源类型
      * @param pszResName LPCTSTR -- 资源名
      * @return size_t -- 资源大小（byte)，失败返回0
      */
     STDMETHOD_(size_t, GetRawBufferSize)(THIS_ LPCTSTR pszType, LPCTSTR pszResName) PURE;
 
     /**
      * @brief 获得资源内存块
      * @param pszType LPCTSTR -- 资源类型
      * @param pszResName LPCTSTR -- 资源名
      * @param pBuf LPVOID -- 输出内存块
      * @param size size_t -- 内存大小
      * @return BOOL -- TRUE: 成功
      * @remark 应该先用GetRawBufferSize查询资源大小再分配足够空间
      */
     STDMETHOD_(BOOL, GetRawBuffer)
     (THIS_ LPCTSTR pszType, LPCTSTR pszResName, LPVOID pBuf, size_t size) PURE;
 
     /**
      * @brief 枚举资源
      * @param funEnumCB EnumResCallback -- 枚举使用的回调函数
      * @param lp LPARAM -- 回调参数
      * @return void
      */
     STDMETHOD_(void, EnumResource)(THIS_ EnumResCallback funEnumCB, LPARAM lp) PURE;
 
     /**
      * @brief 枚举资源文件,返回文件路径
      * @param funEnumCB EnumFileCallback -- 枚举使用的回调函数
      * @param lp LPARAM -- 回调参数
      * @return void
      */
     STDMETHOD_(void, EnumFile)(THIS_ EnumFileCallback funEnumCB, LPARAM lp) PURE;
 };
 
 SNSEND
 
 #endif //_SRESPROVIDER_I_
 
 