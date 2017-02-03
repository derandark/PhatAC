
#include "StdAfx.h"
#include "PhysicsObj.h"
#include "Monster.h"
#include "Player.h"

//Network access.
#include "Client.h"
#include "ClientEvents.h"
#include "BinaryWriter.h"
#include "ObjectMsgs.h"

//Database access.
#include "Database.h"
#include "Database2.h"
#include "CharacterDatabase.h"

//World access.
#include "World.h"

CBasePlayer::CBasePlayer(CClient *pClient, DWORD dwGUID)
{
	m_pClient = pClient;
	m_dwGUID = dwGUID;

	m_dwStats[eTotalBurden] = 1;
	m_dwStats[eTotalPyreal] = 100000;
	m_dwStats[eTotalXP] = -1;
	m_dwStats[eUnassignedXP] = 2000000000;
	m_dwStats[eLevel] = 240;
	m_dwStats[eRank] = 10;
	m_dwStats[eFrozen] = TRUE;
	m_dwStats[eDeathCount] = 0;
	m_dwStats[eUnknown2] = 0;
	m_dwStats[eBirth] = DWORD(time(NULL));
	m_dwStats[eAge] = 0;
	m_dwStats[eExpTillVitaeRegain] = 0;
	m_dwStats[eUnknown4] = 0;

	m_bStatSequence = 0;

	m_PhysicsState = PhysicsState::HIDDEN_PS | PhysicsState::IGNORE_COLLISIONS_PS | PhysicsState::EDGE_SLIDE_PS | PhysicsState::GRAVITY_PS;

	// Human Female by Default

	m_dwAnimationSet = 0x09000001;
	m_dwModel = 0x0200004E;
	m_dwSoundSet = 0x20000002;
	m_dwEffectSet = 0x34000004;
	m_fScale = 1.0f;

#if FALSE
	if (m_dwGUID == 0x000004D2) //Pea
	{
		//060000090A0000201E0000340B0A0002
		m_pAnimator->m_dwAnimSet = 0x09000006;
		m_dwSoundSet = 0x2000000A;
		m_dwEffectSet = 0x3400001E;
		m_dwModel = 0x02000A0B;
		m_fScale = 0.4f;
	}
	else if (m_dwGUID == 0x000004DB) //Lane
	{
		m_pAnimator->m_dwAnimSet = 0x09000062;
		m_dwSoundSet = 0x2000003D;
		m_dwEffectSet = 0x3400002D;
		m_dwModel = 0x0200047B;
		m_fScale = 1.0f;
	}
	else if (m_dwGUID == 0x000004E3)
	{
		//270000093200002019000034FF020002
		m_pAnimator->m_dwAnimSet = 0x09000027;
		m_dwSoundSet = 0x20000032;
		m_dwEffectSet = 0x34000019;
		m_dwModel = 0x020002FF;
		m_fScale = 9.0f;
	}
#endif

	// Moon location... loc_t( 0xEFEA0001, 0, 0, 0 );

	if (!g_pGameDatabase->LoadedPortals())
	{
		// Aerfalle's Keep is default if the server wasn't configured with portals.
		m_Origin = loc_t(0x01f50229, 50.0f, -179.0f, 0.0);
		m_Angles = heading_t(1.0, 0, 0, 0);
	}
	else
	{
		// Otherwise outside Abandoned Mines
		m_Origin = loc_t(0xC3AA0028, 111.904579f, 188.143036f, 116.005005f);
		m_Angles = heading_t(0.998854f, 0.000000, 0.000000, 0.047870f);
	}

	SetThink(&CBasePlayer::PlayerThink);
#if 0
	FILE *tmp = g_pDB->DataFileCreate("class\\smalllugian", "wb");
	if (tmp)
	{
		/*fread( &m_dwModel, sizeof(DWORD), 1, pClassData );
		fread( &m_dwAnimationSet, sizeof(DWORD), 1, pClassData );
		fread( &m_dwSoundSet, sizeof(DWORD), 1, pClassData );
		fread( &m_dwEffectSet, sizeof(DWORD), 1, pClassData );
		fread( &m_fScale, sizeof(float), 1, pClassData );*/

		DWORD dwModel = 0x02000A0B;
		DWORD dwSoundSet = 0x2000000A;
		DWORD dwAnimSet = 0x09000006;
		DWORD dwEffectSet = 0x3400001E;
		float fScale = 0.4f;

		fwrite(&dwModel, sizeof(DWORD), 1, tmp);
		fwrite(&dwAnimSet, sizeof(DWORD), 1, tmp);
		fwrite(&dwSoundSet, sizeof(DWORD), 1, tmp);
		fwrite(&dwEffectSet, sizeof(DWORD), 1, tmp);
		fwrite(&fScale, sizeof(float), 1, tmp);
		fclose(tmp);
	}
#endif
}

