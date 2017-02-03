
#include "StdAfx.h"
#include "Server.h"
#include "Rules.h"
#include "Database.h"
#include "World.h"
#include "Network.h"
#include "BinaryWriter.h"
#include "ChatMsgs.h"
#include "Database2.h"

CDatabase *g_pDB = 0;
CMYSQLDatabase *g_pDB2 = 0;
CGameDatabase *g_pGameDatabase = 0;
CWorld *g_pWorld = 0;
CNetwork *g_pNetwork = 0;
TURBINEPORTAL *g_pPortal = 0;
TURBINECELL *g_pCell = 0;
GAMERULES *g_pGameRules = 0;

CPhatServer::CPhatServer(in_addr hostmachine, u_short hostport)
{
	m_lastframe = g_pGlobals->m_CounterTime;
	m_frames = 0;

	//
	memcpy(&m_hostaddr, &hostmachine, sizeof(in_addr));
	m_hostport = hostport;

	g_pGlobals->ResetPackets();

	m_socketCount = 10;
	for (int i = 0; i < m_socketCount; i++)
		m_sockets[i] = INVALID_SOCKET;

	InitializeSocket(hostport);

	g_pPortal = new TURBINEPORTAL();
	g_pCell = new TURBINECELL();
	g_pGameRules = new GAMERULES();
	g_pDB = new CDatabase; // Old, dumb, bad
	g_pDB2 = new CMYSQLDatabase("127.0.0.1", 0, "root", "", "phatac"); // Newer, shiny, makes pancakes in the morning
	g_pGameDatabase = new CGameDatabase();
	g_pWorld = new CWorld();
	g_pNetwork = new CNetwork(m_sockets, m_socketCount);

	g_pGameDatabase->Init();

	LOG(Temp, Normal, "The server is now online.\n");

	int missing = 0;
	int has = 0;
	int missingE = 0;
	int hasE = 0;
	int hasD = 0;
	int missingD = 0;
	int hasUD = 0;
	int hasUDT = 0;
	int hasM = 0;
	int missingM = 0;
	FILE *fp = NULL;

#if 0 // ndef _DEBUG
	if (GetAsyncKeyState(VK_F12))
	{
		fp = fopen("missingCells.txt", "wt");
	}

	for (int i = 0; i <= 0xFFFF; i++)
	{
		if (!g_pCell->FileExists((i << 16) | 0xFFFF))
		{
			missing++;
		}
		else
		{
			has++;

			if (TURBINEFILE *pCell = g_pCell->GetFile((i << 16) | 0xFFFF))
			{
				if (pCell->GetLength() >= 8)
				{
					DWORD value = ((DWORD *)pCell->GetData())[1];

					if (value)
					{
						if (!g_pCell->FileExists((i << 16) | 0xFFFE))
						{
							missingE++;
						}
						else
						{
							hasE++;

							if (TURBINEFILE *pCellData = g_pCell->GetFile((i << 16) | 0xFFFE))
							{
								DWORD value = ((DWORD *)pCellData->GetData())[1];

								if (value >= 10000)
								{
									__asm int 3;
								}

								for (unsigned int j = 0; j < value; j++)
								{
									if (g_pCell->FileExists((i << 16) | (0x100 + j)))
									{
										if (TURBINEFILE *pEnvData = g_pCell->GetFile((i << 16) | (0x100 + j)))
										{
											BYTE *ptr = pEnvData->GetData();

											ptr += 12;

											BYTE numSurface = *ptr;
											ptr += 4;
											ptr += 2 * numSurface;

											WORD envID = *(WORD *)ptr;

											if (g_pPortal->FileExists(0x0D000000UL | envID))
											{
												hasM++;
											}
											else
											{
												missingM++;
											}

											delete pEnvData;
										}

										hasD++;
									}
									else
									{

										if (fp)
										{
											fprintf(fp, "%08X\n", (i << 16) | (0x100 + j));
										}

										missingD++;
									}
								}

								delete pCellData;
							}
						}
					}
				}

				delete pCell;
			}
		}
	}

	if (fp)
	{
		fclose(fp);
	}

	LOG(Temp, Normal, "Has %u landblocks, missing %u (%.2f%%)\n", has, missing, ((float)has / (has+missing)) * 100.00);
	LOG(Temp, Normal, "Has %u landblock environments, missing %u of the ones known (%.2f%%)\n", hasE, missingE, ((float)hasE / (hasE + missingE)) * 100.00);
	LOG(Temp, Normal, "Has %u dungeon cells, missing %u of the ones known (%.2f%%)\n", hasD, missingD, ((float)hasD / (hasD + missingD)) * 100.00);
	LOG(Temp, Normal, "Resolved %u dungeon meshes, missing %u of the ones attempted (%.2f%%)\n", hasM, missingM, ((float)hasM / (hasM + missingM)) * 100.00);
#endif
}

