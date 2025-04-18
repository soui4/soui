/**
 * @file       SImgDecoder-i.h
 * @brief      Interface definitions for image decoding.
 * @version    v1.0
 * @author     SOUI group
 * @date       2014-05-28
 *
 * This file contains the interface definitions for image decoding, including
 * interfaces for image frames, image data, and image decoder factories.
 */

#ifndef __SIMGDECODER_I__H__
#define __SIMGDECODER_I__H__

#include <interface/obj-ref-i.h>

SNSBEGIN

/**
 * @interface IImgFrame
 * @brief Interface for an image frame.
 *
 * This interface represents a single frame of an image, which can be part of an
 * animated image or a single static image.
 */
#undef INTERFACE
#define INTERFACE IImgFrame
DECLARE_INTERFACE_(IImgFrame, IObjRef)
{
    /**
     * @brief Get the size of the image in pixels.
     * @param [out] UINT * pWid - Pointer to store the image width.
     * @param [out] UINT * pHei - Pointer to store the image height.
     * @return BOOL - TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, GetSize)(THIS_ UINT * pWid, UINT * pHei) PURE;

    /**
     * @brief Get the pixel buffer of the image.
     * @return const VOID * - Pointer to the pixel buffer.
     */
    STDMETHOD_(const VOID *, GetPixels)(CTHIS) SCONST PURE;

    /**
     * @brief Get the delay time for a GIF image frame.
     * @return int - Delay time in 10-millisecond units.
     */
    STDMETHOD_(int, GetDelay)(THIS) PURE;
};

/**
 * @interface IImgX
 * @brief Interface for image data.
 *
 * This interface represents the image data, which can contain multiple frames
 * for animated images.
 */
#undef INTERFACE
#define INTERFACE IImgX
DECLARE_INTERFACE_(IImgX, IObjRef)
{
    /**
     * @brief Increment the reference count.
     * @return long - Current reference count.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count.
     * @return long - Current reference count.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Final release of the object.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Load image data from a memory buffer.
     * @param pBuf - Pointer to the buffer containing image data.
     * @param bufLen - Size of the buffer.
     * @return int - Result of the operation.
     */
    STDMETHOD_(int, LoadFromMemory)(THIS_ void *pBuf, size_t bufLen) PURE;

    /**
     * @brief Load image data from a file (Unicode encoding).
     * @param pszFileName - File name (Unicode encoding).
     * @return int - Result of the operation.
     */
    STDMETHOD_(int, LoadFromFileW)(THIS_ LPCWSTR pszFileName) PURE;

    /**
     * @brief Load image data from a file (ANSI encoding).
     * @param pszFileName - File name (ANSI encoding).
     * @return int - Result of the operation.
     */
    STDMETHOD_(int, LoadFromFileA)(THIS_ LPCSTR pszFileName) PURE;

    /**
     * @brief Get the number of frames in the image.
     * @return UINT - Number of frames.
     */
    STDMETHOD_(UINT, GetFrameCount)(THIS) PURE;

    /**
     * @brief Get a specific frame of the image.
     * @param iFrame - Index of the frame to retrieve.
     * @return IImgFrame * - Pointer to the retrieved frame.
     */
    STDMETHOD_(IImgFrame *, GetFrame)(THIS_ UINT iFrame) PURE;
};

/**
 * @enum ImgFmt
 * @brief Image format enumeration.
 *
 * This enumeration defines the supported image formats.
 */
typedef enum _ImgFmt
{
    Img_PNG,  /**< PNG format */
    Img_BMP,  /**< BMP format */
    Img_TIFF, /**< TIFF format */
    Img_JPG,  /**< JPG format */
    Img_Webp, /**< WebP format */
    Img_TGA,  /**< TGA format */
} ImgFmt;

/**
 * @interface IImgDecoderFactory
 * @brief Interface for image decoder factory.
 *
 * This interface provides methods to create image decoders and save images in
 * various formats.
 */
#undef INTERFACE
#define INTERFACE IImgDecoderFactory
DECLARE_INTERFACE_(IImgDecoderFactory, IObjRef)
{
    /**
     * @brief Increment the reference count.
     * @return long - Current reference count.
     */
    STDMETHOD_(long, AddRef)(THIS) PURE;

    /**
     * @brief Decrement the reference count.
     * @return long - Current reference count.
     */
    STDMETHOD_(long, Release)(THIS) PURE;

    /**
     * @brief Final release of the object.
     */
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    /**
     * @brief Create an IImgX object.
     * @param [out] IImgX **ppImgDecoder - Pointer to store the created IImgX object.
     * @return BOOL - TRUE if successful, FALSE otherwise.
     */
    STDMETHOD_(BOOL, CreateImgX)(THIS_ IImgX * *ppImgDecoder) PURE;

    /**
     * @brief Save an image to a file with a specified format.
     * @param [in] BYTE *pBits - Pointer to the pixel data.
     * @param [in] int nWid - Width of the image.
     * @param [in] int nHei - Height of the image.
     * @param [in] LPCWSTR pszFileName - File name (Unicode encoding).
     * @param [in] const void *pFormat - Format-specific parameters.
     * @return HRESULT - Result of the operation.
     */
    STDMETHOD_(HRESULT, SaveImage)
    (CTHIS_ BYTE * pBits, int nWid, int nHei, LPCWSTR pszFileName, const void *pFormat) SCONST PURE;

    /**
     * @brief Save an image to a file with a specified format.
     * @param [in] BYTE *pBits - Pointer to the pixel data.
     * @param [in] int nWid - Width of the image.
     * @param [in] int nHei - Height of the image.
     * @param [in] LPCWSTR pszFileName - File name (Unicode encoding).
     * @param [in] ImgFmt imgFmt - Image format.
     * @return HRESULT - Result of the operation.
     */
    STDMETHOD_(HRESULT, SaveImage2)
    (CTHIS_ BYTE * pBits, int nWid, int nHei, LPCWSTR pszFileName, ImgFmt imgFmt) SCONST PURE;

    /**
     * @brief Get the description of the image decoder.
     * @return LPCWSTR - Description of the image decoder.
     */
    STDMETHOD_(LPCWSTR, GetDescription)(CTHIS) SCONST PURE;
};

SNSEND
#endif // __SIMGDECODER_I__H__