
#include "StdAfx.h"
#include "Database2.h"
#include "World.h"
#include "Portal.h"
#include "PhysicsDesc.h"
#include "PublicWeenieDesc.h"
#include "Door.h"
#include "Lifestone.h"
#include "Item.h"

CMYSQLResult::CMYSQLResult(MYSQL_RES *Result)
{
	m_Result = Result;
}

CMYSQLResult::~CMYSQLResult()
{
	Free();
}

void CMYSQLResult::Free()
{
	if (m_Result)
	{
		mysql_free_result(m_Result);
		m_Result = NULL;
	}
}

SQLResultRow_t CMYSQLResult::FetchRow()
{
	if (!m_Result)
	{
		return NULL;
	}

	return (SQLResultRow_t)mysql_fetch_row(m_Result);
}

uint64_t CMYSQLResult::ResultRows()
{
	if (!m_Result)
	{
		return 0;
	}

	return (uint64_t)mysql_num_rows(m_Result);
}

int CSQLConnection::s_NumConnectAttempts = 0;

CMYSQLConnection::CMYSQLConnection(MYSQL *connection)
{
	m_InternalConnection = connection;
}

CMYSQLConnection::~CMYSQLConnection()
{
	Close();
}

CMYSQLConnection *CMYSQLConnection::Create(const char *host, unsigned int port, const char *user, const char *password, const char *defaultdatabase)
{
	MYSQL *sqlobject = mysql_init(NULL);

	if (!sqlobject)
	{
		return NULL;
	}

	int connect_timeout = MYSQL_CONNECT_TIMEOUT;
	mysql_options(sqlobject, MYSQL_OPT_CONNECT_TIMEOUT, (char *)&connect_timeout);

	MYSQL *sqlconnection;

	{
		s_NumConnectAttempts++;

		CStopWatch connectTiming;

		sqlconnection = mysql_real_connect(sqlobject, host, user, password, defaultdatabase, port, NULL, 0);

		if (sqlconnection && connectTiming.GetElapsed() >= 1.0)
		{
			LOG(Database, Warning, "mysql_real_connect() took %.1f s!\n", connectTiming.GetElapsed());
		}
	}

	if (sqlconnection == NULL)
	{
		if (mysql_errno(sqlobject) == EINTR)
		{
			mysql_close(sqlobject);

			sqlobject = mysql_init(NULL);

			if (!sqlobject)
			{
				return NULL;
			}

			int connect_timeout = MYSQL_CONNECT_TIMEOUT;
			mysql_options(sqlobject, MYSQL_OPT_CONNECT_TIMEOUT, (char *)&connect_timeout);

			s_NumConnectAttempts++;

			CStopWatch connectTiming;

			sqlconnection = mysql_real_connect(sqlobject, host, user, password, defaultdatabase, port, NULL, 0);

			if (sqlconnection && connectTiming.GetElapsed() >= 1.0)
			{
				LOG(Database, Warning, "mysql_real_connect() re-attempt took %.1f s!\n", connectTiming.GetElapsed());
			}

			if (sqlconnection == NULL)
			{
				LOG(Database, Warning, "Failed to create mysql connection after two tries:\n%s\n", mysql_error(sqlobject));

				mysql_close(sqlobject);
				return NULL;
			}
			else
			{
				LOG(Database, Normal, "Received EINTR while attempting to connect to mysql, but re-attempt succeeded.\n");
			}
		}
		else
		{
			if (CSQLConnection::s_NumConnectAttempts > 1)
			{
				// Only show warning if not the first connection attempt
				LOG(Database, Warning, "mysql_real_connect() failed:\n%s\n", mysql_error(sqlobject));
			}

			mysql_close(sqlobject);
			return NULL;
		}
	}

	if (sqlconnection)
	{
		// Disable auto-reconnect (probably already disabled)
		sqlconnection->reconnect = 0;

		return new CMYSQLConnection(sqlconnection);
	}

	return NULL;
}

void CMYSQLConnection::Close()
{
	if (m_InternalConnection)
	{
		mysql_close(m_InternalConnection);
		m_InternalConnection = NULL;
	}
}

bool CMYSQLConnection::Query(const char *query)
{
	if (!m_InternalConnection)
	{
		return false;
	}

	int errorCode;

	{
		CStopWatch queryTiming;

		errorCode = mysql_query(m_InternalConnection, query);

		if (queryTiming.GetElapsed() >= 1.0)
		{
			LOG(Database, Warning, "MYSQL query \"%s\" took %f seconds.\n", query, queryTiming.GetElapsed());
		}
	}

	if (errorCode != 0)
	{
		LOG(Database, Error, "MYSQL query errored %d for \"%s\"\n", errorCode, query);
		return false;
	}

	return true;
}

unsigned int CMYSQLConnection::LastError()
{
	return m_InternalConnection ? (unsigned int)mysql_errno(m_InternalConnection) : 0;
}

CSQLResult *CMYSQLConnection::GetResult()
{
	if (!m_InternalConnection)
	{
		return NULL;
	}

	MYSQL_RES *Result = mysql_store_result(m_InternalConnection);

	if (!Result)
	{
		return NULL;
	}

	return new CMYSQLResult(Result);
}

CDatabase2::CDatabase2()
{
	m_pConnection = NULL;
}

CDatabase2::~CDatabase2()
{
	if (m_pConnection != nullptr)
	{
		m_pConnection->Close();
		delete m_pConnection;
	}
}

void CDatabase2::Tick()
{
}

bool CDatabase2::Query(const char *format, ...)
{
	if (!m_pConnection)
	{
		return false;
	}

	bool success = false;

	va_list args;
	va_start(args, format);

	int charCount = _vscprintf(format, args) + 1;

	char *charBuffer = new char[charCount];
	charBuffer[0] = '\0';
	_vsnprintf(charBuffer, charCount, format, args);
	charBuffer[charCount - 1] = '\0';

	if (m_pConnection)
	{
		success = m_pConnection->Query(charBuffer);
	}

	delete[] charBuffer;

	va_end(args);

	return success;
}

CSQLResult *CDatabase2::GetResult()
{
	if (!m_pConnection)
	{
		return NULL;
	}

	return m_pConnection->GetResult();
}

CMYSQLDatabase::CMYSQLDatabase(const char *host, unsigned int port, const char *user, const char *password, const char *defaultdatabasename)
{
	m_DatabaseHost = host;
	m_DatabasePort = port;
	m_DatabaseUser = user;
	m_DatabasePassword = password;
	m_DatabaseName = defaultdatabasename;
	m_bDisabled = false;

	RefreshConnection();

	if (!m_pConnection)
	{
		// If we can't connect the first time, just disable this feature.
		LOG(Database, Warning, "MySQL database functionality disabled.\n");
		m_bDisabled = true;
	}
}

