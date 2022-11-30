#include <windows.h>
#include "Encoder.h"
#include <string>
#include <assert.h>

using namespace std;
string Encoder::UrlEncode(const string &str)
{
    string strResult;
    size_t nLength = str.length();
    unsigned char* pBytes = (unsigned char*)str.c_str();
    char szAlnum[2];
    char szOther[4];
    for (size_t i = 0; i < nLength; i++)
    {
        if (isalnum((BYTE)str[i]))
        {
            sprintf_s(szAlnum, sizeof(szAlnum), "%c", str[i]);
            strResult.append(szAlnum);
        }
        else if (isspace((BYTE)str[i]))
        {
            strResult.append("+");
        }
        else
        {
            sprintf_s(szOther, sizeof(szOther), "%%%X%X", pBytes[i] >> 4, pBytes[i] % 16);
            strResult.append(szOther);
        }
    }
    return strResult;
}

string Encoder::UrlDecode(const string &str)
{
    string strResult;
    char szTemp[2];
    size_t i = 0;
    size_t nLength = str.length();
    while (i < nLength)
    {
        if (str[i] == '%')
        {
            szTemp[0] = str[i + 1];
            szTemp[1] = str[i + 2];
            strResult += StrToBin(szTemp);
            i = i + 3;
        }
        else if (str[i] == '+')
        {
            strResult += ' ';
            i++;
        }
        else
        {
            strResult += str[i];
            i++;
        }
    }
    return strResult;
}


char Encoder::CharToInt(char ch)
{
    if (ch >= '0' && ch <= '9')
    {
        return (char)(ch - '0');
    }
    if (ch >= 'a' && ch <= 'f')
    {
        return (char)(ch - 'a' + 10);
    }
    if (ch >= 'A' && ch <= 'F')
    {
        return (char)(ch - 'A' + 10);
    }
    return -1;
}

char Encoder::StrToBin(char *pString)
{
    char szBuffer[2];
    char ch;
    szBuffer[0] = CharToInt(pString[0]); //make the B to 11 -- 00001011 
    szBuffer[1] = CharToInt(pString[1]); //make the 0 to 0 -- 00000000 
    ch = (szBuffer[0] << 4) | szBuffer[1]; //to change the BO to 10110000 
    return ch;
}
