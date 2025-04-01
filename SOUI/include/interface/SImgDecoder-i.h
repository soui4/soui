/**
 * Copyright (C) 2014-2050 SOUI group
 * All rights reserved.
 *
 * @file       imgdecoder-i.h
 * @brief
 * @version    v1.0
 * @author     soui group
 * @date       2014-05-28
 *
 * Describe
 */

#ifndef __SIMGDECODER_I__H__
#define __SIMGDECODER_I__H__

#include <interface/obj-ref-i.h>

SNSBEGIN
/**
 * @struct     IImgFrame
 * @brief      a image frame
 *
 * Describe
 */
#undef INTERFACE
#define INTERFACE IImgFrame
DECLARE_INTERFACE_(IImgFrame, IObjRef)
{
    /**
     * @brief 获取图像大小（以像素为单位）
     * @param [out] UINT * pWid -- 图像宽度
     * @param [out] UINT * pHei -- 图像高度
     * @return BOOL -- TRUE: 成功; FALSE: 失败
     */
    STDMETHOD_(BOOL, GetSize)(THIS_ UINT * pWid, UINT * pHei) PURE;

    /**
     * @brief 获取像素缓冲区
     * @return const VOID * -- 像素缓冲区指针
     */
    STDMETHOD_(const VOID *, GetPixels)(CTHIS) SCONST PURE;

    /**
     * @brief 获取GIF图像帧的延迟时间
     * @return int -- 时间（以10毫秒为单位）
     */
    STDMETHOD_(int, GetDelay)(THIS) PURE;
};

/**
 * @struct     IImgX
 * @brief      image data
 *
 * Describe
 */
#undef INTERFACE
#define INTERFACE IImgX
DECLARE_INTERFACE_(IImgX, IObjRef)
{
    //! 添加引用
    /*!
     * @return long - 当前引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //! 释放引用
    /*!
     * @return long - 当前引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //! 释放对象
    /*!
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief 从内存缓冲区加载图像数据
     * @param pBuf - 缓冲区指针
     * @param bufLen - 缓冲区大小
     * @return int
     */
    STDMETHOD_(int, LoadFromMemory)(THIS_ void *pBuf, size_t bufLen) PURE;

    /**
     * @brief 从文件加载图像数据（Unicode编码）
     * @param pszFileName - 文件名（Unicode编码）
     * @return int
     */
    STDMETHOD_(int, LoadFromFileW)(THIS_ LPCWSTR pszFileName) PURE;

    /**
     * @brief 从文件加载图像数据（ANSI编码）
     * @param pszFileName - 文件名（ANSI编码）
     * @return int
     */
    STDMETHOD_(int, LoadFromFileA)(THIS_ LPCSTR pszFileName) PURE;

    /**
     * @brief 获取图像帧数
     * @return UINT -- 图像帧数
     */
    STDMETHOD_(UINT, GetFrameCount)(THIS) PURE;

    /**
     * @brief 获取图像帧数据
     * @param iFrame - 目标帧索引
     * @return IImgFrame * -- 与输入帧索引关联的图像帧
     */
    STDMETHOD_(IImgFrame *, GetFrame)(THIS_ UINT iFrame) PURE;
};

typedef enum _ImgFmt
{
    Img_PNG,
    Img_BMP,
    Img_TIFF,
    Img_JPG,
    Img_Webp,
} ImgFmt;

/**
 * @struct     IImgDecoderFactory
 * @brief      image decoder factory
 *
 * Describe
 */
#undef INTERFACE
#define INTERFACE IImgDecoderFactory
DECLARE_INTERFACE_(IImgDecoderFactory, IObjRef)
{
    //! 添加引用
    /*!
     * @return long - 当前引用计数
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    //! 释放引用
    /*!
     * @return long - 当前引用计数
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    //! 释放对象
    /*!
     * @return void
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief 创建一个IImgX对象
     * @param [out] IImgX **ppImgDecoder -- 创建的IImgX对象
     * @return BOOL
     */
    STDMETHOD_(BOOL, CreateImgX)(THIS_ IImgX * *ppImgDecoder) PURE;

    /**
     * @brief 将IBitmap对象写入文件，指定格式
     * @param [in] BYTE *pBits -- 像素数据
     * @param [in] int nWid -- 宽度
     * @param [in] int nHei -- 高度
     * @param [in] LPCWSTR pszFileName -- 文件名
     * @param [in] const void *pFormat -- 格式参数
     * @return HRESULT
     */
    STDMETHOD_(HRESULT, SaveImage)
    (CTHIS_ BYTE * pBits, int nWid, int nHei, LPCWSTR pszFileName, const void *pFormat) SCONST PURE;

    /**
     * @brief 将IBitmap对象写入文件，指定格式
     * @param [in] BYTE *pBits -- 像素数据
     * @param [in] int nWid -- 宽度
     * @param [in] int nHei -- 高度
     * @param [in] LPCWSTR pszFileName -- 文件名
     * @param [in] ImgFmt imgFmt -- 图像格式
     * @return HRESULT
     */
    STDMETHOD_(HRESULT, SaveImage2)
    (CTHIS_ BYTE * pBits, int nWid, int nHei, LPCWSTR pszFileName, ImgFmt imgFmt) SCONST PURE;

    /**
     * @brief 查询图像解码器描述
     * @return LPCWSTR
     */
    STDMETHOD_(LPCWSTR, GetDescription)(CTHIS) SCONST PURE;
};

SNSEND
#endif // __SIMGDECODER_I__H__
