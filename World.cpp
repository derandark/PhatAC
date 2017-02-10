
#include "StdAfx.h"
#include "World.h"
#include "Client.h"
#include "PhysicsObj.h"
#include "Monster.h"
#include "Player.h"
#include "ChatMsgs.h"

//Database access
#include "Database.h"
#include "CharacterDatabase.h"

#include "TurbineDungeon.h"
#include "GameMode.h"

CWorld::CWorld()
{
	LOG(Temp, Normal, "Initializing World..\n");
	m_dwHintStaticGUID = 0x10000000;
	m_dwHintPlayerGUID = 0x50000000;
	m_dwHintItemGUID = 0x60000000;
	m_dwHintDynamicGUID = 0xC0000000;

	ZeroMemory(m_pBlocks, sizeof(m_pBlocks));

	LoadStateFile();
	LoadDungeonsFile();
	LoadMOTD();
	EnumerateDungeonsFromCellData();

	m_fLastSave = g_pGlobals->Time();

	m_pGameMode = NULL;
}

void CWorld::SaveWorld()
{
	SaveDungeonsFile();
	SaveStateFile();
}

CWorld::~CWorld()
{
	if (m_pGameMode)
	{
		delete m_pGameMode;
		m_pGameMode = NULL;
	}

	for (LandblockVector::iterator it = m_vBlocks.begin(); it != m_vBlocks.end(); it++)
		delete (*it);
	m_vBlocks.clear();

	SaveWorld();

	m_mDungeons.clear();
	for (DungeonDescMap::iterator it = m_mDungeonDescs.begin(); it != m_mDungeonDescs.end(); it++)
	{
		DungeonDesc_t* pdd = &it->second;

		SafeDeleteArray(pdd->szDungeonName);
		SafeDeleteArray(pdd->szDescription);
		SafeDeleteArray(pdd->szAuthor);
	}
	m_mDungeonDescs.clear();
}


loc_t CWorld::FindDungeonDrop()
{
	if (m_mDungeons.empty())
	{
		loc_t di;
		memset(&di, 0, sizeof(di));
		return di;
	}

	LocationMap::iterator dit = m_mDungeons.begin();
	long index = RandomLong(0, (long)m_mDungeons.size() - 1);
	while (index > 0) {
		index--;
		dit++;
	}

	return dit->second;
}

loc_t CWorld::FindDungeonDrop(WORD wBlockID)
{
	LocationMap::iterator i = m_mDungeons.upper_bound(((DWORD(wBlockID) << 16) + 0x100) - 1);

	if ((i == m_mDungeons.end()) || (BLOCK_WORD(i->second.landcell) != wBlockID))
	{
		loc_t di;
		memset(&di, 0, sizeof(di));
		return di;
	}

	return i->second;
}

void CWorld::LoadStateFile()
{
	FILE *ws = g_pDB->DataFileOpen("worldstate");
	if (ws)
	{
		fread(&m_dwHintStaticGUID, sizeof(DWORD), 1, ws);
		fread(&m_dwHintItemGUID, sizeof(DWORD), 1, ws);
		fread(&m_dwHintPlayerGUID, sizeof(DWORD), 1, ws);
		fread(&m_dwHintDynamicGUID, sizeof(DWORD), 1, ws);
		fclose(ws);
	}
}

void CWorld::SaveStateFile()
{
	FILE *ws = g_pDB->DataFileCreate("worldstate");
	if (ws)
	{
		fwrite(&m_dwHintStaticGUID, sizeof(DWORD), 1, ws);
		fwrite(&m_dwHintItemGUID, sizeof(DWORD), 1, ws);
		fwrite(&m_dwHintPlayerGUID, sizeof(DWORD), 1, ws);
		fwrite(&m_dwHintDynamicGUID, sizeof(DWORD), 1, ws);
		fclose(ws);
	}
	else
		MsgBox(MB_ICONHAND, "Error opening WorldState file! Close NOW to avoid corruption!");
}

const char* CWorld::GetMOTD()
{
	return m_strMOTD.c_str();
}

