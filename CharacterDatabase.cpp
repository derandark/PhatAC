
#include "StdAfx.h"
#include "Database.h"
#include "CharacterDatabase.h"

CCharacterDatabase::CCharacterDatabase(CDatabase *DB)
{
	m_DB = DB;
	m_hSTMT = DB->m_hSTMT;
}

_CHARDESC *CCharacterDatabase::GetCharacterDesc(DWORD dwGUID, _CHARDESC *pBuffer)
{
	char *command = csprintf("SELECT Account, Name, DeletePeriod, Instances, WorldClass FROM Characters WHERE ID=%u;", dwGUID);

	SQLPrepare(m_hSTMT, (unsigned char *)command, SQL_NTS);
	SQLExecute(m_hSTMT);

	pBuffer->dwGUID = dwGUID;
	*pBuffer->szAccount = 0;
	*pBuffer->szName = 0;
	pBuffer->dwDeletePeriod = 0;
	pBuffer->dwInstances = 0;
	*pBuffer->szWorldClass = 0;

	SQLBindCol(m_hSTMT, 1, SQL_C_CHAR, &pBuffer->szAccount, 60, NULL);
	SQLBindCol(m_hSTMT, 2, SQL_C_CHAR, &pBuffer->szName, 60, NULL);
	SQLBindCol(m_hSTMT, 3, SQL_C_ULONG, &pBuffer->dwDeletePeriod, sizeof(DWORD), NULL);
	SQLBindCol(m_hSTMT, 4, SQL_C_ULONG, &pBuffer->dwInstances, sizeof(DWORD), NULL);
	SQLBindCol(m_hSTMT, 5, SQL_C_CHAR, &pBuffer->szWorldClass, 40, NULL);

	RETCODE retcode = SQLFetch(m_hSTMT);
	if (!(retcode == SQL_SUCCESS) && !(retcode == SQL_SUCCESS_WITH_INFO))
		pBuffer = NULL;

	SQLCloseCursor(m_hSTMT);
	SQLFreeStmt(m_hSTMT, SQL_UNBIND);

	return pBuffer;
}

_CHARDESC *CCharacterDatabase::GetCharacterDesc(const char* szName, _CHARDESC *pBuffer)
{
	char *command = csprintf("SELECT Account, ID, DeletePeriod, Instances, WorldClass FROM Characters WHERE ((Name = \'%s\'));", szName);

	SQLPrepare(m_hSTMT, (unsigned char *)command, SQL_NTS);
	SQLExecute(m_hSTMT);

	*pBuffer->szAccount = 0;
	pBuffer->dwGUID = 0;
	strncpy(pBuffer->szName, szName, 60);
	pBuffer->dwDeletePeriod = 0;
	pBuffer->dwInstances = 0;
	*pBuffer->szWorldClass = 0;

	SQLBindCol(m_hSTMT, 1, SQL_C_CHAR, &pBuffer->szAccount, 60, NULL);
	SQLBindCol(m_hSTMT, 2, SQL_C_ULONG, &pBuffer->dwGUID, sizeof(DWORD), NULL);
	SQLBindCol(m_hSTMT, 3, SQL_C_ULONG, &pBuffer->dwDeletePeriod, sizeof(DWORD), NULL);
	SQLBindCol(m_hSTMT, 4, SQL_C_ULONG, &pBuffer->dwInstances, sizeof(DWORD), NULL);
	SQLBindCol(m_hSTMT, 5, SQL_C_CHAR, &pBuffer->szWorldClass, 40, NULL);

	RETCODE retcode = SQLFetch(m_hSTMT);
	if (!(retcode == SQL_SUCCESS) && !(retcode == SQL_SUCCESS_WITH_INFO))
		pBuffer = NULL;

	SQLCloseCursor(m_hSTMT);
	SQLFreeStmt(m_hSTMT, SQL_UNBIND);

	return pBuffer;
}

void CCharacterDatabase::CreateCharacterDesc(const char* szAccount, DWORD dwGUID, const char* szName)
{
	char *command;

	command = csprintf("INSERT INTO Characters (Account, ID, Name, DeletePeriod, Instances, WorldClass) VALUES ('%s', %lu, '%s', 0, 1, 'human-male');", szAccount, dwGUID, szName);

	SQLPrepare(m_hSTMT, (unsigned char *)command, SQL_NTS);
	SQLExecute(m_hSTMT);
	SQLFreeStmt(m_hSTMT, SQL_UNBIND);
}

DWORD CCharacterDatabase::IncCharacterInstance(DWORD dwGUID, DWORD dwLastInstance)
{
	DWORD dwNewInstance = dwLastInstance + 1;
	char *command = csprintf("UPDATE Characters SET Instances=%u WHERE ID=%u;", dwNewInstance, dwGUID);

	SQLPrepare(m_hSTMT, (unsigned char *)command, SQL_NTS);
	SQLExecute(m_hSTMT);

	SQLCloseCursor(m_hSTMT);
	SQLFreeStmt(m_hSTMT, SQL_UNBIND);

	return dwNewInstance;
}

DWORD CCharacterDatabase::GetCharacters(const char *account, DWORD *dwGUIDs)
{
	char *accountlwr = strlwr(strdup(account));
	char *command = csprintf("SELECT ID FROM Characters WHERE (LCase(Account) = \'%s\');", accountlwr);
	free(accountlwr);

	SQLPrepare(m_hSTMT, (unsigned char *)command, SQL_NTS);
	SQLExecute(m_hSTMT);

	DWORD dwCount = 0;
	DWORD dwGUID;

	SQLBindCol(m_hSTMT, 1, SQL_C_ULONG, &dwGUID, sizeof(DWORD), NULL);

	RETCODE rc;

	while (dwCount < 5)
	{
		rc = SQLFetch(m_hSTMT);
		if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
		{
			dwGUIDs[dwCount] = dwGUID;
			dwCount++;
		}
		else
			break;
	}

	//MsgBox("Chars: %lu", dwCount);

	SQLCloseCursor(m_hSTMT);
	SQLFreeStmt(m_hSTMT, SQL_UNBIND);

	return dwCount;
}

