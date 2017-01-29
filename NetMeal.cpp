
#include "StdAfx.h"
#include "NetMeal.h"

NetMeal::NetMeal(BYTE *pData, DWORD dwSize)
{
	m_dwErrorCode = 0;

	m_pData = pData;
	m_pStart = m_pData;
	m_pEnd = m_pStart + dwSize;
}

NetMeal::NetMeal(BlobPacket_s *p)
{
	m_dwErrorCode = 0;

	m_pData = p->data;
	m_pStart = m_pData;
	m_pEnd = m_pStart + p->header.wSize;
}

NetMeal::~NetMeal()
{
	for (std::list<char *>::iterator it = m_lStrings.begin(); it != m_lStrings.end(); it++)
		delete[](*it);

	m_lStrings.clear();
}

void NetMeal::ReadAlign()
{
	DWORD dwOffset = (DWORD)(m_pData - m_pStart);
	if ((dwOffset % 4) != 0)
		m_pData += (4 - (dwOffset % 4));
}

void *NetMeal::ReadArray(size_t size)
{
	void *retval = m_pData;
	m_pData += size;

	if (m_pData > m_pEnd)
	{
		OutputConsole("Error in reading array from data stream.\r\n");
		m_dwErrorCode = 2;
		return NULL;
	}

	return retval;
}

char *NetMeal::ReadString(void)
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

BYTE *NetMeal::GetDataStart()
{
	return m_pStart;
}

BYTE *NetMeal::GetDataPtr()
{
	return m_pData;
}

BYTE *NetMeal::GetDataEnd()
{
	return m_pEnd;
}

DWORD NetMeal::GetDataLen()
{
	return (DWORD)(m_pEnd - m_pStart);
}

DWORD NetMeal::GetOffset()
{
	return (DWORD)(m_pData - m_pStart);
}

DWORD NetMeal::GetLastError()
{
	return m_dwErrorCode;
}







