#ifndef _ENCODER_H_
#define _ENCODER_H_
#include <windows.h>
#include <string>

class Encoder {
  public:
    /**
     * URL编码 - 根据RFC 3986标准
     * 保留字符：A-Z a-z 0-9 - _ . ~
     * 空格转换为 + (application/x-www-form-urlencoded)
     * 其他字符转换为 %XX 格式
     */
    static std::string UrlEncode(const std::string &str)
    {
        std::string strResult;
        size_t nLength = str.length();
        
        // 预分配空间以提高性能
        strResult.reserve(nLength * 3);
        
        for (size_t i = 0; i < nLength; i++)
        {
            unsigned char c = (unsigned char)str[i];
            
            // 保留字符：字母、数字
            if ((c >= 'A' && c <= 'Z') || 
                (c >= 'a' && c <= 'z') || 
                (c >= '0' && c <= '9'))
            {
                strResult += (char)c;
            }
            // 保留的特殊字符：- _ . ~
            else if (c == '-' || c == '_' || c == '.' || c == '~')
            {
                strResult += (char)c;
            }
            // 空格转换为 +
            else if (c == ' ')
            {
                strResult += '+';
            }
            // 其他字符进行百分号编码
            else
            {
                char szEncoded[4];
                sprintf_s(szEncoded, sizeof(szEncoded), "%%%02X", c);
                strResult.append(szEncoded);
            }
        }
        
        return strResult;
    }

    /**
     * URL解码 - 处理百分号编码和加号
     * %XX 转换为对应字符
     * + 转换为空格
     */
    static std::string UrlDecode(const std::string &str)
    {
        std::string strResult;
        size_t nLength = str.length();
        
        // 预分配空间
        strResult.reserve(nLength);
        
        for (size_t i = 0; i < nLength; i++)
        {
            if (str[i] == '%' && i + 2 < nLength)
            {
                // 检查后面两个字符是否是有效的十六进制数字
                char high = str[i + 1];
                char low = str[i + 2];
                
                if (IsValidHexChar(high) && IsValidHexChar(low))
                {
                    // 转换十六进制为字节
                    unsigned char byte = (HexCharToValue(high) << 4) | HexCharToValue(low);
                    strResult += (char)byte;
                    i += 2; // 跳过已处理的两个字符
                }
                else
                {
                    // 无效的百分号编码，保留原样
                    strResult += str[i];
                }
            }
            else if (str[i] == '+')
            {
                // 加号转换为空格
                strResult += ' ';
            }
            else
            {
                // 普通字符直接添加
                strResult += str[i];
            }
        }
        
        return strResult;
    }

  private:
    /**
     * 判断字符是否是有效的十六进制字符
     */
    static bool IsValidHexChar(char ch)
    {
        return (ch >= '0' && ch <= '9') ||
               (ch >= 'a' && ch <= 'f') ||
               (ch >= 'A' && ch <= 'F');
    }

    /**
     * 将十六进制字符转换为数值
     */
    static unsigned char HexCharToValue(char ch)
    {
        if (ch >= '0' && ch <= '9')
        {
            return (unsigned char)(ch - '0');
        }
        if (ch >= 'a' && ch <= 'f')
        {
            return (unsigned char)(ch - 'a' + 10);
        }
        if (ch >= 'A' && ch <= 'F')
        {
            return (unsigned char)(ch - 'A' + 10);
        }
        return 0;
    }
};

#endif //_ENCODER_H_