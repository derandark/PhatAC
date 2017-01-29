
#pragma once 

class CMonsterPet : public CBaseMonster
{
public:
	CMonsterPet();

	BOOL IdleThink();

	void SetFollow(CPhysicsObj *pTarget);
	BOOL FollowThink();

private:
	DWORD dwTargetID;
};