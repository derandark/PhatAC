
#include "StdAfx.h"

#include "Client.h"
#include "ClientEvents.h"

// Network access
#include "Network.h"
#include "PacketController.h"
#include "BinaryReader.h"
#include "BinaryWriter.h"

// Database access
#include "Database.h"
#include "AccountDatabase.h"
#include "CharacterDatabase.h"

// World access
#include "World.h"

// Player access
#include "PhysicsObj.h"
#include "Monster.h"
#include "Player.h"

// Command access
#include "ClientCommands.h"


///////////
// Name: CClient
// Desc: Presents an interface for client/server interaction.
///////////
CClient::CClient(SOCKADDR_IN *peer, WORD slot, char *account, int accessLevel)
{
	memcpy(&m_vars.addr, peer, sizeof(SOCKADDR_IN));
	m_vars.slot = slot;
	m_vars.account = account;

	m_vars.initdats = FALSE;
	m_vars.inworld = FALSE;
	m_vars.needchars = TRUE;

	m_AccessLevel = accessLevel;

	m_pPC = new CPacketController(this);
	m_pEvents = new CClientEvents(this);
}

CClient::~CClient()
{
	SafeDelete(m_pEvents);
	SafeDelete(m_pPC);
}

void CClient::IncomingBlob(BlobPacket_s *pData)
{
	if (!IsAlive())
		return;

	if (!m_pPC || !m_pPC->IsAlive())
		return;

	m_pPC->IncomingBlob(pData);
}

void CClient::Think()
{
	if (!IsAlive())
		return;

	if (m_pPC)
	{
		if (m_pPC->HasConnection())
		{
			WorldThink();
		}

		m_pPC->Think();

		//If the packetcontroller is dead, so are we.
		if (!m_pPC->IsAlive())
			Kill(__FILE__, __LINE__);
	}
	else
		Kill(__FILE__, __LINE__);
}

void CClient::ThinkOutbound()
{
	if (!IsAlive())
		return;

	if (m_pPC && m_pPC - IsAlive())
	{
		m_pPC->ThinkOutbound();
	}
}

void CClient::WorldThink()
{
	if (!m_vars.inworld)
	{
		if (!m_vars.initdats)
		{
			//'PHAT AC'
			BinaryWriter EraseFile;

			/*
			EraseFile.WriteDWORD( 0xF7BB );
			EraseFile.WriteDWORD( 7 );
			EraseFile.WriteDWORD( 0xEFE9FFFF );
			EraseFile.WriteDWORD( 0xEDEAFFFF );
			EraseFile.WriteDWORD( 0xEEEAFFFF );
			EraseFile.WriteDWORD( 0xEFEAFFFF );
			EraseFile.WriteDWORD( 0xF0EAFFFF );
			EraseFile.WriteDWORD( 0xEEE9FFFF );
			EraseFile.WriteDWORD( 0xEFE9FFFF );
			SendMessage(EraseFile.GetData(), EraseFile.GetSize(), EVENT_MSG);
			*/

			//CUSTOM BLOCKS!
			WIN32_FIND_DATA data;

			if (g_pDB->DataFileFindFirst("land\\cell-*", &data))
			{
				DWORD dwCount = 1;
				DWORD dwID;

				sscanf(data.cFileName, "cell-%08X", &dwID);

				BinaryWriter CustomBlocks;
				CustomBlocks.WriteDWORD(dwID);

				while (g_pDB->DataFileFindNext(&data))
				{
					sscanf(data.cFileName, "cell-%08X", &dwID);
					CustomBlocks.WriteDWORD(dwID);
					dwCount++;

					if ((dwID & 0xFFFF) == 0xFFFF) {
						CustomBlocks.WriteDWORD(dwID & 0xFFFFFFFE);
						dwCount++;
					}
				}

				BinaryWriter EraseCustomBlocks;
				EraseCustomBlocks.WriteDWORD(0xF7BB);
				EraseCustomBlocks.WriteDWORD(dwCount);
				EraseCustomBlocks.AppendData(CustomBlocks.GetData(), CustomBlocks.GetSize());

				// LOG(Temp, Normal, "Erasing custom blocks: %lu\n", dwCount);
				SendMessage(EraseCustomBlocks.GetData(), EraseCustomBlocks.GetSize(), EVENT_MSG);

				g_pDB->DataFileFindClose();
			}

			BinaryWriter UpdateDats;

			UpdateDats.WriteLong(0xF7B8);
			UpdateDats.WriteLong(m_vars.portalstamp);
			UpdateDats.WriteLong(0);
			UpdateDats.WriteLong(m_vars.cellstamp);
			UpdateDats.WriteLong(0);
			UpdateDats.WriteLong(0);
			UpdateDats.WriteLong(0);

			SendMessage(UpdateDats.GetData(), UpdateDats.GetSize(), EVENT_MSG);
			SendMessage(UpdateDats.GetData(), UpdateDats.GetSize(), EVENT_MSG);

			m_vars.initdats = TRUE;
		}

		if (m_vars.needchars)
		{
			UpdateLoginScreen();
			m_vars.needchars = FALSE;
		}
	}

	m_pEvents->Think();
}

