
#include "StdAfx.h"
#include "PhysicsObj.h"
#include "Monster.h"
#include "Player.h"
#include "Item.h"

//Network access.
#include "Client.h"
#include "NetFood.h"
#include "ObjectMsgs.h"

#define MESSAGE_BEGIN(x)	NetFood *x = new NetFood
#define MESSAGE_END(x)		return x

NetFood *GetClassData(CPhysicsObj *pEntity)
{
	NetFood Class;
	DWORD dwSections = 0; //0x00800016

	if (pEntity->Container_HasContents())
	{
		dwSections |= 0x00000002;
		Class.WriteBYTE(pEntity->Container_MaxInventorySlots());

		dwSections |= 0x00000004;
		Class.WriteBYTE(pEntity->Container_MaxContainerSlots());
	}

	if (pEntity->HasValue())
	{
		dwSections |= 0x00000008;
		Class.WriteDWORD(pEntity->GetValue());
	}

	//Use item? 1 = Cannot be used?
	dwSections |= 0x00000010;
	Class.WriteDWORD((DWORD)pEntity->m_Usability);

	float flApproachDist = pEntity->GetApproachDist();
	if (flApproachDist > 0.0f)
	{
		dwSections |= 0x00000020;
		Class.WriteFloat(flApproachDist);
	}

	DWORD dwHighlight = pEntity->GetHighlightColor();
	if (dwHighlight)
	{
		dwSections |= 0x00000080;
		Class.WriteDWORD(dwHighlight);
	}

	if (pEntity->HasEquipType())
	{
		dwSections |= 0x00000200;
		Class.WriteDWORD(pEntity->GetEquipType());
	}

	if (pEntity->IsContained())
	{
		dwSections |= 0x00004000;
		Class.WriteDWORD(pEntity->GetContainerID());
	}

	if (pEntity->HasCoverage())
	{
		CBaseItem *pItem = (CBaseItem*)pEntity;
		DWORD dwCoverage1 = pItem->GetCoverage1();
		DWORD dwCoverage2 = pItem->GetCoverage2();
		DWORD dwCoverage3 = pItem->GetCoverage3();

		if (dwCoverage1)
		{
			dwSections |= 0x00010000;
			Class.WriteDWORD(dwCoverage1);
		}
		if (dwCoverage2)
		{
			dwSections |= 0x00020000;
			Class.WriteDWORD(dwCoverage1);
		}
		if (dwCoverage3)
		{
			dwSections |= 0x00040000;
			Class.WriteDWORD(dwCoverage1);
		}
	}

	if (pEntity->HasRadarDot())
	{
		dwSections |= 0x00100000;
		Class.WriteBYTE(pEntity->GetRadarColor());
	}

	// if (pEntity->IsPlayer())
	{
		dwSections |= 0x00800000;
		Class.WriteBYTE((BYTE)pEntity->m_RadarVis);
	}

	if (pEntity->HasBurden())
	{
		dwSections |= 0x00200000;
		Class.WriteWORD(pEntity->GetBurden());
	}

	NetFood *ClassData = new NetFood;
	ClassData->WriteDWORD(dwSections);
	ClassData->WriteString(pEntity->GetName());
	ClassData->WriteWORD(pEntity->m_wTypeID);
	ClassData->WriteWORD(pEntity->m_wIcon);
	ClassData->WriteDWORD(pEntity->m_ItemType);
	ClassData->WriteDWORD(pEntity->GetDescFlags());
	ClassData->AppendData(Class.GetData(), Class.GetSize());

	return ClassData;
}

