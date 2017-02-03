
#include "StdAfx.h"

#include "Network.h"
#include "Client.h"
#include "World.h"

// Database access
#include "Database.h"
#include "AccountDatabase.h"
#include "CharacterDatabase.h"

// Network access
#include "crc.h"
#include "BinaryWriter.h"
#include "BinaryReader.h"
#include "PacketController.h"

// NOTES:
// A client can easily perform denial of service attacks by issuing a large number of connection requests if there ever comes a time this matters to fix

CNetwork::CNetwork(SOCKET *sockets, int socketCount)
{
	m_sockets = sockets;
	m_socketCount = socketCount;

	//
	m_wID = 0x0B; // 0x005;

	//
	m_freeslot = 1;
	m_slotrange = 0;

	//
	memset(m_clients, 0, sizeof(m_clients));
}

CNetwork::~CNetwork()
{
	for (int i = 0; i < 400; i++)
	{
		SafeDelete(m_clients[i]);
	}
}

WORD CNetwork::GetServerID(void)
{
	return m_wID;
}

void CNetwork::SendConnectlessBlob(SOCKADDR_IN *peer, BlobPacket_s *blob,
	DWORD dwFlags, DWORD dwSequence = 0, WORD wTime = 0)
{
	BlobHeader_s *header = &blob->header;

	header->dwSequence = dwSequence;
	header->dwFlags = dwFlags;
	header->dwCRC = 0;
	header->wRecID = GetServerID();
	header->wTime = wTime;
	header->wTable = 0x01;

	GenericCRC(blob);

	SendPacket(peer, blob, BLOBLEN(blob));
}

void CNetwork::SendPacket(SOCKADDR_IN *peer, void *data, DWORD len)
{
	SOCKET socket = m_sockets[0];
	if (socket == INVALID_SOCKET) return;

#ifdef _DEBUG
	LOG(Network, Verbose, "Sent:\n");
	LOG_BYTES(Network, Verbose, data, len);
#endif

	sendto(socket, (char *)data, len, 0, (sockaddr *)peer, sizeof(SOCKADDR_IN));
	g_pGlobals->PacketSent(len);
}

void CNetwork::ThinkSocket(SOCKET socket)
{
	if (socket == INVALID_SOCKET) return;

	static BYTE			buffer[0x1E4];
	static int			bloblen;
	static int			clientaddrlen = { sizeof(sockaddr_in) };
	static sockaddr_in	clientaddr;

	while (TRUE)
	{
		// Doing it similar to AC..
		int bloblen = recvfrom(socket, (char *)buffer, 0x1E4, NULL, (sockaddr *)&clientaddr, &clientaddrlen);

		if (bloblen == SOCKET_ERROR)
		{
			DWORD dwCode = WSAGetLastError();

			if (dwCode != 10035)
				LOG(Temp, Normal, "Winsock Error %lu\n", dwCode);
			break;
		}
		else if (!bloblen)
			break;
		else if (bloblen < sizeof(BlobHeader_s))
			continue;

		g_pGlobals->PacketRecv(bloblen);

		BlobPacket_s *blob = reinterpret_cast<BlobPacket_s*>(buffer);
		bloblen -= sizeof(BlobHeader_s);

		WORD wSize = blob->header.wSize;
		WORD wRecID = blob->header.wRecID;

		if (bloblen != wSize)
			continue;

		blob->header.dwCRC -= CalcTransportCRC((DWORD *)blob);

#ifdef _DEBUG
		LOG(Network, Verbose, "Received:\n");
		LOG_BYTES(Network, Verbose, &blob->header, blob->header.wSize + sizeof(blob->header));
#endif

		if (!wRecID)
		{
			ProcessConnectionless(&clientaddr, blob);
		}
		else
		{
			CClient *client = ValidateClient(wRecID, &clientaddr);
			if (client)
				client->IncomingBlob(blob);
		}
	}

}

void CNetwork::Think()
{
	for (int i = 0; i < m_socketCount; i++)
		ThinkSocket(m_sockets[i]);

	if (m_slotrange >= 400)
		m_slotrange = 399;

	for (WORD index = 1; index <= m_slotrange; index++)
	{
		CClient *client = m_clients[index];
		if (client)
		{
			client->Think();

			if (!client->IsAlive())
				KillClient(index);
		}
	}
}

CClient* CNetwork::GetClient(WORD index)
{
	if (!index || index >= 400)
		return NULL;

	return m_clients[index];
}

void CNetwork::KickClient(CClient *pClient)
{
	if (!pClient)
		return;

	LOG(Temp, Normal, "Client #%u (%s) is being kicked.\n", pClient->GetIndex(), pClient->GetAccount());
	BinaryWriter KC;
	KC.WriteLong(0xF7DC);
	KC.WriteLong(0);

	pClient->SendMessage(KC.GetData(), KC.GetSize(), PRIVATE_MSG);
	pClient->ThinkOutbound();
	pClient->Kill(NULL, NULL);
}