void CClient::UpdateLoginScreen()
{
	char *account = (char*)m_vars.account.c_str();

	DWORD pdwGUIDs[5];
	DWORD dwCount = g_pDB->CharDB()->GetCharacters(account, pdwGUIDs);

	BinaryWriter CharacterList;
	CharacterList.WriteLong(0xF658);
	CharacterList.WriteLong(0);
	CharacterList.WriteLong(dwCount);

	_CHARDESC desc;
	for (unsigned int i = 0; i < dwCount; i++)
	{
		g_pDB->CharDB()->GetCharacterDesc(pdwGUIDs[i], &desc);

		CharacterList.WriteLong(desc.dwGUID);
		CharacterList.WriteString(desc.szName);
		CharacterList.WriteLong(desc.dwDeletePeriod);
	}

	CharacterList.WriteLong(0);
	CharacterList.WriteLong(11);
	CharacterList.WriteString(account);
	CharacterList.WriteLong(1);
	CharacterList.WriteLong(1);
	SendMessage(CharacterList.GetData(), CharacterList.GetSize(), 9);

	/*std::string strMOTD = "You are in the world of PHATAC.\n\n";
	strMOTD += "Welcome to Asheron's Call!\n\n";
	strMOTD += "Sat 04/18/2004\n=============\nFeatures added\nAbout a months worth..\n\n\n";
	strMOTD += "Sat 03/20/2004\n=============\nFeatures added:\n....\n\n\n";
	strMOTD += "Wed 03/17/2004\n=============\nFeatures added:\nStraf & Backwards movement now uses decoded Formulas.\nCombat modes now properly animate.\nNow documenting build changes. All previous info is estimated.\n\n\n";
	strMOTD += "Tue 03/16/2004\n=============\nFeatures added:\nTeleporting to players. (@tele)\nTeleporting to coordinates. (@teleto)\n\n\n";
	strMOTD += "Sun 03/14/2004\n=============\nFeatures added:\nParticle effects. (@effect)\nRunning anim w/ decoded Formulas.\nWorldstate save/restore for ID propagation.\nSingle-state emote animations.\n\n\n";
	strMOTD += "Wed 03/10/2004\n=============\nFeatures added:\nMultiplayer.\nRoaming.\n\n\n";
	strMOTD += "Wed 03/03/2004\n=============\nEmulator packet controllers (netcode) complete.\n\n\n";
	strMOTD += "Mon 03/01/2004\n=============\nEmulator is born.\n\n\n";*/

	/*
	BinaryWriter ServerMOTD;
	ServerMOTD.WriteLong( 0xF65A );
	ServerMOTD.WriteString( csprintf("Currently %u clients connected.\n", g_pGlobals->GetClientCount() ) );
	ServerMOTD.WriteString( g_pWorld->GetMOTD() );
	SendMessage(ServerMOTD.GetData(), ServerMOTD.GetSize(), PRIVATE_MSG);
	*/

	BinaryWriter ServerName;
	ServerName.WriteLong(0xF7E1);
	ServerName.WriteLong(0x32); // Num connections
	ServerName.WriteLong(-1); // Max connections
	ServerName.WriteString("Peaville");
	SendMessage(ServerName.GetData(), ServerName.GetSize(), 9);

	BinaryWriter ServerUnk;
	ServerUnk.WriteLong(0xF7E5);
	ServerUnk.WriteLong(1); // servers region
	ServerUnk.WriteLong(1); // name rule language
	ServerUnk.WriteLong(1); // product id
	ServerUnk.WriteLong(2); // supports languages (2)
	ServerUnk.WriteLong(0); // language #1
	ServerUnk.WriteLong(1); // language #2
	SendMessage(ServerUnk.GetData(), ServerUnk.GetSize(), 5);
}

