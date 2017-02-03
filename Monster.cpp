
#include "StdAfx.h"
#include "PhysicsObj.h"
#include "Monster.h"
#include "World.h"
#include "Item.h"

CBaseMonster::CBaseMonster()
{
	m_ItemType = TYPE_CREATURE;
}

CBaseMonster::~CBaseMonster()
{
}

void CBaseMonster::Precache(void)
{
	CalculateSpeed();
}

void CBaseMonster::CalculateSpeed(void)
{
	DWORD dwRunSkill;
	ATTRIBUTE *quick = GetAttributeData(eQuickness);
	SKILL *run = GetSkillData(eRun);

	dwRunSkill = quick->data.base + quick->data.raises;
	dwRunSkill += run->data.bonus + run->data.raises;

	m_fSpeedMod = Calc_AnimSpeed(dwRunSkill, GetBurdenPercent()) * m_fScale;
}

void CBaseMonster::MonsterThink(void)
{
	Animation_Think();
	Movement_Think();
}

float CBaseMonster::GetBurdenPercent(void)
{
	//should base this off our total burden and strength
	return 0.0f;
}

//	returns the new number of raises, or zero if unchanged.
DWORD CBaseMonster::GiveAttributeXP(eAttribute index, DWORD dwXP)
{
	ATTRIBUTE* pAttrib = GetAttributeData(index);
	if (!pAttrib)
		return 0;
	//if ( WillOF( pAttrib->data.exp, dwXP ) )
	//	return 0;

	DWORD dwMaxXP = GetAttributeMaxXP();
	DWORD dwOldAttribXP = pAttrib->data.exp;
	DWORD dwNewAttribXP = dwOldAttribXP + dwXP;
	if (dwNewAttribXP > dwMaxXP)
		dwNewAttribXP = dwMaxXP;

	DWORD dwOldRaises = pAttrib->data.raises;
	DWORD dwNewRaises = GetAttributeLevel(dwNewAttribXP);

	pAttrib->data.exp = dwNewAttribXP;
	pAttrib->data.raises = dwNewRaises;

	if (index == eEndurance)
	{
		DWORD dwTotalValue = GetAttributeMax(pAttrib, 0.0f);
		DWORD dwMaxHealth = dwTotalValue / 2;
		if (dwTotalValue % 2) dwMaxHealth++;
		DWORD dwMaxStamina = dwTotalValue;

		//GetVitalData( eHealth )->data.base	= dwMaxHealth;
		//GetVitalData( eStamina )->data.base	= dwMaxStamina;
	}
	else if (index == eQuickness)
		CalculateSpeed();
	else if (index == eSelf)
	{
		DWORD dwTotalValue = GetAttributeMax(pAttrib, 0.0f);
		DWORD dwMaxMana = dwTotalValue;

		//GetVitalData( eMana )->data.base	= dwMaxMana;
	}

	return (dwOldRaises != dwNewRaises) ? dwNewRaises : 0;
}

DWORD CBaseMonster::GiveVitalXP(eVital index, DWORD dwXP)
{
	VITAL* pVital = GetVitalData(index);
	if (!pVital)
		return 0;
	//if ( WillOF( pVital->data.exp, dwXP ) )
	//	return 0;

	DWORD dwMaxXP = GetVitalMaxXP();
	DWORD dwOldVitalXP = pVital->data.exp;
	DWORD dwNewVitalXP = dwOldVitalXP + dwXP;
	if (dwNewVitalXP > dwMaxXP)
		dwNewVitalXP = dwMaxXP;

	DWORD dwOldRaises = pVital->data.raises;
	DWORD dwNewRaises = GetVitalLevel(dwNewVitalXP);

	pVital->data.exp = dwNewVitalXP;
	pVital->data.raises = dwNewRaises;

	return (dwOldRaises != dwNewRaises) ? dwNewRaises : 0;
}

