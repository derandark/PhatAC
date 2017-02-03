
#include "StdAfx.h"

#include "Client.h"
#include "ClientCommands.h"
#include "ClientEvents.h"
#include "World.h"

// Database access
#include "Database.h"
#include "CharacterDatabase.h"

// Network helpers
#include "BinaryWriter.h"
#include "ChatMsgs.h"
#include "ObjectMsgs.h"

// Objects
#include "PhysicsObj.h"
#include "Item.h"
#include "Monster.h"
#include "Player.h"
#include "Door.h"

CClientEvents::CClientEvents(CClient *parent)
{
	m_pClient = parent;

	m_pPlayer = NULL;
	m_fLogout = 0;
}

CClientEvents::~CClientEvents()
{
	if (m_pPlayer)
	{
		g_pWorld->RemoveEntity(m_pPlayer);
	}
}

void CClientEvents::DetachPlayer()
{
	m_pPlayer = NULL;
}

DWORD CClientEvents::GetPlayerID()
{
	if (!m_pPlayer)
		return NULL;

	return m_pPlayer->m_dwGUID;
}

CBasePlayer* CClientEvents::GetPlayer()
{
	return m_pPlayer;
}

void CClientEvents::ExitWorld()
{
	if (m_pPlayer)
	{
		g_pWorld->RemoveEntity(m_pPlayer);
		DetachPlayer();
	}

	m_pClient->ExitWorld();
	m_fLogout = 0;
}

void CClientEvents::Think()
{
	if (m_fLogout && ((m_fLogout + 5.00f) < g_pGlobals->Time()))
	{
		ExitWorld();
	}
}

void CClientEvents::BeginLogout()
{
	if (m_pPlayer && !m_fLogout)
	{
		m_pPlayer->Animation_ClipMotions(0);
		m_pPlayer->Animation_PlayPrimary(0x11B, 1.0f, 5.0f);

		m_fLogout = g_pGlobals->Time();
	}
}

void CClientEvents::LoginError(int iError)
{
	DWORD ErrorPackage[2];

	ErrorPackage[0] = 0xF659;
	ErrorPackage[1] = iError;

	m_pClient->SendMessage(ErrorPackage, sizeof(ErrorPackage), PRIVATE_MSG);
}

void CClientEvents::LoginCharacter(DWORD dwGUID, const char *szAccount)
{
	_CHARDESC desc;

	if (m_pPlayer)
	{
		// LOG(Temp, Normal, "Character already logged in!\n");
		LoginError(13);
		LOG(Client, Warning, "Login request, but character already logged in!\n");
		return;
	}

	if (!g_pDB->CharDB()->GetCharacterDesc(dwGUID, &desc))
		return;

	if (stricmp(szAccount, desc.szAccount))
	{
		LoginError(15);
		LOG(Client, Warning, "Bad account for login: \"%s\" \"%s\"\n", szAccount, desc.szAccount);
		return;
	}

	m_pPlayer = new CBasePlayer(m_pClient, dwGUID);
	g_pWorld->CreateEntity(m_pPlayer);
	m_pPlayer->LoginCharacter();

	return;
}

void CClientEvents::SendText(const char *szText, long lColor)
{
	m_pClient->SendMessage(ServerText(szText, lColor), PRIVATE_MSG, FALSE, TRUE);
}

void CClientEvents::UpdateBurdenUI()
{
}

void CClientEvents::Attack(DWORD dwTarget, DWORD dwHeight, float flPower)
{
	m_pPlayer->Attack(dwTarget, dwHeight, flPower);
}

