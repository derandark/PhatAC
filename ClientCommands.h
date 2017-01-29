
using namespace std;

class CBasePlayer;

class CommandBase;
class ClientCommand;
class ServerCommand;

//Note: Server commands don't require a source player.
typedef bool(*pfnCommandCallback)(class CBasePlayer* player, char* argv[], int argc);

enum {
	BASIC_ACCESS = 1,
	PRIVILEGED_ACCESS,
	ADMIN_ACCESS,
	SERVER_ACCESS,
	DUMMY_ACCESS
};

#define MAX_ARGUMENTS 12

struct CommandEntry {
	const char* name;
	const char* args;
	const char* help;
	pfnCommandCallback func;
	int access;
	bool source;
};
typedef map<string, CommandEntry> CommandMap;

class CommandBase
{
	friend class ClientCommand;
	friend class ServerCommand;

public:
	static bool Execute(char* szCommand, CBasePlayer* pSource, int iAccessLevel);

protected:
	static void Create(const char* szName, const char* szArguments, const char* szHelp, pfnCommandCallback pCallback, int iAccessLevel, bool bSource);
	static const char* Info(CommandEntry* pCommand);
	static CommandEntry* FindCommand(const char* szName, int iAccessLevel = -1);

	static CommandMap m_mCommands;
};

class ClientCommand : CommandBase
{
	friend class CommandBase;
public:
	ClientCommand(const char* szName, const char* szArguments, const char* szHelp, pfnCommandCallback pCallback, int iAccessLevel);
};

class ServerCommand : CommandBase
{
	friend class CommandBase;
public:
	ServerCommand(const char* szName, const char* szArguments, const char* szHelp, pfnCommandCallback pCallback, int iAccessLevel);
};

#define CLIENT_COMMAND( name, args, help, access ) \
 static bool name(CBasePlayer* pPlayer, char* argv[], int argc); \
 static ClientCommand name##_command( #name, args, help, name, access ); \
 static bool name(CBasePlayer* pPlayer, char* argv[], int argc)

#define SERVER_COMMAND( name, args, help, access ) \
 static bool name(CBasePlayer* pPlayer, char* argv[], int argc); \
 static ServerCommand name##_command( #name, args, help, name, access ); \
 static bool name(CBasePlayer* pPlayer, char* argv[], int argc)


