
#pragma once

#include "PhysicsObj.h"

class CPortal : public CPhysicsObj
{
public:
	CPortal();
	virtual ~CPortal();

	virtual void Precache();
	virtual void Use(CPhysicsObj *);

	virtual BOOL IsAttackable() override { return FALSE; }
	virtual DWORD GetDescFlags() override;

	void Teleport(CPhysicsObj *pTarget);
	BOOL ProximityThink();

protected:

	placement_t m_Destination;
};