CBasePlayer::~CBasePlayer()
{
	CClientEvents* pEvents;
	if (m_pClient && (pEvents = m_pClient->GetEvents()))
	{
		pEvents->DetachPlayer();
	}
}

void CBasePlayer::Precache()
{
	m_miBaseModel.dwBasePalette = 0x7E;

	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x4A7, 0x00, 0x18));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x2F2, 0x18, 0x08));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x2BD, 0x20, 0x08));

	_CHARDESC desc;
	if (g_pDB->CharDB()->GetCharacterDesc(m_dwGUID, &desc))
	{
		m_strName = desc.szName;
		m_wInstance = (WORD)g_pDB->CharDB()->IncCharacterInstance(m_dwGUID, desc.dwInstances);

		if ((*desc.szWorldClass) != 0)
		{
			FILE* pClassData = g_pDB->DataFileOpen(csprintf("class\\%s", desc.szWorldClass), "rb");

			if (pClassData)
			{

				fread(&m_dwModel, sizeof(DWORD), 1, pClassData);
				fread(&m_dwAnimationSet, sizeof(DWORD), 1, pClassData);
				fread(&m_dwSoundSet, sizeof(DWORD), 1, pClassData);
				fread(&m_dwEffectSet, sizeof(DWORD), 1, pClassData);
				fread(&m_fScale, sizeof(float), 1, pClassData);

				fclose(pClassData);
			}

			if (!stricmp("human-male", desc.szWorldClass))
			{
				m_miBaseModel.lTextures.push_back(TextureRpl(0x10, 0x98, 0x10B7));
				m_miBaseModel.lTextures.push_back(TextureRpl(0x10, 0x24C, 0x110B));
				m_miBaseModel.lTextures.push_back(TextureRpl(0x10, 0x2F5, 0x1162));
				m_miBaseModel.lTextures.push_back(TextureRpl(0x10, 0x25C, 0x11CB));

				m_miBaseModel.lModels.push_back(ModelRpl(0x00, 0x4E));
				m_miBaseModel.lModels.push_back(ModelRpl(0x01, 0x4F));
				m_miBaseModel.lModels.push_back(ModelRpl(0x02, 0x4D));
				m_miBaseModel.lModels.push_back(ModelRpl(0x03, 0x4C));
				m_miBaseModel.lModels.push_back(ModelRpl(0x04, 0x4B));
				m_miBaseModel.lModels.push_back(ModelRpl(0x05, 0x53));
				m_miBaseModel.lModels.push_back(ModelRpl(0x06, 0x51));
				m_miBaseModel.lModels.push_back(ModelRpl(0x07, 0x50));
				m_miBaseModel.lModels.push_back(ModelRpl(0x08, 0x42));
				m_miBaseModel.lModels.push_back(ModelRpl(0x09, 0x54));
				m_miBaseModel.lModels.push_back(ModelRpl(0x0A, 0x497));
				m_miBaseModel.lModels.push_back(ModelRpl(0x0B, 0x495));
				m_miBaseModel.lModels.push_back(ModelRpl(0x0C, 0x76));
				m_miBaseModel.lModels.push_back(ModelRpl(0x0D, 0x4AD));
				m_miBaseModel.lModels.push_back(ModelRpl(0x0E, 0x496));
				m_miBaseModel.lModels.push_back(ModelRpl(0x0F, 0x77));
				m_miBaseModel.lModels.push_back(ModelRpl(0x10, 0x5A));
			}
			if (!stricmp("human-envoy", desc.szWorldClass))
			{
				m_miBaseModel.dwBasePalette = 0x7E;

				m_miBaseModel.lPalettes.clear();
				m_miBaseModel.lPalettes.push_back(PaletteRpl(0x2BA, 0x00, 0x18));
				m_miBaseModel.lPalettes.push_back(PaletteRpl(0x2E3, 0x18, 0x08));
				m_miBaseModel.lPalettes.push_back(PaletteRpl(0x4AE, 0x20, 0x08));
				m_miBaseModel.lPalettes.push_back(PaletteRpl(0x179C, 0x28, 0xC8));
				m_miBaseModel.lPalettes.push_back(PaletteRpl(0x17A6, 0xF0, 0x10));

				m_miBaseModel.lTextures.push_back(TextureRpl(0x10, 0x98, 0x11FD));
				m_miBaseModel.lTextures.push_back(TextureRpl(0x10, 0x24C, 0x1113));
				m_miBaseModel.lTextures.push_back(TextureRpl(0x10, 0x2F5, 0x117D));
				m_miBaseModel.lTextures.push_back(TextureRpl(0x10, 0x25C, 0x11CF));
				m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0x1879, 0x22FF));
				m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0x1878, 0x2302));
				m_miBaseModel.lTextures.push_back(TextureRpl(0x01, 0x1880, 0x2307));
				m_miBaseModel.lTextures.push_back(TextureRpl(0x02, 0x187F, 0x2308));
				m_miBaseModel.lTextures.push_back(TextureRpl(0x05, 0x1880, 0x2307));
				m_miBaseModel.lTextures.push_back(TextureRpl(0x06, 0x187F, 0x2308));
				m_miBaseModel.lTextures.push_back(TextureRpl(0x09, 0x3D5, 0x2305));
				m_miBaseModel.lTextures.push_back(TextureRpl(0x09, 0x3D4, 0x2303));
				m_miBaseModel.lTextures.push_back(TextureRpl(0x0A, 0x187B, 0x2304));
				m_miBaseModel.lTextures.push_back(TextureRpl(0x0B, 0x187A, 0x2300));
				m_miBaseModel.lTextures.push_back(TextureRpl(0x0D, 0x187B, 0x2306));
				m_miBaseModel.lTextures.push_back(TextureRpl(0x0E, 0x187A, 0x2301));

				m_miBaseModel.lModels.push_back(ModelRpl(0x0C, 0x0076));
				m_miBaseModel.lModels.push_back(ModelRpl(0x0F, 0x0077));
				m_miBaseModel.lModels.push_back(ModelRpl(0x00, 0x1A16));
				m_miBaseModel.lModels.push_back(ModelRpl(0x01, 0x1A0D));
				m_miBaseModel.lModels.push_back(ModelRpl(0x02, 0x1A06));
				m_miBaseModel.lModels.push_back(ModelRpl(0x03, 0x01EC));
				m_miBaseModel.lModels.push_back(ModelRpl(0x04, 0x01EC));
				m_miBaseModel.lModels.push_back(ModelRpl(0x05, 0x1A11));
				m_miBaseModel.lModels.push_back(ModelRpl(0x06, 0x1A09));
				m_miBaseModel.lModels.push_back(ModelRpl(0x07, 0x01EC));
				m_miBaseModel.lModels.push_back(ModelRpl(0x08, 0x01EC));
				m_miBaseModel.lModels.push_back(ModelRpl(0x09, 0x120D));
				m_miBaseModel.lModels.push_back(ModelRpl(0x0A, 0x19F7));
				m_miBaseModel.lModels.push_back(ModelRpl(0x0B, 0x19EF));
				m_miBaseModel.lModels.push_back(ModelRpl(0x0D, 0x19FF));
				m_miBaseModel.lModels.push_back(ModelRpl(0x0E, 0x19EF));
				m_miBaseModel.lModels.push_back(ModelRpl(0x10, 0x3148));
			}
		}

		LOG(Temp, Normal, "Loaded player %s (%u)\n", m_strName.c_str(), m_wInstance);
	}

	Container_InitContents(MAX_PLAYER_EQUIPMENT, MAX_PLAYER_INVENTORY, MAX_PLAYER_CONTAINERS);


	// L337 stats?