void CClient::EnterWorld()
{
	DWORD EnterWorld = 0xF7DF; // 0xF7C7;

	SendMessage(&EnterWorld, sizeof(DWORD), 9);
	LOG(Client, Normal, "Client #%u is entering the world.\n", m_vars.slot);

	m_vars.inworld = TRUE;
}

void CClient::ExitWorld()
{
	DWORD ExitWorld = 0xF653;
	SendMessage(&ExitWorld, sizeof(DWORD), PRIVATE_MSG);
	LOG(Client, Normal, "Client #%u is exiting the world.\n", m_vars.slot);

	m_pPC->ResetEvent();

	UpdateLoginScreen();

	m_vars.inworld = FALSE;
}

void CClient::SendMessage(BinaryWriter* pMessage, WORD group, BOOL event, BOOL del)
{
	if (!pMessage)
		return;

	SendMessage(pMessage->GetData(), pMessage->GetSize(), group, event);

	if (del)
		delete pMessage;
}
void CClient::SendMessage(void *data, DWORD length, WORD group, BOOL game_event)
{
	if (!IsAlive())
		return;

	if (!data || !length)
		return;

	if (length > 4)
	{
		DWORD dwMessageCode = *((DWORD *)data);

		//if ( dwMessageCode == 0x0000F745 )
		//	OutputConsoleBytes( data, length );
	}


	if (m_pPC && m_pPC->IsAlive())
	{
		if (!game_event)
			m_pPC->SendMessage(data, length, group);
		else
		{
			EventHeader *Event = (EventHeader *)new BYTE[sizeof(EventHeader) + length];
			Event->dwF7B0 = 0xF7B0;
			Event->dwPlayer = m_pEvents->GetPlayerID();
			Event->dwSequence = m_pPC->GetNextEvent();
			memcpy((BYTE *)Event + sizeof(EventHeader), data, length);

			m_pPC->SendMessage(Event, sizeof(EventHeader) + length, group);
			delete[] Event;
		}
	}
}

BOOL CClient::CheckNameValidity(const char *name)
{
	int len = (int)strlen(name);

	if ((len < 3) || (len > 16))
		return FALSE;

	int i = 0;
	while (i < len)
	{
		char letter = name[i];
		if (!(letter >= 'A' && letter <= 'Z') && !(letter >= 'a' || letter <= 'z') &&
			/*!(letter == '\'') && */!(letter == ' ') && !(letter == '-'))
			break;
		i++;
	}
	if (i == len) return TRUE;

	return FALSE;
}