void CWorld::LoadMOTD()
{
	FILE *motd = g_pDB->DataFileOpen("motd.txt", "rt");
	if (motd)
	{
		long lFileSize = fsize(motd);
		char* pcFileData = new char[lFileSize + 1];

		long lEnd = (long)fread(pcFileData, sizeof(char), lFileSize, motd);
		pcFileData[lEnd] = 0;

		m_strMOTD = pcFileData;

		delete[] pcFileData;
		fclose(motd);
	}
	else
		m_strMOTD = "No MOTD set.";
}

void CWorld::LoadDungeonsFile()
{
	FILE *wd = g_pDB->DataFileOpen("worlddesc");
	if (wd)
	{
		long lSize = fsize(wd);
		BYTE* pbData = new BYTE[lSize];
		long lRead = (long)fread(pbData, sizeof(BYTE), lSize, wd);
		BinaryReader input(pbData, lRead);

		DWORD dwDungeonCount = input.ReadDWORD();

		if (!input.GetLastError())
		{
			for (DWORD i = 0; i < dwDungeonCount; i++)
			{
				DungeonDesc_t dd;
				dd.wBlockID = input.ReadWORD();
				dd.szDungeonName = input.ReadString();
				dd.szAuthor = input.ReadString();
				dd.szDescription = input.ReadString();

				loc_t* origin = (loc_t *)input.ReadArray(sizeof(loc_t));
				heading_t* angles = (heading_t *)input.ReadArray(sizeof(heading_t));
				if (input.GetLastError()) break;

				//Avoid using a buffer thats going to be deleted.
				dd.szDungeonName = _strdup(dd.szDungeonName);
				dd.szAuthor = _strdup(dd.szAuthor);
				dd.szDescription = _strdup(dd.szDescription);

				memcpy(&dd.origin, origin, sizeof(loc_t));
				memcpy(&dd.angles, angles, sizeof(heading_t));

				m_mDungeonDescs[dd.wBlockID] = dd;
			}
		}

		delete[] pbData;
		fclose(wd);
	}
}

void CWorld::SaveDungeonsFile()
{
	FILE *wd = g_pDB->DataFileCreate("worlddesc");
	if (wd)
	{
		BinaryWriter output;

		output.WriteDWORD((DWORD)m_mDungeonDescs.size());

		for (DungeonDescMap::iterator i = m_mDungeonDescs.begin(); i != m_mDungeonDescs.end(); i++)
		{
			output.WriteWORD(i->second.wBlockID);
			output.WriteString(i->second.szDungeonName);
			output.WriteString(i->second.szAuthor);
			output.WriteString(i->second.szDescription);
			output.AppendData(i->second.origin);
			output.AppendData(i->second.angles);
		}

		fwrite(output.GetData(), output.GetSize(), sizeof(BYTE), wd);
		fclose(wd);
	}
	else
		MsgBox(MB_ICONHAND, "Error opening WorldDesc file! Close NOW to avoid corruption!");
}

BOOL CWorld::DungeonExists(WORD wBlockID)
{
	LocationMap::iterator i = m_mDungeons.upper_bound(((DWORD(wBlockID) << 16) + 0x100) - 1);

	if ((i == m_mDungeons.end()) || (BLOCK_WORD(i->second.landcell) != wBlockID))
		return FALSE;

	return TRUE;
}

LocationMap* CWorld::GetDungeons()
{
	return &m_mDungeons;
}

DungeonDescMap* CWorld::GetDungeonDescs()
{
	return &m_mDungeonDescs;
}

DungeonDesc_t* CWorld::GetDungeonDesc(const char* szDungeonName)
{
	DungeonDescMap::iterator i = m_mDungeonDescs.begin();
	DungeonDescMap::iterator iend = m_mDungeonDescs.end();

	while (i != iend)
	{
		if (!stricmp(szDungeonName, i->second.szDungeonName))
			return &i->second;

		i++;
	}

	return NULL;
}

DungeonDesc_t* CWorld::GetDungeonDesc(WORD wBlockID)
{
	DungeonDescMap::iterator it = m_mDungeonDescs.find(wBlockID);

	if (it != m_mDungeonDescs.end())
		return &it->second;
	else
		return NULL;
}

