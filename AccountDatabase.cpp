
#include "StdAfx.h"
#include "Database.h"
#include "AccountDatabase.h"
#include "CharacterDatabase.h"
#include "World.h"
#include "ClientCommands.h"

CAccountDatabase::CAccountDatabase(CDatabase *DB)
{
	m_DB = DB;
	m_hSTMT = DB->m_hSTMT;
}

#define ValidAccountChar(x) (((x >= '0') && (x <= '9')) || ((x >= 'A') && (x <= 'Z')) || ((x >= 'a') && (x <= 'z')))

BOOL ValidAccountText(const char *account, const char *password)
{
	if (!account || !password)
		return FALSE;

	int account_len = (int)strlen(account);
	int password_len = (int)strlen(password);

	if (account_len <= 0 || account_len >= 40)
		return FALSE;
	if (password_len <= 0 || password_len >= 20)
		return FALSE;

	for (int i = 0; i < account_len; i++)
	{
		if (!ValidAccountChar(account[i]))
			return FALSE;
	}
	for (int i = 0; i < password_len; i++)
	{
		if (!ValidAccountChar(password[i]))
			return FALSE;
	}

	return TRUE;
}

extern DWORD g_dwMagicNumber;

BOOL CAccountDatabase::CheckAccount(const char *account, const char *password, int *accessLevel)
{
	*accessLevel = BASIC_ACCESS;

	if (!ValidAccountText(account, password))
	{
		LOG(Database, Normal, "Invalid characters in account/password! Username: %s Password: %s\n", account, password);
		return FALSE;
	}

	char szCorrectPassword[50];
	if (!stricmp(account, "admin"))
	{
		_snprintf(szCorrectPassword, 50, "%06lu", g_dwMagicNumber);
		if (!strcmp(password, szCorrectPassword))
		{
			*accessLevel = ADMIN_ACCESS;
			return TRUE;
		}
	}
	szCorrectPassword[0] = '\0';

	char *accountlwr = _strlwr(_strdup(account));
	char *command = csprintf("SELECT Password FROM Accounts WHERE (Username = \'%s\');", accountlwr);
	free(accountlwr);

	SQLPrepare(m_hSTMT, (unsigned char *)command, SQL_NTS);
	SQLExecute(m_hSTMT);

	SQLBindCol(m_hSTMT, 1, SQL_C_CHAR, &szCorrectPassword, 50, NULL);

	RETCODE rc = SQLFetch(m_hSTMT);

	SQLCloseCursor(m_hSTMT);
	SQLFreeStmt(m_hSTMT, SQL_UNBIND);

	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
	{
		if (!strcmp(szCorrectPassword, password))
		{
			//LOG(Temp, Normal, "Successful login from %s:%s\n", account, password);
			return TRUE;
		}
		else
			LOG(Database, Normal, "Bad password on %s:%s (guess: %s)\n", account, szCorrectPassword, password);
	}
	else
	{
		char *accountlwr = _strlwr(_strdup(account));
		LOG(Database, Normal, "Creating new account %s:%s\n", accountlwr, password);
		command = csprintf("INSERT INTO Accounts (Username, Password) VALUES (\'%s\', \'%s\');", accountlwr, password);

		SQLPrepare(m_hSTMT, (unsigned char *)command, SQL_NTS);
		rc = SQLExecute(m_hSTMT);
		SQLFreeStmt(m_hSTMT, SQL_UNBIND);

		if (rc != SQL_ERROR)
		{
			char* szCharacterName = _strlwr(_strdup(account));
			char leadchar = szCharacterName[0];
			if (leadchar > 0x60 && leadchar < 0x7B)
				szCharacterName[0] = leadchar - 0x20;

			CCharacterDatabase* pCharDB;
			if ((pCharDB = m_DB->CharDB()) && g_pWorld)
			{
				_CHARDESC buffer;

				if (!pCharDB->GetCharacterDesc(szCharacterName, &buffer))
				{
					pCharDB->CreateCharacterDesc(accountlwr, g_pWorld->GenerateGUID(ePlayerGUID), szCharacterName);
				}
			}

			free(szCharacterName);
			free(accountlwr);
			return TRUE;
		}

		free(accountlwr);
	}

	return FALSE;
}