CMYSQLDatabase::~CMYSQLDatabase()
{
}

void CMYSQLDatabase::RefreshConnection()
{
	// If we have an old connection, close that one.
	if (m_pConnection)
	{
		delete m_pConnection;
		m_pConnection = NULL;
	}

	m_pConnection = CMYSQLConnection::Create(m_DatabaseHost.c_str(), m_DatabasePort, m_DatabaseUser.c_str(), m_DatabasePassword.c_str(), m_DatabaseName.c_str());
	m_fLastRefresh = g_pGlobals->Time();
}

void CMYSQLDatabase::Tick()
{
	if (m_fLastRefresh < (g_pGlobals->Time() + 60.0))
	{
		// Refresh connection every 60 seconds to avoid stale connections.
		RefreshConnection();
	}
}

CGameDatabase::CGameDatabase()
{
	m_bLoadedPortals = false;
}

CGameDatabase::~CGameDatabase()
{
	for (auto &data : m_CapturedAerfalleData)
	{
		delete data;
	}

	for (auto &data : m_CapturedMonsterData)
	{
		delete data.second;
	}

	for (auto &data : m_CapturedItemData)
	{
		delete data.second;
	}

	for (auto &data : m_CapturedStaticsData)
	{
		delete data;
	}

	for (auto &data : m_CapturedArmorData)
	{
		delete data.second;
	}
}

void CGameDatabase::Init()
{
	// Loads from MYSQL database
	LoadPortals();
	LoadTeleTownList();
	LoadMonsterTemplates();

	// Load data files for now
	LoadCapturedMonsterData();
	LoadCapturedItemData();
	LoadCapturedArmorData();
	LoadStaticsData();

	LoadAerfalle();
}

void CGameDatabase::SpawnAerfalle()
{
	for (auto pSpawnInfo : m_CapturedAerfalleData)
	{
		CPhysicsObj *pSpawn;

		if (pSpawnInfo->weenie._bitfield & BitfieldIndex::BF_CORPSE)
			continue;
		if (pSpawnInfo->weenie._bitfield & BitfieldIndex::BF_PLAYER)
			continue;
		if (pSpawnInfo->physics.state & PhysicsState::MISSILE_PS)
			continue;

		if (pSpawnInfo->weenie._bitfield & BitfieldIndex::BF_DOOR)
		{
			pSpawn = new CBaseDoor();
			pSpawn->m_PhysicsState = pSpawnInfo->physics.state;
			pSpawn->m_WeenieBitfield = pSpawnInfo->weenie._bitfield;
		}
		else if (pSpawnInfo->weenie._type == ITEM_TYPE::TYPE_CREATURE)
		{
			pSpawn = new CBaseMonster();
			pSpawn->m_PhysicsState = pSpawnInfo->physics.state;
			pSpawn->m_WeenieBitfield = pSpawnInfo->weenie._bitfield;
		}
		else
		{
			pSpawn = new CPhysicsObj();
			pSpawn->m_PhysicsState = pSpawnInfo->physics.state;
			pSpawn->m_WeenieBitfield = pSpawnInfo->weenie._bitfield;
		}

		pSpawn->m_dwGUID = pSpawnInfo->guid; // g_pWorld->GenerateGUID(eDynamicGUID);
		pSpawn->m_miBaseModel = pSpawnInfo->appearance;
		pSpawn->m_dwModel = pSpawnInfo->physics.setup_id;
		pSpawn->m_dwAnimationSet = pSpawnInfo->physics.mtable_id;
		pSpawn->m_dwSoundSet = pSpawnInfo->physics.stable_id;
		pSpawn->m_dwEffectSet = pSpawnInfo->physics.phstable_id;
		pSpawn->m_fScale = pSpawnInfo->physics.object_scale;
		pSpawn->m_ItemType = pSpawnInfo->weenie._type;
		pSpawn->m_strName = pSpawnInfo->weenie._name;

		pSpawn->m_Origin.landcell = pSpawnInfo->physics.pos.objcell_id;
		pSpawn->m_Origin.x = pSpawnInfo->physics.pos.frame.m_fOrigin.x;
		pSpawn->m_Origin.y = pSpawnInfo->physics.pos.frame.m_fOrigin.y;
		pSpawn->m_Origin.z = pSpawnInfo->physics.pos.frame.m_fOrigin.z;
		pSpawn->m_Angles.w = pSpawnInfo->physics.pos.frame.qw;
		pSpawn->m_Angles.x = pSpawnInfo->physics.pos.frame.qx;
		pSpawn->m_Angles.y = pSpawnInfo->physics.pos.frame.qy;
		pSpawn->m_Angles.z = pSpawnInfo->physics.pos.frame.qz;

		if (pSpawnInfo->physics.movement_buffer)
		{
			pSpawn->m_AnimOverrideData = pSpawnInfo->physics.movement_buffer;
			pSpawn->m_AnimOverrideDataLen = pSpawnInfo->physics.movement_buffer_length;
			pSpawn->m_AutonomousMovement = pSpawnInfo->physics.autonomous_movement;
		}

		pSpawn->m_bDontClear = true;

		g_pWorld->CreateEntity(pSpawn);
	}
}

void CGameDatabase::LoadAerfalle()
{
	// TEMPORARY FOR TESTING DUNGEONS
	// Consolidate or remove this method of loading (this was copied and pasted)

	BYTE *data;
	DWORD length;

	if (LoadDataFromFile("aerfalle.dat", &data, &length))
	{
		BinaryReader reader(data, length);
		unsigned int count = reader.ReadDWORD();

		for (unsigned int i = 0; i < count; i++)
		{
			reader.ReadString();

			CCapturedWorldObjectInfo *pObjectInfo = new CCapturedWorldObjectInfo;

			DWORD a = reader.ReadDWORD(); // size of this
			DWORD b = reader.ReadDWORD(); // 0xF745
			pObjectInfo->guid = reader.ReadDWORD(); // GUID

			reader.ReadBYTE();
			BYTE numPalette = reader.ReadBYTE();
			BYTE numTex = reader.ReadBYTE();
			BYTE numModel = reader.ReadBYTE();

			if (numPalette)
			{
				pObjectInfo->appearance.dwBasePalette = reader.ReadPackedDWORD();
				for (int j = 0; j < numPalette; j++)
				{
					DWORD replacement = reader.ReadPackedDWORD();
					BYTE offset = reader.ReadBYTE();
					BYTE range = reader.ReadBYTE();
					pObjectInfo->appearance.lPalettes.push_back(PaletteRpl(replacement, offset, range));
				}
			}

			for (int j = 0; j < numTex; j++)
			{
				BYTE index = reader.ReadBYTE();
				DWORD oldT = reader.ReadPackedDWORD();
				DWORD newT = reader.ReadPackedDWORD();
				pObjectInfo->appearance.lTextures.push_back(TextureRpl(index, oldT, newT));
			}

			for (int j = 0; j < numModel; j++)
			{
				BYTE index = reader.ReadBYTE();
				DWORD newM = reader.ReadPackedDWORD();
				pObjectInfo->appearance.lModels.push_back(ModelRpl(index, newM));
			}

			reader.ReadAlign();

			pObjectInfo->physics.Unpack(reader);
			pObjectInfo->weenie.Unpack(reader);

			if (!reader.GetLastError())
			{
				m_CapturedAerfalleData.push_back(pObjectInfo);
			}
			else
			{
				DEBUG_BREAK();
				delete pObjectInfo;

				break;
			}
		}

		delete [] data;
	}
}

