
#include "StdAfx.h"
#include "PhysicsObj.h"
#include "Monster.h"
#include "Player.h"
#include "Item.h"

//Network access.
#include "Client.h"
#include "BinaryWriter.h"
#include "ObjectMsgs.h"

#define MESSAGE_BEGIN(x)	BinaryWriter *x = new BinaryWriter
#define MESSAGE_END(x)		return x

BinaryWriter *GetWeenieObjData(CPhysicsObj *pEntity)
{
	BinaryWriter OptionalWeenieObjData;
	DWORD dwSections = 0;

	if (pEntity->Container_HasContents())
	{
		dwSections |= PublicWeenieDescPackHeader::PWD_Packed_ItemsCapacity;
		OptionalWeenieObjData.WriteBYTE(pEntity->Container_MaxInventorySlots()); // item capacity

		dwSections |= PublicWeenieDescPackHeader::PWD_Packed_ContainersCapacity;
		OptionalWeenieObjData.WriteBYTE(pEntity->Container_MaxContainerSlots()); // container capacity
	}

	if (pEntity->HasValue())
	{
		dwSections |= PublicWeenieDescPackHeader::PWD_Packed_Value;
		OptionalWeenieObjData.WriteDWORD(pEntity->GetValue());
	}

	//Use item? 1 = Cannot be used?
	dwSections |= PublicWeenieDescPackHeader::PWD_Packed_Useability;
	OptionalWeenieObjData.WriteDWORD((DWORD)pEntity->m_Usability);

	float flApproachDist = pEntity->GetApproachDist();
	if (flApproachDist > 0.0f)
	{
		dwSections |= PublicWeenieDescPackHeader::PWD_Packed_UseRadius;
		OptionalWeenieObjData.WriteFloat(flApproachDist);
	}

	DWORD dwHighlight = pEntity->GetHighlightColor();
	if (dwHighlight)
	{
		dwSections |= PublicWeenieDescPackHeader::PWD_Packed_UIEffects;
		OptionalWeenieObjData.WriteDWORD(dwHighlight);
	}

	if (pEntity->HasEquipType())
	{
		dwSections |= PublicWeenieDescPackHeader::PWD_Packed_CombatUse;
		OptionalWeenieObjData.WriteDWORD(pEntity->GetEquipType());
	}

	if (pEntity->IsContained())
	{
		dwSections |= PublicWeenieDescPackHeader::PWD_Packed_ContainerID;
		OptionalWeenieObjData.WriteDWORD(pEntity->GetContainerID());
	}

	if (pEntity->HasCoverage())
	{
		CBaseItem *pItem = (CBaseItem*)pEntity;
		DWORD dwCoverage1 = pItem->GetCoverage1();
		DWORD dwCoverage2 = pItem->GetCoverage2();
		DWORD dwCoverage3 = pItem->GetCoverage3();

		if (dwCoverage1)
		{
			dwSections |= PublicWeenieDescPackHeader::PWD_Packed_ValidLocations;
			OptionalWeenieObjData.WriteDWORD(dwCoverage1);
		}
		if (dwCoverage2)
		{
			dwSections |= PublicWeenieDescPackHeader::PWD_Packed_Location;
			OptionalWeenieObjData.WriteDWORD(dwCoverage1);
		}
		if (dwCoverage3)
		{
			dwSections |= PublicWeenieDescPackHeader::PWD_Packed_Priority;
			OptionalWeenieObjData.WriteDWORD(dwCoverage1);
		}
	}

	if (pEntity->HasRadarDot())
	{
		dwSections |= PublicWeenieDescPackHeader::PWD_Packed_BlipColor;
		OptionalWeenieObjData.WriteBYTE(pEntity->GetRadarColor());
	}

	// if (pEntity->IsPlayer())
	{
		dwSections |= PublicWeenieDescPackHeader::PWD_Packed_RadarEnum;
		OptionalWeenieObjData.WriteBYTE((BYTE)pEntity->m_RadarVis);
	}

	if (pEntity->HasBurden())
	{
		dwSections |= PublicWeenieDescPackHeader::PWD_Packed_Burden;
		OptionalWeenieObjData.WriteWORD(pEntity->GetBurden());
	}

	BinaryWriter *WeenieObjData = new BinaryWriter;
	WeenieObjData->WriteDWORD(dwSections);
	WeenieObjData->WriteString(pEntity->GetName());
	WeenieObjData->WriteWORD(pEntity->m_wTypeID);
	WeenieObjData->WriteWORD(pEntity->m_wIcon);
	WeenieObjData->WriteDWORD(pEntity->m_ItemType);
	WeenieObjData->WriteDWORD(pEntity->GetDescFlags());
	WeenieObjData->AppendData(OptionalWeenieObjData.GetData(), OptionalWeenieObjData.GetSize());

	return WeenieObjData;
}

