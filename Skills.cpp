
#include "StdAfx.h"
#include "skills.h"

#include "TurbineXPTable.h"
#include "Rules.h"

const char *GetSkillName(eSkill index)
{
	switch (index)
	{
	case eAxe: return "Axe";
	case eBow: return "Bow";
	case eCrossbow: return "Crossbow";
	case eDagger: return "Dagger";
	case eMace: return "Mace";
	case eMeleeDefense: return "Melee Defense";
	case eMissileDefense: return "Missile Defense";
	case eSpear: return "Spear";
	case eStaff: return "Staff";
	case eSword: return "Sword";
	case eThrownWeapons: return "Thrown Weapons";
	case eUnarmedCombat: return "Unarmed Combat";
	case eArcaneLore: return "Arcane Lore";
	case eMagicDefense: return "Magic Defense";
	case eManaConversion: return "Mana Conversion";
	case eAppraiseItem: return "Appraise Item";
	case eAssessPerson: return "Assess Person";
	case eDeception: return "Deception";
	case eHealing: return "Healing";
	case eJump: return "Jump";
	case eLockpick: return "Lockpick";
	case eRun: return "Run";
	case eAssessCreature: return "Assess Creature";
	case eAppraiseWeapon: return "Appraise Weapon";
	case eAppraiseArmor: return "Appraise Armor";
	case eAppraiseMagicItem: return "Appraise Magic Item";
	case eCreatureEnchantment: return "Creature Enchantment";
	case eItemEnchantment: return "Item Enchantment";
	case eLifeMagic: return "Life Magic";
	case eWarMagic: return "War Magic";
	case eLeadership: return "Leadership";
	case eLoyalty: return "Loyalty";
	case eFletching: return "Fletching";
	case eAlchemy: return "Alchemy";
	case eCooking: return "Cooking";
	default: return "";
	}
}

DWORD GetTrainedMaxXP()
{
	XPTABLE* pXPTable = g_pGameRules->GetXPTable();

	if (pXPTable)
		return pXPTable->GetTrainedMaxXP();
	else
		return 0;
}

DWORD GetSpecializedMaxXP()
{
	XPTABLE* pXPTable = g_pGameRules->GetXPTable();

	if (pXPTable)
		return pXPTable->GetSpecializedMaxXP();
	else
		return 0;
}

DWORD GetTrainedXP(DWORD Level)
{
	XPTABLE* pXPTable = g_pGameRules->GetXPTable();

	if (pXPTable)
		return pXPTable->GetTrainedXP(Level);
	else
		return 0;
}

DWORD GetTrainedLevel(DWORD XP)
{
	XPTABLE* pXPTable = g_pGameRules->GetXPTable();

	if (pXPTable)
		return pXPTable->GetTrainedLevel(XP);
	else
		return 0;
}

DWORD GetSpecializedXP(DWORD Level)
{
	XPTABLE* pXPTable = g_pGameRules->GetXPTable();

	if (pXPTable)
		return pXPTable->GetSpecializedXP(Level);
	else
		return 0;
}

DWORD GetSpecializedLevel(DWORD XP)
{
	XPTABLE* pXPTable = g_pGameRules->GetXPTable();

	if (pXPTable)
		return pXPTable->GetSpecializedLevel(XP);
	else
		return 0;
}

DWORD GetLevelXP(DWORD Level)
{
	XPTABLE* pXPTable = g_pGameRules->GetXPTable();

	if (pXPTable)
		return pXPTable->GetLevelXP(Level);
	else
		return 0;
}

DWORD GetLevel(DWORD XP)
{
	XPTABLE* pXPTable = g_pGameRules->GetXPTable();

	if (pXPTable)
		return pXPTable->GetLevel(XP);
	else
		return 0;
}

//direct client code
float Calc_BurdenMod(float flBurden)
{
	if (flBurden < 1.0f)
		return 1.0f;

	if (flBurden < 2.0f)
		return (2.0f - flBurden);

	return 0.0f;
}

//direct client code
float Calc_AnimSpeed(DWORD dwRunSkill, float flBurden)
{
	float flBurdenMod = Calc_BurdenMod(flBurden);
	float fSpeed = (float)dwRunSkill;

	if (fSpeed == 800.0f)
		return (18.0f / 4.0f);

	fSpeed /= (fSpeed + 200.0f);
	fSpeed *= 11.0f * flBurdenMod;
	return ((fSpeed + 4.0f) / 4.0f);
}

DWORD GetSkillMax(SKILL *pSkill, float fVitae)
{
	//MUST ALSO ADD ATTRIBUTES VIA FORMULAS :(
	return (pSkill->data.bonus + pSkill->data.raises);
}


