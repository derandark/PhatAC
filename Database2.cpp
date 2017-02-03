
#include "StdAfx.h"
#include "Database2.h"
#include "World.h"
#include "Portal.h"
#include "PhysicsDesc.h"
#include "PublicWeenieDesc.h"
#include "Door.h"

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
}

void CGameDatabase::Init()
{
	// Loads from MYSQL database
	LoadPortals();
	LoadTeleTownList();
	LoadMonsterTemplates();

	// Load data files for now
	LoadAerfalle();
	LoadCapturedMonsterData();

	SpawnAerfalle();
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
			pSpawn = new CBaseDoor();
		else if (pSpawnInfo->weenie._type == ITEM_TYPE::TYPE_CREATURE)
			pSpawn = new CBaseMonster();
		else
			pSpawn = new CPhysicsObj();

		pSpawn->m_dwGUID = pSpawnInfo->dwGUID; // g_pWorld->GenerateGUID(eDynamicGUID);
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

		g_pWorld->CreateEntity(pSpawn);
	}
}

void CGameDatabase::LoadAerfalle()
{
	FILE *fp = fopen("aerfalle.dat", "rb");

	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		unsigned int len = (unsigned)ftell(fp);
		fseek(fp, 0, SEEK_SET);
		BYTE *data = new BYTE[len];
		fread(data, len, 1, fp);

		BinaryReader reader(data, len);
		unsigned int count = reader.ReadDWORD();

		for (unsigned int i = 0; i < count; i++)
		{
			reader.ReadString();

			CCapturedWorldObjectInfo *pMonsterInfo = new CCapturedWorldObjectInfo;

			// make all lowercase and ignore spaces for searching purposes

			DWORD a = reader.ReadDWORD(); // size of this
			DWORD b = reader.ReadDWORD(); // 0xf745
			pMonsterInfo->dwGUID = reader.ReadDWORD(); // GUID

											   //if (dwGUID == 0xD851865B)
											   //	__asm int 3;

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

			if (!reader.GetLastError())
			{
				m_CapturedAerfalleData.push_back(pMonsterInfo);
			}
			else
			{
				delete pMonsterInfo;
				break;
			}
		}

		fclose(fp);
		delete[]data;
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
	std::string searchName = name;
	std::transform(searchName.begin(), searchName.end(), searchName.begin(), ::tolower);
	searchName.erase(remove_if(searchName.begin(), searchName.end(), isspace), searchName.end());

	std::map<std::string, CCapturedWorldObjectInfo *>::iterator i = m_CapturedMonsterData.find(searchName);

	if (i == m_CapturedMonsterData.end())
		return NULL;

	return i->second;
}

void CGameDatabase::LoadCapturedMonsterData()
{
	FILE *fp = fopen("monsters.dat", "rb");

	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		unsigned int len = (unsigned) ftell(fp);
		fseek(fp, 0, SEEK_SET);
		BYTE *data = new BYTE[len];
		fread(data, len, 1, fp);

		BinaryReader reader(data, len);
		unsigned int count = reader.ReadDWORD();

		for (unsigned int i = 0; i < count; i++)
		{
			CCapturedWorldObjectInfo *pMonsterInfo = new CCapturedWorldObjectInfo;
			pMonsterInfo->m_ObjName = reader.ReadString();

			std::transform(pMonsterInfo->m_ObjName.begin(), pMonsterInfo->m_ObjName.end(), pMonsterInfo->m_ObjName.begin(), ::tolower);
			pMonsterInfo->m_ObjName.erase(remove_if(pMonsterInfo->m_ObjName.begin(), pMonsterInfo->m_ObjName.end(), isspace), pMonsterInfo->m_ObjName.end());

			// make all lowercase and ignore spaces for searching purposes
			
			DWORD a = reader.ReadDWORD(); // size of this
			DWORD b = reader.ReadDWORD(); // 0xf745
			DWORD dwGUID = reader.ReadDWORD(); // GUID
			
			//if (dwGUID == 0xD851865B)
			//	__asm int 3;

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

			if (!reader.GetLastError())
			{
				m_CapturedMonsterData.insert(std::pair<std::string, CCapturedWorldObjectInfo*>(pMonsterInfo->m_ObjName, pMonsterInfo));
				
				if (!(pMonsterInfo->weenie._bitfield & BitfieldIndex::BF_PLAYER))
				{
					m_CapturedMonsterDataList.push_back(pMonsterInfo);
				}
			}
			else
			{
				delete pMonsterInfo;
				break;
			}
		}

		fclose(fp);
		delete[]data;
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
			DEBUGOUT("Added %d Teleport Locations.\r\n", Result->ResultRows());
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

			LOG(World, Verbose, "Spawned %d portals.\n", portalCount);
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



