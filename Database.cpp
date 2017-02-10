
#include "StdAfx.h"

//Database access.
#include "Database.h"
#include "AccountDatabase.h"
#include "CharacterDatabase.h"

void CDatabase::FatalError(SQLSMALLINT type, SQLHANDLE *handle)
{
	Kill(__FILE__, __LINE__);
	MsgBox(MB_ICONHAND, "Fatal database error.\r\n");
}

CDatabase::CDatabase()
{
	LOG(Temp, Normal, "Initializing Database..\n");

	m_database = "main.mdb";
	m_pAccountDB = NULL;
	m_pCharDB = NULL;
	m_hSearchHandle = INVALID_HANDLE_VALUE;

	Initialize();

	if (!IsAlive())
		LOG(Temp, Normal, "Database failure!\n");
}

CDatabase::~CDatabase()
{
	Shutdown();

	if (m_hSearchHandle != INVALID_HANDLE_VALUE)
	{
		FindClose(m_hSearchHandle);
		m_hSearchHandle = INVALID_HANDLE_VALUE;
	}
}

void CDatabase::Initialize()
{
	HWND	hSQLWnd = g_pGlobals->GetWindowHandle();

	char*	ci = csprintf("Driver={Microsoft Access Driver (*.mdb)};DBQ=%s\\Data\\%s;", g_pGlobals->GetGameDirectory(), m_database.c_str());
	char	co[MAX_PATH + 120];
	short	colen;
	short	retcode;

	StartSQL();

	retcode = SQLAllocHandle(SQL_HANDLE_DBC, m_hENV, (SQLHDBC FAR *)&m_hDBC);
	if (!(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO || retcode == SQL_NO_DATA))
	{
		FatalError(SQL_HANDLE_DBC, &m_hDBC); return;
	}

	retcode = SQLDriverConnect(m_hDBC, hSQLWnd, (SQLCHAR *)ci, SQL_NTS, (SQLCHAR *)co, MAX_PATH + 120, &colen, SQL_DRIVER_COMPLETE_REQUIRED);
	if (!(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO || retcode == SQL_NO_DATA))
	{
		FatalError(SQL_HANDLE_DBC, &m_hDBC); return;
	}

	retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_hDBC, &m_hSTMT);
	if (!(retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO || retcode == SQL_NO_DATA))
	{
		FatalError(SQL_HANDLE_STMT, &m_hSTMT); return;
	}

	m_pAccountDB = new CAccountDatabase(this);
	m_pCharDB = new CCharacterDatabase(this);

	if (!CreateDirectory("Data", NULL))
	{
		int dwError = GetLastError();

		if (dwError != ERROR_ALREADY_EXISTS)
		{
			MsgBoxError(dwError, "creating data folder");
		}
	}
}

void CDatabase::Shutdown()
{
	SafeDelete(m_pCharDB);
	SafeDelete(m_pAccountDB);

	SQLFreeHandle(SQL_HANDLE_STMT, m_hSTMT);
	SQLDisconnect(m_hDBC);
	SQLFreeHandle(SQL_HANDLE_DBC, m_hDBC);

	EndSQL();
}

void CDatabase::StartSQL()
{
	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hENV);
	SQLSetEnvAttr(m_hENV, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
}

void CDatabase::EndSQL()
{
	SQLFreeHandle(SQL_HANDLE_ENV, m_hENV);
}

FILE* CDatabase::DataFileOpen(const char* filename, const char* mode)
{
	std::string filepath = "Data\\";
	filepath += filename;
	FILE *fp = fopen(filepath.c_str(), mode);
	return fp;
}

FILE* CDatabase::DataFileCreate(const char* filename, const char* mode)
{
	std::string filepath = "Data\\";
	filepath += filename;
	FILE *fp = fopen(filepath.c_str(), mode);
	return fp;
}

BOOL CDatabase::DataFileFindFirst(const char* filemask, WIN32_FIND_DATA* data)
{
	std::string filepath = "Data\\";
	filepath += filemask;

	m_hSearchHandle = FindFirstFile(filepath.c_str(), data);

	return (m_hSearchHandle != INVALID_HANDLE_VALUE) ? TRUE : FALSE;
}

BOOL CDatabase::DataFileFindNext(WIN32_FIND_DATA* data)
{
	if (m_hSearchHandle == INVALID_HANDLE_VALUE)
		return FALSE;

	return FindNextFile(m_hSearchHandle, data);
}

void CDatabase::DataFileFindClose()
{
	if (m_hSearchHandle != INVALID_HANDLE_VALUE)
	{
		FindClose(m_hSearchHandle);
		m_hSearchHandle = INVALID_HANDLE_VALUE;
	}
}

CAccountDatabase* CDatabase::AccountDB()
{
	return m_pAccountDB;
}

CCharacterDatabase* CDatabase::CharDB()
{
	return m_pCharDB;
}




