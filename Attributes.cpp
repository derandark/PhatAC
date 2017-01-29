
#include "StdAfx.h"
#include "attributes.h"

#include "TurbineXPTable.h"
#include "Rules.h"

DWORD GetAttributeXP(DWORD Level)
{
	XPTABLE* pXPTable = g_pGameRules->GetXPTable();

	if (pXPTable)
		return pXPTable->GetAttributeXP(Level);
	else
		return 0;
}

DWORD GetAttributeLevel(DWORD XP)
{
	XPTABLE* pXPTable = g_pGameRules->GetXPTable();

	if (pXPTable)
		return pXPTable->GetAttributeLevel(XP);
	else
		return 0;
}

DWORD GetAttributeMax()
{
	XPTABLE* pXPTable = g_pGameRules->GetXPTable();

	if (pXPTable)
		return pXPTable->GetAttributeMax();
	else
		return 0;
}

DWORD GetAttributeMaxXP()
{
	XPTABLE* pXPTable = g_pGameRules->GetXPTable();

	if (pXPTable)
		return pXPTable->GetAttributeMaxXP();
	else
		return 0;
}

const char* GetAttributeName(eAttribute index)
{
	switch (index)
	{
	case eStrength: return "Strength";
	case eEndurance: return "Endurance";
	case eCoordination: return "Coordination";
	case eQuickness: return "Quickness";
	case eFocus: return "Focus";
	case eSelf: return "Self";
	default: return "";
	}
}

DWORD GetAttributeMax(ATTRIBUTE *pAttrib, float fVitae)
{
	return (pAttrib->data.base + pAttrib->data.raises);
}
