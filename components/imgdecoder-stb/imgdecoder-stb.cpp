
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include "imgdecoder-stb.h"
#include <string/strcpcvt.h>
#include <upng.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

SNSBEGIN
    //////////////////////////////////////////////////////////////////////////
    //  SImgFrame_STB
SImgFrame_STB::SImgFrame_STB(const BYTE *data, int w, int h, int nDelay,BOOL bMove)
        :m_nWid(w),m_nHei(h)
        ,m_nDelay(nDelay)
    {
        if (bMove)
        {
            m_data = (unsigned char *)data;
        }
        else
        {
            m_data = (unsigned char *)malloc(w * h * 4);
            assert(m_data);
            memcpy(m_data, data, w * h * 4);
        }
    }

    SImgFrame_STB::~SImgFrame_STB()
    {
        if(m_data) free(m_data);
    }

    BOOL SImgFrame_STB::GetSize( UINT *pWid,UINT *pHei )
    {
        *pWid = m_nWid;
        *pHei = m_nHei;
        return TRUE;
    }

    int SImgFrame_STB:: GetDelay()
    {
        return m_nDelay;
    }

    const VOID * SImgFrame_STB::GetPixels(CTHIS) SCONST{
        return m_data;
    }

    static int upng_inflate(char *out, int outsize, const char *in, int insize)
    {
        return stbi_zlib_decode_buffer(out, outsize, in, insize) == outsize?0:-1;
    }
    //////////////////////////////////////////////////////////////////////////
    // SImgX_STB
    int SImgX_STB::LoadFromMemory( void *pBuf,size_t bufLen )
    {
        if(!pBuf) return 0;

        int width, height, frames = 0, comp;
        //test for apng
        upng_t *upng = upng_new_from_bytes((const unsigned char *)pBuf, (unsigned long)bufLen);
        // Accept RGBA format or formats that will be auto-converted to RGBA (PLT, RGB, LUMA, LUM)
        if (upng_header(upng) == UPNG_EOK)
        { // apng
            upng_set_inflate(upng_inflate);
            width = upng_get_width(upng);
            height = upng_get_height(upng);
            frames = upng_get_frame_count(upng);
            if (frames == 0)
            {
                if (UPNG_EOK == upng_decode_default(upng, NULL, 0))
                {
                    m_arrFrames.SetCount(1);
                    uint8_t *buf = upng_move_frame_buffer(upng);
                    _DoPromultiply(buf, width, height);
                    SImgFrame_STB *pFrame = new SImgFrame_STB(buf, width, height, 0, TRUE);
                    m_arrFrames.SetAt(0, pFrame);
                    pFrame->Release();
                }
            }
            else
            {
                m_arrFrames.SetCount(frames);
                uint32_t buf_size = width * height * 4;
                uint8_t *buf = (uint8_t *)calloc(buf_size,1);
                for (int i = 0; i < frames; i++)
                {
                    if(UPNG_EOK==upng_decode_next_frame(upng, buf, buf_size))
                    {
                        int nDelay = upng_get_frame_delay(upng);
                        _DoPromultiply(buf, width, height);
                        // don't move frame buffer from upng. the frame buffer will be reused by upng.
                        SImgFrame_STB *pFrame = new SImgFrame_STB(buf, width, height, nDelay,FALSE);
                        m_arrFrames.SetAt(i, pFrame);
                        pFrame->Release();
                    }
                    else {
                        m_arrFrames.RemoveAll();
                        break;
                    }
                }
                free(buf);
            }
            upng_free(upng);
            return m_arrFrames.GetCount();
        }
        else
        {
            upng_free(upng);
        }
        int *delays = NULL;
        //test for gif
        unsigned char *data = stbi_load_gif_from_memory((const stbi_uc *)pBuf, (int)bufLen, &delays, &width, &height, &frames, &comp, STBI_rgb_alpha);
        if (!data)
        { // test for other image format
            data = stbi_load_from_memory((stbi_uc const *)pBuf, (int)bufLen, &width, &height, NULL, 4);
            if (!data)
            {
                return 0;
            }
            delays = (int *)malloc(sizeof(int));
            *delays = 0;
            frames = 1;
        }
        SASSERT(data);
        m_arrFrames.SetCount(frames);
        unsigned char *p = data;
        for (int i = 0; i < frames; i++)
        {
            _DoPromultiply(p, width, height);
            SImgFrame_STB *pFrame = new SImgFrame_STB(p, width, height, delays[i],FALSE);
            m_arrFrames.SetAt(i, pFrame);
            p += (width * height * 4);
            pFrame->Release();
        }
        free(delays);
        stbi_image_free(data);
        return frames;
    }

    #ifndef MAX_PATH
    #define MAX_PATH 1000
    #endif

    int SImgX_STB::LoadFromFileW( LPCWSTR pszFileName )
    {
#ifdef _WIN32
        FILE *f=_wfopen(pszFileName,L"rb");
        if(!f) return 0;
        fseek(f, 0, SEEK_END);
        LONG len = ftell(f);
        fseek(f, 0, SEEK_SET);
        char *buf = (char *)malloc(len);
        fread(buf, 1, len, f);
        int ret = LoadFromMemory(buf, len);
        fclose(f);
        return ret;
#else
        char szFileName[MAX_PATH]={0};
        WideCharToMultiByte(CP_UTF8,0,pszFileName,-1,szFileName,MAX_PATH,NULL,NULL);
        return LoadFromFileA(szFileName);
#endif
    }


    int SImgX_STB::LoadFromFileA( LPCSTR pszFileName )
    {
        #ifdef _WIN32
        wchar_t wszFileName[MAX_PATH+1];
        int cp = CP_ACP;
        MultiByteToWideChar(cp,0,pszFileName,-1,wszFileName,MAX_PATH);
        if(GetLastError()==ERROR_INSUFFICIENT_BUFFER) return 0;
        return LoadFromFileW(wszFileName);
        #else
        FILE *f=fopen(pszFileName,"rb");
        if(!f) return 0;
        fseek(f, 0, SEEK_END);
        LONG len = ftell(f);
        fseek(f, 0, SEEK_SET);
        char *buf = (char *)malloc(len);
        int readed = fread(buf, 1, len, f);
        fclose(f);
        if(readed != len){
            free(buf);
            return 0;
        }
        int ret = LoadFromMemory(buf, len);
        return ret;
        #endif//_WIN32
    }

    SImgX_STB::SImgX_STB(BOOL bPremultiple)
        :m_bPremultiple(bPremultiple)
    {

    }

    SImgX_STB::~SImgX_STB( void )
    {
    }

    void SImgX_STB::_DoPromultiply( BYTE *pdata,int nWid,int nHei )
    {
        //swap rgba to bgra and do premultiply
        BYTE *p=pdata;
        int pixel_count = nWid * nHei;
        for (int i=0; i < pixel_count; ++i) {
            stbi_uc a = p[3];
            stbi_uc t = p[0];
            if (a) 
            {
                if (m_bPremultiple)
                {
                    p[0] = (p[2] * a) / 255;
                    p[1] = (p[1] * a) / 255;
                    p[2] = (t * a) / 255;
                }
                else
                {// swap r and b
                    p[0] = p[2]; 
                    p[2] = t; 
                }
            }else
            {
                memset(p,0,4);
            }
            p += 4;
        }    
    }

	IImgFrame * SImgX_STB::GetFrame(UINT iFrame)
	{
		if(iFrame >= GetFrameCount()) return NULL;
		return m_arrFrames[iFrame];
	}

	UINT SImgX_STB::GetFrameCount()
	{
        return (UINT)m_arrFrames.GetCount();
	}

    //////////////////////////////////////////////////////////////////////////
    //  SImgDecoderFactory_STB

    SImgDecoderFactory_STB::SImgDecoderFactory_STB( )
    {
    }

    SImgDecoderFactory_STB::~SImgDecoderFactory_STB()
    {

    }

    BOOL SImgDecoderFactory_STB::CreateImgX( IImgX **ppImgDecoder )
    {
        *ppImgDecoder = new SImgX_STB(TRUE);
        return TRUE;
    }
    
    LPCWSTR SImgDecoderFactory_STB::GetDescription() const
    {
        return DESC_IMGDECODER;
    }

	HRESULT SImgDecoderFactory_STB::SaveImage(BYTE* pBits, int nWid,int nHei, LPCWSTR pszFileName, const void * pFormat) SCONST
	{
		return E_NOTIMPL;
	}

	HRESULT SImgDecoderFactory_STB::SaveImage2(BYTE* pBits, int nWid,int nHei, LPCWSTR pszFileName, ImgFmt imgFmt) SCONST
	{
        //swap red and blue.
        BYTE *p=pBits;
        for(int y=0;y<nHei;y++){
            for(int x=0;x<nWid;x++){
                BYTE t = p[0];
                p[0]=p[2];
                p[2]=t;
                p+=4;
            }
        }
        SStringA fileu8 = S_CW2A(pszFileName, CP_UTF8);
        if (imgFmt == Img_PNG)
        {
            int ret = stbi_write_png(fileu8.c_str(), nWid, nHei, 4, pBits, nWid * 4);
            return ret ? S_OK : E_FAIL;
        }
        else if(imgFmt == Img_TGA){
            int ret = stbi_write_tga(fileu8.c_str(),nWid,nHei,4,pBits);
        }
        else if (imgFmt == Img_BMP)
        {
            int ret = stbi_write_bmp(fileu8.c_str(), nWid, nHei, 4, pBits);
            return ret ? S_OK : E_FAIL;
        }
        else if (imgFmt == Img_JPG)
        {
            int ret = stbi_write_jpg(fileu8.c_str(), nWid, nHei, 4, pBits, 90);
            return ret ? S_OK : E_FAIL;
        }
		return E_NOTIMPL;
	}

    //////////////////////////////////////////////////////////////////////////
    namespace IMGDECODOR_STB{
        SOUI_COM_C BOOL SOUI_COM_API SCreateInstance(IObjRef **pImgDecoderFactory)
        {
            *pImgDecoderFactory = new SImgDecoderFactory_STB();
            return TRUE;
        }
    }
SNSEND

EXTERN_C BOOL Decoder_Stb_SCreateInstance(IObjRef **pImgDecoderFactory)
{
    SOUI::SImgDecoderFactory_STB *p = new SOUI::SImgDecoderFactory_STB();
    *pImgDecoderFactory = p;
    return TRUE;
}