void CWorld::SetDungeonDesc(WORD wBlockID, const char* szDungeonName, const char* szAuthor, const char* szDescription, loc_t origin, heading_t angles)
{
	DungeonDescMap::iterator it = m_mDungeonDescs.find(wBlockID);

	if (it != m_mDungeonDescs.end())
	{
		DungeonDesc_t* pdd = &it->second;
		SafeDeleteArray(pdd->szDungeonName);
		SafeDeleteArray(pdd->szDescription);
		SafeDeleteArray(pdd->szAuthor);
		m_mDungeonDescs.erase(it);
	}

	DungeonDesc_t dd;
	dd.wBlockID = wBlockID;
	dd.szDungeonName = _strdup(szDungeonName);
	dd.szAuthor = _strdup(szAuthor);
	dd.szDescription = _strdup(szDescription);
	dd.origin = origin;
	dd.angles = angles;

	m_mDungeonDescs[wBlockID] = dd;
}

void CWorld::InitializeHintGUIDs()
{
	_CHARDESC dummy;
	CCharacterDatabase *CharDB = g_pDB->CharDB();

	while (CharDB->GetCharacterDesc(m_dwHintPlayerGUID, &dummy))
		m_dwHintPlayerGUID++;
}

//Hackish.
DWORD CWorld::GenerateGUID(eGUIDClass type)
{
	switch (type)
	{
	case ePresetGUID: return 0;
	case ePlayerGUID:
	{
		// 0x50000000 - 0x60000000
		if (m_dwHintPlayerGUID >= 0x60000000)
		{
			LOG(Temp, Normal, "Player GUID overflow!\n");
			return 0;
		}

		_CHARDESC dummy;
		CCharacterDatabase *CharDB = g_pDB->CharDB();

		while (CharDB->GetCharacterDesc(m_dwHintPlayerGUID, &dummy))
			m_dwHintPlayerGUID++;

		//Temporary, use and increment the hint counter.
		return (m_dwHintPlayerGUID++);
	}
	case eStaticGUID:
	{
		/* 0x10000000 - 0x50000000 */
		if (m_dwHintStaticGUID >= 0x50000000)
		{
			LOG(Temp, Normal, "Static GUID overflow!\n");
			return 0;
		}

		//Temporary, use and increment the hint counter.
		return (m_dwHintStaticGUID++);
	}
	case eDynamicGUID:
	{
		// 0xC0000000 - 0xF0000000
		if (m_dwHintDynamicGUID >= 0xF0000000)
		{
			LOG(Temp, Normal, "Dynamic GUID overflow!\n");
			return 0;
		}

		//Temporary, use and increment the hint counter.
		return (m_dwHintDynamicGUID++);
	}
	case eItemGUID:
	{
		// 0x60000000 - 0xC0000000
		if (m_dwHintItemGUID >= 0xC0000000)
		{
			LOG(Temp, Normal, "Item GUID overflow!\n");
			return 0;
		}

		//Temporary, use and increment the hint counter.
		return (m_dwHintItemGUID++);
	}
	}

	return 0;
}

void CWorld::ClearAllSpawns()
{
	for (DWORD i = 0; i < (256 * 256); i++)
	{
		if (m_pBlocks[i])
			m_pBlocks[i]->ClearSpawns();
	}
}

CLandBlock* CWorld::GetLandblock(WORD wHeader)
{
	return m_pBlocks[wHeader];
}

CLandBlock* CWorld::ActivateBlock(WORD wHeader)
{
	CLandBlock **ppBlock = &m_pBlocks[wHeader];
	CLandBlock *pBlock;

#if _DEBUG
	pBlock = *ppBlock;
	if (pBlock != NULL)
	{
		LOG(Temp, Normal, "Landblock already active!\n");
		return pBlock;
	}
#endif

	pBlock = new CLandBlock(this, wHeader);
	m_vSpawns.push_back(pBlock);
	*ppBlock = pBlock;

	return pBlock;
}

