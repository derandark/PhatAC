
#pragma once

#include <sql.h>
#include <sqlext.h>

class CAccountDatabase;
class CCharacterDatabase;

class CDatabase : public CKillable
{
	friend class CAccountDatabase;
	friend class CCharacterDatabase;

public:
	CDatabase();
	~CDatabase();

	FILE *DataFileOpen(const char*, const char* mode = "rb");
	FILE *DataFileCreate(const char*, const char* mode = "wb");
	BOOL DataFileFindFirst(const char* filemask, WIN32_FIND_DATA* data);
	BOOL DataFileFindNext(WIN32_FIND_DATA* data);
	void DataFileFindClose();

	CAccountDatabase *AccountDB();
	CCharacterDatabase *CharDB();

private:
	void FatalError(SQLSMALLINT type, SQLHANDLE *handle);

	//Startup and shutdown routines.
	void Initialize();
	void Shutdown();
	void StartSQL();
	void EndSQL();

	std::string m_database;

	HENV m_hENV;
	HDBC m_hDBC;
	HSTMT m_hSTMT;
	int	m_column;

	CAccountDatabase *m_pAccountDB;
	CCharacterDatabase *m_pCharDB;

	HANDLE m_hSearchHandle;
};


