
#include "StdAfx.h"
#include "Logging.h"

CLogger g_Logger;

CLogFile::CLogFile()
{
	m_File = NULL;
}

CLogFile::~CLogFile()
{
	Close();
}

bool CLogFile::Open(const char *filepath)
{
	Close();
	m_File = fopen(filepath, "wt");

	return m_File != NULL;
}

void CLogFile::Close()
{
	if (m_File)
	{
		fclose(m_File);
		m_File = NULL;
	}
}

void CLogFile::Write(const char *text)
{
	if (m_File)
	{
		fwrite(text, sizeof(char), strlen(text), m_File);
	}
}

CLogger::CLogger()
{
}

CLogger::~CLogger()
{
	Close();
}

bool CLogger::Open()
{
	return m_Log.Open(g_pGlobals->GetGameFile("console.txt").c_str());
}

void CLogger::Close()
{
	m_Log.Close();
}

void CLogger::Write(int category, int level, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	int charcount = _vscprintf(format, args) + 1;
	char *charbuffer = new char[charcount];
	_vsnprintf(charbuffer, charcount, format, args);

#ifdef _DEBUG
	OutputDebugStringA(charbuffer);
#endif

	m_Log.Write(charbuffer);

	for (auto& callback : m_LogCallbacks)
	{
		callback(category, level, charbuffer);
	}

	delete[] charbuffer;

	va_end(args);
}

