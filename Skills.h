
#pragma once

enum eSkill
{
	eAxe = 0x01,
	eBow = 0x02,
	eCrossbow = 0x03,
	eDagger = 0x04,
	eMace = 0x05, //05
	eMeleeDefense = 0x06,
	eMissileDefense = 0x07,
	eSpear = 0x09,
	eStaff = 0x0A,
	eSword = 0x0B, //10
	eThrownWeapons = 0x0C,
	eUnarmedCombat = 0x0D,
	eArcaneLore = 0x0E,
	eMagicDefense = 0x0F,
	eManaConversion = 0x10, //15
	eAppraiseItem = 0x12,
	eAssessPerson = 0x13,
	eDeception = 0x14,
	eHealing = 0x15,
	eJump = 0x16, //20
	eLockpick = 0x17,
	eRun = 0x18,
	eAssessCreature = 0x1B,
	eAppraiseWeapon = 0x1C,
	eAppraiseArmor = 0x1D, //25
	eAppraiseMagicItem = 0x1E,
	eCreatureEnchantment = 0x1F,
	eItemEnchantment = 0x20,
	eLifeMagic = 0x21,
	eWarMagic = 0x22, //30
	eLeadership = 0x23,
	eLoyalty = 0x24,
	eFletching = 0x25,
	eAlchemy = 0x26,
	eCooking = 0x27, //35
};

enum eTraining
{
	eUnusable = 0x00,
	eUntrained = 0x01,
	eTrained = 0x02,
	eSpecialized = 0x03,
};

#pragma pack(push, 1)
struct _SKILLDATA
{
	WORD raises;
	WORD unknown;
	DWORD training;
	DWORD exp;
	DWORD bonus;
	DWORD unknown2;
	double unknown3;
};
#pragma pack(pop)

struct SKILL;

extern DWORD GetTrainedMaxXP();
extern DWORD GetSpecializedMaxXP();
extern const char* GetSkillName(eSkill index);

extern DWORD GetTrainedXP(DWORD Level);
extern DWORD GetTrainedLevel(DWORD XP);
extern DWORD GetSpecializedXP(DWORD Level);
extern DWORD GetSpecializedLevel(DWORD XP);
extern DWORD GetLevelXP(DWORD Level);
extern DWORD GetLevel(DWORD XP);
extern DWORD GetSkillMax(SKILL *pSkill, float fVitae);

extern float Calc_AnimSpeed(DWORD dwRunSkill, float flBurden);

struct SKILL
{
	SKILL()
	{
		data.raises = 0;
		data.unknown = 1;
		data.training = eUntrained;
		data.exp = 0;
		data.bonus = 0;
		data.unknown2 = 0;
		*((__int64 *)&data.unknown3) = 0x4192A971E4904E7F;
	}
	void SetTraining(DWORD dwTraining)
	{
		//
	}
	void SetRaises(DWORD dwRaises)
	{
		//
	}
	void SetExperience(DWORD dwXP)
	{
		//
	}

	_SKILLDATA data;
};