void CClient::CreateCharacter(BinaryReader *in)
{
	DWORD dwError = 5;

	char *szAccount = in->ReadString();
	if (!szAccount || strcmp(szAccount, m_vars.account.c_str()))
		return;

	if (in->ReadDWORD() != 1) return;

	DWORD dwRace = in->ReadDWORD(); // 5
	DWORD dwGender = in->ReadDWORD(); // 1

	DWORD dwForeheadTex = in->ReadDWORD(); // 6
	DWORD dwNoseTex = in->ReadDWORD(); // 1
	DWORD dwChinTex = in->ReadDWORD(); // 1

	DWORD dwHairColor = in->ReadDWORD(); // 0 
	DWORD dwEyeColor = in->ReadDWORD(); // 0
	DWORD dwHairStyle = in->ReadDWORD(); // 1 
	DWORD dwHatType = in->ReadDWORD(); // -1
	DWORD dwHatColor = in->ReadDWORD(); // 0
	DWORD dwShirtType = in->ReadDWORD(); // 0
	DWORD dwShirtColor = in->ReadDWORD(); // 2
	DWORD dwPantsType = in->ReadDWORD(); // 0
	DWORD dwPantsColor = in->ReadDWORD(); // 2
	DWORD dwShoeType = in->ReadDWORD(); // 0
	DWORD dwShoeColor = in->ReadDWORD(); // 2

	DWORD dwSkinPalette = in->ReadDWORD();
	float flSkinShade = in->ReadFloat();
	DWORD dwHairPalette = in->ReadDWORD();
	float flHairShade = in->ReadFloat();
	DWORD dwHatPalette = in->ReadDWORD();
	float flHatShade = in->ReadFloat();
	DWORD dwPantsPalette = in->ReadDWORD();
	float flPantsShade = in->ReadFloat();
	DWORD dwShirtPalette = in->ReadDWORD();
	float flShirtShade = in->ReadFloat();
	DWORD dwShoePalette = in->ReadDWORD();
	float dwShoeShade = in->ReadFloat();

	DWORD dwProfession = in->ReadDWORD(); // 0

	DWORD dwStrength = in->ReadDWORD(); // 10
	DWORD dwEndurance = in->ReadDWORD(); // 20
	DWORD dwCoordination = in->ReadDWORD(); // 30
	DWORD dwQuickness = in->ReadDWORD(); // 80
	DWORD dwFocus = in->ReadDWORD(); // 90
	DWORD dwSelf = in->ReadDWORD(); // 100

	DWORD dwUnknown = in->ReadDWORD(); // -1
	DWORD dwAccessLevel = in->ReadDWORD(); // 1    //0xE40 = sentinels.

	DWORD dwNumSkills = in->ReadDWORD(); // 0x37 (55)
	DWORD *dwSkillStatus = (DWORD *)in->ReadArray(dwNumSkills * sizeof(DWORD));

	char *szCharacterName = in->ReadString();
	DWORD dwUnknown2 = in->ReadDWORD();
	DWORD dwUnknown3 = in->ReadDWORD();
	DWORD dwStarterTown = in->ReadDWORD();
	DWORD dwUnknown4 = in->ReadDWORD();

	if (in->GetLastError()) goto BadData;

	//should convert name by removing duplicate spaces, '-', or '\''
	if (!CheckNameValidity(szCharacterName))
	{
		dwError = 4;
		goto BadData;
	}

	//should check variables to make sure everythings within restriction

	//wHairTextures[m_wGender][m_wHairStyle];
	//WORD wHairTextures[2][4] = { { 0x10B8, 0x10B8, 0x10B8, 0x10B7 }, { 0x11FD, 0x11FD, 0x11FD, 0x10B7 } };

	//group 4, 0x0000F643, 0x00000001, <guid>, <char name[str]>, 0x00000000
	{
		DWORD dwGUID = 0;
		CCharacterDatabase* pCharDB;
		if ((pCharDB = g_pDB->CharDB()) && g_pWorld)
		{
			_CHARDESC buffer;

			if (!pCharDB->GetCharacterDesc(szCharacterName, &buffer))
			{
				dwGUID = g_pWorld->GenerateGUID(ePlayerGUID);
				pCharDB->CreateCharacterDesc(GetAccount(), dwGUID, szCharacterName);
			}
		}

		if (dwGUID != 0)
		{
			BinaryWriter Success;
			Success.WriteDWORD(0xF643);
			Success.WriteDWORD(1);
			Success.WriteDWORD(dwGUID);
			Success.WriteString(szCharacterName);
			Success.WriteDWORD(0);
			SendMessage(Success.GetData(), Success.GetSize(), PRIVATE_MSG);
		}
		else
		{
			BinaryWriter BadCharGen;
			BadCharGen.WriteDWORD(0xF643);
			BadCharGen.WriteDWORD(3); // name already exists
			SendMessage(BadCharGen.GetData(), BadCharGen.GetSize(), PRIVATE_MSG);
		}
	}
	return;
BadData:
	{
		BinaryWriter BadCharGen;
		BadCharGen.WriteDWORD(0xF643);
		BadCharGen.WriteDWORD(dwError);
		SendMessage(BadCharGen.GetData(), BadCharGen.GetSize(), PRIVATE_MSG);
	}
}

