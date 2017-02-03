
#pragma once

#define MAX_MEALSTRING_LEN 0x0800

class BinaryReader
{
public:
	BinaryReader(BYTE *pData, DWORD dwSize);
	~BinaryReader();

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

	STREAM_OUT(ReadByte, BYTE);
	STREAM_OUT(ReadInt8, char);
	STREAM_OUT(ReadUInt8, BYTE);
	STREAM_OUT(ReadInt16, short);
	STREAM_OUT(ReadUInt16, WORD);
	STREAM_OUT(ReadInt32, int);
	STREAM_OUT(ReadUInt32, DWORD);

	STREAM_OUT(ReadSingle, float);

	__forceinline DWORD ReadPackedDWORD()
	{
		DWORD ret;
		BOUND_CHECK((m_pData + sizeof(WORD)) <= m_pEnd);
		ret = *((WORD *)m_pData);
		if (ret & 0x8000)
		{
			BOUND_CHECK((m_pData + sizeof(DWORD)) <= m_pEnd);
			DWORD src = *((DWORD *)m_pData);
			ret = (((src & 0x3FFF) << 16) | (src >> 16));
			m_pData += sizeof(DWORD);
		}
		else
		{
			m_pData += sizeof(WORD);
		}
		return ret;
	}

	void ReadAlign(void);
	void *ReadArray(size_t size);
	char *ReadString(void);

	BYTE *GetDataStart(void);
	BYTE *GetDataPtr(void);
	BYTE *GetDataEnd(void);
	DWORD GetDataLen(void);
	DWORD GetOffset(void);
	DWORD GetLastError(void);
	DWORD GetDataRemaining(void);

private:
	DWORD m_dwErrorCode;

	BYTE *m_pData;
	BYTE *m_pStart;
	BYTE *m_pEnd;
	std::list<char *> m_lStrings;
};