std::string CGameDatabase::ConvertNameForLookup(std::string name)
{
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	name.erase(remove_if(name.begin(), name.end(), isspace), name.end());
	return name;
}

void CGameDatabase::LoadStaticsData()
{
	// THIS IS ALL TEMPORARY FOR TESTING
	unsigned int spawned = 0;

	BYTE *data;
	DWORD length;

	if (LoadDataFromFile("statics.dat", &data, &length))
	{
		BinaryReader reader(data, length);
		unsigned int count = reader.ReadDWORD();

		for (unsigned int i = 0; i < count; i++)
		{
			CCapturedWorldObjectInfo *pObjectInfo = new CCapturedWorldObjectInfo;
			
			reader.ReadString();
			DWORD dwObjDataLen = reader.ReadDWORD(); // size of this

			reader.ReadDWORD(); // 0xf745
			pObjectInfo->guid = reader.ReadDWORD(); // GUID

			reader.ReadBYTE();
			BYTE numPalette = reader.ReadBYTE();
			BYTE numTex = reader.ReadBYTE();
			BYTE numModel = reader.ReadBYTE();

			if (numPalette)
			{
				pObjectInfo->appearance.dwBasePalette = reader.ReadPackedDWORD(); // actually packed, fix this
				for (int j = 0; j < numPalette; j++)
				{
					DWORD replacement = reader.ReadPackedDWORD(); // actually packed, fix this
					BYTE offset = reader.ReadBYTE();
					BYTE range = reader.ReadBYTE();
					pObjectInfo->appearance.lPalettes.push_back(PaletteRpl(replacement, offset, range));
				}
			}

			for (int j = 0; j < numTex; j++)
			{
				BYTE index = reader.ReadBYTE();
				DWORD oldT = reader.ReadPackedDWORD();
				DWORD newT = reader.ReadPackedDWORD();
				pObjectInfo->appearance.lTextures.push_back(TextureRpl(index, oldT, newT));
			}


			for (int j = 0; j < numModel; j++)
			{
				BYTE index = reader.ReadBYTE();
				DWORD newM = reader.ReadPackedDWORD();
				pObjectInfo->appearance.lModels.push_back(ModelRpl(index, newM));
			}

			reader.ReadAlign();

			pObjectInfo->physics.Unpack(reader);
			pObjectInfo->weenie.Unpack(reader);

			if (reader.GetLastError())
			{
				DEBUG_BREAK();
				delete pObjectInfo;

				break;
			}

			DWORD dwIDDataLen = reader.ReadDWORD();
			DWORD offsetStart = reader.GetOffset();

			if (dwIDDataLen > 0)
			{
				BinaryReader idReader(reader.GetDataPtr(), dwIDDataLen);

				idReader.ReadDWORD(); // 0xf7b0
				idReader.ReadDWORD(); // character
				idReader.ReadDWORD(); // sequence
				idReader.ReadDWORD(); // game event (0xc9)

				idReader.ReadDWORD(); // object
				DWORD flags = idReader.ReadDWORD(); // flags
				idReader.ReadDWORD(); // success

				enum AppraisalProfilePackHeader {
					Packed_None = 0,
					Packed_IntStats = (1 << 0),
					Packed_BoolStats = (1 << 1),
					Packed_FloatStats = (1 << 2),
					Packed_StringStats = (1 << 3),
					Packed_SpellList = (1 << 4),
					Packed_WeaponProfile = (1 << 5),
					Packed_HookProfile = (1 << 6),
					Packed_ArmorProfile = (1 << 7),
					Packed_CreatureProfile = (1 << 8),
					Packed_ArmorEnchant = (1 << 9),
					Packed_ResistEnchant = (1 << 10),
					Packed_WeaponEnchant = (1 << 11),
					Packed_DataIDStats = (1 << 12),
					Packed_Int64Stats = (1 << 13),
					Packed_ArmorLevels = (1 << 14)
				};

				if (flags & Packed_IntStats)
				{
					pObjectInfo->dwordProperties = idReader.ReadMap<DWORD, DWORD>();
				}

				if (flags & Packed_Int64Stats)
					pObjectInfo->qwordProperties = idReader.ReadMap<DWORD, UINT64>();

				if (flags & Packed_BoolStats)
					pObjectInfo->boolProperties = idReader.ReadMap<DWORD, DWORD>();

				if (flags & Packed_FloatStats)
					pObjectInfo->floatProperties = idReader.ReadMap<DWORD, double>();

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}
			}			

			reader.SetOffset(offsetStart + dwIDDataLen);

			m_CapturedStaticsData.push_back(pObjectInfo);
		}

		delete [] data;
	}

	for (auto pSpawnInfo : m_CapturedStaticsData)
	{
		CPhysicsObj *pSpawn;

		if (pSpawnInfo->guid >= 0x80000000)
			continue; // Dynamic perhaps

		if (pSpawnInfo->weenie._bitfield & BitfieldIndex::BF_CORPSE)
			continue;
		if (pSpawnInfo->weenie._bitfield & BitfieldIndex::BF_PLAYER)
			continue;
		if (pSpawnInfo->physics.state & PhysicsState::MISSILE_PS)
			continue;

		if (pSpawnInfo->weenie._bitfield & BitfieldIndex::BF_DOOR)
			pSpawn = new CBaseDoor();
		else if (pSpawnInfo->weenie._type == ITEM_TYPE::TYPE_CREATURE)
			pSpawn = new CBaseMonster();
		else if (pSpawnInfo->weenie._type == ITEM_TYPE::TYPE_PORTAL)
			pSpawn = new CPortal();
		else if (pSpawnInfo->weenie._type == ITEM_TYPE::TYPE_LIFESTONE)
			pSpawn = new CBaseLifestone();
		else
			pSpawn = new CPhysicsObj();

		pSpawn->m_dwGUID = pSpawnInfo->guid; // g_pWorld->GenerateGUID(eDynamicGUID);
		pSpawn->m_miBaseModel = pSpawnInfo->appearance;
		pSpawn->m_dwModel = pSpawnInfo->physics.setup_id;
		pSpawn->m_dwAnimationSet = pSpawnInfo->physics.mtable_id;
		pSpawn->m_dwSoundSet = pSpawnInfo->physics.stable_id;
		pSpawn->m_dwEffectSet = pSpawnInfo->physics.phstable_id;
		pSpawn->m_fScale = pSpawnInfo->physics.object_scale;
		pSpawn->m_ItemType = pSpawnInfo->weenie._type;
		pSpawn->m_strName = pSpawnInfo->weenie._name;

		pSpawn->m_Origin.landcell = pSpawnInfo->physics.pos.objcell_id;
		pSpawn->m_Origin.x = pSpawnInfo->physics.pos.frame.m_fOrigin.x;
		pSpawn->m_Origin.y = pSpawnInfo->physics.pos.frame.m_fOrigin.y;
		pSpawn->m_Origin.z = pSpawnInfo->physics.pos.frame.m_fOrigin.z;
		pSpawn->m_Angles.w = pSpawnInfo->physics.pos.frame.qw;
		pSpawn->m_Angles.x = pSpawnInfo->physics.pos.frame.qx;
		pSpawn->m_Angles.y = pSpawnInfo->physics.pos.frame.qy;
		pSpawn->m_Angles.z = pSpawnInfo->physics.pos.frame.qz;

		if (pSpawnInfo->physics.movement_buffer)
		{
			pSpawn->m_AnimOverrideData = pSpawnInfo->physics.movement_buffer;
			pSpawn->m_AnimOverrideDataLen = pSpawnInfo->physics.movement_buffer_length;
			pSpawn->m_AutonomousMovement = pSpawnInfo->physics.autonomous_movement;
		}

		pSpawn->m_bDontClear = true;
		pSpawn->m_RadarVis = pSpawnInfo->weenie._radar_enum;
		pSpawn->m_BlipColor = pSpawnInfo->weenie._blipColor;
		pSpawn->m_TargetType = pSpawnInfo->weenie._targetType;
		pSpawn->m_wTypeID = pSpawnInfo->weenie._wcid;
		pSpawn->m_Value = pSpawnInfo->weenie._value;
		pSpawn->m_Burden = pSpawnInfo->weenie._burden;
		pSpawn->m_PhysicsState = pSpawnInfo->physics.state;
		pSpawn->m_Translucency = pSpawnInfo->physics.translucency;
		pSpawn->m_WeenieBitfield = pSpawnInfo->weenie._bitfield;

		g_pWorld->CreateEntity(pSpawn);

		spawned++;
	}
	
	if (!spawned)
	{
		LOG(Temp, Warning, "Spawn data not included. Spawning functionality may be limited.\n");
	}
}