BinaryWriter *GetPhysicsObjData(CPhysicsObj *pEntity)
{
	BinaryWriter OptionalPhysicsObjData;
	DWORD dwSections = 0; //0x019803

	if (pEntity->m_dwAnimationSet)
	{
		dwSections |= PhysicsDescInfo::MOVEMENT;

		if (!pEntity->m_AnimOverrideData)
		{
			BinaryWriter *AnimInfo = pEntity->Animation_GetAnimationInfo();

			OptionalPhysicsObjData.WriteDWORD(AnimInfo->GetSize());
			OptionalPhysicsObjData.AppendData(AnimInfo->GetData(), AnimInfo->GetSize());

			OptionalPhysicsObjData.WriteDWORD(pEntity->m_AutonomousMovement); // autonomous movement?

			delete AnimInfo;
		}
		else
		{
			OptionalPhysicsObjData.WriteDWORD(pEntity->m_AnimOverrideDataLen);
			OptionalPhysicsObjData.AppendData(pEntity->m_AnimOverrideData, pEntity->m_AnimOverrideDataLen);
			OptionalPhysicsObjData.WriteDWORD(pEntity->m_AutonomousMovement);
		}
	}

	if (pEntity->CanPickup())
	{
		dwSections |= PhysicsDescInfo::ANIMFRAME_ID;
		OptionalPhysicsObjData.WriteDWORD(0x00000065); // animation frame ID????
	}

	if (!pEntity->HasOwner())
	{
		dwSections |= PhysicsDescInfo::POSITION;
		OptionalPhysicsObjData.AppendData(&pEntity->m_Origin, sizeof(loc_t));
		OptionalPhysicsObjData.AppendData(&pEntity->m_Angles, sizeof(heading_t));
	}

	if (pEntity->m_dwAnimationSet)
	{
		dwSections |= PhysicsDescInfo::MTABLE;
		OptionalPhysicsObjData.WriteDWORD(pEntity->m_dwAnimationSet);
	}

	DWORD dwSoundSet = pEntity->GetSoundSet();
	if (dwSoundSet)
	{
		dwSections |= PhysicsDescInfo::STABLE;
		OptionalPhysicsObjData.WriteDWORD(dwSoundSet);
	}

	DWORD dwEffectSet = pEntity->GetEffectSet();
	if (dwEffectSet)
	{
		dwSections |= PhysicsDescInfo::PETABLE;
		OptionalPhysicsObjData.WriteDWORD(dwEffectSet);
	}

	DWORD dwModelNumber = pEntity->m_dwModel;
	if (dwModelNumber)
	{
		dwSections |= PhysicsDescInfo::CSetup;
		OptionalPhysicsObjData.WriteDWORD(dwModelNumber);
	}

	if (pEntity->IsWielded())
	{
		dwSections |= PhysicsDescInfo::PARENT;
		OptionalPhysicsObjData.WriteDWORD(pEntity->GetWielderID());
		OptionalPhysicsObjData.WriteDWORD(pEntity->GetEquipSlot());
	}

	ItemVector vItems;
	pEntity->Container_GetWieldedItems(&vItems);

	if (!vItems.empty())
	{
		DWORD	dwEquippedCount = 0;
		BinaryWriter pEquipped;

		ItemVector::iterator iit = vItems.begin();
		ItemVector::iterator iend = vItems.end();
		while (iit != iend)
		{
			CBaseItem* pItem = *iit;

			if (pItem->HasEquipSlot())
			{
				pEquipped.WriteDWORD(pItem->m_dwGUID);
				pEquipped.WriteDWORD(pItem->GetEquipSlot());
				dwEquippedCount++;
			}
			iit++;
		}

		if (dwEquippedCount)
		{
			dwSections |= PhysicsDescInfo::CHILDREN;
			OptionalPhysicsObjData.WriteDWORD(dwEquippedCount);
			OptionalPhysicsObjData.AppendData(pEquipped.GetData(), pEquipped.GetSize());
		}
	}

	if (pEntity->m_fScale != 0.0f && pEntity->m_fScale != 1.0f)
	{
		dwSections |= PhysicsDescInfo::OBJSCALE;
		OptionalPhysicsObjData.WriteFloat(pEntity->m_fScale);
	}

	BinaryWriter *PhysicsObjData = new BinaryWriter;
	PhysicsObjData->WriteDWORD(dwSections);
	PhysicsObjData->WriteDWORD(pEntity->m_PhysicsState); //VIS flags
	PhysicsObjData->AppendData(OptionalPhysicsObjData.GetData(), OptionalPhysicsObjData.GetSize());

	//Moved from CO
	PhysicsObjData->WriteWORD(pEntity->m_wNumMovements);
	PhysicsObjData->WriteWORD(pEntity->m_wNumAnimInteracts);
	PhysicsObjData->WriteWORD(pEntity->m_wNumBubbleModes);
	PhysicsObjData->WriteWORD(pEntity->m_wNumJumps);
	PhysicsObjData->WriteWORD(pEntity->m_wNumPortals);
	PhysicsObjData->WriteWORD(pEntity->m_wAnimCount);
	PhysicsObjData->WriteWORD(pEntity->m_wNumOverride);
	PhysicsObjData->WriteWORD(pEntity->m_wNumModelChanges);
	PhysicsObjData->WriteWORD(pEntity->m_wInstance);
	PhysicsObjData->WriteWORD(pEntity->m_wSeagreen10);

	return PhysicsObjData;
}

