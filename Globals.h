
#pragma once

class CGlobals
{
public:
	CGlobals(HWND hWindow);
	~CGlobals();

	double Time();
	void Update();

	char *GetGameDirectory();
	HWND GetWindowHandle();

	void SetClientCount(WORD wCount);
	WORD GetClientCount();

	void PacketSent(DWORD dwLength);
	void PacketRecv(DWORD dwLength);
	UINT64 GetPacketSendCount();
	UINT64 GetPacketRecvCount();
	UINT64 GetPacketSendSize();
	UINT64 GetPacketRecvSize();
	void ResetPackets();

	UINT64 m_CounterStart;
	UINT64 m_CounterTime;
	UINT64 m_CounterFreq;

private:
	HWND m_hWnd;
	char m_GameDir[MAX_PATH + 2];

	WORD m_wClientCount;
	UINT64 m_cPacketSendCount;
	UINT64 m_cPacketRecvCount;
	UINT64 m_cPacketSendSize;
	UINT64 m_cPacketRecvSize;

	double m_fTime;
};

extern CGlobals *g_pGlobals;

class CStopWatch
{
public:
	CStopWatch()
	{
		m_fStartTime = g_pGlobals->Time();
	}

	double GetElapsed()
	{
		return (g_pGlobals->Time() - m_fStartTime);
	}

private:
	double m_fStartTime;
};

