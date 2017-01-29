
#pragma once

struct _CHARDESC {
	char szAccount[60];
	DWORD dwGUID;
	char szName[60];
	DWORD dwDeletePeriod;
	DWORD dwInstances;
	char szWorldClass[40];
};

class CCharacterDatabase
{
public:
	CCharacterDatabase(CDatabase *);

	void CreateCharacterDesc(const char* szAccount, DWORD dwGUID, const char* szName);
	_CHARDESC *GetCharacterDesc(DWORD dwGUID, _CHARDESC *);
	_CHARDESC *GetCharacterDesc(const char* szName, _CHARDESC *);
	DWORD GetCharacters(const char *, DWORD *dwGUIDs);
	DWORD IncCharacterInstance(DWORD dwGUID, DWORD dwLastInstance);

private:
	CDatabase *m_DB;
	SQLHANDLE m_hSTMT;
};