BinaryWriter *CreateObject(CPhysicsObj *pEntity)
{
	BinaryWriter *CO = new BinaryWriter;

	CO->WriteDWORD(0xF745);
	CO->WriteDWORD(pEntity->m_dwGUID);

	BinaryWriter *ModelData = pEntity->GetModelData();
	CO->AppendData(ModelData->GetData(), ModelData->GetSize());
	delete ModelData;

	BinaryWriter *PhysicsObjData = GetPhysicsObjData(pEntity);
	CO->AppendData(PhysicsObjData->GetData(), PhysicsObjData->GetSize());
	delete PhysicsObjData;

	BinaryWriter *WeenieObjData = GetWeenieObjData(pEntity);
	CO->AppendData(WeenieObjData->GetData(), WeenieObjData->GetSize());
	delete WeenieObjData;

	CO->Align();
	return CO;
}

BinaryWriter *UpdateObject(CPhysicsObj *pEntity)
{
	BinaryWriter *UO = new BinaryWriter;

	UO->WriteDWORD(0xF7DB);
	UO->WriteDWORD(pEntity->m_dwGUID);

	BinaryWriter *ModelData = pEntity->GetModelData();
	UO->AppendData(ModelData->GetData(), ModelData->GetSize());
	delete ModelData;

	BinaryWriter *PhysicsObjData = GetPhysicsObjData(pEntity);
	UO->AppendData(PhysicsObjData->GetData(), PhysicsObjData->GetSize());
	delete PhysicsObjData;

	BinaryWriter *WeenieObjData = GetWeenieObjData(pEntity);
	UO->AppendData(WeenieObjData->GetData(), WeenieObjData->GetSize());
	delete WeenieObjData;

	UO->Align();
	return UO;
}

