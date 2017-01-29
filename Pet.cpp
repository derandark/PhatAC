

#include "StdAfx.h"
#include "PhysicsObj.h"
#include "Monster.h"
#include "World.h"
#include "pet.h"

CMonsterPet::CMonsterPet()
{
	dwTargetID = 0;

	SetThink(&CMonsterPet::IdleThink);
}

void CMonsterPet::SetFollow(CPhysicsObj *pTarget)
{
	if (!pTarget)
		return;
	if (pTarget->IsItem())
		return;

	dwTargetID = pTarget->m_dwGUID;

	SetThink(&CMonsterPet::FollowThink);
}

BOOL CMonsterPet::IdleThink()
{
	CBaseMonster::MonsterThink();
	return TRUE;
}

BOOL CMonsterPet::FollowThink()
{
	CPhysicsObj*	pTarget;

	if (!dwTargetID || !(pTarget = g_pWorld->FindWithinPVS(this, dwTargetID)))
	{
		//We lost our target!
		dwTargetID = 0;
		SetThink(&CMonsterPet::IdleThink);
		return TRUE;
	}

	Vector origin(m_Origin);
	Vector goal(pTarget->m_Origin);
	Vector goal_angle;
	AngleVectors(goal - origin, &goal_angle);

	m_fNextThink = g_pGlobals->Time() + 0.1f;
	CBaseMonster::MonsterThink();
	return TRUE;
}

