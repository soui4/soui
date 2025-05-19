﻿// imgdecoder-gdip.cpp : Defines the exported functions for the DLL application.
//

#include <Windows.h>

#include <gdiplus.h>
#pragma comment(lib,"gdiplus")

#include "imgdecoder-gdip.h"
#include <interface/SRender-i.h>

using namespace Gdiplus;

SNSBEGIN
    //////////////////////////////////////////////////////////////////////////
    //  SImgFrame_PNG
    SImgFrame_GDIP::SImgFrame_GDIP()
        :m_pdata(NULL)
        ,m_nWid(0)
        ,m_nHei(0)
        ,m_nFrameDelay(0)
    {

    }


    SImgFrame_GDIP::~SImgFrame_GDIP()
    {
        if(m_pdata) delete []m_pdata;
    }

    void SImgFrame_GDIP::Attach( const BYTE * pdata,int nWid,int nHei,int nDelay )
    {
        int bufSize = nWid*nHei*4;
        m_pdata=new BYTE[bufSize];
        
        BYTE *pDst = m_pdata;
        const BYTE *pSrc = pdata;
        int pixel_count = nWid * nHei;
        for (int i=0; i < pixel_count; ++i) {
            BYTE a = pSrc[3];
            if (a) 
            {
                pDst[0] = (pSrc[0] *a)/255;
                pDst[1] = (pSrc[1] *a)/255;
                pDst[2] = (pSrc[2] *a)/255;
                pDst[3] = a;
            }else
            {
                memset(pDst,0,4);
            }
            pDst += 4;
            pSrc += 4;
        }

        m_nWid=(nWid);
        m_nHei=(nHei);
        m_nFrameDelay=(nDelay);
    }

    BOOL SImgFrame_GDIP::GetSize( UINT *pWid,UINT *pHei )
    {
        if(!m_pdata) return FALSE;
        *pWid = m_nWid;
        *pHei = m_nHei;
        return TRUE;
    }

    const VOID * SImgFrame_GDIP::GetPixels(CTHIS) SCONST{
        return m_pdata;
    }

    //////////////////////////////////////////////////////////////////////////
    // SImgX_PNG
    int SImgX_GDIP::LoadFromMemory( void *pBuf,size_t bufLen )
    {
        HGLOBAL hMem = ::GlobalAlloc(GMEM_FIXED, bufLen);
        BYTE* pMem = (BYTE*)::GlobalLock(hMem);
        memcpy(pMem,pBuf,bufLen);

        IStream* pStm = NULL;
        ::CreateStreamOnHGlobal(hMem, TRUE, &pStm);
        
        Bitmap *  bmpSrc= new Bitmap(pStm);

        pStm->Release();
        ::GlobalUnlock(hMem);
        
        if (!bmpSrc) return 0;
        
        int nRet =  _InitFromGdipBitmap(bmpSrc);
        delete bmpSrc;
        
        return nRet;   
    }

    int SImgX_GDIP::LoadFromFileW( LPCWSTR pszFileName )
    {
        Bitmap * bmpSrc= new Bitmap(pszFileName);    
        if (!bmpSrc) return 0;
        int nRet =  _InitFromGdipBitmap(bmpSrc);
        delete bmpSrc;
        return nRet;   
    }

    int SImgX_GDIP::LoadFromFileA( LPCSTR pszFileName )
    {
        wchar_t wszFileName[MAX_PATH+1];
        MultiByteToWideChar(CP_ACP,0,pszFileName,-1,wszFileName,MAX_PATH);
        if(GetLastError()==ERROR_INSUFFICIENT_BUFFER) return 0;
        return LoadFromFileW(wszFileName);
    }

    SImgX_GDIP::SImgX_GDIP( BOOL bPremultiplied )
        :m_bPremultiplied(bPremultiplied)
        ,m_pImgArray(NULL)
        ,m_nFrameCount(0)
    {

    }

    SImgX_GDIP::~SImgX_GDIP( void )
    {
        if(m_pImgArray) delete []m_pImgArray;
    }

    UINT SImgX_GDIP::GetFrameCount()
    {
        return m_nFrameCount;
    }

    int SImgX_GDIP::_InitFromGdipBitmap(Bitmap * bmpSrc)
    {
        GUID   pageGuid = FrameDimensionTime;
        // Get the number of frames in the first dimension.
        m_nFrameCount = bmpSrc->GetFrameCount(&pageGuid);
        if(m_nFrameCount == 0) m_nFrameCount = 1;
        SIZE imSize={(LONG)bmpSrc->GetWidth(),(LONG)bmpSrc->GetHeight()};

        if (m_nFrameCount>1)
        {
            m_pImgArray=new SImgFrame_GDIP[m_nFrameCount];
            int nSize = bmpSrc->GetPropertyItemSize(PropertyTagFrameDelay);
            // Allocate a buffer to receive the property item.
            PropertyItem* pDelays = (PropertyItem*) new char[nSize];
            bmpSrc->GetPropertyItem(PropertyTagFrameDelay, nSize, pDelays);
            Bitmap *bmp = new  Bitmap(imSize.cx,imSize.cy,PixelFormat32bppARGB);
            for (int i=0; i<m_nFrameCount; i++)
            {
                GUID pageGuid = FrameDimensionTime;
                bmpSrc->SelectActiveFrame(&pageGuid, i);
                
                
                Graphics g(bmp);
                g.Clear(Color(0,0,0,0));
                g.DrawImage(bmpSrc,0,0,imSize.cx,imSize.cy);
                
                int nFrameDelay =10*smax(((int*) pDelays->value)[i], 10);
                
                BitmapData* bitmapData = new BitmapData;
                Rect rect(0,0, imSize.cx,imSize.cy);

                bmp->LockBits(
                    &rect,
                    ImageLockModeRead,
                    PixelFormat32bppARGB,
                    bitmapData);
                m_pImgArray[i].Attach((BYTE*)bitmapData->Scan0,imSize.cx,imSize.cy,nFrameDelay);

                bmp->UnlockBits(bitmapData);

                delete bitmapData;
            }
            delete bmp; 
            delete [] pDelays;
        }
        else
        {
            m_pImgArray = new SImgFrame_GDIP[1];

            BitmapData* bitmapData = new BitmapData;
            Rect rect(0,0, imSize.cx,imSize.cy);

            bmpSrc->LockBits(
                &rect,
                ImageLockModeRead,
                PixelFormat32bppARGB,
                bitmapData);

            m_pImgArray[0].Attach((BYTE*)bitmapData->Scan0,imSize.cx,imSize.cy,0);
            
            bmpSrc->UnlockBits(bitmapData);
            
            delete bitmapData;
        }
        
        return m_nFrameCount;
    }

	IImgFrame * SImgX_GDIP::GetFrame(UINT iFrame)
	{
		if(iFrame >= GetFrameCount()) return NULL;
		return m_pImgArray+iFrame;
	}

    //////////////////////////////////////////////////////////////////////////
    //  SImgDecoderFactory_GDIP

    SImgDecoderFactory_GDIP::SImgDecoderFactory_GDIP( )
    {
        GdiplusStartupInput gdiplusStartupInput;

        BOOL bOK = Ok == GdiplusStartup(&_gdiPlusToken, &gdiplusStartupInput, NULL);
        if(!bOK)
        {
            SASSERT(0);
        }
    }

    SImgDecoderFactory_GDIP::~SImgDecoderFactory_GDIP()
    {
        if (_gdiPlusToken != 0)
        {
            GdiplusShutdown(_gdiPlusToken);
            _gdiPlusToken = 0;
        }
    }

    BOOL SImgDecoderFactory_GDIP::CreateImgX( IImgX **ppImgDecoder )
    {
        *ppImgDecoder = new SImgX_GDIP(TRUE);
        return TRUE;
    }

    CLSID FindCodecForFileType( REFGUID guidFileType, const Gdiplus::ImageCodecInfo* pCodecs, UINT nCodecs )
    {
        for( UINT iCodec = 0; iCodec < nCodecs; iCodec++ )
        {
            if( pCodecs[iCodec].FormatID == guidFileType )
            {
                return( pCodecs[iCodec].Clsid );
            }
        }

        return( CLSID_NULL );
    }

	HRESULT SImgDecoderFactory_GDIP::SaveImage(BYTE* pBits, int nWid,int nHei, LPCWSTR pszFileName, const void * pFormat) SCONST
    {
        const GUID * pFmtID = (const GUID*)pFormat;
        
        UINT nEncoders;
        UINT nBytes;
        Gdiplus::Status status;

        status = Gdiplus::GetImageEncodersSize( &nEncoders, &nBytes );
        if( status != Gdiplus::Ok )
        {
            return( E_FAIL );
        }

        //USES_ATL_SAFE_ALLOCA;
        Gdiplus::ImageCodecInfo* pEncoders = static_cast< Gdiplus::ImageCodecInfo* >( malloc(nBytes) );

        if( pEncoders == NULL )
            return E_OUTOFMEMORY;

        status = Gdiplus::GetImageEncoders( nEncoders, nBytes, pEncoders );
        if( status != Gdiplus::Ok )
        {
            free(pEncoders);
            return( E_FAIL );
        }

        CLSID clsidEncoder = FindCodecForFileType( *pFmtID, pEncoders, nEncoders );
        free(pEncoders);

        if( clsidEncoder == CLSID_NULL )
        {
            return( E_FAIL );
        }
        
        Bitmap bmp(nWid,nHei,nWid*4,PixelFormat32bppPARGB,pBits);
        Image *gdipImg = &bmp;
        return Ok == gdipImg->Save(pszFileName,&clsidEncoder,NULL)?S_OK:E_FAIL;
    }

	/*
	DEFINE_GUID(ImageFormatBMP, 0xb96b3cab,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
	DEFINE_GUID(ImageFormatEMF, 0xb96b3cac,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
	DEFINE_GUID(ImageFormatWMF, 0xb96b3cad,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
	DEFINE_GUID(ImageFormatJPEG, 0xb96b3cae,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
	DEFINE_GUID(ImageFormatPNG, 0xb96b3caf,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
	DEFINE_GUID(ImageFormatGIF, 0xb96b3cb0,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
	DEFINE_GUID(ImageFormatTIFF, 0xb96b3cb1,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
	DEFINE_GUID(ImageFormatEXIF, 0xb96b3cb2,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
	DEFINE_GUID(ImageFormatIcon, 0xb96b3cb5,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
	*/
	HRESULT SImgDecoderFactory_GDIP::SaveImage2(BYTE* pBits, int nWid,int nHei, LPCWSTR pszFileName, ImgFmt imgFmt) SCONST
	{
		const GUID *fmt = NULL;
		switch(imgFmt){
			case Img_PNG:
				fmt = &ImageFormatPNG;
				break;
			case Img_BMP:
				fmt = &ImageFormatBMP;
				break;
			case Img_TIFF:
				fmt = &ImageFormatTIFF;
				break;
			case Img_JPG:
				fmt = &ImageFormatJPEG;
				break;
			default:
				break;
		}
		if(!fmt)
			return E_INVALIDARG;
		return SaveImage(pBits,nWid,nHei,pszFileName,(const void*)fmt);
	}

    LPCWSTR SImgDecoderFactory_GDIP::GetDescription() const
    {
        return DESC_IMGDECODER;
    }

    //////////////////////////////////////////////////////////////////////////
    namespace IMGDECODOR_GDIP
    {
        BOOL SCreateInstance( IObjRef **pImgDecoderFactory )
        {
            *pImgDecoderFactory = new SImgDecoderFactory_GDIP();
            return TRUE;
        }
    }

SNSEND

EXTERN_C BOOL Decoder_Gdip_SCreateInstance(IObjRef **pImgDecoderFactory)
{
	return SOUI::IMGDECODOR_GDIP::SCreateInstance(pImgDecoderFactory);
}