void CClientEvents::SendTellByGUID(const char* szText, DWORD dwGUID)
{
	if (strlen(szText) > 180)
		return;

	//should really check for invalid characters and such ;]

	while (szText[0] == ' ') //Skip leading spaces.
		szText++;
	if (szText[0] == '\0') //Make sure the text isn't blank
		return;

	if (dwGUID == m_pPlayer->m_dwGUID)
	{
		m_pPlayer->SendMessage(ServerText("You really need some new friends..", 1), PRIVATE_MSG, FALSE);
		return;
	}

	CBasePlayer *pTarget;

	if (!(pTarget = g_pWorld->FindPlayer(dwGUID)))
		return;

	char szResponse[300];
	_snprintf(szResponse, 300, "You tell %s, \"%s\"", pTarget->GetName(), szText);
	m_pPlayer->SendMessage(ServerText(szResponse, 4), PRIVATE_MSG, FALSE);

	pTarget->SendMessage(DirectChat(szText, m_pPlayer->GetName(), m_pPlayer->m_dwGUID, dwGUID, 3), PRIVATE_MSG, TRUE);
}

void CClientEvents::SendTellByName(const char* szText, const char* szName)
{
	if (strlen(szName) > 180)
		return;
	if (strlen(szText) > 180)
		return;

	//should really check for invalid characters and such ;]

	while (szText[0] == ' ') //Skip leading spaces.
		szText++;
	if (szText[0] == '\0') //Make sure the text isn't blank
		return;

	CBasePlayer *pTarget;

	if (!(pTarget = g_pWorld->FindPlayer(szName)))
		return;

	if (pTarget->m_dwGUID == m_pPlayer->m_dwGUID)
	{
		m_pPlayer->SendMessage(ServerText("You really need some new friends..", 1), PRIVATE_MSG, FALSE);
		return;
	}

	char szResponse[300];
	_snprintf(szResponse, 300, "You tell %s, \"%s\"", pTarget->GetName(), szText);
	m_pPlayer->SendMessage(ServerText(szResponse, 4), PRIVATE_MSG, FALSE, TRUE);

	pTarget->SendMessage(DirectChat(szText, m_pPlayer->GetName(), m_pPlayer->m_dwGUID, pTarget->m_dwGUID, 3), PRIVATE_MSG, TRUE);
}

void CClientEvents::ClientText(char* szText)
{
	if (strlen(szText) > 1000)
		return;

	//should really check for invalid characters and such ;]

	while (szText[0] == ' ') //Skip leading spaces.
		szText++;
	if (szText[0] == '\0') //Make sure the text isn't blank
		return;

	if (szText[0] == '!' || szText[0] == '@' || szText[0] == '/')
	{
		CommandBase::Execute(++szText, m_pPlayer, m_pClient->GetAccessLevel());
	}
	else
		m_pPlayer->SpeakLocal(szText);
}

void CClientEvents::EmoteText(char* szText)
{
	if (strlen(szText) > 180)
		return;

	//TODO: Check for invalid characters and such ;)

	while (szText[0] == ' ') //Skip leading spaces.
		szText++;
	if (szText[0] == '\0') //Make sure the text isn't blank
		return;

	m_pPlayer->EmoteLocal(szText);
}

void CClientEvents::ActionText(char* szText)
{
	if (strlen(szText) > 180)
		return;

	//TODO: Check for invalid characters and such ;)

	while (szText[0] == ' ') //Skip leading spaces.
		szText++;
	if (szText[0] == '\0') //Make sure the text isn't blank
		return;

	m_pPlayer->ActionLocal(szText);
}

void CClientEvents::ChannelText(DWORD dwChannel, const char* szText)
{
	if (strlen(szText) > 180)
		return;

	//TODO: Check for invalid characters and such ;)
	while (szText[0] == ' ')
		szText++;
	if (szText[0] == '\0')
		return;

	// 0x00000400 = Urgent Assistance "help channel"
	// 0x00000800 = Fellowship
	switch (dwChannel)
	{
	case 0x400:
	case 0x800:
		//For now we'll just spam them to everyone!
		g_pWorld->BroadcastGlobal(ChannelChat(dwChannel, m_pPlayer->GetName(), szText), PRIVATE_MSG, m_pPlayer->m_dwGUID, TRUE);
	}

	//Give a special copy to this player?
	switch (dwChannel)
	{
	case 0x400:
	case 0x800:
		m_pClient->SendMessage(ChannelChat(dwChannel, NULL, szText), PRIVATE_MSG, TRUE);
		break;
	default:
		SendText("The server does not support this channel.", 1);
		break;
	}

	if (dwChannel == 0x800)
	{
		LOG(Client, Normal, "[%s] %s says, \"%s\"\n", timestamp(), m_pPlayer->GetName(), szText);
	}
}

