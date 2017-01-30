
#pragma once

//
#include "PhysicsObj.h"
#include "Monster.h"
#include "Player.h"

typedef stdext::hash_map<DWORD, CPhysicsObj *> EntityMap;
typedef std::vector<CPhysicsObj *> EntityVector;
typedef stdext::hash_map<DWORD, CBasePlayer *> PlayerMap;
typedef std::vector<CBasePlayer *> PlayerVector;

//
class CWorld;

#ifdef _DEBUG
#define DELETE_ENTITY(x) OutputDebugString(csprintf("Delete Entity %u(%08X) @ %s:%u\r\n", pEntity->m_dwGUID, (DWORD)pEntity, __FUNCTION__)); delete pEntity
#else
#define DELETE_ENTITY(x) delete pEntity
#endif

//
class CLandBlock
{
public:
	CLandBlock(CWorld *pWorld, WORD wHeader);
	~CLandBlock();

	WORD GetHeader();
	BOOL Think();

	void SetEntityLive(CPhysicsObj *pEntity);
	void ClearSpawns();

	void Insert(CPhysicsObj *pEntity, WORD wOld = 0, BOOL bNew = FALSE);
	void Destroy(CPhysicsObj *pEntity);
	void Release(CPhysicsObj *pEntity);
	void ExchangePVS(CPhysicsObj *pSource, WORD wOld);
	void ExchangeData(CPhysicsObj *pSource);

	CBasePlayer* FindPlayer(DWORD dwGUID);
	CPhysicsObj* FindEntity(DWORD dwGUID);

	void Broadcast(void *_data, DWORD _len, WORD _group, DWORD ignore_ent, BOOL _game_event);

	DWORD PlayerCount() { return (DWORD)m_vPlayers.size(); }
	DWORD LiveCount() { return (DWORD)m_vLiveEnts.size(); }
	DWORD DormantCount() { return (DWORD)m_vDormantEnts.size(); }

	void EnumNearby(CPhysicsObj *pSource, float fRange, std::list<CPhysicsObj *> *pResults);

protected:
	CWorld* m_pWorld;

	WORD m_wHeader;

	PlayerMap m_mPlayers;
	EntityMap m_mEntities;

	PlayerVector m_vPlayers; //Players, used for message broadcasting.
	EntityVector m_vSpawnEnts;
	EntityVector m_vLiveEnts;
	EntityVector m_vDormantEnts; //Entities that do not think are "DEAD"

	BOOL m_bThinking;
	double m_fDeathTime;
};