
#include "StdAfx.h"
#include "BinaryWriter.h"

// Expandable buffer with AC style data IO

BinaryWriter::BinaryWriter()
{
	m_pbData = new BYTE[0x20];
	m_dwDataSize = 0x20;

	m_pbDataPos = m_pbData;
	m_dwSize = 0;
}

BinaryWriter::~BinaryWriter()
{
	if (m_pbData)
	{
		delete[] m_pbData;
		m_pbData = NULL;
	}
}

void BinaryWriter::ExpandBuffer(size_t len)
{
	if (m_dwDataSize < len)
	{
		DWORD dwExpandedSize;
		if (len <= (m_dwDataSize + 0x80))
			dwExpandedSize = m_dwDataSize + 0x80;
		else
			dwExpandedSize = (DWORD)len;

		BYTE *pbExpandedBuffer = new BYTE[dwExpandedSize];
		memcpy(pbExpandedBuffer, m_pbData, m_dwSize);

		delete[] m_pbData;
		m_pbData = pbExpandedBuffer;
		m_dwDataSize = dwExpandedSize;
		m_pbDataPos = m_pbData + m_dwSize;
	}
}

void BinaryWriter::AppendString(const char *szString)
{
	if (szString)
	{
		WORD len = (WORD)strlen(szString);
		AppendData(&len, sizeof(WORD));
		AppendData(szString, len);
	}
	else
		AppendData((WORD)0);

	Align();
}

void BinaryWriter::AppendData(const void *pData, size_t len)
{
	ExpandBuffer(m_dwSize + len);

	memcpy(m_pbDataPos, pData, len);
	m_pbDataPos += len;
	m_dwSize += (DWORD)len;
}

void BinaryWriter::Align(void)
{
	DWORD offset = DWORD(m_pbDataPos - m_pbData);

	if ((offset % 4) != 0)
	{
		DWORD len = 4 - (offset % 4);

		ExpandBuffer(m_dwSize + len);

		memset(m_pbDataPos, 0, len);
		m_pbDataPos += len;
		m_dwSize += len;
	}
}

BYTE* BinaryWriter::GetData(void)
{
	return m_pbData;
}

DWORD BinaryWriter::GetSize(void)
{
	return m_dwSize;
}