void CClientEvents::RequestHealthUpdate(DWORD dwGUID)
{
	CPhysicsObj *pEntity = g_pWorld->FindWithinPVS(m_pPlayer, dwGUID);

	if (pEntity) {
		if (pEntity->IsMonster())
		{
			m_pClient->SendMessage(HealthUpdate((CBaseMonster *)pEntity), PRIVATE_MSG, TRUE, TRUE);
		}
	}
}

void CClientEvents::EquipItem(DWORD dwItemID, DWORD dwCoverage)
{

	// Scenarios to consider:
	// 1. Item being equipped from the GROUND!
	// 2. Item being equipped from different equip slot.
	// 3. Item being equipped from the player's inventory.

	CPhysicsObj *		pEntity;
	CBaseItem *			pItem;

	//Does the player have an inventory?
	if (!(m_pPlayer->Container_HasContents()))
		return;

	//Find the item.
	if (!(pEntity = g_pWorld->FindWithinPVS(m_pPlayer, dwItemID)))
		return;
	if (!(pEntity->IsItem()))
		return;
	pItem = (CBaseItem *)pEntity;

	//Can this item be equipped?
	if (!(pItem->CanEquip()))
		return;
	if (!(m_pPlayer->Container_CanEquip(pItem, dwCoverage)))
		return;

	//Equip it
	DWORD dwCell = m_pPlayer->GetLandcell();

	//Take it out of whatever slot it may be in.
	if (pItem->HasOwner())
		m_pPlayer->Container_ReleaseItem(pItem, FALSE);

	if (pItem->m_ItemType != TYPE_ARMOR)
		pItem->SetWorldWielder(dwCell, m_pPlayer);
	pItem->SetWorldContainer(dwCell, NULL);
	pItem->SetWorldCoverage(dwCell, dwCoverage);

	//The container will auto-correct this slot into a valid range.
	m_pPlayer->Container_EquipItem(dwCell, pItem, dwCoverage);

	m_pPlayer->EmitSound(0x77, 1.0f);
	m_pClient->SendMessage(InventoryEquip(dwItemID, dwCoverage), PRIVATE_MSG, TRUE);

	UpdateBurdenUI();

	if (pItem->m_ItemType == TYPE_ARMOR)
		m_pPlayer->UpdateModel();
}

void CClientEvents::StoreItem(DWORD dwItemID, DWORD dwContainer, char cSlot)
{
	if (cSlot < 0)
		return;

	// Scenarios to consider:
	// 1. Item being stored is equipped.
	// 2. Item being stored is on the GROUND!
	// 3. Item being stored is already in the inventory.

	CPhysicsObj *		pEntity;
	CPhysicsObj *		pContainer;
	CBaseItem *			pItem;

	//Find the container.
	if (!(m_pPlayer->Container_HasContents()))
		return;
	if (!(pContainer = m_pPlayer->Container_FindContainer(dwContainer)))
		return;

	//Find the item.
	if (!(pEntity = g_pWorld->FindWithinPVS(m_pPlayer, dwItemID)))
		return;
	if (!(pEntity->IsItem()))
		return;

	pItem = (CBaseItem *)pEntity;
	if (!(pItem->CanPickup()))
		return;
	if (!(pContainer->Container_CanStore(pItem)))
		return;
	if (pItem->GetSlotType()) //Only store/move ordinary items for now.
		return;

	DWORD dwCell = m_pPlayer->GetLandcell();

	//Take it out of whatever slot it's in.
	pItem->SetWorldContainer(dwCell, pContainer);
	if (pItem->IsWielded())
		pItem->SetWorldWielder(dwCell, NULL);

	m_pPlayer->Container_ReleaseItem(pItem, FALSE);

	//The container will auto-correct this slot into a valid range.
	cSlot = pContainer->Container_InsertInventoryItem(dwCell, pItem, cSlot);

	m_pPlayer->EmitSound(0x78, 1.0f);
	m_pClient->SendMessage(InventoryMove(dwItemID, dwContainer, cSlot, 0), PRIVATE_MSG, TRUE);

	UpdateBurdenUI();

	if (pItem->m_ItemType == TYPE_ARMOR)
		m_pPlayer->UpdateModel();
}

