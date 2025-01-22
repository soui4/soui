#include "stdafx.h"
#include "SSkinGif.h"
#include <helper/SplitString.h>
#include <interface/SImgDecoder-i.h>
#include <interface/SRender-i.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
namespace SOUI
{

    HRESULT SSkinGif::OnAttrSrc( const SStringW &strValue,BOOL bLoading )
    {
        SStringTList strLst;
        size_t nSegs=ParseResID(S_CW2T(strValue),strLst);
        LPBYTE pBuf=NULL;
        size_t szBuf=0;

        if(nSegs == 2)
        {
            szBuf=GETRESPROVIDER->GetRawBufferSize(strLst[0],strLst[1]);
            if(szBuf)
            {
                pBuf=new BYTE[szBuf];
                GETRESPROVIDER->GetRawBuffer(strLst[0],strLst[1],pBuf,szBuf);
            }
        }else
        {//自动从GIF资源类型里查找资源
            szBuf=GETRESPROVIDER->GetRawBufferSize(_T("gif"),strLst[0]);
            if(szBuf)
            {
                pBuf=new BYTE[szBuf];
                GETRESPROVIDER->GetRawBuffer(_T("gif"),strLst[0],pBuf,szBuf);
            }
        }
        if(pBuf)
        {
            LoadFromMemory(pBuf,szBuf);
            delete []pBuf;
        }
        return S_OK;
    }

int SSkinGif::LoadFromFile( LPCTSTR pszFileName )
{
    FILE *f = _tfopen(pszFileName, _T("rb"));
    if (!f)
        return 0;
    fseek(f, 0, SEEK_END);
    LONG len = ftell(f); 
    fseek(f, 0, SEEK_SET);
    char *buf = (char*)malloc(len);
    fread(buf, 1, len, f);
    int ret = LoadFromMemory(buf, len);
    fclose(f);
    return ret;
}

static void swap_rb_and_premultiply_alpha(unsigned char *pdata, int nWid, int nHei)
{
    // swap rgba to bgra and do premultiply
    BYTE *p = pdata;
    int pixel_count = nWid * nHei;
    for (int i = 0; i < pixel_count; ++i)
    {
        stbi_uc a = p[3];
        stbi_uc t = p[0];
        if (a)
        {
            p[0] = (p[2] * a) / 255;
            p[1] = (p[1] * a) / 255;
            p[2] = (t * a) / 255;
        }
        else
        {
            memset(p, 0, 4);
        }
        p += 4;
    }
}

int SSkinGif::LoadFromMemory( LPVOID pBuf,size_t dwSize )
{
    int width, height, frames=0, comp;
    int *delays = NULL;
    unsigned char *data = stbi_load_gif_from_memory((const stbi_uc *)pBuf, dwSize, &delays, &width, &height, &frames, & comp, STBI_rgb_alpha);

    if (data == NULL || frames==0)
    {
        printf("Failed to load image\n");
        return 1;
    }
    m_pFrames = new SAniFrame[frames];
    unsigned char *p = data;
    for (int i = 0; i < frames; i++)
    {
        m_pFrames[i].nDelay = delays[i]/10;
        GETRENDERFACTORY->CreateBitmap((IBitmapS **)&m_pFrames[i].pBmp);
        swap_rb_and_premultiply_alpha(p, width, height);
        m_pFrames[i].pBmp->Init(width, height, p);
        p += (width * height * 4);
    }
    free(delays);
    stbi_image_free(data);
    m_nFrames = frames;
    return m_nFrames;
}

/**
* GetFrameDelay
* @brief    获得指定帧的显示时间
* @param    int iFrame --  帧号,为-1时代表获得当前帧的延时
* @return   long -- 延时时间(*10ms)
* Describe
*/

long SSkinGif::GetFrameDelay(int iFrame) const
{
	if (iFrame == -1) iFrame = m_iFrame;
	long nRet = -1;
	if (m_nFrames>1 && iFrame >= 0 && iFrame<m_nFrames)
	{
		nRet = m_pFrames[iFrame].nDelay;
	}
	return nRet;
}

void SSkinGif::_DrawByIndex2(IRenderTarget *pRT, LPCRECT rcDraw, int iState, BYTE byAlpha) const
{
	if (iState < m_nFrames)
	{
		//LoadFrame(iState, m_pImg);
		CRect rcSrc(CPoint(0, 0), GetSkinSize());
		if (m_bEnableScale)
            pRT->DrawBitmapEx(rcDraw, m_pFrames[m_iFrame].pBmp, rcSrc, m_bTile ? EM_TILE : EM_STRETCH, byAlpha);
		else
            pRT->DrawBitmapEx(rcDraw, m_pFrames[m_iFrame].pBmp, rcSrc, EM_NULL, byAlpha);
	}
}

SIZE SSkinGif::GetSkinSize() const
{
	CSize szRet;
	if (m_pFrames && m_nFrames>0)
	{
        szRet.cx = m_pFrames[m_iFrame].pBmp->Width();
        szRet.cy = m_pFrames[m_iFrame].pBmp->Height();
	}
	return szRet;
}

int SSkinGif::GetStates() const
{
	return m_nFrames;
}

}//end of namespace SOUI