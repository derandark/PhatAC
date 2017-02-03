
#include "StdAfx.h"
#include "BinaryReader.h"

BinaryReader::BinaryReader(BYTE *pData, DWORD dwSize)
{
	m_dwErrorCode = 0;

	m_pData = pData;
	m_pStart = m_pData;
	m_pEnd = m_pStart + dwSize;
}

BinaryReader::~BinaryReader()
{
	for (std::list<char *>::iterator it = m_lStrings.begin(); it != m_lStrings.end(); it++)
		delete[](*it);

	m_lStrings.clear();
}

void BinaryReader::ReadAlign()
{
	DWORD dwOffset = (DWORD)(m_pData - m_pStart);
	if ((dwOffset % 4) != 0)
		m_pData += (4 - (dwOffset % 4));
}

void *BinaryReader::ReadArray(size_t size)
{
	static BYTE dummyData[10000];

	void *retval = m_pData;
	m_pData += size;

	if (m_pData > m_pEnd)
	{
		m_dwErrorCode = 2;
		return dummyData;
	}

	return retval;
}

char *BinaryReader::ReadString(void)
{
	WORD wLen = ReadWORD();

	if (m_dwErrorCode || wLen > MAX_MEALSTRING_LEN)
		return NULL;

	char *szArray = (char *)ReadArray(wLen);
	if (!szArray)
		return NULL;

	char *szString = new char[wLen + 1];
	szString[wLen] = 0;
	memcpy(szString, szArray, wLen);
	m_lStrings.push_back(szString);

	ReadAlign();

	return szString;
}

BYTE *BinaryReader::GetDataStart()
{
	return m_pStart;
}

BYTE *BinaryReader::GetDataPtr()
{
	return m_pData;
}

BYTE *BinaryReader::GetDataEnd()
{
	return m_pEnd;
}

DWORD BinaryReader::GetDataLen()
{
	return (DWORD)(m_pEnd - m_pStart);
}

DWORD BinaryReader::GetOffset()
{
	return (DWORD)(m_pData - m_pStart);
}

DWORD BinaryReader::GetLastError()
{
	return m_dwErrorCode;
}

DWORD BinaryReader::GetDataRemaining()
{
	return m_pEnd - m_pData;
}



