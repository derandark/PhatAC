
#pragma once
#include "Client.h"
#include "FragStack.h"

class CPhatServer
{
public:
	CPhatServer(in_addr, u_short);
	~CPhatServer();

	void Tick(void);
	void KickClient(WORD);
	void BanClient(WORD);

	void SystemBroadcast(char *text);

	u_short	GetPort();

private:

	void InitializeSocket(u_short port);

	UINT64 m_lastframe;
	UINT64 m_frames;

	SOCKET m_sockets[10];
	int m_socketCount;

	in_addr	m_hostaddr;
	u_short	m_hostport;
};


