
#pragma once

#include "PhysicsObj.h"

class CPortal : public CPhysicsObj
{
public:
	CPortal();
	virtual ~CPortal();

	virtual void Precache();
	virtual void Use(CPhysicsObj *);

	void Teleport(CPhysicsObj *pTarget);
	BOOL ProximityThink();

	bool m_bHasDestination;
	placement_t m_Destination;

	double m_fLastCacheClear;
	std::set<DWORD> m_RecentlyTeleported;
};