void CClientEvents::DropItem(DWORD dwItemID)
{
	// Scenarios to consider:
	// 1. Item being stored is equipped.
	// 3. Item being stored is in the inventory.

	CBaseItem *			pItem;

	//Find the container.
	if (!(m_pPlayer->Container_HasContents()))
		return;

	//Find the item.
	if (!(pItem = m_pPlayer->Container_FindItem(dwItemID)))
		return;

	DWORD dwCell = m_pPlayer->GetLandcell();

	//Take it out of whatever slot it's in.
	m_pPlayer->Container_ReleaseItem(pItem, FALSE);

	pItem->SetWorldContainer(dwCell, NULL);
	//if ( pItem->IsWielded() )
	pItem->SetWorldWielder(dwCell, NULL);

	m_pPlayer->EmitSound(0x7B, 1.0f);
	m_pClient->SendMessage(InventoryDrop(dwItemID), PRIVATE_MSG, TRUE);

	g_pWorld->InsertEntity(pItem);

	pItem->Movement_Teleport(m_pPlayer->m_Origin, m_pPlayer->m_Angles);

	UpdateBurdenUI();

	if (pItem->m_ItemType == TYPE_ARMOR)
		m_pPlayer->UpdateModel();
}

void CClientEvents::ChangeCombatStance(DWORD dwStance)
{
	m_pPlayer->Animation_SetCombatMode((WORD)dwStance);

	ActionComplete();
}

void CClientEvents::ExitPortal()
{
	m_pPlayer->ExitPortal();
}

void CClientEvents::Ping()
{
	// Pong!
	DWORD Pong = 0x1EA;
	m_pClient->SendMessage(&Pong, sizeof(Pong), PRIVATE_MSG, TRUE);
}

void CClientEvents::UseItemEx(DWORD dwSourceID, DWORD dwDestID)
{
	CPhysicsObj *pSource = m_pPlayer->FindChild(dwSourceID);
	CPhysicsObj *pDest = m_pPlayer->FindChild(dwDestID);

	if (!pSource)
		pSource = g_pWorld->FindWithinPVS(m_pPlayer, dwSourceID);

	if (!pDest)
		pDest = g_pWorld->FindWithinPVS(m_pPlayer, dwDestID);

	if (pSource && pDest)
		pDest->UseEx(m_pPlayer, pSource);
	else
		ActionComplete();
}

void CClientEvents::UseObject(DWORD dwEID)
{
	CPhysicsObj *pTarget = m_pPlayer->FindChild(dwEID);

	if (!pTarget)
		pTarget = g_pWorld->FindWithinPVS(m_pPlayer, dwEID);

	if (pTarget)
		pTarget->Use(m_pPlayer);
	else
		ActionComplete();
}

void CClientEvents::ActionComplete()
{
	DWORD ActionComplete[2];
	ActionComplete[0] = 0x1C7;
	ActionComplete[1] = 0;

	m_pClient->SendMessage(ActionComplete, sizeof(ActionComplete), PRIVATE_MSG, TRUE);
}