NetFood *GetOrientData(CPhysicsObj *pEntity)
{
	NetFood Orient;
	DWORD dwSections = 0; //0x019803

	if (pEntity->m_dwAnimationSet)
	{
		dwSections |= 0x10000;

		NetFood *AnimInfo = pEntity->Animation_GetAnimationInfo();

		Orient.WriteDWORD(AnimInfo->GetSize()); //byte size
		Orient.AppendData(AnimInfo->GetData(), AnimInfo->GetSize());

		//unk:
		Orient.WriteDWORD(0);

		delete AnimInfo;
	}

	if (pEntity->CanPickup())
	{
		dwSections |= 0x20000;
		Orient.WriteDWORD(0x00000065); //not sure what this does
	}

	if (!pEntity->HasOwner())
	{
		dwSections |= 0x8000;
		Orient.AppendData(&pEntity->m_Origin, sizeof(loc_t));
		Orient.AppendData(&pEntity->m_Angles, sizeof(heading_t));
	}

	if (pEntity->m_dwAnimationSet)
	{
		dwSections |= 0x0002;
		Orient.WriteDWORD(pEntity->m_dwAnimationSet);
	}

	DWORD dwSoundSet = pEntity->GetSoundSet();
	if (dwSoundSet)
	{
		dwSections |= 0x0800;
		Orient.WriteDWORD(dwSoundSet);
	}

	DWORD dwEffectSet = pEntity->GetEffectSet();
	if (dwEffectSet)
	{
		dwSections |= 0x1000;
		Orient.WriteDWORD(dwEffectSet);
	}

	DWORD dwModelNumber = pEntity->m_dwModel;
	if (dwModelNumber)
	{
		dwSections |= 0x0001;
		Orient.WriteDWORD(dwModelNumber);
	}

	if (pEntity->IsWielded())
	{
		dwSections |= 0x0020;
		Orient.WriteDWORD(pEntity->GetWielderID());
		Orient.WriteDWORD(pEntity->GetEquipSlot());
	}

	ItemVector vItems;
	pEntity->Container_GetWieldedItems(&vItems);

	if (!vItems.empty())
	{
		DWORD	dwEquippedCount = 0;
		NetFood pEquipped;

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
			dwSections |= 0x0040;
			Orient.WriteDWORD(dwEquippedCount);
			Orient.AppendData(pEquipped.GetData(), pEquipped.GetSize());
		}
	}

	float flScale = pEntity->m_fScale;
	if (flScale && flScale != 1.0f)
	{
		dwSections |= 0x0080;
		Orient.WriteFloat(flScale);
	}

	NetFood *OrientData = new NetFood;
	OrientData->WriteDWORD(dwSections);
	OrientData->WriteDWORD(pEntity->m_VisFlags); //VIS flags
	OrientData->AppendData(Orient.GetData(), Orient.GetSize());

	//Moved from CO
	OrientData->WriteWORD(pEntity->m_wNumMovements);
	OrientData->WriteWORD(pEntity->m_wNumAnimInteracts);
	OrientData->WriteWORD(pEntity->m_wNumBubbleModes);
	OrientData->WriteWORD(pEntity->m_wNumJumps);
	OrientData->WriteWORD(pEntity->m_wNumPortals);
	OrientData->WriteWORD(pEntity->m_wAnimCount);
	OrientData->WriteWORD(pEntity->m_wNumOverride);
	OrientData->WriteWORD(pEntity->m_wNumModelChanges);
	OrientData->WriteWORD(pEntity->m_wInstance);
	OrientData->WriteWORD(pEntity->m_wSeagreen10);

	return OrientData;
}

NetFood *CreateObject(CPhysicsObj *pEntity)
{
	NetFood *CO = new NetFood;

	CO->WriteDWORD(0xF745);
	CO->WriteDWORD(pEntity->m_dwGUID);

	NetFood *ModelData = pEntity->GetModelData();
	CO->AppendData(ModelData->GetData(), ModelData->GetSize());
	delete ModelData;

	NetFood *OrientData = GetOrientData(pEntity);
	CO->AppendData(OrientData->GetData(), OrientData->GetSize());
	delete OrientData;

	NetFood *ClassData = GetClassData(pEntity);
	CO->AppendData(ClassData->GetData(), ClassData->GetSize());
	delete ClassData;

	CO->Align();
	return CO;
}

NetFood *UpdateObject(CPhysicsObj *pEntity)
{
	NetFood *UO = new NetFood;

	UO->WriteDWORD(0xF7DB);
	UO->WriteDWORD(pEntity->m_dwGUID);

	NetFood *ModelData = pEntity->GetModelData();
	UO->AppendData(ModelData->GetData(), ModelData->GetSize());
	delete ModelData;

	NetFood *OrientData = GetOrientData(pEntity);
	UO->AppendData(OrientData->GetData(), OrientData->GetSize());
	delete OrientData;

	NetFood *ClassData = GetClassData(pEntity);
	UO->AppendData(ClassData->GetData(), ClassData->GetSize());
	delete ClassData;

	UO->Align();
	return UO;
}

NetFood *IdentifyObject_DwordData(CPhysicsObj *pEntity)
{
	NetFood *Dwords = new NetFood;
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

	NetFood *DwordData = new NetFood;

	DwordData->WriteWORD(DwordCount);
	DwordData->WriteWORD(0x40);
	DwordData->AppendData(Dwords->GetData(), Dwords->GetSize());
	delete Dwords;

	return DwordData;
}

NetFood *IdentifyObject_StringData(CPhysicsObj *pEntity)
{
	if (!pEntity->IsItem())
		return NULL;

	NetFood *Strings = new NetFood;
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

	NetFood *StringData = new NetFood;

	StringData->WriteWORD(wStringCount);
	StringData->WriteWORD(0x10);
	StringData->AppendData(Strings->GetData(), Strings->GetSize());
	delete Strings;

	return StringData;
}

