
#include "StdAfx.h"
#include "Server.h"
#include "Rules.h"
#include "Database.h"
#include "World.h"
#include "Network.h"
#include "NetFood.h"
#include "ChatMsgs.h"

CDatabase *g_pDB = 0;
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
	g_pDB = new CDatabase;
	g_pWorld = new CWorld;
	g_pNetwork = new CNetwork(m_sockets, m_socketCount);

	OutputConsole("The server is now online.\r\n");
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
			OutputConsole("Bound to port %u!\r\n", port);
			break;
		}
		OutputConsole("Failed bind on port %u!\r\n", port);
		port++;
	}

	if (port == 9080)
	{
		OutputConsole("Failure to bind socket!\r\n");
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
				OutputConsole("Failure to bind socket port %d!\r\n", basePort + i);
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

	OutputConsole("Broadcast, \"%s\"\r\n", text);
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