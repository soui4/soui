#include "stdafx.h"
#include "SChatEdit.h"
#ifdef _WIN32
#include "reole/RichEditOle.h"
#endif//_WIN32
#ifndef LY_PER_INCH
#define LY_PER_INCH 1440
#endif

SNSBEGIN

const SStringW KLabelColor = L"color";
const SStringW KLabelLink = L"link";
const SStringW KLabelFont = L"font";
const SStringW KLabelUnderline = L"underline";
const SStringW KLabelItalic = L"italic";
const SStringW KLabelBold = L"bold";
const SStringW KLabelStrike = L"strike";
const SStringW KLabelSize = L"size";
const SStringW KLabelSmiley = L"smiley";

SChatEdit::SChatEdit(void)
{
    GetEventSet()->addEvent(EVENTID(EventChatEditKeyReturn));
}

SChatEdit::~SChatEdit(void)
{
}

BOOL SChatEdit::AppendFormatText(const SStringW &strMsg, BOOL bNewLine, BOOL bCanUndo)
{
    BOOL bRet = FALSE;
    SStringW strBuf = L"<msg>" + strMsg + L"</msg>";
    LPWSTR pszBuf = strBuf.GetBuffer(strBuf.GetLength());
    {
        SXmlDoc doc;
        if (doc.load_buffer_inplace(pszBuf, strBuf.GetLength() * sizeof(wchar_t), xml_parse_default, sizeof(wchar_t) == 2 ? enc_utf16 : enc_utf32))
        {
            bRet = AppendFormatText(doc.root().child(L"msg"), bNewLine, bCanUndo);
        }
    }
    strBuf.ReleaseBuffer();
    return bRet;
}

BOOL SChatEdit::AppendFormatText(const SXmlNode xmlMsg, BOOL bNewLine, BOOL bCanUndo)
{
    TCHAR szRet[] = { 0x0a, 0 };
    int nLen = (int)SSendMessage(WM_GETTEXTLENGTH);
    if (bNewLine)
    { //插入一个换行符
        SSendMessage(EM_SETSEL, nLen, nLen);
        SSendMessage(EM_REPLACESEL, bCanUndo, (LPARAM)L"\r\n");
        nLen = (int)SSendMessage(WM_GETTEXTLENGTH);
    }
    SSendMessage(EM_SETSEL, nLen, nLen);
    long iCaret = nLen;

    CHARFORMATW cf = { 0 };
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_ALL;
    SSendMessage(EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
    cf.dwEffects &= ~CFE_AUTOCOLOR;
    _InsertFormatText(iCaret, cf, xmlMsg, bCanUndo);

    SSendMessage(WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0));
    return TRUE;
}

