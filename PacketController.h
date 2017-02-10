
#pragma once

class FragmentStack;

typedef std::list<BlobPacket_s *> BlobList;
typedef std::list<FragmentStack *> FragmentList;

#ifdef _DEBUG
#define FlagEvilClient() EvilClient( __FILE__, __LINE__, FALSE )
#else
#define FlagEvilClient() EvilClient( NULL, NULL, TRUE )
#endif

#pragma pack(push, 4)
struct EventHeader
{
	DWORD dwF7B0;
	DWORD dwPlayer;
	DWORD dwSequence;
};
#pragma pack(pop)

class CNetwork;
class CPacketController : public CKillable
{
public:
	CPacketController(CClient *);
	~CPacketController();

	void Think(void); //Generic work cycle.
	void ThinkInbound(void);
	void ThinkOutbound(void);

	BOOL HasConnection();

	void IncomingBlob(BlobPacket_s *);

	DWORD GetNextSequence(void);
	DWORD GetLastSequence(void);
	DWORD GetNextEvent(void);
	DWORD GetLastEvent(void);
	void ResetEvent(void);

	BOOL SendNetMessage(void *data, DWORD length, WORD group);

	template<class T>
	void EraseInternal(T *data);
	template<class T>
	void EraseInternalA(T *data);

private:
	WORD GetElapsedTime(void);

	void Cleanup(void);
	void FlushFragments(void);
	void FlushPeerCache(void);
	void PerformLoginSync();
	void UpdatePeerTime(void);
	void UpdateCharacters(void);

	void ProcessMessage(BYTE *data, DWORD length, WORD);
	void ProcessBlob(BlobPacket_s *);

	void EvilClient(const char* szSource, DWORD dwLine, BOOL bKill);
	void ResendBlob(BlobPacket_s *);
	void SendGenericBlob(BlobPacket_s *, DWORD dwFlags, DWORD dwSequence);
	void SendFragmentBlob(BlobPacket_s *);

	void QueueFragment(FragHeader_s *header, BYTE *data);

	CClient	*m_pClient;
	SOCKADDR_IN	*m_pPeer;
	BOOL m_bEvil;

	struct InputPCVars_t
	{
		InputPCVars_t()
		{
			logintime = g_pGlobals->Time();
			lastactivity = g_pGlobals->Time();
			lastrequest = g_pGlobals->Time();

			sequence = 0;
			activesequence = 1;
			flushsequence = 0;
		}

		double logintime; // When the client connected. Used for time subtraction.
		double lastactivity; // When the last SUCCESSFUL packet received.
		double lastrequest;	//When the last lost packets request was made.

		//Sequencing.
		DWORD sequence; //The blob counter. The last one we know the client sent.
		DWORD activesequence; //The blob counter. The last sequence we processed.
		DWORD flushsequence;	//The last sequence the client processed.

		//CRC
		DWORD clientcrc[3];

		std::list<FragmentStack *> fragstack; // All incomplete messages stack here.
		std::list<BlobPacket_s *> blobstack; // All queued blobs wait here. (for sequencing)
	} m_in;

	struct OutputPCVars_t
	{
		//Network variables for outgoing data.

		OutputPCVars_t()
		{
			lastflush = g_pGlobals->Time();
			lasttimeupdate = -1000.0; // g_pGlobals->Time();

			lastloginsync = 0;
			loginsyncs = 0; // -1;

			sequence = 1;
			fragment_counter = 1;
			event_counter = 0;

			initchars = FALSE;
		}

		double lastflush; //Used to clean the peer's blob cache.
		double lasttimeupdate; //Used to correct the client's time.

		double lastloginsync; //Used to sync the time at login.
		long loginsyncs; //The number of login syncs performed.

		BOOL initchars;

		//Sequencing.
		DWORD sequence;	//The blob counter.
		DWORD fragment_counter;	//The fragment counter.
		DWORD event_counter; //The game event counter.

		//CRC
		DWORD servercrc[3];

		std::list<FragPacket_s *> fragqueue; //All fragments to be sent will wait here.
		std::list<BlobPacket_s *> blobcache; //Remember blobs until the client receives them.
	} m_out;
};
