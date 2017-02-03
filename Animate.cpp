
#include "StdAfx.h"
#include "PhysicsObj.h"
#include "World.h"
#include "AnimationPackage.h"

AnimationPackage::AnimationPackage(WORD wStance, WORD wIndex, float fSpeed)
{
	m_wStance = wStance;
	m_wIndex = wIndex;
	m_fSpeed = fSpeed;

	m_dwTarget = 0;
	m_dwAction = 0;
}

bool AnimationPackage::Initialize()
{
#if _TODO // Should be looking up the animation/s from the set.
#else
	m_fStartTime = g_pGlobals->Time();
	m_dwCurrentFrame = 0x80000000; //Magic number for "null frame"
	m_dwStartFrame = 0;
	m_dwEndFrame = -1;

	if (!m_fSpeed)
		m_fSpeed = 30.0f;
#endif

	return true;
}

SequencedAnimation::SequencedAnimation(WORD wSequence, WORD wStance, WORD wIndex, float fSpeed)
	: AnimationPackage(wStance, wIndex, fSpeed)
{
	m_wSequence = wSequence;
}

void CPhysicsObj::Animation_Init()
{
	m_dwAnimationSet = 0;

	m_bAnimUpdate = FALSE;

	m_wStance = 0x3D;
	m_wForwardAnim = 0;
	m_wStrafAnim = 0;
	m_wTurnAnim = 0;
	m_fForwardSpeed = 0.0f;
	m_fStrafSpeed = 0.0f;
	m_fTurnSpeed = 0.0f;
	m_fSpeedMod = 1.0f;

	m_wAnimSequence = 0;
	m_bForced = FALSE;
}

void CPhysicsObj::Animation_Shutdown()
{
}

void CPhysicsObj::Animation_PlayEmote(WORD wIndex, float fSpeed)
{
	animation_t anim;
	anim.bType = 0;
	anim.wIndex = wIndex;
	anim.wSequence = 0x8000 | (++m_wAnimSequence);
	anim.fSpeed = fSpeed;

	anim.dwAction = ANIM_EMOTE;
	anim.dwTarget = 0;

	//TODO: Calculate animation length.
	anim.fStartTime = g_pGlobals->Time();
	anim.fEndTime = g_pGlobals->Time() + 1.0;//flTime;

	m_lAnimQueue.push_back(anim);

	m_wAnimCount += 2;

	m_bAnimUpdate = TRUE;
	m_bForced = FALSE;
	//Animation_Update();
}

void CPhysicsObj::Animation_ClipMotions(int iMaxCount)
{
	int iIndex = 0;

	for (animation_list::iterator it = m_lAnimQueue.begin(); it != m_lAnimQueue.end();)
	{
		if (iIndex >= iMaxCount)
		{
			it = m_lAnimQueue.erase(it);
		}
		else
		{
			it++;
			iIndex++;
		}
	}
}

BOOL CPhysicsObj::Animation_IsActive()
{
	return (m_lAnimQueue.size()) ? TRUE : FALSE;
}

void CPhysicsObj::Animation_PlayPrimary(WORD wAnim, float fSpeed, float fDelay)
{
	m_wAnimCount++;

	//This is only for primary, forced animations
	m_wForwardAnim = wAnim;
	m_fForwardSpeed = fSpeed;

	m_bForced = TRUE;
	m_bAnimUpdate = TRUE;

	Animation_Update();
}

void CPhysicsObj::Animation_Jump(float fPower, float fDir1, float fDir2, float fHeight)
{
	/*
	CL: F61B:
	56 89 E8 3D - power
	00 00 00 00 00 00 00 00 52 A0 27 40 - dir1, dir2, height?
	28 02 63 03 - landcell
	96 5E 56 42 D3 34 44 C2 F0 D6 A3 3B - x, y, z
	9F 57 1E BF 00 00 00 00 00 00 00 00 EE 27 49 BF - angles x y z w
	27 00 2F 00 00 00 00 00
	SV: F74E
	AF 78 0C 50 - player guid
	00 00 00 00 00 00 00 00 52 A0 27 40 - ..? ..? height?
	00 00 00 00 00 00 00 00 00 00 00 00 - blah blah blah
	27 00 15 00
	*/

	if (HasOwner())
		return;

	BinaryWriter JumpMove;
	JumpMove.WriteDWORD(0xF74E);
	JumpMove.WriteDWORD(m_dwGUID);
	JumpMove.WriteFloat(fDir1);
	JumpMove.WriteFloat(fDir2);
	JumpMove.WriteFloat(fHeight);
	JumpMove.WriteDWORD(0);
	JumpMove.WriteDWORD(0);
	JumpMove.WriteDWORD(0);
	JumpMove.WriteWORD(m_wInstance);
	JumpMove.WriteWORD(++m_wNumJumps);

	g_pWorld->BroadcastPVS(GetLandcell(), JumpMove.GetData(), JumpMove.GetSize(), OBJECT_MSG);
}