BinaryWriter *IdentifyObject_DwordData(CPhysicsObj *pEntity)
{
	BinaryWriter *Dwords = new BinaryWriter;
	int DwordCount = 0;

	for (int i = 0; i < 0x100; i++) {
		DWORD dwValue = pEntity->m_dwStats[i];

		if (dwValue)
		{
			DwordCount++;

			Dwords->WriteDWORD(i);
			Dwords->WriteDWORD(dwValue);
		}
	}

	if (!DwordCount) {
		delete Dwords;
		return NULL;
	}

	BinaryWriter *DwordData = new BinaryWriter;

	DwordData->WriteWORD(DwordCount);
	DwordData->WriteWORD(0x40);
	DwordData->AppendData(Dwords->GetData(), Dwords->GetSize());
	delete Dwords;

	return DwordData;
}

BinaryWriter *IdentifyObject_StringData(CPhysicsObj *pEntity)
{
	if (!pEntity->IsItem())
		return NULL;

	BinaryWriter *Strings = new BinaryWriter;
	WORD wStringCount = 0;

	const char* szDescription = pEntity->GetDescription();
	if (szDescription && (strlen(szDescription) > 0))
	{
		wStringCount++;
		Strings->WriteDWORD(0x10);
		Strings->WriteString(szDescription);
	}

	if (!wStringCount)
	{
		delete Strings;
		return NULL;
	}

	BinaryWriter *StringData = new BinaryWriter;

	StringData->WriteWORD(wStringCount);
	StringData->WriteWORD(0x10);
	StringData->AppendData(Strings->GetData(), Strings->GetSize());
	delete Strings;

	return StringData;
}

BinaryWriter *IdentifyObject_PlayerData(CPhysicsObj *pEntity)
{
	if (!pEntity->IsPlayer())
		return NULL;

	CBasePlayer* pPlayer = (CBasePlayer *)pEntity;

	BinaryWriter *PlayerPoo = new BinaryWriter;
	DWORD dwContents = 5;

	if (pPlayer->IsHuman())
		dwContents |= 2;

	PlayerPoo->WriteDWORD(dwContents);
	if (dwContents & 4) {
		PlayerPoo->WriteDWORD(0/*pPlayer->GetObjectStat( eLevel )*/);
		PlayerPoo->WriteDWORD(0/*GetVitalCurrent( pPlayer->GetVitalData( eHealth ), 0 )*/);
		PlayerPoo->WriteDWORD(GetVitalMax(pPlayer->GetVitalData(eHealth), 0));
	}
	if (dwContents & 8) {
		PlayerPoo->WriteDWORD(GetAttributeMax(pPlayer->GetAttributeData(eStrength), 0));
		PlayerPoo->WriteDWORD(GetAttributeMax(pPlayer->GetAttributeData(eEndurance), 0));
		PlayerPoo->WriteDWORD(GetAttributeMax(pPlayer->GetAttributeData(eQuickness), 0));
		PlayerPoo->WriteDWORD(GetAttributeMax(pPlayer->GetAttributeData(eCoordination), 0));
		PlayerPoo->WriteDWORD(GetAttributeMax(pPlayer->GetAttributeData(eFocus), 0));
		PlayerPoo->WriteDWORD(GetAttributeMax(pPlayer->GetAttributeData(eSelf), 0));
		PlayerPoo->WriteDWORD(GetVitalCurrent(pPlayer->GetVitalData(eStamina), 0));
		PlayerPoo->WriteDWORD(GetVitalCurrent(pPlayer->GetVitalData(eMana), 0));
		PlayerPoo->WriteDWORD(GetVitalMax(pPlayer->GetVitalData(eStamina), 0));
		PlayerPoo->WriteDWORD(GetVitalMax(pPlayer->GetVitalData(eMana), 0));
	}

	PlayerPoo->WriteDWORD(0x1F);
	if (dwContents & 2) {
		PlayerPoo->WriteDWORD(pPlayer->GetObjectStat(eLevel));
		PlayerPoo->WriteDWORD(-1);
		PlayerPoo->WriteDWORD(1);
		PlayerPoo->WriteString(pPlayer->GetGenderString());
		PlayerPoo->WriteString(pPlayer->GetRaceString());
		PlayerPoo->WriteString(pPlayer->GetTitleString());
		PlayerPoo->WriteString(NULL);
		PlayerPoo->WriteString(NULL);
		PlayerPoo->WriteString(NULL);
		PlayerPoo->WriteDWORD(0x05001119);
		PlayerPoo->WriteDWORD(0x05001161);
		PlayerPoo->WriteDWORD(0x050011E3);
		PlayerPoo->WriteDWORD(0x04000EB6);//0x040002BD );
		PlayerPoo->WriteDWORD(0x04000EB6);//0x040002F2 );
		PlayerPoo->WriteDWORD(0x04000EB6);//0x040004A8 );
	}
	if (dwContents & 1) {
		PlayerPoo->WriteDWORD(0x1C0);
	}

	return PlayerPoo;
}