void CWorld::CreateEntity(CPhysicsObj *pEntity)
{
	pEntity->Precache();

#if _DEBUG
	if (!pEntity->m_dwGUID)
	{
		LOG(Temp, Normal, "Null entid being placed in world.\n");
		return;
	}
#endif

	WORD wHeader = BLOCK_WORD(pEntity->GetLandcell());

	if (pEntity->IsPlayer())
	{
		DWORD dwGUID = pEntity->m_dwGUID;

		m_mAllPlayers.insert(std::pair< DWORD, CBasePlayer* >(dwGUID, (CBasePlayer *)pEntity));
		BroadcastGlobal(ServerBroadcast("System", csprintf("%s has logged in.", pEntity->GetName()), 1), PRIVATE_MSG, dwGUID, FALSE, TRUE);
	}

	CLandBlock *pBlock = m_pBlocks[wHeader];

	if (pBlock)
	{
		if (pBlock->FindEntity(pEntity->m_dwGUID))
		{
			// DEBUGOUT("Not spawning duplicate entity!\n");
			// Already exists.
			delete pEntity;
			return;
		}
	}

	if (!pBlock)
		pBlock = ActivateBlock(wHeader);

	pBlock->Insert(pEntity, 0, TRUE);

	pEntity->MakeAware(pEntity);
	pEntity->Spawn();
}

void CWorld::InsertTeleportLocation(TeleTownList_s l)
{
	m_vTeleTown.push_back(l);
}
std::string CWorld::GetTeleportList()
{
	std::string result;

	for each (TeleTownList_s var in m_vTeleTown)
	{
		result.append(var.m_teleString).append(", ");
	}
	if (result.back() == 0x20) { //Get out behind of bad formatting
		result.pop_back();
		result.pop_back();
	}

	return result;
}
TeleTownList_s CWorld::GetTeleportLocation(std::string location)
{
	TeleTownList_s val;
	std::transform(location.begin(), location.end(), location.begin(), ::tolower);
	for each (TeleTownList_s var in m_vTeleTown)
	{
		//Lets waste a bunch of time with this.. Hey, if its the first one on the list its O(1)
		std::string town = var.m_teleString;
		std::transform(town.begin(), town.end(), town.begin(), ::tolower);
		if (town.find(location) != std::string::npos) {
			val = var;
			break;
		}
	}
	return val;
}

void CWorld::InsertEntity(CPhysicsObj *pEntity, BOOL bSilent)
{
	WORD wHeader = BLOCK_WORD(pEntity->GetLandcell());

	CLandBlock *pBlock = m_pBlocks[wHeader];
	if (!pBlock)
		pBlock = ActivateBlock(wHeader);

	if (bSilent)
		pBlock->Insert(pEntity, wHeader);
	else
		pBlock->Insert(pEntity);

	if (pEntity->IsPlayer())
		m_mAllPlayers.insert(std::pair< DWORD, CBasePlayer* >(pEntity->m_dwGUID, (CBasePlayer *)pEntity));
}

void CWorld::JuggleEntity(WORD wOld, CPhysicsObj* pEntity)
{
	if (!pEntity->HasOwner())
	{
		WORD wHeader = BLOCK_WORD(pEntity->GetLandcell());

		CLandBlock *pBlock = GetLandblock(wHeader);
		if (!pBlock)
			pBlock = ActivateBlock(wHeader);

		pBlock->Insert(pEntity, wOld);
	}
}

PlayerMap* CWorld::GetPlayers()
{
	return &m_mAllPlayers;
}

//==================================================
//Global, player search by GUID.
//==================================================
CBasePlayer* CWorld::FindPlayer(DWORD dwGUID)
{
	PlayerMap::iterator result = m_mAllPlayers.find(dwGUID);

	if (result == m_mAllPlayers.end())
		return NULL;

	return result->second;
}