BOOL SChatEdit::ReplaceSelectionByFormatText(const SStringW &strMsg, BOOL bCanUndo /*=TRUE*/)
{
    BOOL bRet = FALSE;
    SStringW strBuf = L"<msg>" + strMsg + L"</msg>";
    LPWSTR pszBuf = strBuf.GetBuffer(strBuf.GetLength());
    {
        SXmlDoc doc;
        if (doc.load_buffer_inplace(pszBuf, strBuf.GetLength() * 2, xml_parse_default, sizeof(wchar_t) == 2 ? enc_utf16 : enc_utf32))
        {
            SSendMessage(EM_REPLACESEL, bCanUndo, (LPARAM)L"");

            long iCaret = 0;
            SSendMessage(EM_GETSEL, (WPARAM)&iCaret);

            CHARFORMATW cf = { 0 };
            cf.cbSize = sizeof(cf);
            cf.dwMask = CFM_ALL;
            SSendMessage(EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
            cf.dwEffects &= ~CFE_AUTOCOLOR;
            _InsertFormatText(iCaret, cf, doc.root().child(L"msg"), bCanUndo);

            bRet = TRUE;
        }
    }
    strBuf.ReleaseBuffer();

    return TRUE;
}

int SChatEdit::_InsertFormatText(int iCaret, CHARFORMATW cf, SXmlNode xmlText, BOOL bCanUndo)
{
    SStringW strText = xmlText.value();
#ifdef _WIN32
    if (xmlText.name() == KLabelSmiley)
    { // insert smiley
        SComPtr<ISmileyCtrl> pSmiley;
        HRESULT hr = ::CoCreateInstance(CLSID_SSmileyCtrl, NULL, CLSCTX_INPROC,
                                        __uuidof(ISmileyCtrl), (LPVOID *)&pSmiley);
        if (FAILED(hr))
            return 0;

        SComPtr<IRichEditOle> ole;
        if (SSendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)(void **)&ole) && ole)
        {
            SComPtr<IRichEditOleCallback> pCallback;
            hr = ole->QueryInterface(IID_IRichEditOleCallback, (void **)&pCallback);
            if (FAILED(hr))
                return 0;
            SComPtr<ISmileyHost> host;
            hr = pCallback->QueryInterface(__uuidof(ISmileyHost), (void **)&host);
            if (FAILED(hr))
                return 0;
            SComPtr<ISmileySource> pSource;
            hr = host->CreateSource(&pSource);
            if (FAILED(hr))
                return 0;
            {
                UINT uID = xmlText.attribute(L"id").as_uint(-1);
                SStringW strPath = xmlText.attribute(L"path").value();
                if (uID != -1)
                    hr = pSource->LoadFromID(uID);
                else
                    hr = pSource->LoadFromFile(strPath);
                if (SUCCEEDED(hr))
                {
                    pSmiley->SetSource(pSource);
                    SSendMessage(EM_SETSEL, iCaret, iCaret);
                    pSmiley->Insert2Richedit(ole);
                }
            }
        }
        return SUCCEEDED(hr) ? 1 : 0;
    }
#endif// _WIN32
    CHARFORMATW cfNew = cf;
    cfNew.dwMask = 0;
    if (xmlText.name() == KLabelColor)
    {
        cfNew.crTextColor = GETCOLOR(xmlText.attribute(L"value").value());
        if (cfNew.crTextColor != CR_INVALID)
        {
#ifdef _WIN32
            cfNew.crTextColor &= 0x00ffffff;
#else
            if ((cfNew.crTextColor&0xff000000)==0)
                cfNew.crTextColor |= 0xffffff00;
#endif//_WIN32
            cfNew.dwMask |= CFM_COLOR;
        }
    }
    else if (xmlText.name() == KLabelFont)
    {
        wcscpy(cf.szFaceName, cfNew.szFaceName);
        wcscpy_s(cfNew.szFaceName, LF_FACESIZE - 1, xmlText.attribute(L"value").value());
        cfNew.dwMask |= CFM_FACE;
    }
    else if (xmlText.name() == KLabelUnderline)
    {
        cfNew.dwMask |= CFM_UNDERLINE;
        cfNew.dwEffects |= CFE_UNDERLINE;
    }
    else if (xmlText.name() == KLabelItalic)
    {
        cfNew.dwMask |= CFM_ITALIC;
        cfNew.dwEffects |= CFE_ITALIC;
    }
    else if (xmlText.name() == KLabelBold)
    {
        cfNew.dwMask |= CFM_BOLD;
        cfNew.dwEffects |= CFE_BOLD;
    }
    else if (xmlText.name() == KLabelStrike)
    {
        cfNew.dwMask |= CFM_STRIKEOUT;
        cfNew.dwEffects |= CFE_STRIKEOUT;
    }
    else if (xmlText.name() == KLabelLink)
    {
        cfNew.dwMask |= CFM_LINK;
        cfNew.dwEffects |= CFE_LINK;
        COLORREF cr = GETCOLOR(xmlText.attribute(L"color").value());
        if (cr != CR_INVALID)
        {
            cfNew.dwMask |= CFM_COLOR;
            #ifdef _WIN32
            cr &= 0x00ffffff;
            #else
            if ((cr & 0xff000000) == 0)
                cr |= 0xff000000;
            #endif//_WIN32
            cfNew.crTextColor = cr ;
        }
    }
    else if (xmlText.name() == KLabelSize)
    {
        cfNew.dwMask |= CFM_SIZE;

        HDC hdc = GetDC(0);
        LONG yPixPerInch = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(0, hdc);
        cfNew.yHeight
            = abs(MulDiv(xmlText.attribute(L"value").as_uint(12), LY_PER_INCH, yPixPerInch));
    }

    int nRet = strText.GetLength();

    SSendMessage(EM_REPLACESEL, bCanUndo, (LPARAM)(LPCWSTR)strText);
    int iEnd = iCaret + nRet;
    SSendMessage(EM_SETSEL, iCaret, iEnd);
    SSendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfNew);
    iCaret = iEnd;
    SSendMessage(EM_SETSEL, iCaret, iCaret);

    SXmlNode xmlChild = xmlText.first_child();
    while (xmlChild)
    {
        int nSubLen = _InsertFormatText(iCaret, cfNew, xmlChild, bCanUndo);
        iCaret += nSubLen;
        nRet += nSubLen;

        xmlChild = xmlChild.next_sibling();
    }
    if (cfNew.dwMask)
    {
        cf.dwMask = CFM_ALL;
        SSendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
    }
    return nRet;
}