CCapturedWorldObjectInfo *CGameDatabase::GetRandomCapturedMonsterData()
{
	DWORD numMonsters = m_CapturedMonsterDataList.size();
	if (numMonsters < 1)
		return NULL;

	int monster = RandomLong(0, numMonsters - 1);

	return m_CapturedMonsterDataList[monster];
}

CCapturedWorldObjectInfo *CGameDatabase::GetCapturedMonsterData(const char *name)
{
	std::map<std::string, CCapturedWorldObjectInfo *>::iterator i = m_CapturedMonsterData.find(ConvertNameForLookup(name));

	if (i == m_CapturedMonsterData.end())
		return NULL;

	return i->second;
}

void CGameDatabase::LoadCapturedMonsterData()
{
	// THIS IS ALL TEMPORARY FOR TESTING
	// Consolidate or remove this method of loading (this was copied and pasted)

	BYTE *data;
	DWORD length;

	if (LoadDataFromFile("monsters.dat", &data, &length))
	{
		BinaryReader reader(data, length);
		unsigned int count = reader.ReadDWORD();

		for (unsigned int i = 0; i < count; i++)
		{
			CCapturedWorldObjectInfo *pMonsterInfo = new CCapturedWorldObjectInfo;
			pMonsterInfo->m_ObjName = reader.ReadString();

			DWORD dwObjDataLen = reader.ReadDWORD(); // size of this

			reader.ReadDWORD(); // 0xf745
			DWORD dwGUID = reader.ReadDWORD(); // GUID

			reader.ReadBYTE();
			BYTE numPalette = reader.ReadBYTE();
			BYTE numTex = reader.ReadBYTE();
			BYTE numModel = reader.ReadBYTE();

			if (numPalette)
			{
				pMonsterInfo->appearance.dwBasePalette = reader.ReadPackedDWORD(); // actually packed, fix this
				for (int j = 0; j < numPalette; j++)
				{
					DWORD replacement = reader.ReadPackedDWORD(); // actually packed, fix this
					BYTE offset = reader.ReadBYTE();
					BYTE range = reader.ReadBYTE();
					pMonsterInfo->appearance.lPalettes.push_back(PaletteRpl(replacement, offset, range));
				}
			}

			for (int j = 0; j < numTex; j++)
			{
				BYTE index = reader.ReadBYTE();
				DWORD oldT = reader.ReadPackedDWORD();
				DWORD newT = reader.ReadPackedDWORD();
				pMonsterInfo->appearance.lTextures.push_back(TextureRpl(index, oldT, newT));
			}


			for (int j = 0; j < numModel; j++)
			{
				BYTE index = reader.ReadBYTE();
				DWORD newM = reader.ReadPackedDWORD();
				pMonsterInfo->appearance.lModels.push_back(ModelRpl(index, newM));
			}

			reader.ReadAlign();

			pMonsterInfo->physics.Unpack(reader);
			pMonsterInfo->weenie.Unpack(reader);

			if (reader.GetLastError())
			{
				DEBUG_BREAK();
				delete pMonsterInfo;
				break;
			}

			DWORD dwIDDataLen = reader.ReadDWORD();
			DWORD offsetStart = reader.GetOffset();

			if (dwIDDataLen > 0)
			{
				BinaryReader idReader(reader.GetDataPtr(), dwIDDataLen);

				idReader.ReadDWORD(); // 0xf7b0
				idReader.ReadDWORD(); // character
				idReader.ReadDWORD(); // sequence
				idReader.ReadDWORD(); // game event (0xc9)

				idReader.ReadDWORD(); // object
				DWORD flags = idReader.ReadDWORD(); // flags
				idReader.ReadDWORD(); // success

				enum AppraisalProfilePackHeader {
					Packed_None = 0,
					Packed_IntStats = (1 << 0),
					Packed_BoolStats = (1 << 1),
					Packed_FloatStats = (1 << 2),
					Packed_StringStats = (1 << 3),
					Packed_SpellList = (1 << 4),
					Packed_WeaponProfile = (1 << 5),
					Packed_HookProfile = (1 << 6),
					Packed_ArmorProfile = (1 << 7),
					Packed_CreatureProfile = (1 << 8),
					Packed_ArmorEnchant = (1 << 9),
					Packed_ResistEnchant = (1 << 10),
					Packed_WeaponEnchant = (1 << 11),
					Packed_DataIDStats = (1 << 12),
					Packed_Int64Stats = (1 << 13),
					Packed_ArmorLevels = (1 << 14)
				};

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_IntStats)
				{
					pMonsterInfo->dwordProperties = idReader.ReadMap<DWORD, DWORD>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_Int64Stats)
				{
					pMonsterInfo->qwordProperties = idReader.ReadMap<DWORD, UINT64>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_BoolStats)
				{
					pMonsterInfo->boolProperties = idReader.ReadMap<DWORD, DWORD>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_FloatStats)
				{
					pMonsterInfo->floatProperties = idReader.ReadMap<DWORD, double>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				/* ... etc ...
				if (flags & Packed_StringStats)
					pMonsterInfo->stringProperties = idReader.ReadMap<DWORD>();

				if (flags & Packed_DataIDStats)
					pMonsterInfo->dataIDProperties = idReader.ReadMap<DWORD, DWORD>();
					*/
			}

			reader.SetOffset(offsetStart + dwIDDataLen);

			m_CapturedMonsterData.insert(std::pair<std::string, CCapturedWorldObjectInfo*>(ConvertNameForLookup(pMonsterInfo->m_ObjName), pMonsterInfo));

			if (!(pMonsterInfo->weenie._bitfield & BitfieldIndex::BF_PLAYER))
			{
				m_CapturedMonsterDataList.push_back(pMonsterInfo);
			}
		}

		delete [] data;
	}
}