//==================================================
//Global, case insensitive player name search.
//==================================================
CBasePlayer* CWorld::FindPlayer(const char *szName)
{
	if (*szName == '+')
		szName++;

	PlayerMap::iterator pit = m_mAllPlayers.begin();
	PlayerMap::iterator pend = m_mAllPlayers.end();

	while (pit != pend)
	{
		CBasePlayer *pPlayer = pit->second;

		if (pPlayer)
		{
			const char* szPN = pPlayer->GetName();

			if (*szPN == '+')
				szPN++;

			if (!stricmp(szName, szPN))
				return pPlayer;
		}

		pit++;
	}

	return NULL;
}

void CWorld::BroadcastPVS(DWORD dwCell, void *_data, DWORD _len, WORD _group, DWORD ignore_ent, BOOL _game_event)
{
	if (!dwCell)
		return;

	// Don't ask.
	WORD block = BLOCK_WORD(dwCell);
	WORD cell = CELL_WORD(dwCell);

	DWORD basex = BASE_OFFSET(BLOCK_X(block), CELL_X(cell));
	DWORD basey = BASE_OFFSET(BLOCK_Y(block), CELL_Y(cell));

	DWORD minx = basex;
	DWORD maxx = basex;
	DWORD miny = basey;
	DWORD maxy = basey;

	//if ( cell < 0xFF ) //indoor structure
	{
		if (minx >= (dwMinimumCellX + PVC_RANGE)) minx -= PVC_RANGE; else minx = dwMinimumCellX;
		if (maxx <= (dwMaximumCellX - PVC_RANGE)) maxx += PVC_RANGE; else maxx = dwMaximumCellX;

		if (miny >= (dwMinimumCellY + PVC_RANGE)) miny -= PVC_RANGE; else miny = dwMinimumCellY;
		if (maxy <= (dwMaximumCellY - PVC_RANGE)) maxy += PVC_RANGE; else maxy = dwMaximumCellY;
	}

	minx = BLOCK_OFFSET(minx) << 8;
	miny = BLOCK_OFFSET(miny);
	maxx = BLOCK_OFFSET(maxx) << 8;
	maxy = BLOCK_OFFSET(maxy);

	for (DWORD xit = minx; xit <= maxx; xit += 0x100) {
		for (DWORD yit = miny; yit <= maxy; yit += 1)
		{
			CLandBlock* pBlock = m_pBlocks[xit | yit];
			if (pBlock)
				pBlock->Broadcast(_data, _len, _group, ignore_ent, _game_event);
		}
	}
}

void CWorld::BroadcastGlobal(BinaryWriter *food, WORD _group, DWORD ignore_ent, BOOL _game_event, BOOL del)
{
	BroadcastGlobal(food->GetData(), food->GetSize(), _group, ignore_ent, _game_event);
	if (del)
		delete food;
}

void CWorld::BroadcastGlobal(void *_data, DWORD _len, WORD _group, DWORD ignore_ent, BOOL _game_event)
{
	//This is a.. very.. very bad.

#if TRUE
 //We could send with the player map.

	PlayerMap::iterator pit = m_mAllPlayers.begin();
	PlayerMap::iterator pend = m_mAllPlayers.end();

	while (pit != pend)
	{
		CBasePlayer *pPlayer;

		if (pPlayer = pit->second)
		{
			if (!ignore_ent || (pPlayer->m_dwGUID != ignore_ent))
				pPlayer->SendNetMessage(_data, _len, _group, _game_event);
		}

		pit++;
	}
#else
 //We could also send by landblocks instead.

	LandblockVector::iterator lit = m_vBlocks.begin();
	LandblockVector::iterator lend = m_vBlocks.end();

	while (lit != lend)
	{
		CLandBlock *pBlock;
		if (pBlock = (*lit))
			pBlock->Broadcast(_data, _len, _group, ignore_ent, _game_event);

		lit++;
	}
#endif
}

