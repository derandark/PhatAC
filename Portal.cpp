
#include "StdAfx.h"
#include "Portal.h"
#include "World.h"

CPortal::CPortal()
{
	m_strName = "Portal";
	m_wTypeID = 0x82D;
	m_wIcon = 0x106B;

	m_dwModel = 0x20001B3;

	m_PhysicsState = 0xC0C;
	m_ItemType = TYPE_PORTAL;

	m_Usability = USEABLE_REMOTE;
	m_UseDistance = USEDISTANCE_ANYWHERE;
	
	// Arwic, for testing
	m_Destination.origin = loc_t(0xC6A90023, 102.4f, 70.1f, 44.0f);
	m_Destination.angles = heading_t(0.70710677f, 0, 0, 0.70710677f);
}

CPortal::~CPortal()
{
}

void CPortal::Precache()
{
	// Load entity settings (location, and dynamic properties.)

	SetThink(&CPortal::ProximityThink);
	m_fNextThink = g_pGlobals->Time() + 0.1f; 
}

void CPortal::Teleport(CPhysicsObj *pTarget)
{
	pTarget->Movement_Teleport(m_Destination.origin, m_Destination.angles);
}

BOOL CPortal::ProximityThink()
{
	std::list<CPhysicsObj *> nearbyObjects;
	g_pWorld->EnumNearby(this, 2.0f, &nearbyObjects);

	for (std::list<CPhysicsObj *>::iterator i = nearbyObjects.begin(); i != nearbyObjects.end(); i++)
	{
		CPhysicsObj *pOther = *i;

		if (pOther->IsPlayer())
		{
			Teleport(pOther);
		}
	}

	m_fNextThink = g_pGlobals->Time() + 0.1f;
	return TRUE;
}

void CPortal::Use(CPhysicsObj *pOther)
{
	if (!pOther->IsPlayer())
	{
		return;
	}

	if ((Vector(pOther->m_Origin) - Vector(m_Origin)).Length() < 1.0)
	{
		Teleport(pOther);
	}
}

DWORD CPortal::GetDescFlags()
{
	return (CPhysicsObj::GetDescFlags() | BF_PORTAL);
}