void CPhysicsObj::Animation_SetCombatStance(WORD wStance)
{
	if (m_wStance != wStance)
	{
		m_wAnimCount++;

		m_wStance = wStance;
		m_bForced = TRUE;

		m_wForwardAnim = 0;
		m_wStrafAnim = 0;
		m_wTurnAnim = 0;
		m_fForwardSpeed = 0.0f;
		m_fStrafSpeed = 0.0f;
		m_fTurnSpeed = 0.0f;

		Animation_Update();
	}
}

void CPhysicsObj::Animation_SetCombatMode(WORD wMode)
{
	// 0x003D = peace
	// 0x0049 = spellcasting
	// 0x00E9 = weird weapon stance, like crossbow held down [unaimed].

	if (wMode == 0x01)
		Animation_SetCombatStance(0x003D);
	else if (wMode == 0x02)
		Animation_SetCombatStance(0x003C);
	else if (wMode == 0x04)
		Animation_SetCombatStance(0x003F);
	else if (wMode == 0x08)
		Animation_SetCombatStance(0x0049);
}

void CPhysicsObj::Animation_Attack(DWORD dwTarget, WORD wAnim, float fSpeed, DWORD dwWeaponID)
{
	animation_t anim;
	anim.bType = 0;
	anim.wIndex = wAnim;
	anim.fSpeed = fSpeed;
	anim.dwTarget = dwTarget;
	anim.dwAction = ANIM_ATTACK;
	anim.dwActionData[0] = dwWeaponID; //Used for damage calculations after animation.

	//TODO: Calculate animation length.
	anim.fStartTime = g_pGlobals->Time();
	anim.fEndTime = g_pGlobals->Time() + (fSpeed ? (2.0 / fSpeed) : 0);

	Animation_PlayAnimation(&anim);
}

void CPhysicsObj::Animation_PlaySimpleAnimation(WORD wIndex, float fSpeed, float fDelay, DWORD dwAction, DWORD dwActionEx)
{
	if (!fSpeed)
		fSpeed = 1.0f;

	animation_t anim;
	anim.bType = 0;
	anim.wIndex = wIndex;
	anim.fSpeed = fSpeed;
	anim.dwTarget = 0;
	anim.dwAction = dwAction;
	anim.dwActionData[0] = dwActionEx;

	//TODO: Calculate animation length.
	anim.fStartTime = g_pGlobals->Time();
	anim.fEndTime = g_pGlobals->Time() + (fDelay / fSpeed);

	Animation_PlayAnimation(&anim);
}

void CPhysicsObj::Animation_PlayAnimation(animation_t* data)
{
	animation_t anim;
	anim.bType = data->bType;
	anim.wIndex = data->wIndex;
	anim.wSequence = ++m_wAnimSequence;
	anim.fSpeed = data->fSpeed;

	if (anim.dwAction = data->dwAction)
		memcpy(anim.dwActionData, data->dwActionData, sizeof(DWORD) * 10);

	anim.dwTarget = data->dwTarget;

	//TODO: Calculation animation length.
	if (!data->fStartTime || !data->fEndTime) {
		anim.fStartTime = g_pGlobals->Time();
		anim.fEndTime = g_pGlobals->Time() + 1.0f;//flTime;
	}
	else {
		anim.fStartTime = data->fStartTime;
		anim.fEndTime = data->fEndTime;
	}

	m_lAnimQueue.push_back(anim);

	m_wAnimCount += 2;

	m_bAnimUpdate = TRUE;
	m_bForced = TRUE;

	Animation_Update();
}

