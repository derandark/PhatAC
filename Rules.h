

#pragma once

typedef class TurbineXPTable XPTABLE;

class CGameRules
{
public:
	CGameRules();
	virtual ~CGameRules();

	virtual const char *GetGameTitle();
	virtual XPTABLE *GetXPTable();

protected:
	XPTABLE *m_pXPTable;
};

typedef CGameRules GAMERULES;
