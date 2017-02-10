
#include "StdAfx.h"
#include "PhysicsObj.h"
#include "World.h"

#include "ObjectMsgs.h"

void CPhysicsObj::Movement_Init()
{
	m_fMoveThink = g_pGlobals->Time();
}

void CPhysicsObj::Movement_Shutdown()
{
}

void CPhysicsObj::Movement_Think()
{
	if (HasOwner())
		return;

	if ((m_fMoveThink + 0.05f) < g_pGlobals->Time())
	{
		if (m_lastMoveOrigin.landcell != m_Origin.landcell ||
			m_lastMoveOrigin.x != m_Origin.x ||
			m_lastMoveOrigin.y != m_Origin.y ||
			m_lastMoveOrigin.z != m_Origin.z)
		{
			Movement_UpdatePos();
		}
		else if (m_lastMoveAngles.w != m_Angles.w ||
			m_lastMoveAngles.x != m_Angles.x ||
			m_lastMoveAngles.y != m_Angles.y ||
			m_lastMoveAngles.z != m_Angles.z)
		{
			Movement_UpdatePos();
		}

		m_fMoveThink = g_pGlobals->Time();
	}
}

void CPhysicsObj::Movement_Teleport(loc_t origin, heading_t angles)
{
	m_wNumPortals++;

	m_Origin = origin;
	m_Angles = angles;

	if (IsPlayer())
		((CBasePlayer *)this)->EnterPortal();

	Movement_UpdatePos();
}

void CPhysicsObj::Movement_SendUpdate(DWORD dwCell)
{
	//According to my reading of the ASM
	//The flags (which are only processed as 1 BYTE) work as follows:
	//0x08 0x10 0x20 and 0x40 are X Y Z W for angles, respectively
	//if flagged it means to default to zero, otherwise the message contains the value
	//0x01
	//reads in 3 floats for unknown purpose, these come if you jump.. i know that much
	//0x02
	//reads in float for unknown purpose
	//0x04
	//sets some sort of boolean relating to the update
	//----

	BinaryWriter* poo = MoveUpdate(this);

	g_pWorld->BroadcastPVS(dwCell, poo->GetData(), poo->GetSize());

	delete poo;
}

void CPhysicsObj::Movement_UpdatePos()
{
	if (HasOwner())
		return;

	//QUICKFIX: Broadcast to the old landblock that we've moved from.
	//This sends duplicates if the block is near the other.

	m_wNumMovements++;

	DWORD dwNewCell = GetLandcell();
	DWORD dwOldCell = m_lastMoveOrigin.landcell;

	if (BLOCK_WORD(dwOldCell) != BLOCK_WORD(dwNewCell))
	{
		Movement_SendUpdate(dwOldCell);
	}

	Movement_SendUpdate(dwNewCell);

	m_lastMoveOrigin = m_Origin;
	m_lastMoveAngles = m_Angles;

}
