
#include "StdAfx.h"
#include "PhysicsObj.h"
#include "ObjectMsgs.h"
#include "World.h"

#include "BinaryWriter.h"
#include "ChatMsgs.h"

#include "Item.h"

CPhysicsObj::CPhysicsObj()
{
	m_dwGUID = 0;
	m_strName = "Unknown";

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
	m_UseDistance = -0.1f;
	m_RadarVis = ShowAlways_RadarEnum;

	m_AnimOverrideData = NULL;
	m_AnimOverrideDataLen = 0;
	m_AutonomousMovement = 0;

	Movement_Init();
	Animation_Init();
	Container_Init();
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
		((CBasePlayer *)pSource)->SendMessage(IdentifyObject(this), PRIVATE_MSG, TRUE, TRUE);
	}
}

BinaryWriter* CPhysicsObj::CreateMessage()
{
	return CreateObject(this);
}

void CPhysicsObj::RemovePreviousInstance()
{
	DWORD RPI[3];

	RPI[0] = 0xF747;
	RPI[1] = m_dwGUID;
	RPI[2] = m_wInstance - 1;

	g_pWorld->BroadcastPVS(GetLandcell(), RPI, sizeof(RPI), 10/*OBJECT_MSG*/, 0, FALSE);
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

BinaryWriter* CPhysicsObj::GetModelData()
{
	ModelInfo miCurrentModel;// = m_miBaseModel;

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

		if (pItem->IsArmor())
			miCurrentModel.MergeData(((CBaseArmor *)pItem)->GetArmorModel(), NULL);
	}

	return miCurrentModel.NetData();
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

	BinaryWriter *LC = LocalChat(szText, GetName(), m_dwGUID, lColor);
	g_pWorld->BroadcastPVS(GetLandcell(), LC->GetData(), LC->GetSize(), PRIVATE_MSG);
	delete LC;
}

void CPhysicsObj::EmoteLocal(const char* szText)
{
	if (HasOwner())
		return;

	BinaryWriter *EL = EmoteChat(szText, GetName(), m_dwGUID);
	g_pWorld->BroadcastPVS(GetLandcell(), EL->GetData(), EL->GetSize(), PRIVATE_MSG);
	delete EL;
}

void CPhysicsObj::ActionLocal(const char* szText)
{
	if (HasOwner())
		return;

	BinaryWriter *AL = ActionChat(szText, GetName(), m_dwGUID);
	g_pWorld->BroadcastPVS(GetLandcell(), AL->GetData(), AL->GetSize(), PRIVATE_MSG);
	delete AL;
}

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

	if (IsPlayer())
		dwFlags |= BF_PLAYER_KILLER; // Player Killer

	return dwFlags;
}

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