BinaryWriter *IdentifyObject(CPhysicsObj *pEntity)
{
	BinaryWriter *IdentTotal = new BinaryWriter;
	BinaryWriter *IdentStuff = new BinaryWriter;

	//Possible sections

	IdentTotal->WriteDWORD(0xC9);
	IdentTotal->WriteDWORD(pEntity->m_dwGUID);

	BinaryWriter *Stuffing;
	DWORD dwSections = 0;

	if (Stuffing = IdentifyObject_DwordData(pEntity))
	{
		dwSections |= 0x00000001;
		IdentStuff->AppendData(Stuffing->GetData(), Stuffing->GetSize());
		delete Stuffing;
	}
	if (Stuffing = IdentifyObject_StringData(pEntity))
	{
		dwSections |= 0x00000008;
		IdentStuff->AppendData(Stuffing->GetData(), Stuffing->GetSize());
		delete Stuffing;
	}
	if (Stuffing = IdentifyObject_PlayerData(pEntity))
	{
		dwSections |= 0x00000100;
		IdentStuff->AppendData(Stuffing->GetData(), Stuffing->GetSize());
		delete Stuffing;
	}

	IdentTotal->WriteDWORD(dwSections);
	IdentTotal->WriteDWORD(TRUE); //always success for now
	IdentTotal->AppendData(IdentStuff->GetData(), IdentStuff->GetSize());
	delete IdentStuff;

	return IdentTotal;
}

BinaryWriter *GetStatInfo(CBasePlayer *pPlayer)
{
	//0x05, 0x14, 0x19, 0x1E, 0x62, 0x7D (we'll add unassigned / totalxp too)
	BinaryWriter Stats;

	Stats.WriteDWORD(eTotalBurden);
	Stats.WriteDWORD(pPlayer->GetObjectStat(eTotalBurden));
	Stats.WriteDWORD(eTotalPyreal);
	Stats.WriteDWORD(pPlayer->GetObjectStat(eTotalPyreal));
	Stats.WriteDWORD(eTotalXP);
	Stats.WriteDWORD(pPlayer->GetObjectStat(eTotalXP));
	Stats.WriteDWORD(eUnassignedXP);
	Stats.WriteDWORD(pPlayer->GetObjectStat(eUnassignedXP));
	Stats.WriteDWORD(eLevel);
	Stats.WriteDWORD(pPlayer->GetObjectStat(eLevel));
	Stats.WriteDWORD(eRank);
	Stats.WriteDWORD(pPlayer->GetObjectStat(eRank));
	Stats.WriteDWORD(eBirth);
	Stats.WriteDWORD(pPlayer->GetObjectStat(eBirth));
	Stats.WriteDWORD(eAge);
	Stats.WriteDWORD(pPlayer->GetObjectStat(eAge));
	Stats.WriteDWORD(eFrozen);
	Stats.WriteDWORD(pPlayer->GetObjectStat(eFrozen));
	Stats.WriteDWORD(eUnknown2);
	Stats.WriteDWORD(pPlayer->GetObjectStat(eUnknown2));

	BinaryWriter *StatInfo = new BinaryWriter;

	StatInfo->WriteWORD(0xA);
	StatInfo->WriteWORD(0x40);
	StatInfo->AppendData(Stats.GetData(), Stats.GetSize());

	return StatInfo;
}

