
#include "StdAfx.h"
#include "World.h"

/*

FAQ:

Why track players and entities separatedly?
Because players received network messages.


*/

CLandBlock::CLandBlock(CWorld *pWorld, WORD wHeader)
{
	m_pWorld = pWorld;
	m_wHeader = wHeader;

	m_bThinking = FALSE;
	m_fDeathTime = 0;
}

CLandBlock::~CLandBlock()
{
	EntityVector::iterator eit = m_vSpawnEnts.begin();
	EntityVector::iterator eend = m_vSpawnEnts.end();

	while (eit != eend)
	{
		CPhysicsObj *pEntity = *eit;
		if (pEntity)
		{
			if (this == pEntity->GetBlock())
			{
				pEntity->Save();
				DELETE_ENTITY(pEntity);
			}
		}

		eit++;
	}

	eit = m_vLiveEnts.begin();
	eend = m_vLiveEnts.end();

	while (eit != eend)
	{
		CPhysicsObj *pEntity = *eit;
		if (pEntity)
		{
			if (this == pEntity->GetBlock())
			{
				pEntity->Save();
				DELETE_ENTITY(pEntity);
			}
		}

		eit++;
	}

	eit = m_vDormantEnts.begin();
	eend = m_vDormantEnts.end();

	while (eit != eend)
	{
		CPhysicsObj *pEntity = *eit;
		if (pEntity)
		{
			if (this == pEntity->GetBlock())
			{
				pEntity->Save();
				DELETE_ENTITY(pEntity);
			}
		}

		eit++;
	}

	m_mPlayers.clear();
	m_mEntities.clear();
	m_vPlayers.clear();
	m_vSpawnEnts.clear();
	m_vLiveEnts.clear();
	m_vDormantEnts.clear();
}

WORD CLandBlock::GetHeader()
{
	return m_wHeader;
}

void CLandBlock::Insert(CPhysicsObj *pEntity, WORD wOld, BOOL bNew)
{
	DWORD dwGUID = pEntity->m_dwGUID;

	if (pEntity->IsPlayer())
	{
		m_mPlayers.insert(std::pair<DWORD, CBasePlayer *>(dwGUID, (CBasePlayer *)pEntity));
		m_vPlayers.push_back((CBasePlayer *)pEntity);
	}

	m_mEntities.insert(std::pair<DWORD, CPhysicsObj *>(dwGUID, pEntity));
	m_vSpawnEnts.push_back(pEntity);

	pEntity->Attach(this);

	if (bNew)
		pEntity->RemovePreviousInstance();

	ExchangePVS(pEntity, wOld);
}

CPhysicsObj *CLandBlock::FindEntity(DWORD dwGUID)
{
	EntityMap::iterator eit = m_mEntities.find(dwGUID);

	if (eit == m_mEntities.end())
		return NULL;

	return eit->second;
}

CBasePlayer *CLandBlock::FindPlayer(DWORD dwGUID)
{
	PlayerMap::iterator pit = m_mPlayers.find(dwGUID);

	if (pit == m_mPlayers.end())
		return NULL;

	return pit->second;
}

/*

ExchangeData --
Makes all entities mutually aware of the source.

*/
void CLandBlock::ExchangeData(CPhysicsObj *pSource)
{
	EntityVector::iterator eit = m_vSpawnEnts.begin();
	EntityVector::iterator eend = m_vSpawnEnts.end();

	while (eit != eend)
	{
		CPhysicsObj *pEntity = (*eit);

		if (pEntity)
		{
			if (pEntity != pSource)
			{
				pEntity->MakeAware(pSource);
				pSource->MakeAware(pEntity);
			}
			//else
			//	pEntity->MakeAware(pSource);

			eit++;
			continue;
		}

		eit = m_vSpawnEnts.erase(eit);
		eend = m_vSpawnEnts.end();
	}

	eit = m_vLiveEnts.begin();
	eend = m_vLiveEnts.end();

	while (eit != eend)
	{
		CPhysicsObj *pEntity = (*eit);

		if (pEntity)
		{
			if (pEntity != pSource)
			{
				pEntity->MakeAware(pSource);
				pSource->MakeAware(pEntity);
			}
			//else
			//	pEntity->MakeAware(pSource);

			eit++;
			continue;
		}

		eit = m_vLiveEnts.erase(eit);
		eend = m_vLiveEnts.end();
	}

	eit = m_vDormantEnts.begin();
	eend = m_vDormantEnts.end();

	while (eit != eend)
	{
		CPhysicsObj *pEntity = (*eit);

		if (pEntity)
		{
			if (pEntity != pSource)
			{
				pEntity->MakeAware(pSource);
				pSource->MakeAware(pEntity);
			}
			//else
			//	pEntity->MakeAware(pSource);

			eit++;
			continue;
		}

		eit = m_vDormantEnts.erase(eit);
		eend = m_vDormantEnts.end();
	}

}

