

#pragma once

class CNetwork
{
public:
	CNetwork(SOCKET *sockets, int socketCount);
	~CNetwork();

	void Think();
	void ThinkSocket(SOCKET socket);
	CClient *GetClient(WORD index);

	WORD GetServerID();

	void KickClient(class CClient* pClient);
	void KickClient(WORD index);
	void KillClient(WORD index);
	void SendPacket(SOCKADDR_IN *, void *data, DWORD len);
	void SendConnectlessBlob(SOCKADDR_IN *, BlobPacket_s *, DWORD dwFlags, DWORD dwSequence, WORD wTime);

private:

	BOOL IsBannedIP(in_addr);

	WORD GetClientSlot();

	CClient *ValidateClient(WORD, sockaddr_in *);
	CClient *FindClientByAccount(const char *);

	void ConnectionRequest(sockaddr_in *addr, BlobPacket_s *p);
	void ProcessConnectionless(sockaddr_in *, BlobPacket_s *);

	SOCKET *m_sockets;
	int m_socketCount;

	WORD m_wID;

	WORD m_freeslot;
	WORD m_slotrange;
	CClient* m_clients[400];
};




