
#pragma once

///////////////////////////////////////
//
//CRC.H - PhatAC - 02/28/2004
//
//A collection of checksum functions.
//
///////////////////////////////////////


//Well, hello there Scotty.
DWORD GetMagicNumber(BYTE *pbBuf, WORD wSize, BOOL fIncludeSize);
DWORD CalcTransportCRC(DWORD *pdwWoot);

DWORD GenericCRC(BlobPacket_s *);
DWORD BlobCRC(BlobPacket_s *, DWORD dwXOR);

#include "crcwheel.h"