#include "stdafx.h"
#include "SSkinAni.h"
#include <helper/SplitString.h>
#include <interface/SImgDecoder-i.h>
#include <interface/SRender-i.h>

SNSBEGIN

    SSkinAni::SSkinAni()
        : m_pFrames(NULL)
        , m_nFrames(0)
        , m_iFrame(0)
        , m_bTile(FALSE)
        , m_filterLevel(kLow_FilterLevel)
    {
    }

	SSkinAni::~SSkinAni()
	{
		if(m_pFrames) delete [] m_pFrames;
	}


    HRESULT SSkinAni::OnAttrSrc( const SStringW &strValue,BOOL bLoading )
    {
        SStringTList strLst;
        size_t nSegs=ParseResID(S_CW2T(strValue),strLst);
        LPBYTE pBuf=NULL;
        size_t szBuf=0;

        if(nSegs == 2)
        {
#ifndef _WIN32
            strLst[1].ReplaceChar('\\', '/');
#endif //_WIN32
            szBuf=GETRESPROVIDER->GetRawBufferSize(strLst[0],strLst[1]);
            if(szBuf)
            {
                pBuf=new BYTE[szBuf];
                GETRESPROVIDER->GetRawBuffer(strLst[0],strLst[1],pBuf,szBuf);
            }
        }else
        {//自动从APNG资源类型里查找资源
            szBuf=GETRESPROVIDER->GetRawBufferSize(_T("apng"),strLst[0]);
            if(szBuf)
            {
                pBuf=new BYTE[szBuf];
                GETRESPROVIDER->GetRawBuffer(_T("apng"),strLst[0],pBuf,szBuf);
            }
        }
        if(pBuf)
        {
            LoadFromMemory(pBuf,szBuf);
            delete []pBuf;
        }
        return S_OK;
    }


int SSkinAni::LoadFromFile( LPCTSTR pszFileName )
{
    CAutoRefPtr<IImgX> imgX;
    GETRENDERFACTORY->GetImgDecoderFactory()->CreateImgX(&imgX);
#ifdef _UNICODE
    imgX->LoadFromFileW(pszFileName);
#else
	imgX->LoadFromFileA(pszFileName);
#endif
    return _InitImgFrame(imgX);
}

int SSkinAni::LoadFromMemory( LPVOID pBuf,size_t dwSize )
{
    SAutoRefPtr<IImgX> imgX;
    GETRENDERFACTORY->GetImgDecoderFactory()->CreateImgX(&imgX);
    imgX->LoadFromMemory(pBuf,dwSize);
    return _InitImgFrame(imgX);
}

int SSkinAni::_InitImgFrame( IImgX *pImgX )
{
    if(m_pFrames) delete []m_pFrames;
    m_pFrames = NULL;
    m_nFrames =0;
    m_iFrame = 0;
    if(!pImgX) return 0;

    m_nFrames = pImgX->GetFrameCount();
    m_pFrames = new SAniFrame[m_nFrames];
    for(int i=0;i<m_nFrames;i++)
    {
        GETRENDERFACTORY->CreateBitmap(&m_pFrames[i].pBmp);
        m_pFrames[i].pBmp->Init2(pImgX->GetFrame(i));
        m_pFrames[i].nDelay=pImgX->GetFrame(i)->GetDelay();
    }
    return m_nFrames;
}

void SSkinAni::_DrawByIndex2(IRenderTarget *pRT, LPCRECT rcDraw, int dwState,BYTE byAlpha/*=0xFF*/) const
{
	CRect rcSrc(CPoint(0,0),GetSkinSize());
	if(m_rcMargin.IsRectNull())
		pRT->DrawBitmapEx(rcDraw,m_pFrames[m_iFrame].pBmp,rcSrc,GetExpandCode(),byAlpha);
	else
		pRT->DrawBitmap9Patch(rcDraw,m_pFrames[m_iFrame].pBmp,rcSrc,m_rcMargin,GetExpandCode(),byAlpha);
}

long SSkinAni::GetFrameDelay(int iFrame/*=-1*/) const
{
	if(iFrame==-1) iFrame=m_iFrame;
	long nRet=-1;
	if(m_nFrames>1 && iFrame>=0 && iFrame<m_nFrames)
	{
		nRet=m_pFrames[iFrame].nDelay;
	}
	return nRet;
}

SIZE SSkinAni::GetSkinSize() const
{
	SIZE sz={0};
	if(m_nFrames>0 && m_pFrames)
	{
		sz=m_pFrames[0].pBmp->Size();
	}
	return sz;
}

int SSkinAni::GetStates() const 
{
	return m_nFrames;
}

void SSkinAni::_Scale(ISkinObj *pObj, int nScale)
{
    SSkinObjBase::_Scale(pObj, nScale);
    SSkinAni *pClone = sobj_cast<SSkinAni>(pObj);
    pClone->m_nFrames = m_nFrames;
    pClone->m_iFrame = m_iFrame;
    pClone->m_filterLevel = m_filterLevel;
    pClone->m_bTile = m_bTile;
    int srcScale = GetScale();
    pClone->m_rcMargin.left = MulDiv(m_rcMargin.left, nScale, srcScale);
    pClone->m_rcMargin.top = MulDiv(m_rcMargin.top, nScale, srcScale);
    pClone->m_rcMargin.right = MulDiv(m_rcMargin.right, nScale, srcScale);
    pClone->m_rcMargin.bottom = MulDiv(m_rcMargin.bottom, nScale, srcScale);

    pClone->m_pFrames = new SAniFrame[m_nFrames];

	CSize szSkin = GetSkinSize();
	szSkin.cx = MulDiv(szSkin.cx, nScale, GetScale());
	szSkin.cy = MulDiv(szSkin.cy, nScale, GetScale());
	for(int i=0;i<m_nFrames;i++)
	{
		m_pFrames[i].pBmp->Scale2(&pClone->m_pFrames[i].pBmp, szSkin.cx, szSkin.cy, kHigh_FilterLevel);
		pClone->m_pFrames[i].nDelay = m_pFrames[i].nDelay;
	}
}


SNSEND