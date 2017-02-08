
#include "StdAfx.h"
#include "PhysicsObj.h"
#include "ObjectMsgs.h"
#include "World.h"

#include "BinaryWriter.h"
#include "ChatMsgs.h"

#include "Item.h"
#include "Player.h"
#include "Client.h"
#include "ClientCommands.h"

CPhysicsObj::CPhysicsObj()
{
	m_dwGUID = 0;
	m_strName = "Unknown";

	m_WeenieBitfield = BF_STUCK | BF_ATTACKABLE;

	m_wNumMovements = 0x0000;
	m_wNumAnimInteracts = 0x0001;
	m_wNumBubbleModes = 0x0001;
	m_wNumJumps = 0x0000;
	m_wNumPortals = 0x0000;
	m_wAnimCount = 0x0000;
	m_wNumOverride = 0x0000;
	m_wNumModelChanges = 0x0000;
	m_wInstance = 0x0001;
	m_wSeagreen10 = 0x0000;

	m_wNumCombat = 0x0000;

	m_dwModel = 0x0200026B; //0x02000006 = turbine box
	m_fScale = 1.0f;

	m_wTypeID = 0x0001;
	m_wIcon = 0x1036;

	m_ItemType = TYPE_GEM;
	m_AmmoType = (AMMO_TYPE) 0;

	m_bRemoveMe = FALSE;
	m_pBlock = NULL;
	m_pfnThink = NULL;

	m_dwEffectSet = 0; // 0x34000004;
	m_dwSoundSet = 0; //0x2000000A = lugians

	memset(m_dwStats, 0, sizeof(m_dwStats));

	memset(&m_Origin, 0, sizeof(loc_t));
	memset(&m_Angles, 0, sizeof(heading_t));
	m_PhysicsState = PhysicsState::EDGE_SLIDE_PS | PhysicsState::LIGHTING_ON_PS | PhysicsState::GRAVITY_PS | PhysicsState::REPORT_COLLISIONS_PS;
	m_fNextThink = g_pGlobals->Time();
	
	m_Usability = USEABLE_NO;
	m_UseDistance = USEDISTANCE_ANYWHERE;
	m_RadarVis = ShowAlways_RadarEnum;

	m_Value = 0;
	m_Burden = 0;

	m_AnimOverrideData = NULL;
	m_AnimOverrideDataLen = 0;
	m_AutonomousMovement = 0;

	Movement_Init();
	Animation_Init();
	Container_Init();

	m_bUseModelOverride = false;

	m_bDontClear = false;

	m_BlipColor = 0;
	m_TargetType = (ITEM_TYPE)0;

	m_Translucency = 0.0f;

	m_bLifestoneBound = false;
	m_UIEffects = 0;

	m_dwEquipSlot = eEquipTypeNone;
	m_dwEquipType = eEquipSlotNone;

	m_dwCoverage1 = 0; //What it CAN cover?
	m_dwCoverage2 = 0; //What it IS covering?
	m_dwCoverage3 = 0; //??
}

CPhysicsObj::~CPhysicsObj()
{
	Container_Shutdown();
	Animation_Shutdown();
	Movement_Shutdown();
}

void CPhysicsObj::Spawn()
{
	//initialize children
}

void CPhysicsObj::Attach(CLandBlock *pBlock)
{
	m_pBlock = pBlock;
}

void CPhysicsObj::Detach()
{
	m_pBlock = NULL;
}

BOOL CPhysicsObj::HasOwner()
{
	if (IsContained())
		return TRUE;
	if (IsWielded())
		return TRUE;

	return FALSE;
}

CLandBlock* CPhysicsObj::GetBlock()
{
	return m_pBlock;
}

void CPhysicsObj::MakeLive(void)
{
	if (!m_pBlock)
		return;
	if (m_pfnThink) //already live
		return;

	m_pBlock->SetEntityLive(this);
}

BOOL CPhysicsObj::Think(void)
{
	if (!m_pfnThink) //Entity isn't thinking.
		return FALSE;

	if (m_fNextThink > g_pGlobals->Time()) //We need to wait to think.
		return TRUE;

	if (!(this->*m_pfnThink)()) //Entity going dormant.
		return FALSE;

	return TRUE;
}

void CPhysicsObj::SetLocation(DWORD landcell, float x, float y, float z)
{
	m_Origin.landcell = landcell;
	m_Origin.x = x;
	m_Origin.y = y;
	m_Origin.z = z;
}

void CPhysicsObj::SetAngles(float w, float x, float y, float z)
{
	m_Angles.w = w;
	m_Angles.x = x;
	m_Angles.y = y;
	m_Angles.z = z;
}