void outputCompressed(BYTE *srcData, DWORD srcLen, DWORD dstLen_, BYTE **result_)
{
	BYTE *dest = new BYTE[100000];
	DWORD destLen = dstLen_;

	int result = 0;
	if (Z_OK == (result = uncompress(dest, &destLen, srcData, srcLen)))
	{
		LOG(Temp, Debug, "Decompress:\n");
		LOG_BYTES(Temp, Debug, dest, destLen);
	}
	else
	{
		LOG(Temp, Debug, "Failed decompress: %d\n", result);
	}

	if (destLen != dstLen_)
	{
		__asm int 3;
	}

	*result_ = dest;
}

void CClient::SendLandblock(DWORD dwFileID)
{
	TURBINEFILE* pLandData = g_pCell->GetFile(dwFileID);
	if (!pLandData)
	{
		if (m_pEvents)
		{
			m_pEvents->SendText(csprintf("Your client is requesting cell data (#%08X) that this server does not have!", dwFileID), 1);
			m_pEvents->SendText("If you are in portal mode, type /render radius 5 to escape. The server administrator should reconfigure the server with a FULL cell.dat file!", 1);
		}
		return;
	}

	if ((dwFileID & 0xFFFF) != 0xFFFF)
	{
		LOG(Client, Warning, "Client requested landblock %08X - should end in 0xFFFF\n", dwFileID);
	}

	TURBINEFILE* pObjData = NULL;
	DWORD		dwObjFileID = 0;

	if (pLandData->GetLength() >= 8) {
		DWORD dwFlags = *((DWORD *)pLandData->GetData());
		BOOL bHasObjects = ((dwFlags & 1) ? TRUE : FALSE);

		dwObjFileID = (dwFileID & 0xFFFF0000) | 0xFFFE;
		pObjData = g_pCell->GetFile(dwObjFileID);
	}

	if (pLandData)
	{
		BinaryWriter BlockPackage;

		BlockPackage.WriteDWORD(0xF7E2);

		DWORD dwFileSize = pLandData->GetLength();
		BYTE* pbFileData = pLandData->GetData();

		DWORD dwPackageSize = (DWORD)((dwFileSize * 1.02f) + 12 + 1);
		BYTE* pbPackageData = new BYTE[dwPackageSize];

		if (Z_OK != compress2(pbPackageData, &dwPackageSize, pbFileData, dwFileSize, Z_BEST_COMPRESSION))
		{
			LOG(Client, Error, "Error compressing LandBlock package!\n");
		}

		BlockPackage.WriteDWORD(1);
		BlockPackage.WriteDWORD(2);
		BlockPackage.WriteDWORD(1);
		BlockPackage.WriteDWORD(dwFileID);
		BlockPackage.WriteDWORD(1);
		BlockPackage.WriteBYTE(1); // Compressed
		BlockPackage.WriteDWORD(2);
		BlockPackage.WriteDWORD(dwPackageSize + sizeof(DWORD) * 2);
		BlockPackage.WriteDWORD(dwFileSize);
		BlockPackage.AppendData(pbPackageData, dwPackageSize);

		delete[] pbPackageData;
		delete pLandData;

		//LOG(Temp, Normal, "Sent landblock %08X ..\n", dwFileID);
		SendMessage(BlockPackage.GetData(), BlockPackage.GetSize(), EVENT_MSG, FALSE);
	}

	if (pObjData)
	{
		BinaryWriter BlockInfoPackage;

		BlockInfoPackage.WriteDWORD(0xF7E2);

		DWORD dwFileSize = pObjData->GetLength();
		BYTE* pbFileData = pObjData->GetData();

		DWORD dwPackageSize = (DWORD)((dwFileSize * 1.02f) + 12 + 1);
		BYTE* pbPackageData = new BYTE[dwPackageSize];

		if (Z_OK != compress2(pbPackageData, &dwPackageSize, pbFileData, dwFileSize, Z_BEST_COMPRESSION))
		{
			LOG(Client, Error, "Error compressing LandBlockInfo package!\n");
		}

		BlockInfoPackage.WriteDWORD(1);
		BlockInfoPackage.WriteDWORD(2);
		BlockInfoPackage.WriteDWORD(2); // 1 for 0xFFFF, 2 for 0xFFFE
		BlockInfoPackage.WriteDWORD(dwObjFileID);
		BlockInfoPackage.WriteDWORD(1);
		BlockInfoPackage.WriteBYTE(1);
		BlockInfoPackage.WriteDWORD(2);
		BlockInfoPackage.WriteDWORD(dwPackageSize + sizeof(DWORD) * 2);
		BlockInfoPackage.WriteDWORD(dwFileSize);
		BlockInfoPackage.AppendData(pbPackageData, dwPackageSize);

		delete[] pbPackageData;
		delete pObjData;

		//LOG(Temp, Normal, "Sent objectblock %08X ..\n", dwObjFileID);
		SendMessage(BlockInfoPackage.GetData(), BlockInfoPackage.GetSize(), EVENT_MSG, FALSE);
	}

	//if (m_pEvents)
	//	m_pEvents->SendText( csprintf("The server has sent you block #%04X!", dwFileID >> 16), 1);
}