CPhatServer::~CPhatServer()
{
	if (g_pNetwork)
	{
		SystemBroadcast("ATTENTION - This Asheron's Call Server is shutting down NOW!!!!");
		g_pNetwork->Think();
		SafeDelete(g_pNetwork);
	}
	SafeDelete(g_pWorld);
	SafeDelete(g_pGameDatabase);
	SafeDelete(g_pDB2);
	SafeDelete(g_pDB);
	SafeDelete(g_pGameRules);
	SafeDelete(g_pCell);
	SafeDelete(g_pPortal);

	for (int i = 0; i < m_socketCount; i++)
	{
		if (m_sockets[i] != INVALID_SOCKET)
		{
			closesocket(m_sockets[i]);
			m_sockets[i] = INVALID_SOCKET;
		}
	}
}

void CPhatServer::InitializeSocket(u_short port)
{
	SOCKADDR_IN localhost;
	localhost.sin_family = AF_INET;
	localhost.sin_addr.s_addr = INADDR_ANY;

	for (int i = 0; i < m_socketCount; i++)
		m_sockets[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	u_short startport = port;
	while (port < (startport + 30))
	{
		localhost.sin_port = htons(port);
		if (!bind(m_sockets[0], (struct sockaddr *)&localhost, sizeof(SOCKADDR_IN)))
		{
			LOG(Temp, Normal, "Bound to port %u!\n", port);
			break;
		}
		LOG(Temp, Normal, "Failed bind on port %u!\n", port);
		port++;
	}

	if (port == 9080)
	{
		LOG(Temp, Normal, "Failure to bind socket!\n");
	}
	else
	{
		SendMessage(GetDlgItem(g_pGlobals->GetWindowHandle(), IDC_SERVERPORT), WM_SETTEXT, 0, (LPARAM)csprintf("%u", port));

		int basePort = port;

		// Try to bind the other ports
		for (int i = 1; i < m_socketCount; i++)
		{
			localhost.sin_port = htons(basePort + i);
			if (bind(m_sockets[i], (struct sockaddr *)&localhost, sizeof(SOCKADDR_IN)))
			{
				LOG(Temp, Normal, "Failure to bind socket port %d!\n", basePort + i);
			}
		}
	}

	//Non-blocking sockets are more fun
	for (int i = 0; i < m_socketCount; i++)
	{
		unsigned long arg = 1;
		ioctlsocket(m_sockets[i], FIONBIO, &arg);
	}
}

void CPhatServer::SystemBroadcast(char *text)
{
	if (!g_pNetwork || !g_pWorld)
		return;

	LOG(Temp, Normal, "Broadcast, \"%s\"\n", text);
	g_pWorld->BroadcastGlobal(ServerBroadcast("System", text, 1), PRIVATE_MSG);
}

void CPhatServer::Tick(void)
{
	g_pGlobals->Update();

	if ((m_lastframe + g_pGlobals->m_CounterFreq) < g_pGlobals->m_CounterTime)
	{
		double timediff = (g_pGlobals->m_CounterTime - m_lastframe) / (double)g_pGlobals->m_CounterFreq;
		UpdateFramesHUD((UINT64)(m_frames / timediff));

		m_lastframe = g_pGlobals->m_CounterTime;
		m_frames = 0;
	}

	g_pNetwork->Think();
	g_pWorld->Think();
	g_pCell->Think();

	m_frames++;
}

u_short CPhatServer::GetPort()
{
	return m_hostport;
}

void CPhatServer::KickClient(WORD index)
{
	g_pNetwork->KickClient(index);
}

void CPhatServer::BanClient(WORD index)
{
	KickClient(index); //for now
}