const char* CPhysicsObj::GetDescription()
{
	if (!m_strDescription.length())
	{
		ModelInfo miCurrentModel;
		std::string palettes;

		if (!IsPlayer() || (m_dwModel == 0x02000001 || m_dwModel == 0x0200004E))
		{
			miCurrentModel.bUnknown = m_miBaseModel.bUnknown;		//0x11
			miCurrentModel.dwBasePalette = m_miBaseModel.dwBasePalette;
			miCurrentModel.MergeData(&m_miBaseModel, NULL);

			ItemVector vItems;
			Container_GetEquippedArmor(&vItems);

			ItemVector::iterator iit = vItems.begin();
			ItemVector::iterator iend = vItems.end();
			while (iit != iend)
			{
				CBaseItem* pItem = *iit; iit++;

				if (pItem->IsWearable())
				{
					miCurrentModel.MergeData(&pItem->m_miWornModel, NULL);
				}
			}

			palettes = csprintf("Replacement Palettes (%d):", miCurrentModel.lPalettes.size());

			for (auto& palette : miCurrentModel.lPalettes)
			{
				palettes += csprintf("\n0x%X (0x%X 0x%X)", palette.dwPaletteID, palette.bOffset, palette.bLength);
			}
		}

		return csprintf(
				"GUID: 0x%08X\n"
				"ClassID: 0x%X\n"
				"Type: 0x%X\n"
				"State: 0x%X\n"
				"Bitfield: 0x%X\n"
				"Model: 0x%X\n"
				"Base Palette: 0x%X\n"
				"%s\n",
				m_dwGUID, m_wTypeID, m_ItemType, m_PhysicsState, m_WeenieBitfield, m_dwModel,
				m_miBaseModel.dwBasePalette, palettes.c_str());
	}

	return m_strDescription.c_str();
}

void CPhysicsObj::SetDescription(const char* szDescription)
{
	m_strDescription = szDescription;
}

void CPhysicsObj::Identify(CPhysicsObj *pSource)
{
	if (pSource->IsPlayer())
	{
		((CBasePlayer *)pSource)->SendNetMessage(IdentifyObject(this), PRIVATE_MSG, TRUE, TRUE);
	}
}

BinaryWriter* CPhysicsObj::CreateMessage()
{
	return CreateObject(this);
}

BinaryWriter* CPhysicsObj::UpdateMessage()
{
	return UpdateObject(this);
}

void CPhysicsObj::RemovePreviousInstance()
{
	DWORD RPI[3];

	RPI[0] = 0xF747;
	RPI[1] = m_dwGUID;
	RPI[2] = m_wInstance - 1;

	g_pWorld->BroadcastPVS(GetLandcell(), RPI, sizeof(RPI), 10/*OBJECT_MSG*/, 0, FALSE);
}

float CPhysicsObj::DistanceTo(CPhysicsObj *pOther)
{
	return (float) (Vector(m_Origin)-(pOther->m_Origin)).Length();
}

void CPhysicsObj::UpdateModel()
{
	if (HasOwner())
		return;

	BinaryWriter MU;

	MU.WriteDWORD(0xF625);
	MU.WriteDWORD(m_dwGUID);

	BinaryWriter *MD = GetModelData();
	MU.AppendData(MD->GetData(), MD->GetSize());
	delete MD;

	MU.WriteWORD(m_wInstance);
	MU.WriteWORD(++m_wNumModelChanges);

	g_pWorld->BroadcastPVS(GetLandcell(), MU.GetData(), MU.GetSize());
}

void CPhysicsObj::SetAppearanceOverride(ModelInfo *pAppearance)
{
	if (pAppearance)
	{
		m_miModelOverride = *pAppearance;
		m_bUseModelOverride = true;
	}
	else
		m_bUseModelOverride = false;

	UpdateModel();
}

BinaryWriter* CPhysicsObj::GetModelData()
{
	if (m_bUseModelOverride)
	{
		return m_miModelOverride.NetData();
	}
	else
	{
		ModelInfo miCurrentModel;

		// temporary, replace later
		if (!IsPlayer() || (m_dwModel == 0x02000001 || m_dwModel == 0x0200004E))
		{
			miCurrentModel.bUnknown = m_miBaseModel.bUnknown;		//0x11
			miCurrentModel.dwBasePalette = m_miBaseModel.dwBasePalette;
			miCurrentModel.MergeData(&m_miBaseModel, NULL);

			ItemVector vItems;
			Container_GetEquippedArmor(&vItems);

			ItemVector::iterator iit = vItems.begin();
			ItemVector::iterator iend = vItems.end();
			while (iit != iend)
			{
				CBaseItem* pItem = *iit; iit++;

				if (pItem->IsWearable())
				{
					miCurrentModel.MergeData(&pItem->m_miWornModel, NULL);
				}
			}
		}

		return miCurrentModel.NetData();
	}
}

