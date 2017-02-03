
#pragma once

#pragma warning(disable: 4200)
#pragma pack(push, 1)

struct BlobHeader_s //Size: 0x14
{
	DWORD dwSequence; //0x00
	DWORD dwFlags; //0x04
	DWORD dwCRC; //0x08
	WORD wRecID; //0x0C
	WORD wTime; //0x0E
	WORD wSize; //0x10
	WORD wTable; //0x12
};

struct BlobPacket_s
{
	BlobHeader_s header;
	BYTE data[];
};

#define DUPEBLOB(name, source) \
 BlobPacket_s *name = (BlobPacket_s *)new BYTE[ BLOBLEN(source) ]; \
 memcpy(name, source, BLOBLEN(source));
#define CREATEBLOB(name, size) \
 BlobPacket_s *name = (BlobPacket_s *)new BYTE[ sizeof(BlobHeader_s) + (size) ]; \
 name->header.wSize = (WORD)(size);
#define DELETEBLOB(name) delete [] name
#define BLOBLEN(x) (sizeof(BlobHeader_s) + (x)->header.wSize)

struct FragHeader_s
{
	DWORD dwSequence;
	DWORD dwID;
	WORD wCount;
	WORD wSize;
	WORD wIndex;
	WORD wGroup;
};

struct FragPacket_s
{
	FragHeader_s header;
	BYTE data[];
};
#pragma pack(pop)
#pragma warning(default: 4200)

#define SafeDelete(x) { if (x) { delete x; x = 0; } }
#define SafeDeleteArray(x) { if (x) { delete [] x; x = 0; } }

//Blob types.
#define BT_NULL 0x00000000 //Used on account login packets.
#define BT_RESENT 0x00000001 //
#define BT_REQUESTLOST 0x00001000 // What value is this now?
// #define BT_FLUSH 0x00000004 // What value is this now?
#define BT_DENY 0x00002000 //
#define BT_ACKSEQUENCE 0x00004000 //
#define BT_DISCONNECT 0x00008000 //
#define BT_CONNECTIONACK 0x00080000 //
// #define BT_CRCUPDATE 0x00000100 Not sure what this is now //
#define BT_FRAGMENTS 0x00000004 // 0x00000200 //
#define BT_RESETTIME 0x00000400 //
//#define BT_STRANSFER 0x00000800 0x100? //Server transfers. Used on login servers, primarily.
#define BT_LOGIN 0x00010000 //
//#define BT_WTRANSFER 0x00020000 0x100? //Server transfers. Used on world servers, primarily.
#define BT_LOGINREPLY 0x00040000 //
// #define BT_WAKE 0x00080000 //
#define BT_TIMEUPDATE 0x01000000 //
#define BT_ECHOREQUEST 0x02000000 //hi u
#define BT_ECHORESPONSE 0x04000000 //hi u
#define BT_FLOW 0x08000000 //hi u
#define BT_ERROR 0x00800000 //

#define BT_USES_CRC 0x2

#define OBJECT_MSG 10 // 0x03
#define PRIVATE_MSG 9 // 0x04
#define FELLOW_MSG 0x05
#define EVENT_MSG 5 // 0x07

#define MAX_BLOB_LEN 0x1E8
#define MAX_FRAGMENT_LEN 0x1C0
#define MAX_MESSAGE_LEN (0x1C0 * 0x10)

#pragma pack(push, 1)
typedef struct loc_s
{
	loc_s() {
		landcell = 0;
		x = 0;
		y = 0;
		z = 0;
	}
	loc_s(DWORD _landcell, float _x, float _y, float _z) {
		landcell = _landcell;
		x = _x;
		y = _y;
		z = _z;
	}
	//inline Vector2D operator/(vec_t fl) const { return Vector2D(x/fl, y/fl); }
	DWORD landcell;
	float x;
	float y;
	float z;
} loc_t;
#pragma pack(pop)

inline bool operator !(const loc_t &v) {
	return !v.landcell ? true : false;
}

#pragma pack(push, 1)
typedef struct heading_s
{
	heading_s() {
		w = 1;
		x = 0;
		y = 0;
		z = 0;
	}
	heading_s(float _w, float _x, float _y, float _z) {
		w = _w;
		x = _x;
		y = _y;
		z = _z;
	}
	float w;
	float x;
	float y;
	float z;
} heading_t;
#pragma pack(pop)

typedef struct placement_s
{
	loc_t origin;
	heading_t angles;
} placement_t;