#if FALSE
	GetAttributeData(eStrength)->data.base = 100;
	GetAttributeData(eStrength)->data.raises = 190;
	GetAttributeData(eQuickness)->data.base = 10;
	GetAttributeData(eQuickness)->data.raises = 190;
	GetSkillData(eRun)->data.training = eTrained;
	GetSkillData(eRun)->data.raises = 208;
	GetSkillData(eRun)->data.exp = 4203819496;
	GetSkillData(eRun)->data.bonus = 1000;
#else
	GetAttributeData(eStrength)->data.base = 100;
	GetAttributeData(eStrength)->data.raises = 190;
	GetAttributeData(eEndurance)->data.base = 10;
	GetAttributeData(eEndurance)->data.raises = 190;
	GetAttributeData(eCoordination)->data.base = 10;
	GetAttributeData(eCoordination)->data.raises = 190;
	GetAttributeData(eQuickness)->data.base = 10;
	GetAttributeData(eQuickness)->data.raises = 190;
	GetAttributeData(eFocus)->data.base = 100;
	GetAttributeData(eFocus)->data.raises = 190;
	GetAttributeData(eSelf)->data.base = 100;
	GetAttributeData(eSelf)->data.raises = 190;
	GetSkillData(eLifeMagic)->data.training = eSpecialized;
	GetSkillData(eLifeMagic)->data.raises = 226;
	GetSkillData(eLifeMagic)->data.exp = 4113151665;
	GetSkillData(eLifeMagic)->data.bonus = 10;
	GetSkillData(eWarMagic)->data.training = eSpecialized;
	GetSkillData(eWarMagic)->data.raises = 226;
	GetSkillData(eWarMagic)->data.exp = 4113151665;
	GetSkillData(eWarMagic)->data.bonus = 10;
	GetSkillData(eArcaneLore)->data.training = eTrained;
	GetSkillData(eArcaneLore)->data.raises = 208;
	GetSkillData(eArcaneLore)->data.exp = 4203819496;
	GetSkillData(eArcaneLore)->data.bonus = 5;
	GetSkillData(eCreatureEnchantment)->data.training = eTrained;
	GetSkillData(eCreatureEnchantment)->data.raises = 208;
	GetSkillData(eCreatureEnchantment)->data.exp = 4203819496;
	GetSkillData(eCreatureEnchantment)->data.bonus = 5;
	GetSkillData(eHealing)->data.training = eTrained;
	GetSkillData(eHealing)->data.raises = 208;
	GetSkillData(eHealing)->data.exp = 4203819496;
	GetSkillData(eHealing)->data.bonus = 5;
	GetSkillData(eItemEnchantment)->data.training = eTrained;
	GetSkillData(eItemEnchantment)->data.raises = 208;
	GetSkillData(eItemEnchantment)->data.exp = 4203819496;
	GetSkillData(eItemEnchantment)->data.bonus = 5;
	GetSkillData(eAppraiseItem)->data.training = eTrained;
	GetSkillData(eAppraiseItem)->data.raises = 208;
	GetSkillData(eAppraiseItem)->data.exp = 4203819496;
	GetSkillData(eAppraiseItem)->data.bonus = 5;
	GetSkillData(eJump)->data.training = eTrained;
	GetSkillData(eJump)->data.raises = 208;
	GetSkillData(eJump)->data.exp = 4203819496;
	GetSkillData(eJump)->data.bonus = 2000000;//5;
	GetSkillData(eLockpick)->data.training = eTrained;
	GetSkillData(eLockpick)->data.raises = 208;
	GetSkillData(eLockpick)->data.exp = 4203819496;
	GetSkillData(eLockpick)->data.bonus = 5;
	GetSkillData(eLoyalty)->data.training = eTrained;
	GetSkillData(eLoyalty)->data.raises = 208;
	GetSkillData(eLoyalty)->data.exp = 4203819496;
	GetSkillData(eLoyalty)->data.bonus = 5;
	GetSkillData(eMagicDefense)->data.training = eTrained;
	GetSkillData(eMagicDefense)->data.raises = 208;
	GetSkillData(eMagicDefense)->data.exp = 4203819496;
	GetSkillData(eMagicDefense)->data.bonus = 5;
	GetSkillData(eManaConversion)->data.training = eTrained;
	GetSkillData(eManaConversion)->data.raises = 208;
	GetSkillData(eManaConversion)->data.exp = 4203819496;
	GetSkillData(eManaConversion)->data.bonus = 5;
	GetSkillData(eRun)->data.training = eTrained;
	GetSkillData(eRun)->data.raises = 208;
	GetSkillData(eRun)->data.exp = 4203819496;
	GetSkillData(eRun)->data.bonus = 2000;//5;
	GetSkillData(eUnarmedCombat)->data.training = eTrained;
	GetSkillData(eUnarmedCombat)->data.raises = 208;
	GetSkillData(eUnarmedCombat)->data.exp = 4203819496;
	GetSkillData(eUnarmedCombat)->data.bonus = 5;
