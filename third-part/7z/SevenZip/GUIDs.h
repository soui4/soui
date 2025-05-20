// This file is based on the following file from the LZMA SDK (http://www.7-zip.org/sdk.html):
//   ./CPP/7zip/Guid.txt
#ifndef _GUIDS_H_
#define _GUIDS_H_

#include "../CPP/Common/MyCom.h"
#include "../CPP/7zip/IDecl.h"

namespace SevenZip
{
	DEFINE_GUID(IID_IMyStream, k_7zip_GUID_Data1, k_7zip_GUID_Data2, k_7zip_GUID_Data3_Common, 0, 0, 0, (3), 0, (0x0A), 0, 0);
namespace intl
{
	// IStream.h

	//// {23170F69-40C1-278A-0000-000300010000}
	//DEFINE_GUID(IID_ISequentialInStream,
	//	0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00);

	//// {23170F69-40C1-278A-0000-000300010000}
	//DEFINE_GUID(IID_ISequentialOutStream,
	//	0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x02, 0x00, 0x00);

	//// {23170F69-40C1-278A-0000-000300030000}
	//DEFINE_GUID(IID_IInStream,
	//	0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00);

	//// {23170F69-40C1-278A-0000-000300040000}
	//DEFINE_GUID(IID_IOutStream,
	//	0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x04, 0x00, 0x00);

	//// {23170F69-40C1-278A-0000-000300060000}
	//DEFINE_GUID(IID_IStreamGetSize,
	//	0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x06, 0x00, 0x00);

	//// ICoder.h
	//// {23170F69-40C1-278A-0000-000400040000}
	//DEFINE_GUID(IID_ICompressProgressInfo,
	//	0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00);

	//// IPassword.h

	//// {23170F69-40C1-278A-0000-000500100000}
	//DEFINE_GUID(IID_ICryptoGetTextPassword,
	//	0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x05, 0x00, 0x10, 0x00, 0x00);

	//// {23170F69-40C1-278A-0000-000500110000}
	//DEFINE_GUID(IID_ICryptoGetTextPassword2,
	//	0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x05, 0x00, 0x11, 0x00, 0x00);

	//// IArchive.h

	//// {23170F69-40C1-278A-0000-000600030000}
	//DEFINE_GUID(IID_ISetProperties,
	//	0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x03, 0x00, 0x00);

	//// {23170F69-40C1-278A-0000-000600100000}
	//DEFINE_GUID(IID_IArchiveOpenCallback,
	//	0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x10, 0x00, 0x00);

	//// {23170F69-40C1-278A-0000-000600200000}
	//DEFINE_GUID(IID_IArchiveExtractCallback,
	//	0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x20, 0x00, 0x00);

	//// {23170F69-40C1-278A-0000-000600600000}
	//DEFINE_GUID(IID_IInArchive,
	//	0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x60, 0x00, 0x00);

	//// {23170F69-40C1-278A-0000-000600800000}
	//DEFINE_GUID(IID_IArchiveUpdateCallback,
	//	0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x80, 0x00, 0x00);

	//// {23170F69-40C1-278A-0000-000600820000}
	//DEFINE_GUID(IID_IArchiveUpdateCallback2,
	//	0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x82, 0x00, 0x00);

	//// {23170F69-40C1-278A-0000-000600A00000}
	//DEFINE_GUID(IID_IOutArchive,
	//	0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0xA0, 0x00, 0x00);

	// Handler GUIDs

	// {23170F69-40C1-278A-1000-000110010000}
	DEFINE_GUID(CLSID_CFormatZip,
		0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x01, 0x00, 0x00);

	// {23170F69-40C1-278A-1000-000110020000}
	DEFINE_GUID(CLSID_CFormatBZip2,
		0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x02, 0x00, 0x00);

	// {23170F69-40C1-278A-1000-000110030000}
	DEFINE_GUID(CLSID_CFormatRar,
		0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x03, 0x00, 0x00);

	// {23170F69-40C1-278A-1000-000110070000}
	DEFINE_GUID(CLSID_CFormat7z,
		0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);

	// {23170F69-40C1-278A-1000-000110080000}
	DEFINE_GUID(CLSID_CFormatCab,
		0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x08, 0x00, 0x00);

	// {23170F69-40C1-278A-1000-0001100A0000}
	DEFINE_GUID(CLSID_CFormatLzma,
		0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x0A, 0x00, 0x00);

	// {23170F69-40C1-278A-1000-0001100B0000}
	DEFINE_GUID(CLSID_CFormatLzma86,
		0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x0B, 0x00, 0x00);

	// {23170F69-40C1-278A-1000-000110E70000}
	DEFINE_GUID(CLSID_CFormatIso,
		0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xE7, 0x00, 0x00);

	// {23170F69-40C1-278A-1000-000110EE0000}
	DEFINE_GUID(CLSID_CFormatTar,
		0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xEE, 0x00, 0x00);

	// {23170F69-40C1-278A-1000-000110EF0000}
	DEFINE_GUID(CLSID_CFormatGZip,
		0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xEF, 0x00, 0x00);

	Z7_DEFINE_GUID(CLSID_CArchiveHandler,
			k_7zip_GUID_Data1,
			k_7zip_GUID_Data2,
			k_7zip_GUID_Data3_Common,
			0x10, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00);

		Z7_GUID(ISequentialInStream, 3,0x01);
		Z7_GUID(ISequentialOutStream, 3,0x02);
		Z7_GUID(IInStream,3,0x03);
		Z7_GUID(IOutStream,3,0x04);
		Z7_GUID(IStreamGetSize,3,0x06);
		Z7_GUID(IOutStreamFinish,3,0x07);
		Z7_GUID(IStreamGetProps,3,0x08);
		Z7_GUID(IStreamGetProps2,3,0x09);
		Z7_GUID(IStreamGetProp,3,0x0a);
		Z7_GUID(IStreamSetRestriction,3,0x10);