void WriteSkillData(WORD index, SKILL* skill, BinaryWriter *pFood)
{
	pFood->WriteWORD(index);
	pFood->Align();
	pFood->AppendData(&skill->data, sizeof(_SKILLDATA));
}

BinaryWriter *GetSkillInfo(CBasePlayer *pPlayer)
{
	BinaryWriter Skills;

	WriteSkillData(eAxe, pPlayer->GetSkillData(eAxe), &Skills);
	WriteSkillData(eBow, pPlayer->GetSkillData(eBow), &Skills);
	WriteSkillData(eCrossbow, pPlayer->GetSkillData(eCrossbow), &Skills);
	WriteSkillData(eDagger, pPlayer->GetSkillData(eDagger), &Skills);
	WriteSkillData(eMace, pPlayer->GetSkillData(eMace), &Skills); //05
	WriteSkillData(eMeleeDefense, pPlayer->GetSkillData(eMeleeDefense), &Skills);
	WriteSkillData(eMissileDefense, pPlayer->GetSkillData(eMissileDefense), &Skills);
	WriteSkillData(eSpear, pPlayer->GetSkillData(eSpear), &Skills);
	WriteSkillData(eStaff, pPlayer->GetSkillData(eStaff), &Skills);
	WriteSkillData(eSword, pPlayer->GetSkillData(eSword), &Skills); //10
	WriteSkillData(eThrownWeapons, pPlayer->GetSkillData(eThrownWeapons), &Skills);
	WriteSkillData(eUnarmedCombat, pPlayer->GetSkillData(eUnarmedCombat), &Skills);
	WriteSkillData(eArcaneLore, pPlayer->GetSkillData(eArcaneLore), &Skills);
	WriteSkillData(eMagicDefense, pPlayer->GetSkillData(eMagicDefense), &Skills);
	WriteSkillData(eManaConversion, pPlayer->GetSkillData(eManaConversion), &Skills); //15
	WriteSkillData(eAppraiseItem, pPlayer->GetSkillData(eAppraiseItem), &Skills);
	WriteSkillData(eAssessPerson, pPlayer->GetSkillData(eAssessPerson), &Skills);
	WriteSkillData(eDeception, pPlayer->GetSkillData(eDeception), &Skills);
	WriteSkillData(eHealing, pPlayer->GetSkillData(eHealing), &Skills);
	WriteSkillData(eJump, pPlayer->GetSkillData(eJump), &Skills); //20
	WriteSkillData(eLockpick, pPlayer->GetSkillData(eLockpick), &Skills);
	WriteSkillData(eRun, pPlayer->GetSkillData(eRun), &Skills);
	WriteSkillData(eAssessCreature, pPlayer->GetSkillData(eAssessCreature), &Skills);
	WriteSkillData(eAppraiseWeapon, pPlayer->GetSkillData(eAppraiseWeapon), &Skills);
	WriteSkillData(eAppraiseArmor, pPlayer->GetSkillData(eAppraiseArmor), &Skills); //25
	WriteSkillData(eAppraiseMagicItem, pPlayer->GetSkillData(eAppraiseMagicItem), &Skills);
	WriteSkillData(eCreatureEnchantment, pPlayer->GetSkillData(eCreatureEnchantment), &Skills);
	WriteSkillData(eItemEnchantment, pPlayer->GetSkillData(eItemEnchantment), &Skills);
	WriteSkillData(eLifeMagic, pPlayer->GetSkillData(eLifeMagic), &Skills);
	WriteSkillData(eWarMagic, pPlayer->GetSkillData(eWarMagic), &Skills); //30
	WriteSkillData(eLeadership, pPlayer->GetSkillData(eLeadership), &Skills);
	WriteSkillData(eLoyalty, pPlayer->GetSkillData(eLoyalty), &Skills);
	WriteSkillData(eFletching, pPlayer->GetSkillData(eFletching), &Skills);
	WriteSkillData(eAlchemy, pPlayer->GetSkillData(eAlchemy), &Skills);
	WriteSkillData(eCooking, pPlayer->GetSkillData(eCooking), &Skills);

	BinaryWriter *SkillInfo = new BinaryWriter;
	SkillInfo->WriteWORD(0x23); //skillcount
	SkillInfo->WriteWORD(0x20);
	SkillInfo->AppendData(Skills.GetData(), Skills.GetSize());

	return SkillInfo;
}

