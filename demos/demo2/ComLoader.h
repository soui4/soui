#ifndef _COMLOADER_H_
#define _COMLOADER_H_

#include <interface/obj-ref-i.h>

typedef enum _SComID{
	Decoder_Gdip=0,
	Decoder_Wic,
	Decoder_Stb,
	Render_Gdi,
	Render_Skia,
	Render_D2D,
	Log4Z,
	Resprovider_7Zip,
	Resprovider_Zip,
	Script_Lua,
	TaskLoop,
	Translator
}SComID;

EXTERN_C
BOOL LoadComObj(SComID id,IObjRef ** ppObj);

#endif//_COMLOADER_H_