void CLandBlock::ExchangePVS(CPhysicsObj *pSource, WORD wOld)
{
	if (!pSource)
		return;

	BYTE xold, xstart, xend;
	BYTE yold, ystart, yend;

	xold = (wOld >> 8);
	xstart = (m_wHeader >> 8) - 1; if (xstart == (BYTE)-1) xstart = 0;
	xend = (m_wHeader >> 8) + 1; if (xend == (BYTE)0) xend = 0xFF;
	yold = (wOld & 0xFF);
	ystart = (m_wHeader & 0xFF) - 1; if (ystart == (BYTE)-1) ystart = 0;
	yend = (m_wHeader & 0xFF) + 1; if (yend == (BYTE)0) yend = 0xFF;

	for (WORD xit = xstart; xit <= xend; xit++)
	{
		for (WORD yit = ystart; yit <= yend; yit++)
		{
			if (wOld)
			{
				if ((xold >= (xit - 1)) && (xold <= (xit + 1)))
				{
					if ((yold >= (yit - 1)) && (yold <= (yit + 1)))
						continue;
				}
			}

			WORD wHeader = ((WORD)xit << 8) | yit;
			CLandBlock *pBlock = m_pWorld->GetLandblock(wHeader);

			if (pBlock)
				pBlock->ExchangeData(pSource);
		}
	}
}

void CLandBlock::Broadcast(void *_data, DWORD _len, WORD _group, DWORD ignore_ent, BOOL _game_event)
{
	PlayerVector::iterator pit = m_vPlayers.begin();
	PlayerVector::iterator pend = m_vPlayers.end();

	while (pit != pend)
	{
		CBasePlayer *pPlayer = (*pit);

		if (pPlayer)
		{
			if (!ignore_ent || (ignore_ent != pPlayer->m_dwGUID))
			{
				pPlayer->SendNetMessage(_data, _len, _group, _game_event);
			}

			pit++;
			continue;
		}

		//remove NULL entries.
		pit = m_vPlayers.erase(pit);
		pend = m_vPlayers.end();
	}
}

void CLandBlock::Release(CPhysicsObj *pEntity)
{
	if (pEntity->GetBlock() == this)
		pEntity->Detach();

	if (pEntity->IsPlayer())
	{
		m_mPlayers.erase(pEntity->m_dwGUID);

		PlayerVector::iterator pit = m_vPlayers.begin();
		PlayerVector::iterator pend = m_vPlayers.end();

		while (pit != pend)
		{
			if (pEntity == (*pit))
			{
				if (!m_bThinking)
				{
					pit = m_vPlayers.erase(pit);
					pend = m_vPlayers.end();
					continue;
				}
				else
					*pit = NULL;
			}

			pit++;
		}
	}

	m_mEntities.erase(pEntity->m_dwGUID);

	EntityVector::iterator eit = m_vSpawnEnts.begin();
	EntityVector::iterator eend = m_vSpawnEnts.end();

	while (eit != eend)
	{
		if (pEntity == (*eit))
		{
			if (!m_bThinking)
			{
				eit = m_vSpawnEnts.erase(eit);
				eend = m_vSpawnEnts.end();
				continue;
			}
			else
				*eit = NULL;
		}

		eit++;
	}

	eit = m_vLiveEnts.begin();
	eend = m_vLiveEnts.end();

	while (eit != eend)
	{
		if (pEntity == (*eit))
		{
			if (!m_bThinking)
			{
				eit = m_vLiveEnts.erase(eit);
				eend = m_vLiveEnts.end();
				continue;
			}
			else
				*eit = NULL;
		}

		eit++;
	}

	eit = m_vDormantEnts.begin();
	eend = m_vDormantEnts.end();

	while (eit != eend)
	{
		if (pEntity == (*eit))
		{
			if (!m_bThinking)
			{
				eit = m_vDormantEnts.erase(eit);
				eend = m_vDormantEnts.end();
				continue;
			}
			else
				*eit = NULL;
		}

		eit++;
	}
}

