#ifndef _SOUI_COM_MASK_H_
#define _SOUI_COM_MASK_H_

#define scom_mask_translator  (1 << 0)
#define scom_mask_log4z  (1 << 1)
#define scom_mask_taskloop  (1 << 2)

#define scom_mask_render_skia  (1 << 3)
#define scom_mask_render_gdi  (1 << 4)
#define scom_mask_render_d2d  (1 << 14)

#define scom_mask_imgdecoder_gdip  (1 << 5)
#define scom_mask_imgdecoder_stb  (1 << 7)
#define scom_mask_imgdecoder_wic  (1 << 8)

#define scom_mask_resprovider_zip  (1 << 9)
#define scom_mask_resprovider_7z  (1 << 10)
#define scom_mask_script_lua  (1 << 11)

#define scom_mask_ipcobject   (1<<12)
#define scom_mask_httpclient   (1<<13)

#define scom_mask_scom_all  (0xffffffff)

#endif //_SOUI_COM_MASK_H_