void CWorld::Test()
{
	LOG(Temp, Normal, "<CWorld::Test()>\n");
	LOG(Temp, Normal, "Portal: v%lu, %lu files.\n", g_pPortal->GetVersion(), g_pPortal->GetFileCount());
	LOG(Temp, Normal, "Cell: v%lu, %u files.\n", g_pCell->GetVersion(), g_pCell->GetFileCount());
	LOG(Temp, Normal, "%u players:\n", m_mAllPlayers.size());
	for (PlayerMap::iterator pit = m_mAllPlayers.begin(); pit != m_mAllPlayers.end(); pit++)
	{
		CBasePlayer* pPlayer = pit->second;
		LOG(Temp, Normal, "%08X %s\n", pPlayer->m_dwGUID, pPlayer->GetName());
	}
	LOG(Temp, Normal, "%u active blocks:\n", m_vBlocks.size());
	for (LandblockVector::iterator it = m_vBlocks.begin(); it != m_vBlocks.end(); it++)
	{
		CLandBlock* pBlock = *it;
		LOG(Temp, Normal, "%08X %lu players %lu live %lu dormant\n", pBlock->GetHeader() << 16, pBlock->PlayerCount(), pBlock->LiveCount(), pBlock->DormantCount());
	}

	LOG(Temp, Normal, "</CWorld::Test()>\n");
}

void CWorld::RemoveEntity(CPhysicsObj *pEntity)
{
	if (!pEntity)
	{
		return;
	}

	if (m_pGameMode)
	{
		m_pGameMode->OnRemoveEntity(pEntity);
	}

	DWORD dwGUID = pEntity->m_dwGUID;

	if (pEntity->IsPlayer())
	{
		m_mAllPlayers.erase(dwGUID);

		BroadcastGlobal(ServerBroadcast("System", csprintf("%s has logged out.", pEntity->GetName()), 1), PRIVATE_MSG, dwGUID, FALSE, TRUE);
	}

	CLandBlock* pBlock = pEntity->GetBlock();

	if (!pBlock)
	{
		pEntity->Save();
		DELETE_ENTITY(pEntity);
	}
	else
	{
		pBlock->Destroy(pEntity);
	}
}

void CWorld::Think()
{
	LandblockVector::iterator lit = m_vBlocks.begin();
	LandblockVector::iterator lend = m_vBlocks.end();

	while (lit != lend)
	{
		CLandBlock *pBlock = *lit;

		if (pBlock->Think())
			lit++;
		else
		{
			//dead
			lit = m_vBlocks.erase(lit);
			lend = m_vBlocks.end();
			m_pBlocks[pBlock->GetHeader()] = NULL;
			delete pBlock;
		}
	}

	if (!m_vSpawns.empty())
	{
		std::copy(m_vSpawns.begin(), m_vSpawns.end(), std::back_inserter(m_vBlocks));
		m_vSpawns.clear();
	}

	if ((m_fLastSave + 300.0f) <= g_pGlobals->Time())
	{
		SaveWorld();
		m_fLastSave = g_pGlobals->Time();
	}

	if (m_pGameMode)
	{
		m_pGameMode->Think();
	}
}

CGameMode *CWorld::GetGameMode()
{
	return m_pGameMode;
}

void CWorld::SetNewGameMode(CGameMode *pGameMode)
{
	if (pGameMode)
	{
		g_pWorld->BroadcastGlobal(ServerText(csprintf("Setting game mode to %s", pGameMode->GetName())), PRIVATE_MSG);
	}
	else
	{
		if (!pGameMode && m_pGameMode)
		{
			g_pWorld->BroadcastGlobal(ServerText(csprintf("Turning off game mode %s", m_pGameMode->GetName())), PRIVATE_MSG);
		}
	}

	if (m_pGameMode)
	{
		delete m_pGameMode;
	}

	m_pGameMode = pGameMode;
}