#endif

	CBaseMonster::Precache();
}

void CBasePlayer::EmitSoundUI(DWORD dwIndex, float fSpeed)
{
	BinaryWriter SoundMsg;
	SoundMsg.WriteDWORD(0xF750);
	SoundMsg.WriteDWORD(m_dwGUID);
	SoundMsg.WriteDWORD(dwIndex);
	SoundMsg.WriteFloat(fSpeed);

	SendMessage(&SoundMsg, OBJECT_MSG, FALSE, FALSE);
}

DWORD CBasePlayer::GiveAttributeXP(eAttribute index, DWORD dwXP)
{
	ATTRIBUTE* pAttrib = GetAttributeData(index);
	if (!pAttrib) return 0;

	DWORD dwRaises = CBaseMonster::GiveAttributeXP(index, dwXP);
	if (dwRaises)
	{
		//Your base Strength is now 41! color: 0xD

		EmitSoundUI(0x76, 1.0);

		const char* szAttribName = GetAttributeName(index);
		const char* szNotice = csprintf("Your base %s is now %lu!", szAttribName, pAttrib->data.base + pAttrib->data.raises);

		SendText(szNotice, 13);
	}

	BinaryWriter UpdateAttribute;

	UpdateAttribute.WriteDWORD(0x241);
	UpdateAttribute.WriteWORD(index);
	UpdateAttribute.WriteBYTE(0);
	UpdateAttribute.WriteBYTE(++m_bStatSequence);
	UpdateAttribute.WriteDWORD(pAttrib->data.raises);
	UpdateAttribute.WriteDWORD(pAttrib->data.base);
	UpdateAttribute.WriteDWORD(pAttrib->data.exp);

	SendMessage(&UpdateAttribute, PRIVATE_MSG, FALSE, FALSE);

	return dwRaises;
}