BinaryWriter *LoginCharacter(CBasePlayer *pPlayer)
{
	//Game Event 0x0013
	BinaryWriter *LC = new BinaryWriter;

	LC->WriteDWORD(0x0013);
	LC->WriteDWORD(0x001B); //loginmask1
	LC->WriteDWORD(0x000A);

	BinaryWriter *StatInfo = GetStatInfo(pPlayer);
	LC->AppendData(StatInfo->GetData(), StatInfo->GetSize());
	delete StatInfo;

	//What the hell are these?
	LC->WriteWORD(5);
	LC->WriteWORD(0x20);
	LC->WriteDWORD(0x0004);
	LC->WriteDWORD(0x0000);
	LC->WriteDWORD(0x002C);
	LC->WriteDWORD(0x0000);
	LC->WriteDWORD(0x002D);
	LC->WriteDWORD(0x0000);
	LC->WriteDWORD(0x002E);
	LC->WriteDWORD(0x0000);
	LC->WriteDWORD(0x002F);
	LC->WriteDWORD(0x0000);

	//Character strings
	LC->WriteWORD(4);
	LC->WriteWORD(0x10);
	LC->WriteDWORD(0x0001);
	LC->WriteString(pPlayer->GetName());
	LC->WriteDWORD(0x0003);
	LC->WriteString(pPlayer->GetGenderString());
	LC->WriteDWORD(0x0004);
	LC->WriteString(pPlayer->GetRaceString());
	LC->WriteDWORD(0x0005);
	LC->WriteString(pPlayer->GetTitleString());

	//What the hell is this?
	LC->WriteWORD(1);
	LC->WriteWORD(0x20);
	LC->WriteDWORD(0x00000004);
	LC->WriteDWORD(0x30000000);

	LC->WriteDWORD(0x0103); //loginmask2
	LC->WriteDWORD(0x0001); //???
	LC->WriteDWORD(0x01FF);

	for (int i = 1; i <= 6; i++)
	{
		ATTRIBUTE* attrib = pPlayer->GetAttributeData((eAttribute)i);
		LC->WriteData(&attrib->data, sizeof(_ATTRIBUTEDATA));
	}
	for (int i = 1; i <= 5; i += 2)
	{
		VITAL* vital = pPlayer->GetVitalData((eVital)i);
		LC->WriteData(&vital->data, sizeof(_VITALDATA));
	}

	BinaryWriter *SkillInfo = GetSkillInfo(pPlayer);
	LC->AppendData(SkillInfo->GetData(), SkillInfo->GetSize());
	delete SkillInfo;

	//spellbook:
	//LC->WriteWORD( 55 );
	LC->WriteWORD(6); // Number of spells
	LC->WriteWORD(0x40); // Required ?

	LC->WriteDWORD(2366); //Spell ID
	LC->WriteFloat(2.0f); //Unknown

	for (int i = 0; i < 5; i++) //Add 6 starter spells
	{
		LC->WriteDWORD( i );
		LC->WriteFloat( 2.0f ); 
	}

	LC->WriteDWORD(0x0070); //loginmask3
	LC->WriteDWORD(0x11C4E56A);
	LC->WriteDWORD(0x00000000);//spellbar 1 count
	LC->WriteDWORD(0x00000000);//spellbar 2 count
	LC->WriteDWORD(0x00000000);//spellbar 3 count
	LC->WriteDWORD(0x00000000);//spellbar 4 count
	LC->WriteDWORD(0x00000000);//spellbar 5 count
	LC->WriteDWORD(0x00000000);//spellbar 6 count
	LC->WriteDWORD(0x00000000);//spellbar 7 count
	//LC->WriteDWORD(0x000007FF);//unknown..
	LC->WriteDWORD(0x00000041); //?unk20mask3
	LC->WriteDWORD(0x00000000); //?unk40mask3
	LC->WriteDWORD(0x00000000);//equipment
	LC->WriteDWORD(0x00000000);//inventory

	return LC;
}