void CClientEvents::Identify(DWORD dwObjectID)
{
	SendText("Identify will be incomplete -- it's not done!", 9);

	CPhysicsObj *pSource = m_pPlayer->FindChild(dwObjectID);

	if (!pSource)
		pSource = g_pWorld->FindWithinPVS(m_pPlayer, dwObjectID);

	if (pSource)
		pSource->Identify(m_pPlayer);

	m_pPlayer->SetLastAssessedItem(pSource);

	ActionComplete();
}

void CClientEvents::SpendAttributeXP(DWORD dwAttribute, DWORD dwXP)
{
	if (dwAttribute < 1 || dwAttribute > 6)
		return;

	DWORD dwUnassignedXP = m_pPlayer->GetObjectStat(eUnassignedXP);
	if (dwUnassignedXP < dwXP)
		return;

	m_pPlayer->GiveAttributeXP((eAttribute)dwAttribute, dwXP);
	m_pPlayer->SetObjectStat(eUnassignedXP, dwUnassignedXP - dwXP);
}

void CClientEvents::SpendVitalXP(DWORD dwVital, DWORD dwXP)
{
	if (dwVital != 1 && dwVital != 3 && dwVital != 5)
		return;

	DWORD dwUnassignedXP = m_pPlayer->GetObjectStat(eUnassignedXP);
	if (dwUnassignedXP < dwXP)
		return;

	m_pPlayer->GiveVitalXP((eVital)dwVital, dwXP);
	m_pPlayer->SetObjectStat(eUnassignedXP, dwUnassignedXP - dwXP);
}

void CClientEvents::SpendSkillXP(DWORD dwSkill, DWORD dwXP)
{
	if (dwSkill < 1 || dwSkill > 0x27)
		return;

	DWORD dwUnassignedXP = m_pPlayer->GetObjectStat(eUnassignedXP);
	if (dwUnassignedXP < dwXP)
		return;

	m_pPlayer->GiveSkillXP((eSkill)dwSkill, dwXP);
	m_pPlayer->SetObjectStat(eUnassignedXP, dwUnassignedXP - dwXP);
}

void CClientEvents::LifestoneRecall()
{
	//150 or 163?
	m_pPlayer->Animation_PlaySimpleAnimation(0x163, 1.0f, 18.0f, ANIM_LSRECALL);
}

void CClientEvents::MarketplaceRecall()
{
	//150 or 163?
	m_pPlayer->Animation_PlaySimpleAnimation(0x163, 1.0f, 18.0f, ANIM_MPRECALL);
}

