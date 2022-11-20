#pragma once

namespace SOUI
{
    struct IRenderFactory;
    typedef enum { ZIP7_FILE, ZIP7_PEDATA } ZIP7_TYPE;
    struct ZIP7RES_PARAM
    {
        ZIP7_TYPE type;
        IRenderFactory *pRenderFac;
        union{
            LPCTSTR pszZipFile;
			
            struct{
                HINSTANCE hInst;
                LPCTSTR pszResName;
                LPCTSTR pszResType;
            }peInfo;
        };
        LPCSTR          pszPsw; 
		LPCTSTR			pszChildDir;
    };
    
         inline   void Zip7File(ZIP7RES_PARAM *param,IRenderFactory *_pRenderFac,LPCTSTR _pszFile,LPCSTR _pszPsw =NULL, LPCTSTR _pszChildDir = NULL)
        {
            param->type=ZIP7_FILE;
            param->pszZipFile = _pszFile;
			param->pszChildDir = _pszChildDir;
            param->pRenderFac = _pRenderFac;
            param->pszPsw     = _pszPsw;
        }
       inline  void Zip7Resource(ZIP7RES_PARAM *param,IRenderFactory *_pRenderFac,HINSTANCE hInst,LPCTSTR pszResName,LPCTSTR pszResType=_T("zip"),LPCSTR _pszPsw =NULL, LPCTSTR _pszChildDir = NULL)
        {
            param->type=ZIP7_PEDATA;
            param->pRenderFac = _pRenderFac;
			param->pszChildDir = _pszChildDir;
            param->peInfo.hInst=hInst;
            param->peInfo.pszResName=pszResName;
            param->peInfo.pszResType=pszResType;
            param->pszPsw     = _pszPsw;
        }

}