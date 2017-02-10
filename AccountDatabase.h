
#pragma once

class CAccountDatabase
{
public:
	CAccountDatabase(CDatabase *);

	BOOL CheckAccount(const char *, const char *, int *accessLevel, std::string& actualAccount);

private:
	CDatabase *m_DB;
	SQLHANDLE m_hSTMT;
};

