
#pragma once

class CClientEvents;
class CPacketController;
class BinaryWriter;
class BinaryReader;
typedef struct DungeonDesc_s DungeonDesc_t;

class CClient : public CKillable
{
	friend class CClientEvents;

public:
	CClient(SOCKADDR_IN *, WORD slot, char *account, int accessLevel);
	~CClient();

	void Think();
	void ThinkOutbound();
	void WorldThink();

	BOOL CheckAccount(const char *);
	BOOL CheckAddress(SOCKADDR_IN *);

	const char* GetAccount(); //The client's account name.
	const char* GetDescription(); //The status text to display for this client.
	WORD GetIndex(); //The client's assigned RecipientID.
	SOCKADDR_IN* GetHostAddress(); //The client's address.

	void SetLoginData(DWORD dwUnixTime, DWORD dwPortalStamp, DWORD dwCellStamp);

	void IncomingBlob(BlobPacket_s *);
	void ProcessMessage(BYTE *data, DWORD length, WORD);
	void SendMessage(BinaryWriter*, WORD group, BOOL game_event = 0, BOOL del = 1);
	void SendMessage(void *data, DWORD length, WORD group, BOOL game_event = 0);

	CClientEvents* GetEvents() { return m_pEvents; }

	int GetAccessLevel();

private:
	void UpdateLoginScreen();

	// Non-world events.
	void EnterWorld();
	void ExitWorld();
	void CreateCharacter(BinaryReader *);
	void SendLandblock(DWORD dwFileID);
	void SendLandcell(DWORD dwFileID);

	//
	BOOL CheckNameValidity(const char *name);

	// This is a dumbly separated way of parsing methods. Change this.
	CClientEvents *m_pEvents;

	// This handles parts of the network layer.
	CPacketController *m_pPC;

	struct CL_ClientVars_t
	{
		WORD slot; //The client's assigned RecipientID.
		SOCKADDR_IN addr; //The client's socket address.
		std::string account; //The client's account name.

		//WARNING: These are controlled by the client.
		DWORD unixtime; //The client's unix timestamp at login.
		DWORD portalstamp;//The client's portal.dat version.
		DWORD cellstamp; //The client's cell.dat version.

		//Server controlled variables.
		double logintime; //The high-frequency time the client connected.

		BOOL initdats; //Has the client's dat files been updated?

		BOOL needchars; //Does the client need a character list update?
		BOOL inworld; //Is the client in the world, or not?

	} m_vars;

	int m_AccessLevel;
};