BinaryWriter *HealthUpdate(CBaseMonster *pMonster)
{
	MESSAGE_BEGIN(HealthUpdate);

	HealthUpdate->WriteDWORD(0x1C0);
	HealthUpdate->WriteDWORD(pMonster->m_dwGUID);
	HealthUpdate->WriteFloat(GetVitalPercent(pMonster->GetVitalData(eHealth), 0));

	MESSAGE_END(HealthUpdate);
}

BinaryWriter *InventoryEquip(DWORD dwItemID, DWORD dwCoverage)
{
	MESSAGE_BEGIN(InventoryEquip);

	InventoryEquip->WriteDWORD(0x23);
	InventoryEquip->WriteDWORD(dwItemID);
	InventoryEquip->WriteDWORD(dwCoverage);

	MESSAGE_END(InventoryEquip);
}

BinaryWriter *InventoryMove(DWORD dwItemID, DWORD dwContainerID, DWORD dwSlot, DWORD dwType)
{
	MESSAGE_BEGIN(InventoryMove);

	InventoryMove->WriteDWORD(0x22);
	InventoryMove->WriteDWORD(dwItemID);
	InventoryMove->WriteDWORD(dwContainerID);
	InventoryMove->WriteDWORD(dwSlot);
	InventoryMove->WriteDWORD(dwType);

	MESSAGE_END(InventoryMove);
}

BinaryWriter *InventoryDrop(DWORD dwItemID)
{
	MESSAGE_BEGIN(InventoryDrop);

	InventoryDrop->WriteDWORD(0x19A);
	InventoryDrop->WriteDWORD(dwItemID);

	MESSAGE_END(InventoryDrop);
}

BinaryWriter *MoveUpdate(CPhysicsObj *pEntity)
{
	MESSAGE_BEGIN(MoveUpdate);
	BinaryWriter *MovePayload = new BinaryWriter;

	MoveUpdate->WriteDWORD(0xF748);
	MoveUpdate->WriteDWORD(pEntity->m_dwGUID);
	
	//Nasty, but it reduces the linecount =) Sorry Akilla.
	DWORD dwPayloadFlags = 4;
	if (!pEntity->m_Angles.w) dwPayloadFlags |= 0x00000008;
	else MovePayload->WriteFloat(pEntity->m_Angles.w);
	if (!pEntity->m_Angles.x) dwPayloadFlags |= 0x00000010;
	else MovePayload->WriteFloat(pEntity->m_Angles.x);
	if (!pEntity->m_Angles.y) dwPayloadFlags |= 0x00000020;
	else MovePayload->WriteFloat(pEntity->m_Angles.y);
	if (!pEntity->m_Angles.z) dwPayloadFlags |= 0x00000040;
	else MovePayload->WriteFloat(pEntity->m_Angles.z);
	if (pEntity->CanPickup()) {
		dwPayloadFlags |= 0x00000002;
		MovePayload->WriteDWORD(0x000000065);
	}

	MoveUpdate->WriteDWORD(dwPayloadFlags);
	MoveUpdate->AppendData(pEntity->m_Origin);
	MoveUpdate->AppendData(MovePayload->GetData(), MovePayload->GetSize());

	MoveUpdate->WriteWORD(pEntity->m_wInstance);
	MoveUpdate->WriteWORD(pEntity->m_wNumMovements);
	MoveUpdate->WriteWORD(pEntity->m_wNumPortals);
	MoveUpdate->WriteWORD(0);

	delete MovePayload;
	MESSAGE_END(MoveUpdate);
}