CCapturedWorldObjectInfo *CGameDatabase::GetRandomCapturedItemData()
{
	DWORD numItems = m_CapturedItemDataList.size();

	if (numItems < 1)
	{
		return NULL;
	}

	int Item = RandomLong(0, numItems - 1);

	return m_CapturedItemDataList[Item];
}

CCapturedWorldObjectInfo *CGameDatabase::GetCapturedItemData(const char *name)
{
	std::map<std::string, CCapturedWorldObjectInfo *>::iterator i = m_CapturedItemData.find(ConvertNameForLookup(name));

	if (i == m_CapturedItemData.end())
		return NULL;

	return i->second;
}

void CGameDatabase::LoadCapturedItemData()
{
	// THIS IS ALL TEMPORARY FOR TESTING
	// Consolidate or remove this method of loading (this was copied and pasted)

	BYTE *data;
	DWORD length;

	if (LoadDataFromFile("items.dat", &data, &length))
	{
		BinaryReader reader(data, length);
		unsigned int count = reader.ReadDWORD();

		for (unsigned int i = 0; i < count; i++)
		{
			CCapturedWorldObjectInfo *pItemInfo = new CCapturedWorldObjectInfo;
			pItemInfo->m_ObjName = reader.ReadString();

			std::transform(pItemInfo->m_ObjName.begin(), pItemInfo->m_ObjName.end(), pItemInfo->m_ObjName.begin(), ::tolower);
			pItemInfo->m_ObjName.erase(remove_if(pItemInfo->m_ObjName.begin(), pItemInfo->m_ObjName.end(), isspace), pItemInfo->m_ObjName.end());

			// make all lowercase and ignore spaces for searching purposes

			DWORD dwObjDataLen = reader.ReadDWORD(); // size of this
			DWORD offset_start = reader.GetOffset();

			reader.ReadDWORD(); // 0xf745
			DWORD dwGUID = reader.ReadDWORD(); // GUID

			reader.ReadBYTE();
			BYTE numPalette = reader.ReadBYTE();
			BYTE numTex = reader.ReadBYTE();
			BYTE numModel = reader.ReadBYTE();

			if (numPalette)
			{
				pItemInfo->appearance.dwBasePalette = reader.ReadPackedDWORD(); // actually packed, fix this
				for (int j = 0; j < numPalette; j++)
				{
					DWORD replacement = reader.ReadPackedDWORD(); // actually packed, fix this
					BYTE offset = reader.ReadBYTE();
					BYTE range = reader.ReadBYTE();
					pItemInfo->appearance.lPalettes.push_back(PaletteRpl(replacement, offset, range));
				}
			}

			for (int j = 0; j < numTex; j++)
			{
				BYTE index = reader.ReadBYTE();
				DWORD oldT = reader.ReadPackedDWORD();
				DWORD newT = reader.ReadPackedDWORD();
				pItemInfo->appearance.lTextures.push_back(TextureRpl(index, oldT, newT));
			}


			for (int j = 0; j < numModel; j++)
			{
				BYTE index = reader.ReadBYTE();
				DWORD newM = reader.ReadPackedDWORD();
				pItemInfo->appearance.lModels.push_back(ModelRpl(index, newM));
			}

			reader.ReadAlign();

			pItemInfo->physics.Unpack(reader);
			pItemInfo->weenie.Unpack(reader);

			if (reader.GetLastError())
			{
				delete pItemInfo;
				break;
			}

			if (reader.GetOffset() != (offset_start + dwObjDataLen))
			{
				DEBUG_BREAK();
			}

			DWORD dwIDDataLen = reader.ReadDWORD();
			DWORD offsetStart = reader.GetOffset();

			if (dwIDDataLen > 0)
			{
				BinaryReader idReader(reader.GetDataPtr(), dwIDDataLen);

				idReader.ReadDWORD(); // 0xf7b0
				idReader.ReadDWORD(); // character
				idReader.ReadDWORD(); // sequence
				idReader.ReadDWORD(); // game event (0xc9)

				idReader.ReadDWORD(); // object
				DWORD flags = idReader.ReadDWORD(); // flags
				idReader.ReadDWORD(); // success

				enum AppraisalProfilePackHeader {
					Packed_None = 0,
					Packed_IntStats = (1 << 0),
					Packed_BoolStats = (1 << 1),
					Packed_FloatStats = (1 << 2),
					Packed_StringStats = (1 << 3),
					Packed_SpellList = (1 << 4),
					Packed_WeaponProfile = (1 << 5),
					Packed_HookProfile = (1 << 6),
					Packed_ArmorProfile = (1 << 7),
					Packed_CreatureProfile = (1 << 8),
					Packed_ArmorEnchant = (1 << 9),
					Packed_ResistEnchant = (1 << 10),
					Packed_WeaponEnchant = (1 << 11),
					Packed_DataIDStats = (1 << 12),
					Packed_Int64Stats = (1 << 13),
					Packed_ArmorLevels = (1 << 14)
				};

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_IntStats)
				{
					pItemInfo->dwordProperties = idReader.ReadMap<DWORD, DWORD>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_Int64Stats)
				{
					pItemInfo->qwordProperties = idReader.ReadMap<DWORD, UINT64>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_BoolStats)
				{
					pItemInfo->boolProperties = idReader.ReadMap<DWORD, DWORD>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_FloatStats)
				{
					pItemInfo->floatProperties = idReader.ReadMap<DWORD, double>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				/*
				if (flags & Packed_StringStats)
				pItemInfo->stringProperties = idReader.ReadMap<DWORD>();

				if (flags & Packed_DataIDStats)
				pItemInfo->dataIDProperties = idReader.ReadMap<DWORD, DWORD>();
				*/
			}

			reader.SetOffset(offsetStart + dwIDDataLen);

			m_CapturedItemData.insert(std::pair<std::string, CCapturedWorldObjectInfo*>(pItemInfo->m_ObjName, pItemInfo));

			if (!(pItemInfo->weenie._bitfield & BitfieldIndex::BF_PLAYER))
			{
				m_CapturedItemDataList.push_back(pItemInfo);
			}
		}

		delete [] data;
	}
}