void CPhysicsObj::ChangeVIS(DWORD dwFlags)
{
	m_wNumBubbleModes++;

	if (!HasOwner())
	{
		DWORD UVF[4];

		UVF[0] = 0xF74B;
		UVF[1] = m_dwGUID;
		UVF[2] = dwFlags;
		UVF[3] = (m_wNumBubbleModes << 16) | m_wInstance;

		g_pWorld->BroadcastPVS(GetLandcell(), UVF, sizeof(UVF));
	}

	m_PhysicsState = dwFlags;
}

void CPhysicsObj::EmitSound(DWORD dwIndex, float fSpeed)
{
	if (HasOwner())
		return;

	BinaryWriter SoundMsg;
	SoundMsg.WriteDWORD(0xF750);
	SoundMsg.WriteDWORD(m_dwGUID);
	SoundMsg.WriteDWORD(dwIndex);
	SoundMsg.WriteFloat(fSpeed);

	g_pWorld->BroadcastPVS(GetLandcell(), SoundMsg.GetData(), SoundMsg.GetSize(), OBJECT_MSG, 0);
}

void CPhysicsObj::EmitEffect(DWORD dwIndex, float flScale)
{
	if (HasOwner())
		return;

	BinaryWriter EffectMsg;
	EffectMsg.WriteDWORD(0xF755);
	EffectMsg.WriteDWORD(m_dwGUID);
	EffectMsg.WriteDWORD(dwIndex);
	EffectMsg.WriteFloat(flScale);

	g_pWorld->BroadcastPVS(GetLandcell(), EffectMsg.GetData(), EffectMsg.GetSize(), OBJECT_MSG, 0);
}

void CPhysicsObj::SpeakLocal(const char* szText, long lColor)
{
	if (HasOwner())
		return;

	extern bool g_bSilence;
	if (g_bSilence)
	{
		if (IsPlayer())
		{
			CBasePlayer *pPlayer = (CBasePlayer *)this;
			if (pPlayer->GetClient()->GetAccessLevel() < ADMIN_ACCESS)
			{
				return;
			}
		}
	}

	BinaryWriter *LC = LocalChat(szText, GetName(), m_dwGUID, lColor);
	g_pWorld->BroadcastPVS(GetLandcell(), LC->GetData(), LC->GetSize(), PRIVATE_MSG);
	delete LC;
}

void CPhysicsObj::EmoteLocal(const char* szText)
{
	if (HasOwner())
		return;

	extern bool g_bSilence;
	if (g_bSilence)
	{
		if (IsPlayer())
		{
			CBasePlayer *pPlayer = (CBasePlayer *)this;
			if (pPlayer->GetClient()->GetAccessLevel() < ADMIN_ACCESS)
			{
				return;
			}
		}
	}

	BinaryWriter *EL = EmoteChat(szText, GetName(), m_dwGUID);
	g_pWorld->BroadcastPVS(GetLandcell(), EL->GetData(), EL->GetSize(), PRIVATE_MSG);
	delete EL;
}

void CPhysicsObj::ActionLocal(const char* szText)
{
	if (HasOwner())
		return;

	extern bool g_bSilence;
	if (g_bSilence)
	{
		if (IsPlayer())
		{
			CBasePlayer *pPlayer = (CBasePlayer *)this;
			if (pPlayer->GetClient()->GetAccessLevel() < ADMIN_ACCESS)
			{
				return;
			}
		}
	}

	BinaryWriter *AL = ActionChat(szText, GetName(), m_dwGUID);
	g_pWorld->BroadcastPVS(GetLandcell(), AL->GetData(), AL->GetSize(), PRIVATE_MSG);
	delete AL;
}

/*
DWORD CPhysicsObj::GetDescFlags()
{
	DWORD dwFlags = 0;

	if (IsInscribable())
		dwFlags |= BF_INSCRIBABLE;
	if (!CanPickup())
		dwFlags |= BF_STUCK;
	if (IsPlayer())
		dwFlags |= BF_PLAYER;
	if (IsAttackable())
		dwFlags |= BF_ATTACKABLE;
	if (IsLifestone())
		dwFlags |= BF_LIFESTONE;
	if (IsPlayer())
		dwFlags |= BF_PLAYER_KILLER; // Player Killer

	return dwFlags;
}
*/

DWORD CPhysicsObj::GetObjectStat(eObjectStat index)
{
	if (index >= 0x100)
		return NULL;

	return m_dwStats[index];
}

DWORD CPhysicsObj::SetObjectStat(eObjectStat index, DWORD value)
{
	DWORD oldvalue;

	if (index >= 0x100)
		return NULL;

	oldvalue = m_dwStats[index];
	m_dwStats[index] = value;
	return oldvalue;
}



