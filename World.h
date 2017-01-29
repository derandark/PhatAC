
#pragma once

#include "LandBlock.h"

enum eGUIDClass {
	ePresetGUID = 0,
	ePlayerGUID = 1,
	eStaticGUID = 2,
	eDynamicGUID = 3,
	eItemGUID = 4
};

typedef struct DungeonDesc_s
{
	WORD wBlockID;
	const char* szDungeonName;
	const char* szAuthor;
	const char* szDescription;

	loc_t origin;
	heading_t angles;
} DungeonDesc_t;

typedef std::map<DWORD, loc_t> LocationMap;
typedef std::vector<CLandBlock *> LandblockVector;
typedef std::map<WORD, DungeonDesc_t> DungeonDescMap;

//
class CWorld
{
public:
	CWorld();
	~CWorld();

	void CreateEntity(CPhysicsObj*);
	void InsertEntity(CPhysicsObj*, BOOL bSilent = FALSE);
	void Test();

	CLandBlock* ActivateBlock(WORD wHeader);

	void RemoveEntity(CPhysicsObj*);

	void Think();

	void BroadcastPVS(DWORD dwCell, void *_data, DWORD _len, WORD _group = OBJECT_MSG, DWORD ignore_ent = 0, BOOL _game_event = 0);
	void BroadcastGlobal(void *_data, DWORD _len, WORD _group, DWORD ignore_ent = 0, BOOL _game_event = 0);
	void BroadcastGlobal(NetFood *food, WORD _group, DWORD ignore_ent = 0, BOOL _game_event = 0, BOOL del = 1);

	CLandBlock* GetLandblock(WORD wHeader);
	loc_t FindDungeonDrop();
	loc_t FindDungeonDrop(WORD wBlockID);
	LocationMap* GetDungeons();

	BOOL DungeonExists(WORD wBlockID);
	DungeonDescMap* GetDungeonDescs();
	DungeonDesc_t* GetDungeonDesc(WORD wBlockID);
	DungeonDesc_t* GetDungeonDesc(const char* szDungeonName);
	void SetDungeonDesc(WORD wBlockID, const char* szDungeonName, const char* szAuthor, const char* szDescription, loc_t origin, heading_t angles);

	//
	void JuggleEntity(WORD, CPhysicsObj* pEntity);

	//
	PlayerMap* GetPlayers();
	CBasePlayer* FindPlayer(DWORD);
	CBasePlayer* FindPlayer(const char *);
	CPhysicsObj* FindWithinPVS(CPhysicsObj *pSource, DWORD dwGUID);

	//
	DWORD GenerateGUID(eGUIDClass);

	void SaveWorld();

	const char* GetMOTD();

private:


	void LoadStateFile();
	void SaveStateFile();

	void LoadDungeonsFile();
	void SaveDungeonsFile();

	void LoadMOTD();

	void InitializeHintGUIDs();

	void EnumerateDungeonsFromCellData();

	CLandBlock* m_pBlocks[256 * 256]; // 256 x 256 array of landblocks.
	LandblockVector m_vBlocks; // Vector of active landblocks.
	LandblockVector m_vSpawns; // Vector of spawned landblocks.

	PlayerMap m_mAllPlayers; // Global list of players.
	LocationMap m_mDungeons; // Global list of dungeons.
	DungeonDescMap m_mDungeonDescs; // Dungeon Descriptors

	DWORD m_dwHintStaticGUID;
	DWORD m_dwHintPlayerGUID;
	DWORD m_dwHintItemGUID;
	DWORD m_dwHintDynamicGUID;

	double m_fLastSave;

	std::string m_strMOTD;
};