CCapturedWorldObjectInfo *CGameDatabase::GetRandomCapturedArmorData()
{
	DWORD numItems = m_CapturedArmorDataList.size();
	if (numItems < 1)
		return NULL;

	int Item = RandomLong(0, numItems - 1);
	return m_CapturedArmorDataList[Item];
}

CCapturedWorldObjectInfo *CGameDatabase::GetCapturedArmorData(const char *name)
{
	std::string searchName = name;
	std::transform(searchName.begin(), searchName.end(), searchName.begin(), ::tolower);
	searchName.erase(remove_if(searchName.begin(), searchName.end(), isspace), searchName.end());

	std::map<std::string, CCapturedWorldObjectInfo *>::iterator i = m_CapturedArmorData.find(searchName);

	if (i == m_CapturedArmorData.end())
		return NULL;

	return i->second;
}

void CGameDatabase::LoadCapturedArmorData()
{
	// THIS IS ALL TEMPORARY FOR TESTING
	// Consolidate or remove this method of loading (this was copied and pasted)

	BYTE *data;
	DWORD length;

	if (LoadDataFromFile("armor.dat", &data, &length))
	{
		BinaryReader reader(data, length);
		unsigned int count = reader.ReadDWORD();

		for (unsigned int i = 0; i < count; i++)
		{
			CCapturedWorldObjectInfo *pItemInfo = new CCapturedWorldObjectInfo;
			pItemInfo->m_ObjName = reader.ReadString();

			std::transform(pItemInfo->m_ObjName.begin(), pItemInfo->m_ObjName.end(), pItemInfo->m_ObjName.begin(), ::tolower);
			pItemInfo->m_ObjName.erase(remove_if(pItemInfo->m_ObjName.begin(), pItemInfo->m_ObjName.end(), isspace), pItemInfo->m_ObjName.end());

			// make all lowercase and ignore spaces for searching purposes

			DWORD dwObjDataLen = reader.ReadDWORD(); // size of this
			DWORD offset_start = reader.GetOffset();

			reader.ReadDWORD(); // 0xf745
			DWORD dwGUID = reader.ReadDWORD(); // GUID

			reader.ReadBYTE();
			BYTE numPalette = reader.ReadBYTE();
			BYTE numTex = reader.ReadBYTE();
			BYTE numModel = reader.ReadBYTE();

			if (numPalette)
			{
				pItemInfo->appearance.dwBasePalette = reader.ReadPackedDWORD(); // actually packed, fix this
				for (int j = 0; j < numPalette; j++)
				{
					DWORD replacement = reader.ReadPackedDWORD(); // actually packed, fix this
					BYTE offset = reader.ReadBYTE();
					BYTE range = reader.ReadBYTE();
					pItemInfo->appearance.lPalettes.push_back(PaletteRpl(replacement, offset, range));
				}
			}

			for (int j = 0; j < numTex; j++)
			{
				BYTE index = reader.ReadBYTE();
				DWORD oldT = reader.ReadPackedDWORD();
				DWORD newT = reader.ReadPackedDWORD();
				pItemInfo->appearance.lTextures.push_back(TextureRpl(index, oldT, newT));
			}


			for (int j = 0; j < numModel; j++)
			{
				BYTE index = reader.ReadBYTE();
				DWORD newM = reader.ReadPackedDWORD();
				pItemInfo->appearance.lModels.push_back(ModelRpl(index, newM));
			}

			reader.ReadAlign();

			pItemInfo->physics.Unpack(reader);
			pItemInfo->weenie.Unpack(reader);

			if (reader.GetLastError())
			{
				delete pItemInfo;
				break;
			}

			if (reader.GetOffset() != (offset_start + dwObjDataLen))
			{
				DEBUG_BREAK();
			}

			DWORD dwIDDataLen = reader.ReadDWORD();
			DWORD offsetStart = reader.GetOffset();

			if (dwIDDataLen > 0)
			{
				BinaryReader idReader(reader.GetDataPtr(), dwIDDataLen);

				idReader.ReadDWORD(); // 0xf7b0
				idReader.ReadDWORD(); // character
				idReader.ReadDWORD(); // sequence
				idReader.ReadDWORD(); // game event (0xc9)

				idReader.ReadDWORD(); // object
				DWORD flags = idReader.ReadDWORD(); // flags
				idReader.ReadDWORD(); // success

				enum AppraisalProfilePackHeader {
					Packed_None = 0,
					Packed_IntStats = (1 << 0),
					Packed_BoolStats = (1 << 1),
					Packed_FloatStats = (1 << 2),
					Packed_StringStats = (1 << 3),
					Packed_SpellList = (1 << 4),
					Packed_WeaponProfile = (1 << 5),
					Packed_HookProfile = (1 << 6),
					Packed_ArmorProfile = (1 << 7),
					Packed_CreatureProfile = (1 << 8),
					Packed_ArmorEnchant = (1 << 9),
					Packed_ResistEnchant = (1 << 10),
					Packed_WeaponEnchant = (1 << 11),
					Packed_DataIDStats = (1 << 12),
					Packed_Int64Stats = (1 << 13),
					Packed_ArmorLevels = (1 << 14)
				};

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_IntStats)
					pItemInfo->dwordProperties = idReader.ReadMap<DWORD, DWORD>();

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_Int64Stats)
				{
					pItemInfo->qwordProperties = idReader.ReadMap<DWORD, UINT64>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_BoolStats)
				{
					pItemInfo->boolProperties = idReader.ReadMap<DWORD, DWORD>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				if (flags & Packed_FloatStats)
				{
					pItemInfo->floatProperties = idReader.ReadMap<DWORD, double>();
				}

				if (idReader.GetLastError())
				{
					DEBUG_BREAK();
				}

				/* .... etc...
				if (flags & Packed_StringStats)
				pItemInfo->stringProperties = idReader.ReadMap<DWORD>();

				if (flags & Packed_DataIDStats)
				pItemInfo->dataIDProperties = idReader.ReadMap<DWORD, DWORD>();
				*/
			}

			reader.SetOffset(offsetStart + dwIDDataLen);

			DWORD dwWornModelDataLen = reader.ReadDWORD();
			offsetStart = reader.GetOffset();

			{
				reader.ReadBYTE();
				numPalette = reader.ReadBYTE();
				numTex = reader.ReadBYTE();
				numModel = reader.ReadBYTE();

				if (numPalette)
				{
					pItemInfo->wornAppearance.dwBasePalette = reader.ReadPackedDWORD(); // actually packed, fix this
					for (int j = 0; j < numPalette; j++)
					{
						DWORD replacement = reader.ReadPackedDWORD(); // actually packed, fix this
						BYTE offset = reader.ReadBYTE();
						BYTE range = reader.ReadBYTE();
						pItemInfo->wornAppearance.lPalettes.push_back(PaletteRpl(replacement, offset, range));
					}
				}

				for (int j = 0; j < numTex; j++)
				{
					BYTE index = reader.ReadBYTE();
					DWORD oldT = reader.ReadPackedDWORD();
					DWORD newT = reader.ReadPackedDWORD();
					pItemInfo->wornAppearance.lTextures.push_back(TextureRpl(index, oldT, newT));
				}

				for (int j = 0; j < numModel; j++)
				{
					BYTE index = reader.ReadBYTE();
					DWORD newM = reader.ReadPackedDWORD();
					pItemInfo->wornAppearance.lModels.push_back(ModelRpl(index, newM));
				}
			}

			reader.SetOffset(offsetStart + dwWornModelDataLen);

			m_CapturedArmorData.insert(std::pair<std::string, CCapturedWorldObjectInfo*>(pItemInfo->m_ObjName, pItemInfo));
			m_CapturedArmorDataList.push_back(pItemInfo);
		}

		delete [] data;
	}
}

