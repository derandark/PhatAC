
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
		memset(&returnValue, 0, sizeof(returnValue)); \
		return returnValue; \
	}

	template <typename ReturnType> ReturnType Read()
	{
		ReturnType returnValue;
		BOUND_CHECK((m_pData + sizeof(ReturnType)) <= m_pEnd);
		returnValue = *((ReturnType *)m_pData);
		m_pData += sizeof(ReturnType);
		return returnValue;
	}

#define STREAM_OUT(func, type) type func() { return Read<type>(); }
	
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
		DWORD returnValue;
		BOUND_CHECK((m_pData + sizeof(WORD)) <= m_pEnd);
		returnValue = *((WORD *)m_pData);
		if (returnValue & 0x8000)
		{
			BOUND_CHECK((m_pData + sizeof(DWORD)) <= m_pEnd);
			DWORD src = *((DWORD *)m_pData);
			returnValue = (((src & 0x3FFF) << 16) | (src >> 16));
			m_pData += sizeof(DWORD);
		}
		else
		{
			m_pData += sizeof(WORD);
		}
		return returnValue;
	}

	template<typename A, typename B> std::map<A, B> ReadMap()
	{
		std::map<A, B> table;

		WORD count = ReadWORD();
		ReadWORD();

		while (count > 0 && !m_dwErrorCode)
		{
			A theKey = Read<A>();
			B theValue = Read<B>();
			table.insert(std::pair<A, B>(theKey, theValue));
			count--;
		}

		return table;
	}

	template<typename A> std::map<A, std::string> ReadMap()
	{
		std::map<A, std::string> table;

		WORD count = ReadWORD();
		ReadWORD();

		while (count > 0 && !m_dwErrorCode)
		{
			A theKey = Read<A>();
			std::string theValue = ReadString();
			table.insert(std::pair<A, std::string>(theKey, theValue));
			count--;
		}

		return table;
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

	void SetOffset(DWORD offset);

private:
	DWORD m_dwErrorCode;

	BYTE *m_pData;
	BYTE *m_pStart;
	BYTE *m_pEnd;
	std::list<char *> m_lStrings;
};