// This is it!
void CClientEvents::ProcessEvent(BinaryReader *in)
{
	if (!m_pPlayer)	return;

	DWORD dwSequence = in->ReadDWORD();
	DWORD dwEvent = in->ReadDWORD();
	if (in->GetLastError()) return;

	switch (dwEvent)
	{
		case 0x0008: //Attack
		{
			DWORD dwTarget = in->ReadDWORD();
			DWORD dwHeight = in->ReadDWORD();
			float flPower = in->ReadFloat();
			if (in->GetLastError()) break;

			Attack(dwTarget, dwHeight, flPower);
			break;
		}
		case 0x0015: //Client Text
		{
			char *szText = in->ReadString();
			if (in->GetLastError()) break;

			ClientText(szText);
			break;
		}
		case 0x0019: //Store Item
		{
			DWORD dwItemID = in->ReadDWORD();
			DWORD dwContainer = in->ReadDWORD();
			DWORD dwSlot = in->ReadDWORD();
			if (in->GetLastError()) break;

			StoreItem(dwItemID, dwContainer, (char)dwSlot);
			break;
		}
		case 0x001A: //Equip Item
		{
			DWORD dwItemID = in->ReadDWORD();
			DWORD dwCoverage = in->ReadDWORD();
			if (in->GetLastError()) break;

			EquipItem(dwItemID, dwCoverage);
			break;
		}
		case 0x001B: //Drop Item
		{
			DWORD dwItemID = in->ReadDWORD();
			if (in->GetLastError()) break;

			DropItem(dwItemID);
			break;
		}
		case 0x001F: // TOD 2017
		{
			DWORD dwUnk = in->ReadDWORD(); // value was "1"
			if (in->GetLastError()) break;

			break;
		}
		case 0x0032: //Send Tell by GUID
		{
			char* szText = in->ReadString();
			DWORD dwGUID = in->ReadDWORD();
			if (in->GetLastError()) break;

			SendTellByGUID(szText, dwGUID);
			break;
		}
		case 0x0035: //Use Item Ex
		{
			//SendText("Use extended not implemented yet.", 9);
			DWORD dwSourceID = in->ReadDWORD();
			DWORD dwDestID = in->ReadDWORD();
			if (in->GetLastError()) break;

			UseItemEx(dwSourceID, dwDestID);
			break;
		}
		case 0x0036: //Use Object
		{
			//SendText("Use not implemented yet.", 9);
			DWORD dwEID = in->ReadDWORD();
			if (in->GetLastError()) break;

			UseObject(dwEID);
			break;
		}
		case 0x0044: //Spend Vital XP
		{
			DWORD dwVital = in->ReadDWORD();
			DWORD dwXP = in->ReadDWORD();
			if (in->GetLastError()) break;

			SpendVitalXP(dwVital, dwXP);
			break;
		}
		case 0x0045: //Spend Attribute XP
		{
			DWORD dwAttribute = in->ReadDWORD();
			DWORD dwXP = in->ReadDWORD();
			if (in->GetLastError()) break;

			SpendAttributeXP(dwAttribute, dwXP);
			break;
		}
		case 0x0046: //Spend Skill XP
		{
			DWORD dwSkill = in->ReadDWORD();
			DWORD dwXP = in->ReadDWORD();
			if (in->GetLastError()) break;

			SpendSkillXP(dwSkill, dwXP);
			break;
		}
		case 0x0053: //Change Combat Mode
		{
			DWORD dwStance = in->ReadDWORD();
			if (in->GetLastError()) break;

			ChangeCombatStance(dwStance);
			break;
		}
		case 0x005D: //Send Tell by Name
		{
			char* szText = in->ReadString();
			char* szName = in->ReadString();
			if (in->GetLastError()) break;

			SendTellByName(szText, szName);
			break;
		}
		case 0x0063: //Lifestone Recall
		{
			LifestoneRecall();
			break;
		}
		case 0x00A1: //Exit Portal
		{
			ExitPortal();
			break;
		}
		case 0x00C8: //Identify
		{
			DWORD dwObjectID = in->ReadDWORD();
			if (in->GetLastError()) break;

			Identify(dwObjectID);
			break;
		}
		case 0x0147: //Channel Text
		{
			DWORD dwChannel = in->ReadDWORD();
			char* szText = in->ReadString();
			if (in->GetLastError()) break;

			ChannelText(dwChannel, szText);
		}
		break;
		case 0x01BF: //Request health update
		{
			DWORD dwGUID = in->ReadDWORD();
			if (in->GetLastError()) break;

			RequestHealthUpdate(dwGUID);
			break;
		}
		case 0x01DF: //Indirect Text (@me)
		{
			char *szText = in->ReadString();
			if (in->GetLastError()) break;

			EmoteText(szText);
			break;
		}
		case 0x01E1: //Emote Text (*laugh* sends 'laughs')
		{
			char *szText = in->ReadString();
			if (in->GetLastError()) break;

			ActionText(szText);
			break;
		}
		case 0x01E9: //Ping
		{
			Ping();
			break;
		}
		case 0x028D: //Marketplace Recall
		{
			MarketplaceRecall();
			break;
		}
		case 0xF61B: //Jump Movement
		{
			float flPower = in->ReadFloat();
			float flDir1 = in->ReadFloat();
			float flDir2 = in->ReadFloat();
			float flHeight = in->ReadFloat();

			loc_t *location = (loc_t *)in->ReadArray(sizeof(loc_t));
			heading_t *angles = (heading_t *)in->ReadArray(sizeof(heading_t));
			if (in->GetLastError()) break;

			memcpy(&m_pPlayer->m_Origin, location, sizeof(loc_t));
			memcpy(&m_pPlayer->m_Angles, angles, sizeof(heading_t));

			m_pPlayer->Animation_Jump(flPower, flDir1, flDir2, flHeight);
			m_pPlayer->m_bAnimUpdate = TRUE;
			break;
		}
		case 0xF61C: //Update Vector Movement
		{
			float fSpeedMod = m_pPlayer->m_fSpeedMod;
			float flForwardMod = 0;
			float flStrafMod = 0;
			float flTurnMod = 0;
			WORD wForwardAnim = 0;
			WORD wStrafAnim = 0;
			WORD wTurnAnim = 0;
			WORD wStance = 0x3D;
			BOOL bRun = FALSE;

			DWORD dwFlags = in->ReadDWORD();

			if (dwFlags & 1)
			{
				DWORD dwRunUnk = in->ReadDWORD();
	#ifdef _DEBUG
				if (dwRunUnk != 2)
				{
					LOG(Temp, Normal, "RunUnk is %08X??\n", dwRunUnk);
				}
				else
	#endif
					bRun = TRUE;
			}

			if (!bRun)
				fSpeedMod = 1.0f;

			if (dwFlags & 2)
			{
				wStance = in->ReadWORD();
				in->ReadWORD(); //0x8000?? sequence maybe
			}

			if (dwFlags & 4)
			{
				DWORD dwForwardAnim = in->ReadDWORD();
				flForwardMod = fSpeedMod;

				switch (dwForwardAnim)
				{
				case 0x45000006: //backwards
					dwForwardAnim--;
					flForwardMod *= -backwards_factor;
					break;
				case 0x45000005: //forwards

					if (bRun)
						dwForwardAnim += 2;

					break;
				default:
					//0x45 = run/walk forward/back
					//0x44 = run/walk change
					//0x43 = emotes?
					//0x41 = change position? (kneel, sleep..)
					//0x40 = attack animations?
					//0x13 = drink something

					if ((dwForwardAnim >> 24) != 0x45)
						LOG(Temp, Normal, "Forward: %08X\n", dwForwardAnim);

					flForwardMod = 1.0000f;
					break;
				}
				wForwardAnim = (WORD)dwForwardAnim;
			}

			if (dwFlags & 8) {
				DWORD dwAutorunUnk = in->ReadDWORD();

				if (dwAutorunUnk != 2)
				{
					SendText("You cannot use right-click movement at this time!", 1);
					LOG(Temp, Normal, "Autorun is %08X?\n", dwAutorunUnk);
				}
				else
				{
					if (!bRun)
						wForwardAnim = 0x0005;
					else
						wForwardAnim = 0x0007;
					flForwardMod = fSpeedMod;
				}
			}

			if (dwFlags & 0x20) {
				DWORD dwStrafAnim = in->ReadDWORD();
				flStrafMod = fSpeedMod;

				switch (dwStrafAnim)
				{
				case 0x65000010: //left?
					dwStrafAnim--;
					flStrafMod *= -1.0f;
				case 0x6500000F: //right?
					flStrafMod *= (walk_anim_speed / sidestep_anim_speed) * sidestep_factor;

					if (flStrafMod > max_sidestep_anim_rate)
						flStrafMod = max_sidestep_anim_rate;
					else if (flStrafMod < -max_sidestep_anim_rate)
						flStrafMod = -max_sidestep_anim_rate;

					break;
				default:
					if ((dwStrafAnim >> 24) != 0x65)
					{
						// DebugMe();
						LOG(Animation, Warning, "Strafe anim == 0x%02X", dwStrafAnim >> 24);
					}

					flStrafMod = 1.0000f;
					break;
				}
				wStrafAnim = (WORD)dwStrafAnim;
			}

			if (dwFlags & 0x100) {
				DWORD dwTurnAnim = in->ReadDWORD();

				flTurnMod = 1.0f;
				switch (dwTurnAnim)
				{
				case 0x6500000E: //turn left

					dwTurnAnim--;
					flTurnMod *= -1.0f;

				case 0x6500000D: //turn right
					if (bRun)
						flTurnMod *= run_turn_factor;
					break;
				default:
					if ((dwTurnAnim >> 24) != 0x65)
					{
						LOG(Animation, Warning, "Turn anim == 0x%02X", dwTurnAnim >> 24);
					}

					break;
				}

				wTurnAnim = (WORD)dwTurnAnim;
			}

			DWORD dwEmotes = (dwFlags >> 11) & 7;
			animation_list lEmotes;

			for (unsigned int i = 0; i < dwEmotes; i++)
			{
				animation_t emote;

				emote.wIndex = in->ReadWORD();	//animation
				emote.wSequence = in->ReadWORD();	//counter
				emote.fSpeed = in->ReadFloat();	//play speed

				lEmotes.push_back(emote);
			}

			loc_t *location = (loc_t *)in->ReadArray(sizeof(loc_t));
			heading_t *angles = (heading_t *)in->ReadArray(sizeof(heading_t));

			if (in->GetLastError())
			{
				LOG(Animation, Verbose, "Bad animation message:\n");
				LOG_BYTES(Animation, Verbose, in->GetDataStart(), in->GetDataLen());
				break;
			}

			memcpy(&m_pPlayer->m_Origin, location, sizeof(loc_t));
			memcpy(&m_pPlayer->m_Angles, angles, sizeof(heading_t));

			m_pPlayer->m_fForwardSpeed = flForwardMod;
			m_pPlayer->m_fStrafSpeed = flStrafMod;
			m_pPlayer->m_fTurnSpeed = flTurnMod;

			m_pPlayer->m_wForwardAnim = wForwardAnim;
			m_pPlayer->m_wStrafAnim = wStrafAnim;
			m_pPlayer->m_wTurnAnim = wTurnAnim;
			m_pPlayer->m_wStance = wStance;

			DWORD dwAnimQueued = m_pPlayer->Animation_EmoteQueueCount();
			DWORD dwAnimNeeded = lEmotes.size();
			DWORD dwLastSequence = 0;

			for (unsigned int i = 0; i < dwAnimNeeded; i++)
			{
				if (dwAnimQueued >= MAX_EMOTE_QUEUE)
					break;
				dwAnimQueued++;

				for (animation_list::iterator it = lEmotes.begin(); it != lEmotes.end(); it++)
				{
					//find 0x800x
					if ((it->wSequence & 0xFF) == i)
					{
						m_pPlayer->Animation_PlayEmote(it->wIndex, it->fSpeed);
						break;
					}
				}
			}

			m_pPlayer->m_bAnimUpdate = TRUE;
			m_pPlayer->Movement_UpdatePos();
			break;
		}
		case 0xF753: //Update Exact Position
		{
			loc_t*		location = reinterpret_cast<loc_t*>	(in->ReadArray(sizeof(loc_t)));
			heading_t*	angles = reinterpret_cast<heading_t*>(in->ReadArray(sizeof(heading_t)));
			WORD		instance = in->ReadWORD();

			if (in->GetLastError()) break;
			if (instance != m_pPlayer->m_wInstance)
			{
				LOG(Temp, Normal, "Bad instance!!!!!!!!!!!!\n");
				break;
			}

			memcpy(&m_pPlayer->m_Origin, location, sizeof(loc_t));
			memcpy(&m_pPlayer->m_Angles, angles, sizeof(heading_t));

			m_pPlayer->Movement_UpdatePos();
			break;
		}
		default:
		{
			//Unknown Event
			//LOG(Temp, Normal, "Unknown event %04X:\n", dwEvent );
			//OutputConsoleBytes( in->GetDataPtr(), in->GetDataEnd() - in->GetDataPtr() );
		}
	}
}