DWORD CBasePlayer::GiveVitalXP(eVital index, DWORD dwXP)
{
	VITAL* pVital = GetVitalData(index);
	if (!pVital) return 0;

	DWORD dwRaises = CBaseMonster::GiveVitalXP(index, dwXP);
	if (dwRaises)
	{
		//Your base Strength is now 41! color: 0xD

		EmitSoundUI(0x76, 1.0);

		const char* szVitalName = GetVitalName(index);
		const char* szNotice = csprintf("Your base %s is now %lu!", szVitalName, pVital->data.raises);

		SendText(szNotice, 13);
	}

	BinaryWriter UpdateVital;

	UpdateVital.WriteDWORD(0x243);
	UpdateVital.WriteWORD(index);
	UpdateVital.WriteBYTE(0);
	UpdateVital.WriteBYTE(++m_bStatSequence);
	UpdateVital.WriteDWORD(pVital->data.raises);
	UpdateVital.WriteDWORD(0);
	UpdateVital.WriteDWORD(pVital->data.exp);
	UpdateVital.WriteDWORD(pVital->data.raises);

	SendMessage(&UpdateVital, PRIVATE_MSG, FALSE, FALSE);

	return dwRaises;
}

DWORD CBasePlayer::GiveSkillXP(eSkill index, DWORD dwXP)
{
	SKILL* pSkill = GetSkillData(index);
	if (!pSkill) return 0;

	DWORD dwRaises = CBaseMonster::GiveSkillXP(index, dwXP);
	if (dwRaises)
	{
		//Your base Strength is now 41! color: 0xD

		EmitSoundUI(0x76, 1.0);

		const char* szSkillName = GetSkillName(index);
		const char* szNotice = csprintf("Your base %s is now %lu!", szSkillName, pSkill->data.raises);

		SendText(szNotice, 13);
	}

	BinaryWriter UpdateSkill;

	UpdateSkill.WriteDWORD(0x23E);
	UpdateSkill.WriteWORD(index);
	UpdateSkill.WriteBYTE(0);
	UpdateSkill.WriteBYTE(++m_bStatSequence);
	UpdateSkill.WriteWORD(pSkill->data.raises);
	UpdateSkill.WriteWORD(1);
	UpdateSkill.WriteDWORD(pSkill->data.training);
	UpdateSkill.WriteDWORD(pSkill->data.exp);
	UpdateSkill.WriteDWORD(pSkill->data.bonus);
	UpdateSkill.WriteFloat(1.0f);
	UpdateSkill.WriteDWORD(0);
	UpdateSkill.WriteDWORD(0);

	SendMessage(&UpdateSkill, PRIVATE_MSG, FALSE, FALSE);

	return dwRaises;
}

