
#pragma once
#include "BinaryReader.h"

class CBasePlayer;

const float sidestep_factor = 0.5f;
const float backwards_factor = 0.64999998f;
const float run_turn_factor = 1.5f;
const float run_anim_speed = 4.0f;
const float walk_anim_speed = 3.1199999f;
const float sidestep_anim_speed = 1.25f;
const float max_sidestep_anim_rate = 3.0f;

// Client/World interaction
class CClientEvents
{
public:
	CClientEvents(CClient *);
	~CClientEvents();

	void Think();

	void DetachPlayer();
	DWORD GetPlayerID();
	CBasePlayer* GetPlayer();

	void LoginError(int iError);
	void LoginCharacter(DWORD dwGUID, const char *szAccount);
	void BeginLogout();
	void ExitWorld();

	void ActionComplete();
	void SendText(const char *szText, long lColor);
	void UpdateBurdenUI();

	// Network events
	void ActionText(char* szText);
	void Attack(DWORD dwTarget, DWORD dwHeight, float flPower);
	void ChangeCombatStance(DWORD dwStance);
	void ChannelText(DWORD dwChannel, const char* szText);
	void ClientText(char* szText);
	void DropItem(DWORD dwGUID);
	void EmoteText(char* szText);
	void EquipItem(DWORD dwItemID, DWORD dwCoverage);
	void ExitPortal();
	void Identify(DWORD dwObjectID);
	void LifestoneRecall();
	void MarketplaceRecall();
	void Ping();
	void RequestHealthUpdate(DWORD dwGUID);
	void SendTellByGUID(const char* szText, DWORD dwGUID);
	void SendTellByName(const char* szText, const char* szName);
	void SpendAttributeXP(DWORD dwAttribute, DWORD dwXP);
	void SpendVitalXP(DWORD dwVital, DWORD dwXP);
	void SpendSkillXP(DWORD dwSkill, DWORD dwXP);
	void StoreItem(DWORD dwItemID, DWORD dwContainer, char cSlot);
	void UseItemEx(DWORD dwSourceID, DWORD dwDestID);
	void UseObject(DWORD dwEID);

	void ProcessEvent(BinaryReader *);

private:
	CClient *m_pClient;

	CBasePlayer *m_pPlayer;
	double m_fLogout;
};



