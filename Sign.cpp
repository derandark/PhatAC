
#include "StdAfx.h"

#include "PhysicsObj.h"
#include "sign.h"

//To send server text.
#include "BinaryWriter.h"
#include "ChatMsgs.h"

//This is getting a bit redundant.
#include "Monster.h"
#include "Player.h"

CBaseSign::CBaseSign()
{
	m_dwModel = 0x02000610;
	m_fScale = 1.0f;

	m_PhysicsState = PhysicsState::GRAVITY_PS | PhysicsState::IGNORE_COLLISIONS_PS | PhysicsState::REPORT_COLLISIONS_PS;

	m_strName = "Sign";
	m_wTypeID = 0x1139;
	m_wIcon = 0x12D3;

	m_ItemType = TYPE_MISC;
}

void CBaseSign::Precache()
{
	//Load entity settings (location, and dynamic properties.)
}

void CBaseSign::Use(CPhysicsObj *pEntity)
{
	if (pEntity->IsPlayer())
	{
		BinaryWriter *ST = ServerText("You clicked a sign, but the code isn't done yet .. so too bad!", 1);
		((CBasePlayer *)pEntity)->SendMessage(ST->GetData(), ST->GetSize(), PRIVATE_MSG);
		delete ST;
	}
}