void CBasePlayer::SendMessage(void *_data, DWORD _len, WORD _group, BOOL _event)
{
	if (m_pClient)
		m_pClient->SendMessage(_data, _len, _group, _event);
}

void CBasePlayer::SendMessage(BinaryWriter *_food, WORD _group, BOOL _event, BOOL del)
{
	if (m_pClient)
		m_pClient->SendMessage(_food, _group, _event, del);
}

void CBasePlayer::SendText(const char* szText, long lColor)
{
	if (m_pClient)
	{
		CClientEvents *pEvents = m_pClient->GetEvents();

		if (pEvents)
			pEvents->SendText(szText, lColor);
	}
}

BinaryWriter* CBasePlayer::GetModelData()
{
	//if ( !IsHuman() )
	{
		return CBaseMonster::GetModelData();
	}

	BinaryWriter *MD = new BinaryWriter;

	//Fake Zyrca:

	MD->WriteBYTE(0x11);
	MD->WriteBYTE(0x01);
	MD->WriteBYTE(0x18);
	MD->WriteBYTE(0x11);

	//Palettes
	MD->WriteWORD(0x007E);
	MD->WriteWORD(0x0EB6); MD->WriteBYTE(0x00); MD->WriteBYTE(0x00);

	//Textures
	unsigned char TextureData[] =
	{
	0x10, 0x98, 0x00, 0xB8, 0x10,
	0x10, 0x4C, 0x02, 0x67, 0x10,
	0x10, 0xF5, 0x02, 0x8A, 0x10,
	0x10, 0x5C, 0x02, 0xA3, 0x10,//4
	0x00, 0xBE, 0x0C, 0x01, 0x17,
	0x00, 0xB0, 0x0B, 0x01, 0x17,
	0x01, 0x07, 0x17, 0x07, 0x17,
	0x01, 0x44, 0x18, 0x44, 0x18,//8
	0x02, 0x08, 0x17, 0x08, 0x17,
	0x02, 0x44, 0x18, 0x44, 0x18,
	0x05, 0x07, 0x17, 0x07, 0x17,
	0x05, 0x44, 0x18, 0x44, 0x18,//C
	0x06, 0x08, 0x17, 0x08, 0x17,
	0x06, 0x44, 0x18, 0x44, 0x18,
	0x09, 0xDE, 0x03, 0x30, 0x18,
	0x09, 0xD6, 0x03, 0x2F, 0x18,//10
	0x0A, 0xFF, 0x16, 0x2E, 0x18,
	0x0B, 0xFE, 0x16, 0x4E, 0x17,
	0x0C, 0xD3, 0x03, 0xD3, 0x03,
	0x0D, 0xFF, 0x16, 0x2E, 0x18,//14
	0x0E, 0xFE, 0x16, 0x4E, 0x17,
	0x0F, 0xD3, 0x03, 0xD3, 0x03,
	0x10, 0xC4, 0x0E, 0x32, 0x18,
	0x10, 0xC3, 0x0E, 0xFD, 0x16,//18
	};
	MD->AppendData(TextureData, sizeof(TextureData));

	//Models
	MD->WriteBYTE(0x00); MD->WriteWORD(0x0477);
	MD->WriteBYTE(0x01); MD->WriteWORD(0x16EE);
	MD->WriteBYTE(0x02); MD->WriteWORD(0x16EC);
	MD->WriteBYTE(0x03); MD->WriteWORD(0x1EC);
	MD->WriteBYTE(0x04); MD->WriteWORD(0x1EC);
	MD->WriteBYTE(0x05); MD->WriteWORD(0x16E9);
	MD->WriteBYTE(0x06); MD->WriteWORD(0x16ED);
	MD->WriteBYTE(0x07); MD->WriteWORD(0x01EC);
	MD->WriteBYTE(0x08); MD->WriteWORD(0x01EC);
	MD->WriteBYTE(0x09); MD->WriteWORD(0x04B9);
	MD->WriteBYTE(0x0A); MD->WriteWORD(0x16E7);
	MD->WriteBYTE(0x0B); MD->WriteWORD(0x16E5);
	MD->WriteBYTE(0x0C); MD->WriteWORD(0x0076);
	MD->WriteBYTE(0x0D); MD->WriteWORD(0x16E4);
	MD->WriteBYTE(0x0E); MD->WriteWORD(0x16E6);
	MD->WriteBYTE(0x0F); MD->WriteWORD(0x0077);
	MD->WriteBYTE(0x10); MD->WriteWORD(0x193D);
	MD->Align();

	return MD;
}