void CLandBlock::Destroy(CPhysicsObj *pEntity)
{
	Release(pEntity);

	pEntity->Save();

#if FALSE
	DWORD DestroyObject[2];
	DestroyObject[0] = 0x0024;
	DestroyObject[1] = pEntity->m_dwGUID;

	m_pWorld->BroadcastPVS(pEntity->GetLandcell(), DestroyObject, sizeof(DestroyObject), OBJECT_MSG, 0);
#else

	DWORD RemoveObject[3];
	RemoveObject[0] = 0xF747;
	RemoveObject[1] = pEntity->m_dwGUID;
	RemoveObject[2] = pEntity->m_wInstance;

	m_pWorld->BroadcastPVS(pEntity->GetLandcell(), RemoveObject, sizeof(RemoveObject));

	// LOG(Temp, Normal, "Removing entity %08X %04X @ %08X \n", pEntity->m_dwGUID, pEntity->m_wInstance, pEntity->GetLandcell());
#endif

	DELETE_ENTITY(pEntity);
}

void CLandBlock::SetEntityLive(CPhysicsObj *pEntity)
{
	EntityVector::iterator eit = m_vLiveEnts.begin();
	EntityVector::iterator eend = m_vLiveEnts.end();

	while (eit != eend)
	{
		if (pEntity == (*eit))
			break;

		eit++;
	}

	if (eit == eend)
	{
		if (m_bThinking)
			OutputDebugString("Entity going live during think phase! AHH!\n");

		m_vLiveEnts.push_back(pEntity);
	}

	eit = m_vDormantEnts.begin();
	eend = m_vDormantEnts.end();

	while (eit != eend)
	{
		if (pEntity != (*eit))
		{
			eit++;
			continue;
		}

		eit = m_vDormantEnts.erase(eit);
		eend = m_vDormantEnts.end();
	}
}

BOOL CLandBlock::Think()
{
	m_bThinking = TRUE;

	EntityVector::iterator eit = m_vLiveEnts.begin();
	EntityVector::iterator eend = m_vLiveEnts.end();

	while (eit != eend)
	{
		CPhysicsObj *pEntity = (*eit);

		if (!pEntity || pEntity->GetBlock() != this)
		{
			//remove NULL entries.
			eit = m_vLiveEnts.erase(eit);
			eend = m_vLiveEnts.end();
			continue;
		}

		//This entity *is* under our control

		WORD wHeader = BLOCK_WORD(pEntity->GetLandcell());
		if (wHeader == m_wHeader && !pEntity->HasOwner())
		{
			if (!pEntity->Think())
			{
				//This entity is going dormant.
				eit = m_vLiveEnts.erase(eit);
				eend = m_vLiveEnts.end();

				m_vDormantEnts.push_back(pEntity);
			}
			else
				eit++;

			continue;
		}

		//The entity should shift control.
		if (pEntity->IsPlayer())
		{
			m_mPlayers.erase(pEntity->m_dwGUID);

			PlayerVector::iterator pit = m_vPlayers.begin();
			PlayerVector::iterator pend = m_vPlayers.end();
			while (pit != pend)
			{
				if (pEntity == (*pit))
				{
					pit = m_vPlayers.erase(pit);
					pend = m_vPlayers.end();
				}
				else
					pit++;
			}
		}

		if (pEntity->GetBlock() == this)
			pEntity->Detach();

		m_mEntities.erase(pEntity->m_dwGUID);
		eit = m_vLiveEnts.erase(eit);
		eend = m_vLiveEnts.end();

		m_pWorld->JuggleEntity(m_wHeader, pEntity);
	}

	m_bThinking = FALSE;

	std::copy(m_vSpawnEnts.begin(), m_vSpawnEnts.end(), std::back_inserter(m_vLiveEnts));
	m_vSpawnEnts.clear();

	//This should be changed to be based off the death time.
	if (m_vLiveEnts.empty() && m_vDormantEnts.empty())
	{
		return FALSE;
	}

	return TRUE;
}

void CLandBlock::ClearSpawns()
{
	EntityVector::iterator eit = m_vDormantEnts.begin();
	EntityVector::iterator eend = m_vDormantEnts.end();

	CPhysicsObj *pEntity;

	while (eit != eend)
	{
		pEntity = *eit;

		if (pEntity && !pEntity->m_bDontClear)
		{
			if (pEntity)
			{
				Destroy(pEntity);

				eit = m_vDormantEnts.begin();
				eend = m_vDormantEnts.end();
			}
			else
			{
				eit = m_vDormantEnts.erase(eit);
				eend = m_vDormantEnts.end();
			}
		}
		else
		{
			eit++;
		}
	}
}

void CLandBlock::EnumNearby(CPhysicsObj *pSource, float fRange, std::list<CPhysicsObj *> *pResults)
{
	for (EntityMap::iterator i = m_mEntities.begin(); i != m_mEntities.end(); i++)
	{
		CPhysicsObj *pOther = i->second;

		if (pSource == pOther)
			continue;

		if ((Vector(pSource->m_Origin) - Vector(pOther->m_Origin)).Length() <= fRange)
		{
			pResults->push_back(pOther);
		}
	}
}





