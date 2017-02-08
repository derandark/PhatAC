
#include "StdAfx.h"

#include "PhysicsObj.h"
#include "Door.h"

#define DOOR_ANIMSET		0x09000023
#define DOOR_OPEN			0x0B
#define DOOR_CLOSED			0x0C

CBaseDoor::CBaseDoor()
{
	// Door properties should remain overridable.
	m_bOpen = FALSE;
	m_wOpenAnim = DOOR_OPEN;
	m_wCloseAnim = DOOR_CLOSED;

	m_WeenieBitfield |= BF_DOOR;

	m_strName = "Door";
	m_wTypeID = 0x19C;
	m_wIcon = 0x1317;

	m_dwModel = 0x0200027C; //0x19FF
	m_dwSoundSet = 0x20000023;
	m_dwEffectSet = 0x3400002B;
	m_fScale = 1.0f;

	m_PhysicsState = HAS_PHYSICS_BSP_PS | PhysicsState::REPORT_COLLISIONS_PS;	//who the hell knows, collision related.
	m_ItemType = TYPE_MISC;
	m_Usability = USEABLE_REMOTE;
	m_UseDistance = 5.0f;

	m_dwAnimationSet = DOOR_ANIMSET;
	m_wForwardAnim = m_wCloseAnim;
	m_fForwardSpeed = 1.0f;
	m_fSpeedMod = 1.0f;
}

CBaseDoor::~CBaseDoor()
{
}

void CBaseDoor::Precache()
{
	//Load entity settings (location, and dynamic properties.)

	/*
	m_Origin.landcell = 0xA5B4003C;
	m_Origin.x = 174.3589935f;
	m_Origin.y = 77.19989777f;
	m_Origin.z = 46.08199692f;
	m_Angles.x = 1.0f;
	m_Angles.y = 0.0f;
	m_Angles.z = 0.0f;
	m_Angles.w = 0.0f;
	*/
}

BOOL CBaseDoor::CloseThink()
{
	if (m_bOpen)
		CloseDoor();

	return FALSE;
}

void CBaseDoor::OpenDoor()
{
	Animation_PlayPrimary(m_wOpenAnim, 1.0f, 2.0f);
	m_bOpen = TRUE;

	SetThink(&CBaseDoor::CloseThink);
	m_fNextThink = g_pGlobals->Time() + 30.0f; //Close door after 30 seconds.
}

void CBaseDoor::CloseDoor()
{
	Animation_PlayPrimary(m_wCloseAnim, 1.0f, 2.0f);
	m_bOpen = FALSE;

	HaltThink();
}

void CBaseDoor::Use(CPhysicsObj *pOther)
{
	// should check approach distance in a parent toggle class
	if (Animation_IsActive())
		return;

	if ((Vector(pOther->m_Origin) - Vector(m_Origin)).Length() < m_UseDistance)
	{
		if (!m_bOpen)
			OpenDoor();
		else
			CloseDoor();
	}
}