DWORD CBaseMonster::GiveSkillXP(eSkill index, DWORD dwXP)
{
	SKILL* pSkill = GetSkillData(index);
	if (!pSkill)
		return 0;
	//if ( WillOF( pSkill->data.exp, dwXP ) )
	//	return 0;

	DWORD dwMaxXP;
	switch (pSkill->data.training)
	{
	default:
	case eUnusable:
	case eUntrained:
		return 0;
	case eTrained:
		dwMaxXP = GetTrainedMaxXP();
		break;
	case eSpecialized:
		dwMaxXP = GetSpecializedMaxXP();
		break;
	}
	DWORD dwOldSkillXP = pSkill->data.exp;
	DWORD dwNewSkillXP = dwOldSkillXP + dwXP;
	if (dwNewSkillXP > dwMaxXP)
		dwNewSkillXP = dwMaxXP;

	DWORD dwOldRaises = pSkill->data.raises;

	DWORD dwNewRaises;
	switch (pSkill->data.training)
	{
	case eTrained:
		dwNewRaises = GetTrainedLevel(dwNewSkillXP);
		break;
	case eSpecialized:
		dwNewRaises = GetSpecializedLevel(dwNewSkillXP);
		break;
	}

	pSkill->data.exp = dwNewSkillXP;
	pSkill->data.raises = (WORD) dwNewRaises;

	if (index == eRun)
		CalculateSpeed();

	return (dwOldRaises != dwNewRaises) ? dwNewRaises : 0;
}

SKILL* CBaseMonster::GetSkillData(eSkill index)
{
	if (index < 0x1 || index > 0x27)
		return NULL;

	return &m_skills[index - 1];
}

ATTRIBUTE* CBaseMonster::GetAttributeData(eAttribute index)
{
	if (index < 1 || index > 6)
		return NULL;

	return &m_attributes[index - 1];
}

VITAL* CBaseMonster::GetVitalData(eVital index)
{
	if (index < 1 || index > 6)
		return NULL;

	return &m_vitals[index - 1];
}

void CBaseMonster::Animation_Complete(animation_t *data)
{
	switch (data->dwAction)
	{
	case ANIM_SPELLCAST:
	{
		DWORD dwSpellID = data->dwActionData[0];
		DWORD dwTarget = data->dwTarget;

		LaunchSpell(dwSpellID, dwTarget);

		break;
	}
	case ANIM_LSRECALL:
	{
		Movement_Teleport(loc_t(0xEFEA0001, 0, 0, 0), heading_t(0, 0, 0, 0));
		break;
	}
	case ANIM_MPRECALL:
	{
		Movement_Teleport(loc_t(0x016C01BC, 0, 0, 0), heading_t(0.70339513f, 0, 0, -0.7107991f));
		break;
	}
	default:
		break;
	}

	CPhysicsObj::Animation_Complete(data);
}

void CBaseMonster::LaunchSpell(DWORD dwSpellID, DWORD dwTarget)
{
	switch (dwSpellID)
	{
	case 2366: //Bovine Intervention
	{
		if ((GetLandcell() & 0xFFFF) < 0x100)
		{
			for (int i = 0; i < 30; i++)
			{
				CBaseMonster *pCow = new CBaseMonster();
				pCow->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
				pCow->m_dwModel = 0x02000006;
				pCow->m_fScale = 1.0f;
				pCow->m_strName = "Cow";
				pCow->m_Origin.landcell = m_Origin.landcell;
				pCow->m_Origin.x = m_Origin.x + RandomFloat(-20.0f, 20.0f);
				pCow->m_Origin.y = m_Origin.y + RandomFloat(-20.0f, 20.0f);
				pCow->m_Origin.z = m_Origin.z + RandomFloat(40.0f, 80.0f);
				pCow->m_Angles.w = RandomFloat(-1.0f, 1.0f);
				pCow->m_Angles.z = RandomFloat(-1.0f, 1.0f);
				g_pWorld->CreateEntity(pCow);

				pCow->SetDescription("Mooooo!");
			}
		}
		else {
			if (IsPlayer())
				((CBasePlayer *)this)->SendText("Cows don't live indoors, silly!", 1);
		}

		break;
	}
	}
}

