
#include "StdAfx.h"
#include "World.h"
#include "GameMode.h"
#include "Player.h"
#include "PhysicsObj.h"
#include "ChatMsgs.h"

CGameMode::CGameMode()
{
}

CGameMode::~CGameMode()
{
}

CGameMode_Tag::CGameMode_Tag()
{
	m_pSelectedPlayer = NULL;
}

CGameMode_Tag::~CGameMode_Tag()
{
	UnselectPlayer();
}

const char *CGameMode_Tag::GetName()
{
	return "Tag";
}

void CGameMode_Tag::Think()
{
	if (!m_pSelectedPlayer)
	{
		// Find a player to make "it."
		PlayerMap *pPlayers = g_pWorld->GetPlayers();

		if (pPlayers->size() < 2)
		{
			return;
		}

		int index = RandomLong(0, pPlayers->size() - 1);

		CBasePlayer *pSelected = NULL;
		int i = 0;

		for (auto& player : *pPlayers)
		{
			if (i == index)
			{
				pSelected = player.second;
				break;
			}

			i++;
		}

		SelectPlayer(pSelected);
	}
}

void CGameMode_Tag::SelectPlayer(CBasePlayer *pPlayer)
{
	if (!pPlayer)
	{
		UnselectPlayer();
		return;
	}

	m_pSelectedPlayer = pPlayer;

	ModelInfo appearance;
	appearance.dwBasePalette = 0x7E;
	appearance.lPalettes.push_back(PaletteRpl(0x1705, 0, 0));
	m_pSelectedPlayer->SetAppearanceOverride(&appearance);

	m_pSelectedPlayer->EmitEffect(32, 1.0f);
	g_pWorld->BroadcastGlobal(ServerText(csprintf("%s is it!", m_pSelectedPlayer->m_strName.c_str()), 1), PRIVATE_MSG);
}

void CGameMode_Tag::UnselectPlayer()
{
	if (!m_pSelectedPlayer)
	{
		return;
	}

	m_pSelectedPlayer->SetAppearanceOverride(NULL);
}

void CGameMode_Tag::OnTargetAttacked(CPhysicsObj *pTarget, CPhysicsObj *pSource)
{
	if (pSource == m_pSelectedPlayer)
	{
		if (pTarget->IsPlayer())
		{
			CBasePlayer *pTargetPlayer = (CBasePlayer *)pTarget;

			UnselectPlayer();
			SelectPlayer(pTargetPlayer);
		}
	}
}

void CGameMode_Tag::OnRemoveEntity(CPhysicsObj *pEntity)
{
	if (pEntity)
	{
		if (pEntity == m_pSelectedPlayer)
		{
			UnselectPlayer();
			m_pSelectedPlayer = NULL;
		}
	}
}