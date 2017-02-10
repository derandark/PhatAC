
#include "StdAfx.h"
#include "PhysicsObj.h"
#include "Lifestone.h"

CBaseLifestone::CBaseLifestone()
{
	m_wTypeID = 0x1FD;
	m_wIcon = 0x1355;
	m_ItemType = TYPE_LIFESTONE;
	m_strName = "Life Stone";
	m_WeenieBitfield |= BF_LIFESTONE;

	m_dwModel = 0x020002EE;
	m_dwSoundSet = 0x20000014;
	m_dwAnimationSet = 0x09000026;
	m_fScale = 1.0f;

	m_PhysicsState = PhysicsState::GRAVITY_PS | PhysicsState::IGNORE_COLLISIONS_PS;
	m_Usability = USEABLE_REMOTE;
	m_UseDistance = LIFESTONE_MAX_BIND_DISTANCE;

	m_RadarVis = ShowAlways_RadarEnum;
	m_BlipColor = 1;
}

CBaseLifestone::~CBaseLifestone()
{
}

void CBaseLifestone::Precache()
{
	// Load entity settings (location, and dynamic properties.)
}

void CBaseLifestone::Use(CPhysicsObj *pOther)
{
	// should check approach distance in a parent toggle class
	if (Animation_IsActive())
		return;

	if (pOther->IsPlayer())
	{
		if ((Vector(pOther->m_Origin) - Vector(m_Origin)).Length() < m_UseDistance)
		{			
			pOther->Animation_PlaySimpleAnimation(87, 1.0f, 5.0f, ANIM_BINDLIFESTONE, m_dwGUID);
		}
	}
}