void CWorld::EnumNearby(CPhysicsObj *pSource, float fRange, std::list<CPhysicsObj *> *pResults)
{
	// Enumerate nearby world objects
	if (pSource != NULL && !pSource->HasOwner())
	{
		DWORD dwCell = pSource->GetLandcell();
		WORD block = BLOCK_WORD(dwCell);
		WORD cell = CELL_WORD(dwCell);

		DWORD basex = BASE_OFFSET(BLOCK_X(block), CELL_X(cell));
		DWORD basey = BASE_OFFSET(BLOCK_Y(block), CELL_Y(cell));

		DWORD minx = basex;
		DWORD maxx = basex;
		DWORD miny = basey;
		DWORD maxy = basey;

		//if ( cell < 0xFF ) // indoor structure
		{
			if (minx >= (dwMinimumCellX + PVC_RANGE)) minx -= PVC_RANGE; else minx = dwMinimumCellX;
			if (maxx <= (dwMaximumCellX - PVC_RANGE)) maxx += PVC_RANGE; else maxx = dwMaximumCellX;

			if (miny >= (dwMinimumCellY + PVC_RANGE)) miny -= PVC_RANGE; else miny = dwMinimumCellY;
			if (maxy <= (dwMaximumCellY - PVC_RANGE)) maxy += PVC_RANGE; else maxy = dwMaximumCellY;
		}

		minx = BLOCK_OFFSET(minx) << 8;
		miny = BLOCK_OFFSET(miny);
		maxx = BLOCK_OFFSET(maxx) << 8;
		maxy = BLOCK_OFFSET(maxy);

		for (DWORD xit = minx; xit <= maxx; xit += 0x100) {
			for (DWORD yit = miny; yit <= maxy; yit += 1)
			{
				CLandBlock* pBlock = m_pBlocks[xit | yit];
				if (pBlock)
				{
					pBlock->EnumNearby(pSource, fRange, pResults);
				}
			}
		}
	}
}

CPhysicsObj* CWorld::FindWithinPVS(CPhysicsObj *pSource, DWORD dwGUID)
{
	if (!pSource)
		return NULL;

	//Find nearby world objects.
	if (!pSource->HasOwner())
	{
		DWORD dwCell = pSource->GetLandcell();
		WORD block = BLOCK_WORD(dwCell);
		WORD cell = CELL_WORD(dwCell);

		DWORD basex = BASE_OFFSET(BLOCK_X(block), CELL_X(cell));
		DWORD basey = BASE_OFFSET(BLOCK_Y(block), CELL_Y(cell));

		DWORD minx = basex;
		DWORD maxx = basex;
		DWORD miny = basey;
		DWORD maxy = basey;

		//if ( cell < 0xFF ) //indoor structure
		{
			if (minx >= (dwMinimumCellX + PVC_RANGE)) minx -= PVC_RANGE; else minx = dwMinimumCellX;
			if (maxx <= (dwMaximumCellX - PVC_RANGE)) maxx += PVC_RANGE; else maxx = dwMaximumCellX;

			if (miny >= (dwMinimumCellY + PVC_RANGE)) miny -= PVC_RANGE; else miny = dwMinimumCellY;
			if (maxy <= (dwMaximumCellY - PVC_RANGE)) maxy += PVC_RANGE; else maxy = dwMaximumCellY;
		}

		minx = BLOCK_OFFSET(minx) << 8;
		miny = BLOCK_OFFSET(miny);
		maxx = BLOCK_OFFSET(maxx) << 8;
		maxy = BLOCK_OFFSET(maxy);

		for (DWORD xit = minx; xit <= maxx; xit += 0x100) {
			for (DWORD yit = miny; yit <= maxy; yit += 1)
			{
				CLandBlock* pBlock = m_pBlocks[xit | yit];
				if (pBlock)
				{
					CPhysicsObj *pEntity = pBlock->FindEntity(dwGUID);
					if (pEntity)
						return pEntity;
				}
			}
		}
	}

	if (pSource->Container_HasContents())
		return (CPhysicsObj *)pSource->Container_FindItem(dwGUID);

	return NULL;
}