void CBasePlayer::Save()
{
}

void CBasePlayer::ChangeVIS(DWORD dwFlags)
{
	CBaseMonster::ChangeVIS(dwFlags | PhysicsState::GRAVITY_PS);
}
void CBasePlayer::AddSpellByID(DWORD id)
{
	NetFood AddSpellToSpellbook;
	AddSpellToSpellbook.WriteDWORD(0x02C1);
	AddSpellToSpellbook.WriteDWORD(id);
	AddSpellToSpellbook.WriteDWORD(0x0);
	SendMessage(AddSpellToSpellbook.GetData(), AddSpellToSpellbook.GetSize(), EVENT_MSG,true);
}
void CBasePlayer::EnterPortal()
{
	ChangeVIS(PhysicsState::HIDDEN_PS | PhysicsState::IGNORE_COLLISIONS_PS | PhysicsState::EDGE_SLIDE_PS);

	BinaryWriter EnterPortal;
	EnterPortal.WriteDWORD(0xF751);
	EnterPortal.WriteDWORD(m_wNumPortals);
	SendMessage(EnterPortal.GetData(), EnterPortal.GetSize(), OBJECT_MSG);
}

void CBasePlayer::SpawnCows()
{
	if (!Animation_IsActive())
	{
		SpeakLocal("Zojak Quamoo", 0x11);
		Animation_PlaySimpleAnimation(304, 1.0f, 3.0f, ANIM_SPELLCAST, 2366);
		Animation_PlaySimpleAnimation(313, 3.0f, 1.0f);
		Animation_PlaySimpleAnimation(0, 0);
	}
}

