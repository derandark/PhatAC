
#pragma once

// For data input

#define MAX_MEALSTRING_LEN 0x0800

class NetMeal
{
public:
	NetMeal(BYTE *pData, DWORD dwSize);
	NetMeal(BlobPacket_s *bp);
	~NetMeal();

#define BOUND_CHECK(x) \
	if ( !(x) ) \
	{ \
		m_dwErrorCode = 1; \
		memset(&ret, 0, sizeof(ret)); \
		return ret; \
	}
#define STREAM_OUT(func, type) \
	__forceinline type func () \
	{ \
		type ret; \
		BOUND_CHECK((m_pData + sizeof(type)) <= m_pEnd); \
		ret = *((type *)m_pData); \
		m_pData += sizeof(type); \
		return ret; \
	}
	STREAM_OUT(ReadChar, BYTE);
	STREAM_OUT(ReadShort, BYTE);
	STREAM_OUT(ReadLong, BYTE);

	STREAM_OUT(ReadBYTE, BYTE);
	STREAM_OUT(ReadWORD, WORD);
	STREAM_OUT(ReadDWORD, DWORD);
	STREAM_OUT(ReadFloat, float);
	STREAM_OUT(ReadDouble, double);

	void ReadAlign(void);
	void *ReadArray(size_t size);
	char *ReadString(void);

	BYTE *GetDataStart(void);
	BYTE *GetDataPtr(void);
	BYTE *GetDataEnd(void);
	DWORD GetDataLen(void);
	DWORD GetOffset(void);
	DWORD GetLastError(void);

private:
	DWORD m_dwErrorCode;

	BYTE *m_pData;
	BYTE *m_pStart;
	BYTE *m_pEnd;
	std::list<char *> m_lStrings;
};
