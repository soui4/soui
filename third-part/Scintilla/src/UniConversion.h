// Scintilla source code edit control
/** @file UniConversion.h
 ** Functions to handle UTF-8 and UTF-16 strings.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef UNICONVERSION_H
#define UNICONVERSION_H
#include <assert.h>

#ifdef SCI_NAMESPACE
namespace Scintilla {
#endif

const int UTF8MaxBytes = 4;

const int unicodeReplacementChar = 0xFFFD;

void UTF8FromUTF16(const unsigned short*uptr, unsigned int tlen, char *putf, unsigned int len);
unsigned int UTF8CharLength(unsigned char ch);

size_t UTF16ToUTF8Length(const unsigned short* uptr, size_t len);
size_t UTF16Length(const char *s, size_t len);
size_t UTF16FromUTF8(const char *s, size_t len, unsigned short* tbuf, size_t tlen);
size_t UTF32ToUTF8Length(const unsigned int* uptr, size_t len);
size_t UTF32Length(const char* s, size_t len);
void UTF8FromUTF32(const unsigned int* uptr, unsigned int tlen, char* putf, unsigned int len);
unsigned int UTF32FromUTF8(const char *s, unsigned int len, unsigned int *tbuf, unsigned int tlen);
unsigned int UTF16FromUTF32Character(unsigned int val, wchar_t *tbuf);

inline unsigned int UTF8Length(const wchar_t* uptr, unsigned int tlen) {
#if (WCHAR_SIZE==2)
	assert(sizeof(wchar_t) == 2);
	return (unsigned int)UTF16ToUTF8Length((const unsigned short*)uptr, tlen);
#else
	assert(sizeof(wchar_t) == 4);
	return (unsigned int)UTF32ToUTF8Length((const unsigned int*)uptr, tlen);
#endif
}

inline void UTF8FromWideChar(const wchar_t* uptr, unsigned int tlen, char* putf, unsigned int len) {
#if (WCHAR_SIZE==2)
	assert(sizeof(wchar_t) == 2);
	UTF8FromUTF16((const unsigned short*)uptr, tlen, putf, len);
#else
	assert(sizeof(wchar_t) == 4);
	UTF8FromUTF32((const unsigned int*)uptr, tlen, putf, len);
#endif
}

inline size_t WideCharLength(const char* s, size_t len) {
#if (WCHAR_SIZE==2)
	assert(sizeof(wchar_t) == 2);
	return UTF16Length(s, len);
#else 
	assert(sizeof(wchar_t) == 4);
	return UTF32Length(s, len);
#endif
}

inline size_t WideCharFromUTF8(const char* s, size_t len, wchar_t* tbuf, size_t tlen) {
#if (WCHAR_SIZE==2)
	assert(sizeof(wchar_t) == 2);
	return UTF16FromUTF8(s, len, (unsigned short*)tbuf, tlen);
#else
	assert(sizeof(wchar_t) == 4);
	return UTF32FromUTF8(s, len, (unsigned int*)tbuf, tlen);
#endif
}

extern int UTF8BytesOfLead[256];
void UTF8BytesOfLeadInitialise();

inline bool UTF8IsTrailByte(int ch) {
	return (ch >= 0x80) && (ch < 0xc0);
}

inline bool UTF8IsAscii(int ch) {
	return ch < 0x80;
}

enum { UTF8MaskWidth=0x7, UTF8MaskInvalid=0x8 };
int UTF8Classify(const unsigned char *us, int len);

// Similar to UTF8Classify but returns a length of 1 for invalid bytes
// instead of setting the invalid flag
int UTF8DrawBytes(const unsigned char *us, int len);

// Line separator is U+2028 \xe2\x80\xa8
// Paragraph separator is U+2029 \xe2\x80\xa9
const int UTF8SeparatorLength = 3;
inline bool UTF8IsSeparator(const unsigned char *us) {
	return (us[0] == 0xe2) && (us[1] == 0x80) && ((us[2] == 0xa8) || (us[2] == 0xa9));
}

// NEL is U+0085 \xc2\x85
const int UTF8NELLength = 2;
inline bool UTF8IsNEL(const unsigned char *us) {
	return (us[0] == 0xc2) && (us[1] == 0x85);
}

enum { SURROGATE_LEAD_FIRST = 0xD800 };
enum { SURROGATE_LEAD_LAST = 0xDBFF };
enum { SURROGATE_TRAIL_FIRST = 0xDC00 };
enum { SURROGATE_TRAIL_LAST = 0xDFFF };
enum { SUPPLEMENTAL_PLANE_FIRST = 0x10000 };

inline unsigned int UTF16CharLength(wchar_t uch) {
	return ((uch >= SURROGATE_LEAD_FIRST) && (uch <= SURROGATE_LEAD_LAST)) ? 2 : 1;
}

#ifdef SCI_NAMESPACE
}
#endif

#endif