void CWorld::EnumerateDungeonsFromCellData()
{
	// TODO Need to fix dungeon parsing for ToD first. Broke because of that.
	return;

	// This creates a list of dungeons
	FILEMAP* pFiles = g_pCell->GetFiles();
	FILEMAP::iterator i = pFiles->begin();
	FILEMAP::iterator iend = pFiles->end();

	while (i != iend)
	{
		DWORD dwID = i->first;

		if (((dwID & 0xFFFF) >= 0xFFFE) || ((dwID & 0xFFFF) < 0x100))
		{
			i++;
			continue;
		}

		TURBINEFILE* pGeometryFile = g_pCell->GetFile(dwID);

		BYTE *_data, *data;
		_data = data = pGeometryFile->GetData();

		DWORD dwGeoFlags = *((DWORD *)data);

		if (!(dwGeoFlags & 9))
		{
			data += sizeof(DWORD);
			DWORD dungeonID = *((DWORD*)data);
			data += sizeof(DWORD);
			UCHAR textureCount = *((UCHAR *)data);
			data += sizeof(UCHAR);
			UCHAR connectionCount = *((UCHAR *)data);
			data += sizeof(UCHAR);
			USHORT visibleBlocksCount = *((USHORT *)data);
			data += sizeof(USHORT);
			data += textureCount * sizeof(USHORT);
			if (textureCount & 1)
				data += sizeof(USHORT);

			WORD blockID = *((WORD *)data);
			data += sizeof(WORD);
			WORD partNum = *((WORD *)data);
			data += sizeof(WORD);

			loc_t di;
			di.landcell = dungeonID;
			float _xTrans = *((float *)data);
			data += sizeof(float);
			float _yTrans = *((float *)data);
			data += sizeof(float);
			float _zTrans = *((float *)data);
			data += sizeof(float);
			float rw = *((float *)data);
			data += sizeof(float);
			float rx = *((float *)data);
			data += sizeof(float);
			float ry = *((float *)data);
			data += sizeof(float);
			float rz = *((float *)data);
			data += sizeof(float);

#if 0 //This code look for the dropsite models in dungeons.
			if (dwGeoFlags & 2)
			{
				data += sizeof(DWORD) * connectionCount * 2;
				data += sizeof(USHORT) * visibleBlocksCount;
				if (visibleBlocksCount & 1)
					data += sizeof(USHORT);

				DWORD numObjects = *((DWORD *)data);
				data += sizeof(DWORD);

				for (unsigned int h = 0; h < numObjects; h++)
				{
					DWORD dwModelID = *((DWORD *)data);
					data += sizeof(DWORD);

					if ((dwModelID >= 0x02000C39 && dwModelID <= 0x02000C48) || (dwModelID == 0x02000F4A)) {
						//OutputConsole( "%08X has drop site[%08X] @ %f %f %f\r\n", dwID, dwModelID,
						// ((float*)data)[0], ((float*)data)[1], ((float*)data)[2]);
						break;
					}
					data += sizeof(float) * 7;
				}
			}
#endif

			//if ( BLOCK_WORD( dwID ) == 0xF924 )
			// OutputConsole( "%08X %04X %04X\r\n", dwGeoFlags, blockID, partNum );

			DUNGEON* pDungeon = g_pPortal->GetDungeon(0x0D000000 + blockID);

			if (pDungeon)
			{
				loc_t Point = pDungeon->FindLandingZone(partNum);

				if (Point.landcell)
				{
					Vector pt(Point.x, Point.y, Point.z);
					matrix mm;
					mm.defineByQuaternion(rw, rx, ry, rz);
					mm.applyToVector(pt);
					//Point.x = pt.x;
					//Point.y = pt.y;
					//Point.z = pt.z;

					di.x = (float)(_xTrans + pt.x);
					di.y = (float)(_yTrans + pt.y);
					di.z = (float)(_zTrans + pt.z + 0.025f);

					//if ( BLOCK_WORD( dwID ) == 0xF924 )
					// LOG(Temp, Normal, "%f %f %f\n", di.x, di.y, di.z );

					m_mDungeons[dungeonID] = di;

					//506e0108
				}
				else
				{
					delete pGeometryFile;
					i++;
					continue;
				}

				//50520100 is broke
				//54500000 is broke
			}
			else
			{
				LOG(Temp, Normal, "Dungeon block mismatch. Are portal/cell versions inconsistant?\n");
			}
		}

		delete pGeometryFile;

		if (dwID < (DWORD)(0 - 0x10100))
			i = pFiles->upper_bound(((dwID & 0xFFFF0000) + 0x10100) - 1);
		else
			break;
	}
}