DWORD ParseDWORDFromStringHex(const char *hex)
{
	return strtoul(hex, NULL, 16);
}

float ParseFloatFromStringHex(const char *hex)
{
	DWORD hexVal = ParseDWORDFromStringHex(hex);
	return *((float *)&hexVal);
}

void CGameDatabase::LoadTeleTownList()
{
	bool bQuerySuccess = g_pDB2->Query("SELECT `ID`, `Description`, `Command`, `Landblock`, `Position_X`, `Position_Y`, `Position_Z`, `Orientation_W`, `Orientation_X`, `Orientation_Y`, `Orientation_Z` from tele_locations");

	if (bQuerySuccess)
	{
		CSQLResult *Result = g_pDB2->GetResult();

		if (Result)
		{
			SQLResultRow_t ResultRow;
			while (ResultRow = Result->FetchRow())
			{
				TeleTownList_t t;
				t.m_teleString = ResultRow[2];
				t.loc.landcell = ParseDWORDFromStringHex(ResultRow[3]);
				t.loc.x = ParseFloatFromStringHex(ResultRow[4]);
				t.loc.y = ParseFloatFromStringHex(ResultRow[5]);
				t.loc.z = ParseFloatFromStringHex(ResultRow[6]);
				t.heading.w = ParseFloatFromStringHex(ResultRow[7]);
				t.heading.x = ParseFloatFromStringHex(ResultRow[8]);
				t.heading.y = ParseFloatFromStringHex(ResultRow[9]);
				t.heading.z = ParseFloatFromStringHex(ResultRow[10]);
				g_pWorld->InsertTeleportLocation(t);
			}

			LOG(World, Normal, "Added %d teleport locations.\n", Result->ResultRows());
		}
	}
}
void CGameDatabase::LoadPortals()
{
	// From database Miach provided.
	bool bQuerySuccess = g_pDB2->Query("SELECT name, color, Landblock, Position_X, Position_Y, Position_Z, Orientation_W, Orientation_X, Orientation_Y, Orientation_Z, Dest_Landblock, Dest_Position_X, Dest_Position_Y, Dest_Position_Z, Dest_Orientation_W, Dest_Orientation_X, Dest_Orientation_Y, Dest_Orientation_Z FROM gameobjects_portals ORDER BY ID ASC");

	if (bQuerySuccess)
	{
		CSQLResult *Result = g_pDB2->GetResult();

		if (Result)
		{
			int portalCount = 0;

			SQLResultRow_t ResultRow;
			while (ResultRow = Result->FetchRow())
			{
				CPortal *pPortal = new CPortal();
				pPortal->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);

				pPortal->m_strName = ResultRow[0];
				// ResultRow[1] Color by cmoski
				std::string strColor = ResultRow[1];
				if (strColor == "blue")
					pPortal->m_dwModel = 0x20005D2;
				else if (strColor == "green")
					pPortal->m_dwModel = 0x20005D3;
				else if (strColor == "orange")
					pPortal->m_dwModel = 0x20005D4;
				else if (strColor == "red")
					pPortal->m_dwModel = 0x20005D5;
				else if (strColor == "yellow")
					pPortal->m_dwModel = 0x20005D6;
				else if (strColor == "purple")
					pPortal->m_dwModel = 0x20001B3;
				else if (strColor == "white")
					pPortal->m_dwModel = 0x20001B3;
				else
					pPortal->m_dwModel = 0x20001B3;

				pPortal->m_Origin.landcell = ParseDWORDFromStringHex(ResultRow[2]);
				pPortal->m_Origin.x = ParseFloatFromStringHex(ResultRow[3]);
				pPortal->m_Origin.y = ParseFloatFromStringHex(ResultRow[4]);
				pPortal->m_Origin.z = ParseFloatFromStringHex(ResultRow[5]);
				pPortal->m_Angles.w = ParseFloatFromStringHex(ResultRow[6]);
				pPortal->m_Angles.x = ParseFloatFromStringHex(ResultRow[7]);
				pPortal->m_Angles.y = ParseFloatFromStringHex(ResultRow[8]);
				pPortal->m_Angles.z = ParseFloatFromStringHex(ResultRow[9]);

				pPortal->m_bHasDestination = true;
				pPortal->m_Destination.origin.landcell = ParseDWORDFromStringHex(ResultRow[10]);
				pPortal->m_Destination.origin.x = ParseFloatFromStringHex(ResultRow[11]);
				pPortal->m_Destination.origin.y = ParseFloatFromStringHex(ResultRow[12]);
				pPortal->m_Destination.origin.z = ParseFloatFromStringHex(ResultRow[13]);
				pPortal->m_Destination.angles.w = ParseFloatFromStringHex(ResultRow[14]);
				pPortal->m_Destination.angles.x = ParseFloatFromStringHex(ResultRow[15]);
				pPortal->m_Destination.angles.y = ParseFloatFromStringHex(ResultRow[16]);
				pPortal->m_Destination.angles.z = ParseFloatFromStringHex(ResultRow[17]);

				g_pWorld->CreateEntity(pPortal);
				portalCount++;
			}

			LOG(World, Normal, "Spawned %d portals.\n", portalCount);
			delete Result;

			if (portalCount > 0)
			{
				m_bLoadedPortals = true;
			}
		}
	}
}

