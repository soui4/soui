#ifndef _DECODER_APNG_H_
#define _DECODER_APNG_H_

#ifdef __cplusplus
extern "C"{
#endif

struct APNGDATA
{
    unsigned char * pdata;
    unsigned short *pDelay;
    int   nWid,nHei;
    int   nFrames;
    int   nLoops;
};


APNGDATA * LoadAPNG_from_file(const wchar_t * pszFileName);
APNGDATA * LoadAPNG_from_fileA( const char * pszFileName );
APNGDATA * LoadAPNG_from_memory(const char * pBuf, size_t nLen);

void APNG_Destroy(APNGDATA *apng);

#ifdef __cplusplus
}
#endif

#endif//_DECODER_APNG_H_