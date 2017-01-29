
#include "StdAfx.h"
#include "vitals.h"

#include "TurbineXPTable.h"
#include "Rules.h"

const char *GetVitalName(eVital index)
{
	switch (index)
	{
	case eHealth: return "Health";
	case eStamina: return "Stamina";
	case eMana: return "Mana";
	default: return "";
	}
}

DWORD GetVitalMaxXP()
{
	XPTABLE* pXPTable = g_pGameRules->GetXPTable();

	if (pXPTable)
		return pXPTable->GetVitalMaxXP();
	else
		return 0;
}

DWORD GetVitalXP(DWORD Level)
{
	XPTABLE* pXPTable = g_pGameRules->GetXPTable();

	if (pXPTable)
		return pXPTable->GetVitalXP(Level);
	else
		return 0;
}

DWORD GetVitalLevel(DWORD XP)
{
	XPTABLE* pXPTable = g_pGameRules->GetXPTable();

	if (pXPTable)
		return pXPTable->GetVitalLevel(XP);
	else
		return 0;
}

float GetVitalPercent(VITAL *pVital, float fVitae)
{
	DWORD dwMin = GetVitalMin(pVital, fVitae);
	DWORD dwMax = GetVitalMax(pVital, fVitae);
	DWORD dwCurrent = GetVitalCurrent(pVital, fVitae);

	DWORD dwRange = dwMax - dwMin;
	if (!dwRange)
		return 0;

	float flFraction = (dwCurrent - dwMin) / (float)dwRange;
	return flFraction;
}

DWORD GetVitalCurrent(VITAL *pVital, float fVitae)
{
	return pVital->data.current;
}

DWORD GetVitalMin(VITAL *pVital, float fVitae)
{
	return 0;
}

DWORD GetVitalMax(VITAL *pVital, float fVitae)
{
	return (pVital->data.raises);
}