void CClient::SendLandcell(DWORD dwFileID)
{
	TURBINEFILE* pCellData = g_pCell->GetFile(dwFileID);
	if (!pCellData)
	{
		if (m_pEvents)
		{
			m_pEvents->SendText(csprintf("Your client is requesting cell data (#%08X) that this server does not have!", dwFileID), 1);
			m_pEvents->SendText("If you are in portal mode, type /render radius 5 to escape. The server administrator should reconfigure the server with a FULL cell.dat file!", 1);
		}
		return;
	}

	if (pCellData)
	{
		BinaryWriter CellPackage;

		CellPackage.WriteDWORD(0xF7E2);

		DWORD dwFileSize = pCellData->GetLength();
		BYTE* pbFileData = pCellData->GetData();

		DWORD dwPackageSize = (DWORD)((dwFileSize * 1.02f) + 12 + 1);
		BYTE* pbPackageData = new BYTE[dwPackageSize];

		if (Z_OK != compress2(pbPackageData, &dwPackageSize, pbFileData, dwFileSize, Z_BEST_COMPRESSION))
		{
			// These are CEnvCell if I recall correctly
			LOG(Client, Error, "Error compressing landcell package!\n");
		}

		CellPackage.WriteDWORD(1);
		CellPackage.WriteDWORD(2);
		CellPackage.WriteDWORD(3);
		CellPackage.WriteDWORD(dwFileID);
		CellPackage.WriteDWORD(1);
		CellPackage.WriteBYTE(1);
		CellPackage.WriteDWORD(2);
		CellPackage.WriteDWORD(dwPackageSize + sizeof(DWORD) * 2);
		CellPackage.WriteDWORD(dwFileSize);
		CellPackage.AppendData(pbPackageData, dwPackageSize);

		delete[] pbPackageData;
		delete pCellData;

		//LOG(Temp, Normal, "Sent cell %08X ..\n", dwFileID);

		SendMessage(CellPackage.GetData(), CellPackage.GetSize(), EVENT_MSG, FALSE);
	}

	//if (m_pEvents)
	//	m_pEvents->SendText( csprintf("The server has sent you cell #%04X!", dwFileID >> 16), 1);
}

