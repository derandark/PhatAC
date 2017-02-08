
#pragma once

#include "attributes.h"
#include "vitals.h"
#include "skills.h"

class CBaseMonster : public CPhysicsObj
{
public:
	CBaseMonster();
	~CBaseMonster();

	void MonsterThink();
	virtual void Precache();

	virtual float GetBurdenPercent();
	virtual void CalculateSpeed();

	virtual BOOL IsMonster() { return TRUE; }

	virtual void PostSpawn();

	virtual DWORD GiveAttributeXP(eAttribute index, DWORD dwXP);
	virtual DWORD GiveVitalXP(eVital index, DWORD dwXP);
	virtual DWORD GiveSkillXP(eSkill index, DWORD dwXP);

	virtual ATTRIBUTE* GetAttributeData(eAttribute index);
	virtual VITAL* GetVitalData(eVital index);
	virtual SKILL* GetSkillData(eSkill index);

	virtual void Animation_Complete(animation_t *data);
	virtual void LaunchSpell(DWORD dwSpellID, DWORD dwTarget);
	virtual void Attack(DWORD dwTarget, DWORD dwHeight, float flPower);

	ATTRIBUTE m_attributes[6];
	VITAL m_vitals[6];
	SKILL m_skills[40];
};

class CBaelZharon : public CBaseMonster
{
public:
	CBaelZharon();

	BOOL CrazyThink();
};

class CTargetDrudge : public CBaseMonster
{
public:
	CTargetDrudge();
};


