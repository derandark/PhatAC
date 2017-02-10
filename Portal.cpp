
#include "StdAfx.h"
#include "Portal.h"
#include "World.h"
#include "ChatMsgs.h"

#define PORTAL_TRIGGER_DISTANCE 2.0f
#define PORTAL_TRIGGER_FREQUENCY 1.0f

CPortal::CPortal()
{
	m_strName = "Portal";
	m_wTypeID = 0x82D;
	m_wIcon = 0x106B;

	m_dwModel = 0x20001B3;

	m_PhysicsState = ETHEREAL_PS | REPORT_COLLISIONS_PS | LIGHTING_ON_PS | GRAVITY_PS;
	m_ItemType = TYPE_PORTAL;
	m_WeenieBitfield = BF_PORTAL | BF_STUCK | BF_ATTACKABLE;

	m_Usability = USEABLE_REMOTE;
	m_UseDistance = 20.0;
	
	m_bHasDestination = false;

	// Arwic, for testing if we want to
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
	if (m_bHasDestination)
	{
		pTarget->Movement_Teleport(m_Destination.origin, m_Destination.angles);
	}
	else
	{
		pTarget->SendNetMessage(ServerText("This portal has no destination set.", 7), PRIVATE_MSG, FALSE, TRUE);
	}
}

BOOL CPortal::ProximityThink()
{
	// Clear old teleports cache
	if ((m_fLastCacheClear + PORTAL_TRIGGER_FREQUENCY) < g_pGlobals->Time())
	{
		m_RecentlyTeleported.clear();
		m_fLastCacheClear = g_pGlobals->Time();
	}

	std::list<CPhysicsObj *> nearbyObjects;
	g_pWorld->EnumNearby(this, PORTAL_TRIGGER_DISTANCE, &nearbyObjects);

	for (std::list<CPhysicsObj *>::iterator i = nearbyObjects.begin(); i != nearbyObjects.end(); i++)
	{
		CPhysicsObj *pOther = *i;

		if (pOther->IsPlayer() && !(pOther->m_PhysicsState & PhysicsState::LIGHTING_ON_PS))
		{
			if (m_RecentlyTeleported.find(pOther->m_dwGUID) == m_RecentlyTeleported.end())
			{
				Teleport(pOther);
				m_RecentlyTeleported.insert(pOther->m_dwGUID);
			}
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

	if ((Vector(pOther->m_Origin) - Vector(m_Origin)).Length() < 2.0)
	{
		Teleport(pOther);
	}
}