int SChatEdit::OnCreate(LPVOID)
{
    int nRet = SRichEdit::OnCreate(NULL);
    if (nRet == 0)
    {
        DWORD dwEvtMask = (DWORD)SSendMessage(EM_GETEVENTMASK);
        SSendMessage(EM_SETEVENTMASK, 0, dwEvtMask | ENM_LINK);
    }
    return nRet;
}

void SChatEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (nChar == VK_RETURN)
    {
        EventChatEditKeyReturn evt(this);
		evt.bCancel=FALSE;
		FireEvent(&evt);
        if (evt.bCancel)
            return;
    }
    return SRichEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

SStringW SChatEdit::GetFormatText()
{
    SStringW strTxt;

    TEXTRANGE txtRng;
    txtRng.chrg.cpMin = 0;
    txtRng.chrg.cpMax = (LONG)SSendMessage(WM_GETTEXTLENGTH);
    txtRng.lpstrText = (LPTSTR)strTxt.GetBufferSetLength(txtRng.chrg.cpMax);

    SSendMessage(EM_GETTEXTRANGE, 0, (LPARAM)&txtRng);
    strTxt.ReleaseBuffer();

#ifdef _WIN32
    SComPtr<IRichEditOle> ole;
    SSendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)(void **)&ole);
#endif//_WIN32
    SStringW strMsg;
    int iPlainTxtBegin = 0;
    for (int i = 0; i < strTxt.GetLength(); i++)
    {
        if (strTxt[i] == 0xfffc)
        { //找到一个OLE对象
#ifdef _WIN32
            strMsg += strTxt.Mid(iPlainTxtBegin, i - iPlainTxtBegin);
            iPlainTxtBegin = i + 1;

            REOBJECT reobj = { sizeof(reobj), 0 };
            reobj.cp = i;
            HRESULT hr = ole->GetObject(REO_IOB_USE_CP, &reobj, REO_GETOBJ_POLEOBJ);
            if (SUCCEEDED(hr) && reobj.poleobj)
            {
                if (reobj.clsid == CLSID_SSmileyCtrl)
                {
                    SComPtr<ISmileyCtrl> smiley;
                    hr = reobj.poleobj->QueryInterface(__uuidof(ISmileyCtrl), (void **)&smiley);
                    if (SUCCEEDED(hr))
                    {
                        SComPtr<ISmileySource> source;
                        hr = smiley->GetSource(&source);
                        SASSERT(SUCCEEDED(hr));
                        UINT uID = -1;
                        SStringW strSmiley = L"<smiley";
                        if (SUCCEEDED(source->GetID(&uID)))
                        {
                            strSmiley += SStringW().Format(L" id=\"%d\"", uID);
                        }

                        BSTR strFile;
                        if (SUCCEEDED(source->GetFile(&strFile)))
                        {
                            strSmiley += SStringW().Format(L" path=\"%s\"", strFile);
                            ::SysFreeString(strFile);
                        }
                        strSmiley += L"/>";

                        strMsg += strSmiley;
                    }
                }
                reobj.poleobj->Release();
            }
#endif//_WIN32
        }
    }
    if (iPlainTxtBegin < strTxt.GetLength())
    {
        strMsg += strTxt.Right(strTxt.GetLength() - iPlainTxtBegin);
    }
    return strMsg;
}

SNSEND