BinaryWriter* CPhysicsObj::Animation_GetAnimationInfo()
{
	BYTE	bType;
	DWORD	dwTarget;

	BinaryWriter* AnimInfo = new BinaryWriter;

	if (m_lAnimQueue.empty())
	{
		bType = 0;
		dwTarget = 0;
	}
	else
	{
		animation_t anim = m_lAnimQueue.front();

		bType = anim.bType;
		dwTarget = anim.dwTarget;
	}

	AnimInfo->WriteBYTE(bType);
	AnimInfo->WriteBYTE(dwTarget ? TRUE : FALSE);
	AnimInfo->WriteWORD(m_wStance);

	switch (bType)
	{
	case 255:
	case 0:
		BinaryWriter *Desc = new BinaryWriter;
		DWORD dwFlags = 0;

		if (m_wStance != 0x3D)
		{
			dwFlags |= 1;
			Desc->WriteWORD(m_wStance);
		}
#define CONDITIONAL_WORD(cond, flags, value) if (cond) { dwFlags |= flags; Desc->WriteWORD(value); }
#define CONDITIONAL_FLOAT(cond, flags, value) if (cond) { dwFlags |= flags; Desc->WriteFloat(value); }
		CONDITIONAL_WORD(m_wForwardAnim, 0x00000002, m_wForwardAnim);
		CONDITIONAL_WORD(m_wStrafAnim, 0x00000008, m_wStrafAnim);
		CONDITIONAL_WORD(m_wTurnAnim, 0x00000020, m_wTurnAnim);
		CONDITIONAL_FLOAT(m_wForwardAnim && (m_fForwardSpeed != 1.0f), 0x00000004, m_fForwardSpeed);
		CONDITIONAL_FLOAT(m_wStrafAnim && (m_fStrafSpeed != 1.0f), 0x00000010, m_fStrafSpeed);
		CONDITIONAL_FLOAT(m_wTurnAnim && (m_fTurnSpeed != 1.0f), 0x00000040, m_fTurnSpeed);

		if (!m_lAnimQueue.empty())
		{
			DWORD dwQueueCount = 0;

			for (animation_list::iterator it = m_lAnimQueue.begin(); it != m_lAnimQueue.end(); it++)
			{
				if (it->bType == 0)
				{
					Desc->WriteWORD(it->wIndex);
					Desc->WriteWORD(it->wSequence);
					Desc->WriteFloat(it->fSpeed);
					dwQueueCount++;

					if (dwQueueCount == MAX_MOTION_QUEUE)
						break;
				}
				else
					break;
			}
			dwFlags |= dwQueueCount << 7;
		}

		//** 0x026DF000 = turbine got high **
		AnimInfo->WriteDWORD(0x026DF000 | dwFlags);
		AnimInfo->AppendData(Desc->GetData(), Desc->GetSize());
		AnimInfo->Align();
		delete Desc;
		break;
	}

	if (dwTarget)
		AnimInfo->WriteDWORD(dwTarget);

	return AnimInfo;
}

void CPhysicsObj::Animation_Complete(animation_t *data)
{
	Animation_Update();
}

void CPhysicsObj::Animation_Think()
{
	if (!m_lAnimQueue.empty())
	{
		for (animation_list::iterator it = m_lAnimQueue.begin(); it != m_lAnimQueue.end();)
		{
			if (it->fEndTime <= g_pGlobals->Time())
			{
				Animation_Complete(&(*it));
				it = m_lAnimQueue.erase(it);
			}
			else
				it++;
		}
	}

	if (m_bAnimUpdate)
		Animation_Update();
}

void CPhysicsObj::Animation_Update()
{
	m_bAnimUpdate = FALSE;

	if (HasOwner())
		return;

	BinaryWriter AnimUpdate;
	AnimUpdate.WriteDWORD(0xF74C);
	AnimUpdate.WriteDWORD(m_dwGUID);
	AnimUpdate.WriteWORD(m_wInstance);
	AnimUpdate.WriteWORD(++m_wNumAnimInteracts);
	AnimUpdate.WriteWORD(m_wAnimCount);

	AnimUpdate.WriteWORD((m_bForced) ? FALSE : TRUE);
	m_bForced = FALSE;

	BinaryWriter *AnimInfo = Animation_GetAnimationInfo();
	AnimUpdate.AppendData(AnimInfo->GetData(), AnimInfo->GetSize());
	delete AnimInfo;

	g_pWorld->BroadcastPVS(GetLandcell(), AnimUpdate.GetData(), AnimUpdate.GetSize(), OBJECT_MSG);

}

int CPhysicsObj::Animation_EmoteQueueCount()
{
	int iCount = 0;

	for (animation_list::iterator i = m_lAnimQueue.begin(); i != m_lAnimQueue.end(); i++)
	{
		if (i->dwAction == ANIM_EMOTE)
			iCount++;
	}

	return iCount;
}