void CBaseMonster::Attack(DWORD dwTarget, DWORD dwHeight, float flPower)
{
	WORD wAttackAnim;

	switch (dwHeight)
	{
	case 0x01: wAttackAnim = 0x62; break;
	case 0x02: wAttackAnim = 0x63; break;
	case 0x03: wAttackAnim = 0x64; break;
	default: return;
	}

	if (flPower >= 0.25f)
		wAttackAnim += 3;
	if (flPower >= 0.75f)
		wAttackAnim += 3;

	if (flPower < 0 || flPower > 1)
		return;

	float flTime = 0.5f + (flPower * 0.5f);
	float fSpeed = 1.5f;

	Animation_Attack(dwTarget, wAttackAnim, fSpeed, Container_GetWeaponID());
}


CBaelZharon::CBaelZharon()
{
	m_dwModel = 0x0200099E;
	m_fScale = 1.8f;

	m_strName = "Bael'Zharon";

	m_miBaseModel.dwBasePalette = 0x1071;
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x1072, 0x00, 0x00));
	SetThink(&CBaelZharon::CrazyThink);
}

BOOL CBaelZharon::CrazyThink()
{
	SpeakLocal(csprintf("Today's lucky numbers are %u %u %u", RandomLong(0, 9), RandomLong(0, 9), RandomLong(0, 9)));

	m_fNextThink = g_pGlobals->Time() + 20.0f + RandomFloat(0, 40);
	return TRUE;
}

CTargetDrudge::CTargetDrudge()
{
	m_dwModel = 0x02000034;
	m_fScale = 0.95f;

	m_strName = "Oak Target Drudge";

	m_dwAnimationSet = 0x0900008A;
	m_dwEffectSet = 0x3400006B;
	m_dwSoundSet = 0x20000051;

	m_miBaseModel.SetBasePalette(0x01B9);
	m_miBaseModel.ReplacePalette(0x08B4, 0x00, 0x00);
	m_miBaseModel.ReplaceTexture(0x00, 0x0036, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x01, 0x0031, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x02, 0x0030, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x03, 0x0030, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x04, 0x0D33, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x05, 0x0030, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x06, 0x0030, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x07, 0x0D33, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x08, 0x0030, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x09, 0x0030, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x0A, 0x0035, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x0B, 0x0030, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x0C, 0x0030, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x0D, 0x0035, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x0E, 0x0D33, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x0E, 0x0EE8, 0x0EE8);
	m_miBaseModel.ReplaceTexture(0x0F, 0x0035, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x10, 0x0035, 0x0D33);
	m_miBaseModel.ReplaceModel(0x00, 0x005D);
	m_miBaseModel.ReplaceModel(0x01, 0x005E);
	m_miBaseModel.ReplaceModel(0x02, 0x006E);
	m_miBaseModel.ReplaceModel(0x03, 0x0064);
	m_miBaseModel.ReplaceModel(0x04, 0x18D9);
	m_miBaseModel.ReplaceModel(0x05, 0x006F);
	m_miBaseModel.ReplaceModel(0x06, 0x0316);
	m_miBaseModel.ReplaceModel(0x07, 0x18D9);
	m_miBaseModel.ReplaceModel(0x08, 0x006D);
	m_miBaseModel.ReplaceModel(0x09, 0x006B);
	m_miBaseModel.ReplaceModel(0x0A, 0x005F);
	m_miBaseModel.ReplaceModel(0x0B, 0x006C);
	m_miBaseModel.ReplaceModel(0x0C, 0x0068);
	m_miBaseModel.ReplaceModel(0x0D, 0x0060);
	m_miBaseModel.ReplaceModel(0x0E, 0x18D7);
	m_miBaseModel.ReplaceModel(0x0F, 0x0067);
	m_miBaseModel.ReplaceModel(0x10, 0x0060);
}


