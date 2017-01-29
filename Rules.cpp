
#include "StdAfx.h"
#include "Rules.h"

#include "TurbineXPTable.h"

// The base rules class, can implement basic functionality here and override it if we want
// This could be used if we want to do some kind of crazy game modes with fun variations

CGameRules::CGameRules()
{
	m_pXPTable = g_pPortal ? g_pPortal->GetXPTable(0x0E000018) : NULL;
}

CGameRules::~CGameRules()
{
}

const char*	CGameRules::GetGameTitle()
{
	return "PhatAC";
}

XPTABLE* CGameRules::GetXPTable()
{
	return m_pXPTable;
}




