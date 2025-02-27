#ifndef _ENCODER_H_
#define _ENCODER_H_
#include <string>

class Encoder
{
public:

    static std::string UrlEncode(const std::string &str);
    static std::string UrlDecode(const std::string &str);

private:

    static char CharToInt(char ch);
    static char StrToBin(char *pString);

};

#endif//_ENCODER_H_