BOOL CBasePlayer::PlayerThink()
{
	CBaseMonster::MonsterThink();

	return TRUE;
}

const char* CBasePlayer::GetGenderString()
{
	return "Male";
}

const char* CBasePlayer::GetRaceString()
{
	return "Human";
}

const char* CBasePlayer::GetTitleString()
{
	return "God";
}

void CBasePlayer::MakeAware(CPhysicsObj *pEntity)
{
	BinaryWriter *CM = pEntity->CreateMessage();
	if (CM)
		SendMessage(CM, 10);

	ItemVector vItems;

	pEntity->Container_GetWieldedItems(&vItems);

	ItemVector::iterator iit = vItems.begin();
	ItemVector::iterator iend = vItems.end();

	while (iit != iend)
	{
		BinaryWriter *CM = ((CPhysicsObj *)(*iit))->CreateMessage();
		if (CM)
			SendMessage(CM, 10);

		iit++;
	}
}

void CBasePlayer::LoginCharacter(void)
{
	DWORD SC[2];

	SC[0] = 0xF746;
	SC[1] = m_dwGUID;
	SendMessage(SC, sizeof(SC), 10);

	BinaryWriter *LC = ::LoginCharacter(this);
	SendMessage(LC->GetData(), LC->GetSize(), PRIVATE_MSG, TRUE);
	delete LC;
}

void CBasePlayer::ExitPortal()
{
	ChangeVIS(REPORT_COLLISIONS_PS | PhysicsState::EDGE_SLIDE_PS);
}

void CBasePlayer::UpdateEntity(CPhysicsObj *pEntity)
{
	BinaryWriter *CO = pEntity->CreateMessage();
	if (CO)
	{
		SendMessage(CO->GetData(), CO->GetSize(), OBJECT_MSG);
		delete CO;
	}
}

BOOL CBasePlayer::IsHuman()
{
	return (m_dwAnimationSet == 0x09000001) ? TRUE : FALSE;
}

DWORD CBasePlayer::GetObjectStat(eObjectStat index)
{
	return CPhysicsObj::GetObjectStat(index);
}

DWORD CBasePlayer::SetObjectStat(eObjectStat index, DWORD value)
{
	if (index >= 0x100)
		return NULL;

	BinaryWriter UpdateStatistic;

	UpdateStatistic.WriteDWORD(0x237);
	UpdateStatistic.WriteBYTE(++m_bStatSequence);
	UpdateStatistic.WriteDWORD(index);
	UpdateStatistic.WriteDWORD(value);

	SendMessage(&UpdateStatistic, PRIVATE_MSG, FALSE, FALSE);

	return CPhysicsObj::SetObjectStat(index, value);
}

void CBasePlayer::SetLastAssessedItem(CPhysicsObj* obj)
{
	if (dynamic_cast<CBasePlayer*>(obj) == NULL) //FIXME: Cannot delete players, other objects not removable at this time??
	ppoLastAssessedItem = obj;
}
std::string CBasePlayer::RemoveLastAssessedItem()
{
	if (ppoLastAssessedItem != 0) {
		std::string name = ppoLastAssessedItem->GetName();
		CLandBlock *pBlock = GetBlock();
		ppoLastAssessedItem->RemoveMe();
		pBlock->Destroy(ppoLastAssessedItem);
		ppoLastAssessedItem = 0;
		return name;
	}
	return "";
}