void CGameDatabase::LoadMonsterTemplates()
{
}

CPhysicsObj *CGameDatabase::CreateFromCapturedData(CCapturedWorldObjectInfo *pObjectInfo)
{
	if (!pObjectInfo)
	{
		return NULL;
	}

	CPhysicsObj *pObject;
	
	if (pObjectInfo->weenie._type & ITEM_TYPE::TYPE_CREATURE)
	{
		pObject = new CBaseMonster();
	}
	else if (pObjectInfo->weenie._location ||
			(pObjectInfo->weenie._type &
				(ITEM_TYPE::TYPE_ARMOR | ITEM_TYPE::TYPE_CLOTHING | ITEM_TYPE::TYPE_FOOD | ITEM_TYPE::TYPE_JEWELRY |
				ITEM_TYPE::TYPE_KEY | ITEM_TYPE::TYPE_MAGIC_WIELDABLE | ITEM_TYPE::TYPE_MANASTONE | ITEM_TYPE::TYPE_CLOTHING |
				ITEM_TYPE::TYPE_FOOD | ITEM_TYPE::TYPE_JEWELRY | ITEM_TYPE::TYPE_KEY | ITEM_TYPE::TYPE_MAGIC_WIELDABLE |
				ITEM_TYPE::TYPE_MANASTONE | ITEM_TYPE::TYPE_MELEE_WEAPON | ITEM_TYPE::TYPE_MISSILE_WEAPON | ITEM_TYPE::TYPE_MONEY |
				ITEM_TYPE::TYPE_PROMISSORY_NOTE | ITEM_TYPE::TYPE_SPELL_COMPONENTS)))
	{
		pObject = new CBaseItem();
	}
	else
	{
		pObject = new CPhysicsObj();
	}

	pObject->m_dwGUID = pObjectInfo->guid;
	pObject->m_miBaseModel = pObjectInfo->appearance;
	pObject->m_dwModel = pObjectInfo->physics.setup_id;
	pObject->m_dwAnimationSet = pObjectInfo->physics.mtable_id;
	pObject->m_dwSoundSet = pObjectInfo->physics.stable_id;
	pObject->m_dwEffectSet = pObjectInfo->physics.phstable_id;
	pObject->m_fScale = pObjectInfo->physics.object_scale;
	pObject->m_ItemType = pObjectInfo->weenie._type;
	pObject->m_strName = pObjectInfo->weenie._name;

	pObject->m_Origin.landcell = pObjectInfo->physics.pos.objcell_id;
	pObject->m_Origin.x = pObjectInfo->physics.pos.frame.m_fOrigin.x;
	pObject->m_Origin.y = pObjectInfo->physics.pos.frame.m_fOrigin.y;
	pObject->m_Origin.z = pObjectInfo->physics.pos.frame.m_fOrigin.z;
	pObject->m_Angles.w = pObjectInfo->physics.pos.frame.qw;
	pObject->m_Angles.x = pObjectInfo->physics.pos.frame.qx;
	pObject->m_Angles.y = pObjectInfo->physics.pos.frame.qy;
	pObject->m_Angles.z = pObjectInfo->physics.pos.frame.qz;

	if (pObjectInfo->physics.movement_buffer)
	{
		pObject->m_AnimOverrideData = pObjectInfo->physics.movement_buffer;
		pObject->m_AnimOverrideDataLen = pObjectInfo->physics.movement_buffer_length;
		pObject->m_AutonomousMovement = pObjectInfo->physics.autonomous_movement;
	}

	pObject->m_wIcon = pObjectInfo->weenie._iconID & 0xFFFFFF;
	pObject->m_UIEffects = pObjectInfo->weenie._effects;
	pObject->m_dwEquipSlot = pObjectInfo->weenie._combatUse; // This isn't correct.
	pObject->m_dwEquipType = pObjectInfo->weenie._combatUse;
	pObject->m_dwCoverage1 = pObjectInfo->weenie._valid_locations;
	pObject->m_dwCoverage2 = pObjectInfo->weenie._location;
	pObject->m_dwCoverage3 = pObjectInfo->weenie._priority;

	pObject->m_dwordProperties = pObjectInfo->dwordProperties;
	pObject->m_qwordProperties = pObjectInfo->qwordProperties;
	pObject->m_boolProperties = pObjectInfo->boolProperties;
	pObject->m_floatProperties = pObjectInfo->floatProperties;
	pObject->m_stringProperties = pObjectInfo->stringProperties;
	pObject->m_dataIDProperties = pObjectInfo->dataIDProperties;

	pObject->m_miWornModel = pObjectInfo->wornAppearance;

	return pObject;
}