void CNetwork::KickClient(WORD index)
{
	KickClient(GetClient(index));
}

CClient* CNetwork::ValidateClient(WORD index, sockaddr_in *peer)
{
	CClient* pClient = GetClient(index);

	if (!pClient)
		return NULL;

	if (!pClient->CheckAddress(peer))
		return NULL;

	return pClient;
}

void CNetwork::KillClient(WORD index)
{

	CClient *pClient = GetClient(index);
	if (!pClient)
		return;

	LOG(Temp, Normal, "Client(%s, %s) disconnected. (%s)\n", pClient->GetAccount(), inet_ntoa(pClient->GetHostAddress()->sin_addr), timestamp());

	delete pClient;
	m_clients[index] = NULL;

	if (index == m_slotrange)	m_slotrange--;
	if (index < m_freeslot)		m_freeslot = index;

	UpdateClientsHUD(m_clients, m_slotrange);
}

CClient* CNetwork::FindClientByAccount(const char* account)
{
	for (WORD index = 1; index <= m_slotrange; index++)
	{
		CClient *client = m_clients[index];
		if (client)
		{
			if (client->CheckAccount(account))
				return client;
		}
	}

	return NULL;
}

void CNetwork::ConnectionRequest(sockaddr_in *addr, BlobPacket_s *p)
{
	BinaryReader cr(p->data, p->header.wSize);

	char *szVersion = cr.ReadString();	//client version stamp

	cr.ReadDWORD(); // 0x20 ?

	enum eAuthMethod {
		ePhatAC = 1,
		eUnk = 2,
		eLiveAC = 3
	};

	DWORD dwAuthMethod = cr.ReadDWORD();

	cr.ReadDWORD(); // 0x0 ?

	DWORD dwUnixTime = cr.ReadDWORD();	//client unix timestamp

	char AC2DLove[20];
	char *szAccount;

	switch (dwAuthMethod)
	{
	case ePhatAC:
		szAccount = cr.ReadString();
		break;
	case eLiveAC:
	{
		DWORD dwTicketLength = cr.ReadDWORD();
		if (dwTicketLength >= 256)
			return;
		cr.ReadArray(dwTicketLength);

		strcpy(AC2DLove, "actwod:love");
		szAccount = AC2DLove;
	}
	break;
	default:
		return;
	}

	DWORD dwPortalStamp = cr.ReadDWORD();
	DWORD dwCellStamp = cr.ReadDWORD();

	if (cr.GetLastError()) return;

	char *szPassword = strstr(szAccount, ":");
	if (!szPassword) return;

	*(szPassword) = '\0';
	szPassword++;

	int accessLevel;

	if (!g_pDB->AccountDB()->CheckAccount(szAccount, szPassword, &accessLevel))
	{
		//Bad login.
		CREATEBLOB(BadLogin, sizeof(DWORD));
		*((DWORD *)BadLogin->data) = 0x00000000;

		SendConnectlessBlob(addr, BadLogin, BT_ERROR, NULL);

		DELETEBLOB(BadLogin);
		LOG(Temp, Normal, "Invalid login from %s, used %s:%s\n", inet_ntoa(addr->sin_addr), szAccount, szPassword);
	}
	else
	{
		CClient *pExistingClient = FindClientByAccount(szAccount);

		if (pExistingClient)
		{
			KickClient(pExistingClient);
			
			// TODO don't allow this player to login for a few seconds while the world handles the other player
		}

		WORD index = GetClientSlot();

		if (index == NULL)
		{
			//Server unavailable.
			CREATEBLOB(ServerFull, sizeof(DWORD));
			*((DWORD *)ServerFull->data) = 0x00000005;

			SendConnectlessBlob(addr, ServerFull, BT_ERROR, NULL);

			DELETEBLOB(ServerFull);
		}

		LOG(Temp, Normal, "Client(%s, %s) connected on slot #%u (%s)\n", szAccount, inet_ntoa(addr->sin_addr), index, timestamp());

		CClient *client = m_clients[index] = new CClient(addr, index, szAccount, accessLevel);
		client->SetLoginData(dwUnixTime, dwPortalStamp, dwCellStamp);

		if (index > m_slotrange) m_slotrange = index;
		if (index == m_freeslot) m_freeslot++;

		//Add the client to the HUD
		UpdateClientsHUD(m_clients, m_slotrange);

		BinaryWriter AcceptConnect;

		//Some server variables.
		AcceptConnect.WriteDouble(g_pGlobals->Time());

		/*
		AcceptConnect.WriteDWORD( 0 );
		AcceptConnect.WriteDWORD( 0 );
		AcceptConnect.WriteWORD( index );

		//Mock the client's version.
		DWORD dwVersionLen = (DWORD)strlen( szVersion );
		AcceptConnect.WriteDWORD( dwVersionLen );
		AcceptConnect.AppendData( szVersion, dwVersionLen );
		*/

		/*
		AcceptConnect.WriteDWORD(0x587335ff);
		AcceptConnect.WriteDWORD(0xbd1ab10b);
		AcceptConnect.WriteWORD(0);
		AcceptConnect.WriteWORD(0);
		*/

		/*
		//Now for the CRC information
		AcceptConnect.WriteDWORD(0x20002000);

		//This CRC information could be expanded, but it would be useless to do so.
		static DWORD CRCData[] =
		{
			//The seeds will be 'AC2D' =)
			0x33667788,
			0x00000008,
			0x08C563FB, 0x3180C716, 0xFEE543FF,
			0x69E3F38A, 0x82F8CF23, 0x8059C4B9,
			0x08B63099, 0xDD80C2E5,	0xADCDADBB,

			0x00000003,
			//AC2D:
			0x8E3E7E34, 0x8E3E7E34, 0x8E3E7E34
			//ACE:
			//0x74EFCAF8, 0x4BFC9E54, 0xBBDF4885
		};

		AcceptConnect.WriteDWORD( sizeof(CRCData) );
		AcceptConnect.AppendData( CRCData, sizeof(CRCData) );
		*/

		/*
		AcceptConnect.WriteDWORD(0xAC2DAC2D);
		AcceptConnect.WriteDWORD(0xAC2DAC2D);
		*/

		/*
		AcceptConnect.WriteDWORD(0);
		AcceptConnect.WriteDWORD(0);
		AcceptConnect.WriteDWORD(0x587335ff);
		AcceptConnect.WriteDWORD(0xbd1ab10b);
		AcceptConnect.WriteWORD(0);
		AcceptConnect.WriteWORD(0);
		AcceptConnect.WriteDWORD(0x45EBD7CD);
		AcceptConnect.WriteDWORD(0x6D54AF60);
		AcceptConnect.WriteDWORD(0);
		*/

		BYTE canned[] = {
			0xbe, 0xc8, 0x8a, 0x58, 0x0b, 0x1e, 0x99, 0x43
		};
		AcceptConnect.WriteData(canned, sizeof(canned));

		/*
		BYTE canned[] = {
			0x13, 0x24, 0x46, 0x80, 0x96, 0x03, 0xc4, 0x41,
			0xbe, 0xc8, 0x8a, 0x58, 0x0b, 0x1e, 0x99, 0x43,
			0x2c, 0x00, 0x00, 0x00, 0x5e, 0x77, 0xb2, 0x33,
			0x0c, 0x76, 0x08, 0x76, 0x02, 0x00, 0x00, 0x00
		};
		AcceptConnect.WriteData(canned, sizeof(canned));
		*/

		AcceptConnect.WriteDWORD(index);
		AcceptConnect.WriteDWORD(0xAC2DAC2D);
		AcceptConnect.WriteDWORD(0xAC2DAC2D);
		AcceptConnect.WriteDWORD(2);

		DWORD dwLength = AcceptConnect.GetSize();

		if (dwLength <= 0x1D0)
		{
			CREATEBLOB(Woot, (WORD)dwLength);
			memcpy(Woot->data, AcceptConnect.GetData(), dwLength);

			SendConnectlessBlob(addr, Woot, BT_LOGINREPLY, 0x00000000);

			DELETEBLOB(Woot);
		}
		else
		{
			LOG(Temp, Normal, "AcceptConnect.GetSize() > 0x1D0");
		}
	}
}

WORD CNetwork::GetClientSlot()
{
	//Find an available slot for a connecting client.

	WORD index = m_freeslot;
	while (index < 400)
	{
		if (!m_clients[index])
			return index;

		index++;
	}

	return NULL;
}

void CNetwork::ProcessConnectionless(sockaddr_in *peer, BlobPacket_s *blob)
{
	DWORD dwFlags = blob->header.dwFlags;

	if (dwFlags == BT_LOGIN)
	{
		//if (blob->header.dwSequence != 1)
		//	LOG(Temp, Normal, "Client connecting with bad sequence?\n");

		if (!IsBannedIP(peer->sin_addr))
		{
			ConnectionRequest(peer, blob);
		}
		return;
	}

	LOG(Network, Verbose, "Unhandled connectionless packet received: 0x%08X Look into this\n", dwFlags);
}

BOOL CNetwork::IsBannedIP(in_addr ip)
{
	return FALSE;
}






