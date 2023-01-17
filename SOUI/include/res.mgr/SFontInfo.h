#pragma once

SNSBEGIN

/**
 * @union      FONTSTYLE
 * @brief      FONT的风格
 *
 * Describe
 */
union FONTSTYLE {
    uint64_t syle; // DWORD版本的风格
    struct
    {
        uint64_t byCharset : 8;  //字符集
        uint64_t byWeight : 8;   // weight/4
        uint64_t fItalic : 1;    //斜体标志位
        uint64_t fUnderline : 1; //下划线标志位
        uint64_t fBold : 1;      //粗体标志位
        uint64_t fStrike : 1;    //删除线标志位
		uint64_t fEscapement : 12;   //角度, 以0.1角度为单位
		uint64_t reserved : 13;   //reserved
		uint64_t szIsAdding : 1;   //cSize is adding
		uint64_t szUnit:2;		//cSize unit,(0-3)
        uint64_t nSize : 16;     //字体大小，为short有符号类型
    } attr;

    FONTSTYLE(uint64_t _style = 0)
        : syle(_style)
    {
    }
};

struct FontInfo
{
    FONTSTYLE style;
    SStringW strFaceName;
    SStringW strPropEx;
	int		 scale;
};
SNSEND