		Z7_GUID(IArchiveOpenCallback,6,0x10);
		Z7_GUID(IArchiveExtractCallback,6,0x20);
		Z7_GUID(IArchiveExtractCallbackMessage2, 6, 0x22);
		Z7_GUID(IArchiveOpenVolumeCallback, 6, 0x30);
		Z7_GUID(IInArchiveGetStream, 6, 0x40);
		Z7_GUID(IArchiveOpenSetSubArchiveName, 6, 0x50);
		Z7_GUID(IInArchive, 6, 0x60);
		Z7_GUID(IArchiveGetRawProps, 6, 0x70);
		Z7_GUID(IArchiveGetRootProps, 6, 0x71);
		Z7_GUID(IArchiveOpenSeq, 6, 0x61);
		Z7_GUID(IArchiveOpen2, 6, 0x62);
		Z7_GUID(IArchiveUpdateCallback, 6,  0x80);
		Z7_GUID(IArchiveUpdateCallback2,  6, 0x82);
		Z7_GUID(IArchiveUpdateCallbackFile, 6, 0x83);
		Z7_GUID(IArchiveGetDiskProperty, 6, 0x84);
		Z7_GUID(IArchiveUpdateCallbackArcProp, 6, 0x85);
		Z7_GUID(IOutArchive, 6, 0xA0);
		Z7_GUID(ISetProperties, 6, 0x03);
		Z7_GUID(IArchiveKeepModeForNextOpen, 6, 0x04);
		Z7_GUID(IArchiveAllowTail, 6, 0x05);
		Z7_GUID(IArchiveRequestMemoryUseCallback, 6, 0x09);


		#define Z7_IFACE_CONSTR_GUID(i, n) Z7_GUID(i,4, n); 
Z7_IFACE_CONSTR_GUID(ICompressProgressInfo, 0x04)
Z7_IFACE_CONSTR_GUID(ICompressCoder, 0x05)
Z7_IFACE_CONSTR_GUID(ICompressCoder2, 0x18)
Z7_IFACE_CONSTR_GUID(ICompressSetCoderPropertiesOpt, 0x1F)
Z7_IFACE_CONSTR_GUID(ICompressSetCoderProperties, 0x20)
Z7_IFACE_CONSTR_GUID(ICompressSetDecoderProperties, 0x21)
Z7_IFACE_CONSTR_GUID(ICompressSetDecoderProperties2, 0x22)
Z7_IFACE_CONSTR_GUID(ICompressWriteCoderProperties, 0x23)
Z7_IFACE_CONSTR_GUID(ICompressGetInStreamProcessedSize, 0x24)
Z7_IFACE_CONSTR_GUID(ICompressSetCoderMt, 0x25)
Z7_IFACE_CONSTR_GUID(ICompressSetFinishMode, 0x26)
Z7_IFACE_CONSTR_GUID(ICompressGetInStreamProcessedSize2, 0x27)
Z7_IFACE_CONSTR_GUID(ICompressSetMemLimit, 0x28)
Z7_IFACE_CONSTR_GUID(ICompressReadUnusedFromInBuf, 0x29)
Z7_IFACE_CONSTR_GUID(ICompressGetSubStreamSize, 0x30)
Z7_IFACE_CONSTR_GUID(ICompressSetInStream, 0x31)
Z7_IFACE_CONSTR_GUID(ICompressSetOutStream, 0x32)
Z7_IFACE_CONSTR_GUID(ICompressSetInStreamSize, 0x33)
Z7_IFACE_CONSTR_GUID(ICompressSetOutStreamSize, 0x34)
Z7_IFACE_CONSTR_GUID(ICompressSetBufSize, 0x35)
Z7_IFACE_CONSTR_GUID(ICompressInitEncoder, 0x36)
Z7_IFACE_CONSTR_GUID(ICompressSetInStream2, 0x37)
Z7_IFACE_CONSTR_GUID(ICompressSetOutStream2, 0x38)
Z7_IFACE_CONSTR_GUID(ICompressSetInStreamSize2, 0x39)
Z7_IFACE_CONSTR_GUID(ICompressInSubStreams, 0x3A)
Z7_IFACE_CONSTR_GUID(ICompressOutSubStreams, 0x3B)
Z7_IFACE_CONSTR_GUID(ICompressFilter, 0x40)
Z7_IFACE_CONSTR_GUID(ICompressCodecsInfo, 0x60)
Z7_IFACE_CONSTR_GUID(ISetCompressCodecsInfo, 0x61)
Z7_IFACE_CONSTR_GUID(ICryptoProperties, 0x80)
Z7_IFACE_CONSTR_GUID(ICryptoResetSalt, 0x88)
Z7_IFACE_CONSTR_GUID(ICryptoResetInitVector, 0x8C)
Z7_IFACE_CONSTR_GUID(ICryptoSetPassword, 0x90)
Z7_IFACE_CONSTR_GUID(ICryptoSetCRC, 0xA0)
Z7_IFACE_CONSTR_GUID(IHasher, 0xC0)
Z7_IFACE_CONSTR_GUID(IHashers, 0xC1)

#define Z7_IFACE_CONSTR_PASSWORD_GUID(i,n) Z7_GUID(i,5,n);
Z7_IFACE_CONSTR_PASSWORD_GUID(ICryptoGetTextPassword, 0x10)
Z7_IFACE_CONSTR_PASSWORD_GUID(ICryptoGetTextPassword2, 0x11)

}
}

#endif//_GUIDS_H_