void CClient::ProcessMessage(BYTE *data, DWORD length, WORD group)
{
	BinaryReader in(data, length);

	DWORD dwMessageCode = in.ReadDWORD();

	if (in.GetLastError())
	{
		LOG(Client, Warning, "Error processing message.\n");
		return;
	}
	switch (dwMessageCode)
	{
		case 0xF653:
		{
			if (m_vars.inworld)
			{
				m_pEvents->BeginLogout();
			}

			break;
		}
		break;
		case 0xF656: // Create Character
		{
			if (!m_vars.inworld)
			{
				CreateCharacter(&in);
			}

			break;
		}

		case 0xF6EA: // Request Object
		{
			if (m_vars.inworld)
			{
				DWORD dwEID = in.ReadDWORD();
				if (in.GetLastError()) break;

				CBasePlayer *pPlayer;

				if ((m_pEvents) && (pPlayer = m_pEvents->GetPlayer()))
				{
					CPhysicsObj *pTarget = pPlayer->FindChild(dwEID);

					if (!pTarget)
						pTarget = g_pWorld->FindWithinPVS(pPlayer, dwEID);

					if (pTarget)
						pPlayer->MakeAware(pTarget);
				}
			}
			break;
		}

		case 0xF7E6:
		{
			BinaryWriter ServerUnk2;
			ServerUnk2.WriteLong(0xF7EA);
			SendMessage(ServerUnk2.GetData(), ServerUnk2.GetSize(), 5);
			break;
		}

		case 0xF7EA:
		{
			//BinaryWriter EndDDD;
			//EndDDD.WriteDWORD(0xF7EA);
			//SendMessage(EndDDD.GetData(), EndDDD.GetSize(), EVENT_MSG);

			break;
		}

		case 0xF7E3: // Request File Data was 0xF7A9
		{
			// This doesn't work for some reason.
			break;

			if (m_vars.inworld)
			{
				DWORD dwFileClass = in.ReadDWORD();
				DWORD dwFileID = in.ReadDWORD();				
				if (in.GetLastError()) break;

				switch (dwFileClass)
				{
				default:
					LOG(Client, Warning, "Unknown download request: %08X %d\n", dwFileID, dwFileClass);
					break;

				case 1:
					SendLandblock(dwFileID); // 1 is landblock 0xFFFF
					break;

				// 2 is 0xFFFE the landblock environment info I believe, but client never requests it directly, it is sent with landblocks

				case 3:
					SendLandcell(dwFileID); // 0x100+
					break;
				}
			}

			break;
		}

		case 0xF7B1:
		{
			//should check the sequence
			if (m_vars.inworld)
			{
				m_pEvents->ProcessEvent(&in);
			}

			break;
		}
		case 0xF7C8:
		{
			if (!m_vars.inworld)
			{
				EnterWorld();
			}

			break;
		}

		case 0xF657:
		{
			if (m_vars.inworld)
			{
				DWORD dwGUID = in.ReadDWORD();
				char* szName = in.ReadString();
				if (in.GetLastError()) break;

				m_pEvents->LoginCharacter(dwGUID, /*szName*/GetAccount());
			}

			break;
		}
		default:
			LOG(Client, Warning, "Unhandled message %08X from the client.\n", dwMessageCode);
			break;
	}

	//if ( dwMessageCode != 0xF7B1 )
	//	LOG(Temp, Normal, "Received message %04X\n", dwMessageCode);
}

BOOL CClient::CheckAccount(const char* cmp)
{
	return !strcmp(m_vars.account.c_str(), cmp);
}

int CClient::GetAccessLevel()
{
	return m_AccessLevel;
}

BOOL CClient::CheckAddress(SOCKADDR_IN *peer)
{
	return !memcmp(GetHostAddress(), peer, sizeof(SOCKADDR_IN));
}

WORD CClient::GetIndex()
{
	return m_vars.slot;
}

const char* CClient::GetAccount()
{
	return m_vars.account.c_str();
}

const char* CClient::GetDescription()
{
	// Must lead with index or the kick/ban feature won't function.
	return csprintf("#%u %s \"%s\"", m_vars.slot, inet_ntoa(m_vars.addr.sin_addr), m_vars.account.c_str());
}

void CClient::SetLoginData(DWORD dwUnixTime, DWORD dwPortalStamp, DWORD dwCellStamp)
{
	m_vars.logintime = g_pGlobals->Time();

	m_vars.unixtime = dwUnixTime;
	m_vars.portalstamp = dwPortalStamp;
	m_vars.cellstamp = dwCellStamp;

	m_vars.initdats = FALSE;
}

SOCKADDR_IN* CClient::GetHostAddress()
{
	return &m_vars.addr;
}