NetFood *IdentifyObject_PlayerData(CPhysicsObj *pEntity)
{
	if (!pEntity->IsPlayer())
		return NULL;

	CBasePlayer* pPlayer = (CBasePlayer *)pEntity;

	NetFood *PlayerPoo = new NetFood;
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

NetFood *IdentifyObject(CPhysicsObj *pEntity)
{
	NetFood *IdentTotal = new NetFood;
	NetFood *IdentStuff = new NetFood;

	//Possible sections

	IdentTotal->WriteDWORD(0xC9);
	IdentTotal->WriteDWORD(pEntity->m_dwGUID);

	NetFood *Stuffing;
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

NetFood *GetStatInfo(CBasePlayer *pPlayer)
{
	//0x05, 0x14, 0x19, 0x1E, 0x62, 0x7D (we'll add unassigned / totalxp too)
	NetFood Stats;

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

	NetFood *StatInfo = new NetFood;

	StatInfo->WriteWORD(0xA);
	StatInfo->WriteWORD(0x40);
	StatInfo->AppendData(Stats.GetData(), Stats.GetSize());

	return StatInfo;
}

void WriteSkillData(WORD index, SKILL* skill, NetFood *pFood)
{
	pFood->WriteWORD(index);
	pFood->Align();
	pFood->AppendData(&skill->data, sizeof(_SKILLDATA));
}

NetFood *GetSkillInfo(CBasePlayer *pPlayer)
{
	NetFood Skills;

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

	NetFood *SkillInfo = new NetFood;
	SkillInfo->WriteWORD(0x23); //skillcount
	SkillInfo->WriteWORD(0x20);
	SkillInfo->AppendData(Skills.GetData(), Skills.GetSize());

	return SkillInfo;
}

NetFood *LoginCharacter(CBasePlayer *pPlayer)
{
	//Game Event 0x0013
	NetFood *LC = new NetFood;

	LC->WriteDWORD(0x0013);
	LC->WriteDWORD(0x001B); //loginmask1
	LC->WriteDWORD(0x000A);

	NetFood *StatInfo = GetStatInfo(pPlayer);
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

	NetFood *SkillInfo = GetSkillInfo(pPlayer);
	LC->AppendData(SkillInfo->GetData(), SkillInfo->GetSize());
	delete SkillInfo;

	//spellbook:
	//LC->WriteWORD( 3000 );
	LC->WriteWORD(1);
	LC->WriteWORD(0x40);
	LC->WriteDWORD(2366);
	LC->WriteFloat(2.0f);
	//for (int i = 0; i < 3000; i++)
	//{
	//	LC->WriteDWORD( i );
	//	LC->WriteFloat( 2.0f );
	//}

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

NetFood *HealthUpdate(CBaseMonster *pMonster)
{
	MESSAGE_BEGIN(HealthUpdate);

	HealthUpdate->WriteDWORD(0x1C0);
	HealthUpdate->WriteDWORD(pMonster->m_dwGUID);
	HealthUpdate->WriteFloat(GetVitalPercent(pMonster->GetVitalData(eHealth), 0));

	MESSAGE_END(HealthUpdate);
}

NetFood *InventoryEquip(DWORD dwItemID, DWORD dwCoverage)
{
	MESSAGE_BEGIN(InventoryEquip);

	InventoryEquip->WriteDWORD(0x23);
	InventoryEquip->WriteDWORD(dwItemID);
	InventoryEquip->WriteDWORD(dwCoverage);

	MESSAGE_END(InventoryEquip);
}

NetFood *InventoryMove(DWORD dwItemID, DWORD dwContainerID, DWORD dwSlot, DWORD dwType)
{
	MESSAGE_BEGIN(InventoryMove);

	InventoryMove->WriteDWORD(0x22);
	InventoryMove->WriteDWORD(dwItemID);
	InventoryMove->WriteDWORD(dwContainerID);
	InventoryMove->WriteDWORD(dwSlot);
	InventoryMove->WriteDWORD(dwType);

	MESSAGE_END(InventoryMove);
}

NetFood *InventoryDrop(DWORD dwItemID)
{
	MESSAGE_BEGIN(InventoryDrop);

	InventoryDrop->WriteDWORD(0x19A);
	InventoryDrop->WriteDWORD(dwItemID);

	MESSAGE_END(InventoryDrop);
}

NetFood *MoveUpdate(CPhysicsObj *pEntity)
{
	MESSAGE_BEGIN(MoveUpdate);
	NetFood *MovePayload = new NetFood;

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
