
#include "StdAfx.h"
#include "Client.h"
#include "ClientEvents.h"
#include "ClientCommands.h"
#include "PhysicsObj.h"
#include "Monster.h"
#include "Player.h"
#include "Door.h"
#include "Item.h"
#include "Network.h"
#include "World.h"
#include "ChatMsgs.h"
#include "Portal.h"
#include "Database2.h"
#include "GameMode.h"
#include "Lifestone.h"

// Most of these commands are just for experimenting and never meant to be used in a real game
// TODO: Add flags to these commands so they are only accessible under certain modes such as a sandbox mode

CommandMap CommandBase::m_mCommands;

void CommandBase::Create(const char* szName, const char* szArguments, const char* szHelp, pfnCommandCallback pCallback, int iAccessLevel, bool bSource)
{
	CommandEntry i =
	{
		szName,
		szArguments,
		szHelp,
		pCallback,
		iAccessLevel,
		bSource
	};

	m_mCommands[std::string(szName)] = i;
}

bool g_bSilence = false;

bool SpawningEnabled(CBasePlayer *pPlayer)
{
	if (g_bSilence)
	{
		if (pPlayer->GetClient()->GetAccessLevel() < ADMIN_ACCESS)
		{
			return false;
		}
	}

	return true;
}

CLIENT_COMMAND(myloc, "", "Info on your current location.", BASIC_ACCESS)
{
	const char* szResponse =
		csprintf("%08X %.2f %.2f %.2f %.2f %.2f %.2f %.2f",
			pPlayer->m_Origin.landcell,
			pPlayer->m_Origin.x, pPlayer->m_Origin.y, pPlayer->m_Origin.z,
			pPlayer->m_Angles.w, pPlayer->m_Angles.x, pPlayer->m_Angles.y, pPlayer->m_Angles.z);

	pPlayer->SendText(szResponse, 1);

	return false;
}

CLIENT_COMMAND(startgame, "[gameid]", "Spawns something by name (right now works for monsters, NPCs, players.)", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;
	
	int game = atoi(argv[0]);

	switch (game)
	{
	case 0:
		g_pWorld->SetNewGameMode(NULL);
		break;

	case 1:
		g_pWorld->SetNewGameMode(new CGameMode_Tag());
		break;
	}

	return false;
}

CLIENT_COMMAND(spawnportal, "", "Spawns a dysfunctional portal near you.", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	CPortal *pPortal = new CPortal();
	pPortal->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	pPortal->m_Origin = pPlayer->m_Origin;
	pPortal->m_Origin.x += 20.0f;
	pPortal->m_Angles = pPlayer->m_Angles;

	g_pWorld->CreateEntity(pPortal);

	return false;
}

CLIENT_COMMAND(spawndoor, "", "Spawns a door at your location.", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	CPhysicsObj *pDoor = new CBaseDoor();
	pDoor->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	pDoor->m_Origin = pPlayer->m_Origin;
	pDoor->m_Angles = pPlayer->m_Angles;

	g_pWorld->CreateEntity(pDoor);

	return false;
}

CLIENT_COMMAND(global, "<text> [color=1]", "Displays text globally.", ADMIN_ACCESS)
{
	if (argc < 1)
	{
		return true;
	}

	if (g_bSilence)
	{
		return false;
	}

	const char* szText = argv[0];
	long lColor = (argc >= 2) ? atoi(argv[1]) : 1;

	g_pWorld->BroadcastGlobal(ServerText(szText, lColor), PRIVATE_MSG);

	return false;
}

CLIENT_COMMAND(animationall, "<num> [speed]", "Performs an animation for everyone.", ADMIN_ACCESS)
{
	if (argc < 1)
	{
		return true;
	}

	WORD wIndex = atoi(argv[0]);
	float fSpeed = (argc >= 2) ? (float)atof(argv[1]) : 1.0f;
	float fDelay = 0.5f;

	PlayerMap *pPlayers = g_pWorld->GetPlayers();
	for (PlayerMap::iterator i = pPlayers->begin(); i != pPlayers->end(); i++)
	{
		i->second->Animation_PlayPrimary(wIndex, fSpeed, fDelay);
	}

	return false;
}

CLIENT_COMMAND(freezeall, "", "Freezes or unfreezes everyone.", ADMIN_ACCESS)
{
	if (argc < 1)
	{
		return true;
	}

	PlayerMap *pPlayers = g_pWorld->GetPlayers();
	for (PlayerMap::iterator i = pPlayers->begin(); i != pPlayers->end(); i++)
	{
		i->second->ChangeVIS(i->second->m_PhysicsState ^ (DWORD)(FROZEN_PS));
	}

	return false;
}

CLIENT_COMMAND(lineall, "", "Moves everyone.", ADMIN_ACCESS)
{
	loc_t loc = pPlayer->m_Origin;
	heading_t angles = pPlayer->m_Angles;

	PlayerMap *pPlayers = g_pWorld->GetPlayers();
	for (PlayerMap::iterator i = pPlayers->begin(); i != pPlayers->end(); i++)
	{
		if (i->second != pPlayer)
		{
			loc.x += 1.5f;
			i->second->SendText("Teleporting you...", 1);
			i->second->Movement_Teleport(loc, angles);
		}
	}

	return false;
}

CLIENT_COMMAND(effect, "<index> [scale=1]", "Emits a particle effect.", BASIC_ACCESS)
{
	if (argc < 1)
		return true;

	DWORD dwIndex;
	float flScale;

	dwIndex = atol(argv[0]);
	if (argc >= 2)	flScale = (float)atof(argv[1]);
	else				flScale = (float)1.0f;

	pPlayer->EmitEffect(dwIndex, flScale);

	return false;
}

CLIENT_COMMAND(sound, "<index> [speed?=1]", "Emits a sound effect.", BASIC_ACCESS)
{
	if (argc < 1)
		return true;

	DWORD dwIndex = atol(argv[0]);
	float flSpeed = (float)((argc >= 2) ? atof(argv[1]) : 1.0f);

	pPlayer->EmitSound(dwIndex, flSpeed);

	return false;
}

CLIENT_COMMAND(arwic, "", "Teleports you to Arwic.", BASIC_ACCESS)
{
	loc_t origin(0xC6A90023, 102.4f, 70.1f, 44.0f);
	heading_t angles(0.70710677f, 0, 0, 0.70710677f);

	pPlayer->SendText("Teleporting you to Arwic..", 1);
	pPlayer->Movement_Teleport(origin, angles);

	return false;
}

CLIENT_COMMAND(removethis, "", "Removes an object.", BASIC_ACCESS)
{
	std::string itemRemoved = pPlayer->RemoveLastAssessed();
	if (itemRemoved != "") {
		pPlayer->SendText(std::string("Removed object: ").append(itemRemoved).c_str(),1);
	}
	else
	{
		pPlayer->SendText(std::string("Please assess a valid object you wish to clear!").append(itemRemoved).c_str(), 1);
		return true;
	}
	return false;
}

CLIENT_COMMAND(tele, "<player name>", "Teleports you to a player.", BASIC_ACCESS)
{
	if (argc < 1)
		return true;

	CBasePlayer *Target = g_pWorld->FindPlayer(argv[0]);

	if (Target)
	{
		pPlayer->SendText(csprintf("Teleporting to %s ..", argv[0]), 1);
		pPlayer->Movement_Teleport(Target->m_Origin, Target->m_Angles);
	}
	else
		pPlayer->SendText(csprintf("Couldn't find player \"%s\" ?", argv[0]), 1);

	return false;
}
CLIENT_COMMAND(teleall, "<target>", "Teleports all players target. If no target specified, teleports to you.", ADMIN_ACCESS)
{
	CBasePlayer* target;
	if (argc < 1)
		target = pPlayer;
	else {
		target = g_pWorld->FindPlayer(argv[0]);
		if (target == NULL)
		{
			pPlayer->SendText("Invalid target!",1);
			return true;
		}
	}
	PlayerMap* map = g_pWorld->GetPlayers();
	PlayerMap::iterator pit = map->begin();
	PlayerMap::iterator pend = map->end();

	//This is probably really bad..
	bool teleportedOne = false;
	while (pit != pend)
	{
		CBasePlayer *them = pit->second;

		if (them != target)
		{
			teleportedOne = true;
			pPlayer->SendText(std::string("Teleported: ").append(them->GetName()).c_str(), 1);
			them->Movement_Teleport(target->m_Origin, target->m_Angles);
			them->SendText(std::string("Teleported by: ").append(pPlayer->GetName()).c_str(), 1);
		}

		pit++;
	}
	if (teleportedOne)
		return false;
	else
	{
		pPlayer->SendText("Didn't teleport anyone! Nobody on server?",1);
	}
	return true;
}
CLIENT_COMMAND(teletown, "<town name>", "Teleports you to a town.", BASIC_ACCESS)
{
	if (argc == 0) {
		pPlayer->SendText("Your teleporting choices are:", 1);
		pPlayer->SendText(g_pWorld->GetTeleportList().c_str(), 1);
		return true;
	}
	std::string cmdString = argv[0];
	for (int i = 1; i < argc; i++)
	{
		cmdString.append(" ");
		cmdString.append(argv[i]);
	}

	TeleTownList_s var = g_pWorld->GetTeleportLocation(cmdString);
	if (var.m_teleString != "")
	{
		pPlayer->SendText(std::string("Portaling To: ").append(var.m_teleString).c_str(), 1);
		pPlayer->Movement_Teleport(var.loc, var.heading);
		return false;
	}
	else
		pPlayer->SendText("Town Not Found! Try again...", 1);

	return true;

}
CLIENT_COMMAND(teleto, "<coords>", "Teleports you to coordinates.", BASIC_ACCESS)
{
	if (argc < 2)
		return true;

	float NS = NorthSouth(argv[0]);
	float EW = EastWest(argv[1]);
	char cNS = ' ';
	char cEW = ' ';
	if (NS < 0)	cNS = 'S'; else if (NS > 0) cNS = 'N';
	if (EW < 0)	cEW = 'W'; else if (EW > 0) cEW = 'E';
	loc_t		origin = GetLocation(NS, EW);
	heading_t	angles;

	if (!origin.landcell)
	{
		pPlayer->SendText("Bad coordinate location!", 1);
	}
	else if (IsWaterBlock(origin.landcell))
	{
		pPlayer->SendText("Bad location! That's a water block, dummy!", 1);
	}
	else
	{
		char* szTP = csprintf("Teleporting to %0.1f%c %0.1f%c (Cell: %08X Z: %.1f)", fabs(NS), cNS, fabs(EW), cEW, origin.landcell, origin.z);
		pPlayer->SendText(szTP, 1);
		pPlayer->Movement_Teleport(origin, angles);
	}

	return false;
}

CLIENT_COMMAND(spawnbox, "", "Spawns a box.", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	CPhysicsObj *pTurbineBox = new CPhysicsObj();
	pTurbineBox->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	pTurbineBox->m_Origin.landcell = pPlayer->GetLandcell();
	pTurbineBox->m_Origin.x = pPlayer->m_Origin.x + 1.0f;
	pTurbineBox->m_Origin.y = pPlayer->m_Origin.y;
	pTurbineBox->m_Origin.z = pPlayer->m_Origin.z + 4.0f;
	g_pWorld->CreateEntity(pTurbineBox);

	return 0;
}

CLIENT_COMMAND(spawnboxes, "", "Spawns 50 boxes. For load testing.", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	for (int i = 0; i < 50; i++)
	{
		CPhysicsObj *pTurbineBox = new CPhysicsObj();
		pTurbineBox->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
		pTurbineBox->m_Origin.landcell = pPlayer->GetLandcell();
		pTurbineBox->m_Origin.x = pPlayer->m_Origin.x + RandomFloat(0, 30);
		pTurbineBox->m_Origin.y = pPlayer->m_Origin.y + RandomFloat(0, 30);
		pTurbineBox->m_Origin.z = 0;
		g_pWorld->CreateEntity(pTurbineBox);
	}

	return 0;
}

CLIENT_COMMAND(spawndoors, "", "Spawns doors at holtburg west.", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	CPhysicsObj *pDoor1 = new CBaseDoor();
	pDoor1->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	CPhysicsObj *pDoor2 = new CBaseDoor();
	pDoor2->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	CPhysicsObj *pDoor3 = new CBaseDoor();
	pDoor3->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	CPhysicsObj *pDoor4 = new CBaseDoor();
	pDoor4->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	CPhysicsObj *pDoor5 = new CBaseDoor();
	pDoor5->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	pDoor1->SetLocation(0xA5B4003C, 174.3589935f, 77.19989777f, 46.08199692f);
	pDoor1->SetAngles(1.0f, 0.0f, 0.0f, 0.0f);
	pDoor2->SetLocation(0xA9B0000F, 36.34f, 150.33f, 58.081997f);
	pDoor2->SetAngles(1.0f, 0.0f, 0.0f, 0.0f);
	pDoor3->SetLocation(0xA9B0000F, 29.780001f, 158.07001f, 58.081997f);
	pDoor3->SetAngles(0.70339513f, 0.0f, 0.0f, -0.7107991f);
	pDoor4->SetLocation(0xA9B0010D, 37.759998f, 107.05f, 64.082001f);
	pDoor4->SetAngles(-0.70710677f, 0.0f, 0.0f, -0.70710677f);
	pDoor5->SetLocation(0xA9B00007, 12.7549f, 150.343f, 58.081997f);
	pDoor5->SetAngles(-0.70710677f, 0.0f, 0.0f, -0.70710677f);

	g_pWorld->CreateEntity(pDoor1);
	g_pWorld->CreateEntity(pDoor2);
	g_pWorld->CreateEntity(pDoor3);
	g_pWorld->CreateEntity(pDoor4);
	g_pWorld->CreateEntity(pDoor5);

	return 0;
}

CLIENT_COMMAND(vismode, "<mode>", "Changes your physics state.", BASIC_ACCESS)
{
	if (argc < 1)
		return true;

	pPlayer->ChangeVIS(strtoul(argv[0], NULL, 16));

	return false;
}

CLIENT_COMMAND(spawnbael, "", "Spawns Bael'Zharon.", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	CBaelZharon *pBael = new CBaelZharon();
	pBael->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	pBael->m_Origin.landcell = pPlayer->GetLandcell();
	pBael->m_Origin.x = pPlayer->m_Origin.x;
	pBael->m_Origin.y = pPlayer->m_Origin.y;
	pBael->m_Origin.z = pPlayer->m_Origin.z + 10;
	g_pWorld->CreateEntity(pBael);

	return false;
}

extern double g_TimeAdjustment;

CLIENT_COMMAND(timeadjust, "", "Time adjustment  Careful.", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	g_TimeAdjustment = atof(argv[0]);
	return false;
}

CLIENT_COMMAND(squelchall, "", "Squelch all.", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;
	
	g_bSilence = atoi(argv[0]) ? true : false;

	if (g_bSilence)
		pPlayer->SendText("Silenced all players and spawning.", 1);
	else
		pPlayer->SendText("Unsilenced all players and spawning.", 1);

	return false;
}

CLIENT_COMMAND(spawnrabbit, "", "Spawns a rabbit.", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	CBaseMonster *pRabbit = new CBaseMonster();

	pRabbit->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	pRabbit->m_Origin = pPlayer->m_Origin;
	pRabbit->m_Origin.z += 20.0;

	pRabbit->m_dwAnimationSet = 0x09000062;
	pRabbit->m_dwSoundSet = 0x2000003D;
	pRabbit->m_dwEffectSet = 0x3400002D;
	pRabbit->m_dwModel = 0x0200047B;
	pRabbit->m_fScale = 7.0f;

	pRabbit->m_strName = "Big White Rabbit";

	pRabbit->m_miBaseModel.SetBasePalette(0x01B4);
	pRabbit->m_miBaseModel.ReplacePalette(0x09AA, 0x00, 0x00);

	g_pWorld->CreateEntity(pRabbit);

	return false;
}

CLIENT_COMMAND(targetdrudge, "", "Spawns a Target Drudge.", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	CTargetDrudge *pDrudge = new CTargetDrudge();
	pDrudge->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	pDrudge->m_Origin.landcell = pPlayer->GetLandcell();
	pDrudge->m_Origin.x = pPlayer->m_Origin.x;
	pDrudge->m_Origin.y = pPlayer->m_Origin.y;
	pDrudge->m_Origin.z = pPlayer->m_Origin.z + 10;
	g_pWorld->CreateEntity(pDrudge);

	return false;
}

CLIENT_COMMAND(spawnwand, "", "Spawns a wand.", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	CBaseWand* pWand = new CBaseWand();
	pWand->m_dwGUID = g_pWorld->GenerateGUID(eItemGUID);
	pWand->m_Origin.landcell = pPlayer->GetLandcell();
	pWand->m_Origin.x = pPlayer->m_Origin.x;
	pWand->m_Origin.y = pPlayer->m_Origin.y;
	pWand->m_Origin.z = pPlayer->m_Origin.z;
	g_pWorld->CreateEntity(pWand);

	return false;
}

CLIENT_COMMAND(teleloc, "<landcell> [x=0] [y=0] [z=0]", "Teleports to a specific location.", BASIC_ACCESS)
{
	if (argc < 1)
		return false;

	loc_t		origin;
	heading_t	angles = pPlayer->m_Angles;

	origin.landcell = strtoul(argv[0], NULL, 16);
	origin.x = (float)((argc >= 2) ? atof(argv[1]) : 0);
	origin.y = (float)((argc >= 3) ? atof(argv[2]) : 0);
	origin.z = (float)((argc >= 4) ? atof(argv[3]) : 0);
	//angles.w = 1.0f;
	//angles.x = 0;
	//angles.y = 0;
	//angles.z = 0;
	pPlayer->Movement_Teleport(origin, angles);

	return false;
}

CLIENT_COMMAND(spawnmodel, "<model index> [scale=1] [name=*]", "Spawns a model.", BASIC_ACCESS)
{
	if (argc < 1)
	{
		return true;
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	DWORD dwModel = strtoul(argv[0], NULL, 16);
	if (!(dwModel & 0xFF000000))
		dwModel |= 0x02000000;

	float flScale = (float)((argc >= 2) ? atof(argv[1]) : 1.0f);
	const char* szName = (argc >= 3) ? argv[2] : csprintf("Model #%08X", dwModel);

	CPhysicsObj *pSpawn = new CPhysicsObj();
	pSpawn->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	pSpawn->m_dwModel = dwModel;
	pSpawn->m_ItemType = TYPE_GEM;
	pSpawn->m_fScale = flScale;
	pSpawn->m_strName = szName;
	pSpawn->m_Origin.landcell = pPlayer->GetLandcell();
	pSpawn->m_Origin.x = pPlayer->m_Origin.x;
	pSpawn->m_Origin.y = pPlayer->m_Origin.y;
	pSpawn->m_Origin.z = pPlayer->m_Origin.z;
	g_pWorld->CreateEntity(pSpawn);

	pSpawn->SetDescription(csprintf("This model was spawned by %s.\nModel #: %08X\nScale: %f\n", pPlayer->GetName(), dwModel, flScale));

	return false;
}

CLIENT_COMMAND(spawnmodels, "<start index> <end index>", "Spawns a range of models.", BASIC_ACCESS)
{
	if (argc < 2)
	{
		return true;
	}

	DWORD dwModelStart = strtoul(argv[0], NULL, 16);
	if (!(dwModelStart & 0xFF000000))
		dwModelStart |= 0x02000000;

	DWORD dwModelEnd = strtoul(argv[1], NULL, 16);
	if (!(dwModelEnd & 0xFF000000))
		dwModelEnd |= 0x02000000;

	if (dwModelStart > dwModelEnd && dwModelStart != 0)
	{
		return true;
	}

	DWORD dwCount = (dwModelEnd - dwModelStart) + 1;
	DWORD dwWidth = (int)sqrt((double)dwCount);

	if ((dwModelEnd - dwModelStart) >= 50)
	{
		return true;
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	int x = 0;
	int y = 0;
	for (DWORD i = dwModelStart; i <= dwModelEnd; i++)
	{
		CPhysicsObj *pSpawn = new CPhysicsObj();
		pSpawn->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
		pSpawn->m_dwModel = i;
		pSpawn->m_ItemType = TYPE_GEM;
		pSpawn->m_fScale = 1.0f;
		pSpawn->m_strName = csprintf("Model #%08X", i);;
		pSpawn->m_Origin.landcell = pPlayer->GetLandcell();
		pSpawn->m_Origin.x = pPlayer->m_Origin.x + (x * 10.0f);
		pSpawn->m_Origin.y = pPlayer->m_Origin.y + (y * 10.0f);
		pSpawn->m_Origin.z = pPlayer->m_Origin.z;
		pSpawn->SetDescription(csprintf("This model was spawned by %s.\nModel #: %08X", pPlayer->GetName(), i));
		g_pWorld->CreateEntity(pSpawn);

		if (x == dwWidth)
		{
			x = 0;
			y += 1;
		}
		else
			x++;

	}

	return false;
}


CLIENT_COMMAND(spawnmonster2, "<model index> <base palette>", "Spawns a monster.", BASIC_ACCESS)
{
	if (argc < 3)
	{
		return true;
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	DWORD dwModel = strtoul(argv[0], NULL, 16);
	if (!(dwModel & 0xFF000000))
		dwModel |= 0x02000000;

	DWORD dwPalette1 = strtoul(argv[1], NULL, 16);
	DWORD dwPalette2 = strtoul(argv[2], NULL, 16);
	dwPalette1 &= 0xFFFFF;
	dwPalette2 &= 0xFFFFF;

	CBaseMonster *pSpawn = new CBaseMonster();
	pSpawn->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	pSpawn->m_dwModel = dwModel;
	pSpawn->m_fScale = 1.0f;
	pSpawn->m_strName = csprintf("0x%X 0x%X 0x%X", dwModel, dwPalette1, dwPalette2);
	pSpawn->m_Origin.landcell = pPlayer->GetLandcell();
	pSpawn->m_Origin.x = pPlayer->m_Origin.x;
	pSpawn->m_Origin.y = pPlayer->m_Origin.y;
	pSpawn->m_Origin.z = pPlayer->m_Origin.z;
	pSpawn->m_miBaseModel.dwBasePalette = dwPalette1;
	pSpawn->m_miBaseModel.lPalettes.push_back(PaletteRpl(dwPalette2, 0, 0));

	g_pWorld->CreateEntity(pSpawn);

	pSpawn->SetDescription(csprintf("This monster was spawned by %s.\nModel #: %08X\nPalette #: %08X %08X\n", pPlayer->GetName(), dwModel, dwPalette1, dwPalette2));
	
	pSpawn->PostSpawn();

	return false;
}

CLIENT_COMMAND(spawnmonster, "<model index> [scale=1] [name=*] [dotcolor]", "Spawns a monster.", BASIC_ACCESS)
{	
	if (argc < 1)
	{
		return true;
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	DWORD dwModel = strtoul(argv[0], NULL, 16);
	if (!(dwModel & 0xFF000000))
		dwModel |= 0x02000000;

	float flScale = (float)((argc >= 2) ? atof(argv[1]) : 1.0f);
	const char* szName = (argc >= 3) ? argv[2] : csprintf("Model #%08X", dwModel);
	int dotColor = (int)((argc >= 4) ? atoi(argv[3]) : 0);

	CBaseMonster *pSpawn = new CBaseMonster();
	pSpawn->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	pSpawn->m_dwModel = dwModel;
	pSpawn->m_fScale = flScale;
	pSpawn->m_strName = szName;
	pSpawn->m_Origin.landcell = pPlayer->GetLandcell();
	pSpawn->m_Origin.x = pPlayer->m_Origin.x;
	pSpawn->m_Origin.y = pPlayer->m_Origin.y;
	pSpawn->m_Origin.z = pPlayer->m_Origin.z;
	pSpawn->m_BlipColor = dotColor;
	g_pWorld->CreateEntity(pSpawn);

	pSpawn->SetDescription(csprintf("This monster was spawned by %s.\nModel #: %08X\nScale: %f\n", pPlayer->GetName(), dwModel, flScale));

	pSpawn->PostSpawn();

	return false;
}

CLIENT_COMMAND(spawnitem, "<model index> [scale=1] [name=*]", "Spawns an item.", BASIC_ACCESS)
{
	if (argc < 1)
	{
		return true;
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	DWORD dwModel = strtoul(argv[0], NULL, 16);
	if (!(dwModel & 0xFF000000))
		dwModel |= 0x02000000;

	float flScale = (float)((argc >= 2) ? atof(argv[1]) : 1.0f);
	const char* szName = (argc >= 3) ? argv[2] : csprintf("Model #%08X", dwModel);

	CBaseItem *pSpawn = new CBaseItem();
	pSpawn->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	pSpawn->m_dwModel = dwModel;
	pSpawn->m_fScale = flScale;
	pSpawn->m_strName = szName;
	pSpawn->m_Origin.landcell = pPlayer->GetLandcell();
	pSpawn->m_Origin.x = pPlayer->m_Origin.x;
	pSpawn->m_Origin.y = pPlayer->m_Origin.y;
	pSpawn->m_Origin.z = pPlayer->m_Origin.z;
	g_pWorld->CreateEntity(pSpawn);

	pSpawn->SetDescription(csprintf("This item was spawned by %s.\nModel #: %08X\nScale: %f\n", pPlayer->GetName(), dwModel, flScale));

	return false;
}


CLIENT_COMMAND(spawnlifestone, "", "Spawns a lifestone.", BASIC_ACCESS)
{
	CBaseLifestone *pSpawn = new CBaseLifestone();
	pSpawn->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	pSpawn->m_Origin.landcell = pPlayer->GetLandcell();
	pSpawn->m_Origin.x = pPlayer->m_Origin.x;
	pSpawn->m_Origin.y = pPlayer->m_Origin.y;
	pSpawn->m_Origin.z = pPlayer->m_Origin.z;
	g_pWorld->CreateEntity(pSpawn);

	pSpawn->SetDescription(csprintf("Life Stone spawned by %s.", pPlayer->GetName()));

	return false;
}

CLIENT_COMMAND(clearspawns, "", "Clears the spawns in your current landblock", BASIC_ACCESS)
{
	CLandBlock *pBlock = pPlayer->GetBlock();

	if (pBlock)
		pBlock->ClearSpawns();

	pPlayer->SendText("Clearing spawns in your landblock.", 1);
	return false;
}

CLIENT_COMMAND(clearallspawns, "", "Clears  all spawns in all landblocks (slow.)", ADMIN_ACCESS)
{
	g_pWorld->ClearAllSpawns();

	pPlayer->SendText("Clearing spawns in all landblocks.", 1);
	return false;
}

SERVER_COMMAND(kick, "<player name>", "Kicks the specified player.", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	if (pPlayer)
	{
		LOG(Command, Normal, "\"%s\" is using the kick command.\n", pPlayer->GetName());
	}
	else
	{
		LOG(Command, Normal, "Server is using the kick command.\n");
	}

	CBasePlayer *pTarget = g_pWorld->FindPlayer(argv[0]);

	if (pTarget)
		g_pNetwork->KickClient(pTarget->GetClient());
	else
		pPlayer->SendText("Couldn't find target player.", 1);

	return false;
}

/*
CLIENT_COMMAND(AddSpellByID, "id", "Adds a spell by ID", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;
	
	int id = atoi(argv[0]);
	pPlayer->AddSpellByID(id);
	
	return false;
}
*/

CLIENT_COMMAND(test, "<index>", "Performs the specified test.", BASIC_ACCESS)
{
	if (argc < 1)
	{
		return true;
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	DWORD dwCell = pPlayer->GetLandcell();
	float x = pPlayer->m_Origin.x;
	float y = pPlayer->m_Origin.y;
	float z = pPlayer->m_Origin.z;

	switch (atoi(argv[0]))
	{
	case 1: {
		float goodZ = CalcSurfaceZ(dwCell, x, y);
		const char* text = csprintf("CalcZ: %f PlayerZ: %f", goodZ, z);
		pPlayer->SendText(text, 1);
		break;
	}
	case 2: {

		CBaseItem *pSpawn = new CAcademyCoat();
		pSpawn->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
		pSpawn->m_Origin.landcell = dwCell;
		pSpawn->m_Origin.x = x;
		pSpawn->m_Origin.y = y;
		pSpawn->m_Origin.z = z;
		g_pWorld->CreateEntity(pSpawn);

		BinaryWriter* co = pSpawn->CreateMessage();
		//OutputConsoleBytes(co->GetData(), co->GetSize());
		delete co;

		break;
	}
	case 3: {
		CBaseItem *pSpawn = new CBoboHelm();
		pSpawn->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
		pSpawn->m_Origin.landcell = dwCell;
		pSpawn->m_Origin.x = x;
		pSpawn->m_Origin.y = y;
		pSpawn->m_Origin.z = z;
		g_pWorld->CreateEntity(pSpawn);

		BinaryWriter* co = pSpawn->CreateMessage();
		//OutputConsoleBytes(co->GetData(), co->GetSize());
		delete co;

		break;
	}
	case 4: {
		CBaseItem *pSpawn = new CPhatRobe();
		pSpawn->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
		pSpawn->m_Origin.landcell = dwCell;
		pSpawn->m_Origin.x = x;
		pSpawn->m_Origin.y = y;
		pSpawn->m_Origin.z = z;
		g_pWorld->CreateEntity(pSpawn);

		BinaryWriter* co = pSpawn->CreateMessage();
		//OutputConsoleBytes(co->GetData(), co->GetSize());
		delete co;

		break;
	}
	case 5: {
		CBaseItem *pSpawn = new CEnvoyShield();
		pSpawn->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
		pSpawn->m_Origin.landcell = dwCell;
		pSpawn->m_Origin.x = x;
		pSpawn->m_Origin.y = y;
		pSpawn->m_Origin.z = z;
		g_pWorld->CreateEntity(pSpawn);

		BinaryWriter* co = pSpawn->CreateMessage();
		//OutputConsoleBytes(co->GetData(), co->GetSize());
		delete co;

		break;
	}
	case 6: {
		if (argc < 3)
			break;

		DWORD start = strtoul(argv[1], 0, 16);
		DWORD end = strtoul(argv[2], 0, 16);
		if (start < 0)
			start = 0;
		if (start > 0xFF)
			start = 0xFF;
		if (end < start)
			end = start;
		if (end > 0xFF)
			end = 0xFF;

		CBaseArmor *pSpawn = new CPhatRobe();
		pSpawn->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
		pSpawn->m_Origin.landcell = dwCell;
		pSpawn->m_Origin.x = x;
		pSpawn->m_Origin.y = y;
		pSpawn->m_Origin.z = z;
		pSpawn->m_miWornModel.lPalettes.clear();
		pSpawn->m_miWornModel.lPalettes.push_back(PaletteRpl(0x137F, (BYTE)start, (BYTE)(end - start)));
		g_pWorld->CreateEntity(pSpawn);

		BinaryWriter* co = pSpawn->CreateMessage();
		//OutputConsoleBytes(co->GetData(), co->GetSize());
		delete co;

		break;
	}
	case 7: {
		if (argc < 3)
			break;

		DWORD start = strtoul(argv[1], 0, 16);
		DWORD end = strtoul(argv[2], 0, 16);
		if (start < 0)
			start = 0;
		if (start > 0xFF)
			start = 0xFF;
		if (end < start)
			end = start;
		if (end > 0xFF)
			end = 0xFF;

		CPhysicsObj *pTarget = g_pWorld->FindWithinPVS(pPlayer, pPlayer->m_LastAssessed);

		if (pTarget)
		{
			pTarget->m_miBaseModel.lPalettes.clear();
			pTarget->m_miBaseModel.lPalettes.push_back(PaletteRpl(0x137F, (BYTE)start, (BYTE)(end - start)));
			pTarget->UpdateModel();
		}

		break;
	}
	}

	return false;
}

CLIENT_COMMAND(spelltest, "", "Performs a spell test.", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	//E0 and E1 are cool spellcasting animations
	//136 = moooooo

	//const char* text;
	//text = csprintf("Running spelltest..");
	//SendText(text, 1);
	if (pPlayer->m_wStance != 0x49)
		return false;

	pPlayer->SpawnCows();

	return false;
}

CLIENT_COMMAND(animation, "<index> [speed=1]", "Plays a primary animation.", BASIC_ACCESS)
{
	if (argc < 1)
		return true;

	WORD wIndex = atoi(argv[0]);
	float fSpeed = (argc >= 2) ? (float)atof(argv[1]) : 1.0f;
	float fDelay = 0.5f;
	pPlayer->Animation_PlayPrimary(wIndex, fSpeed, fDelay);

	ANIMATIONSET* pPlayerSet = g_pPortal->GetAnimationSet(0x09000001);

	if (pPlayerSet) {
		//AnimationEntry* pEntry = pPlayerSet->LookupAnimation(pPlayer->m_wStance, wIndex);
		//if (!pEntry)
		//	pPlayer->SendText("Animation index unknown to server.", 1);
		//else
		//	pPlayer->SendText(csprintf("AnimationID: %08X", pEntry->m_dwAnimID), 1);
	}

	return false;
}

/*
CLIENT_COMMAND(setmodel, "<model>", "Allows you to set your model", BASIC_ACCESS)
{
	if (argc < 1)
		return true;

	WORD wIndex = atoi(argv[0]);

	pPlayer->m_dwModel = 0x02000000 + wIndex;
	pPlayer->UpdateModel();

	return false;
}
*/

CLIENT_COMMAND(setmodel, "[monster]", "Changes your model to a monster.", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	if (argc < 1)
	{
		return true;
	}

	CCapturedWorldObjectInfo *pMonsterInfo = g_pGameDatabase->GetCapturedMonsterData(argv[0]);

	if (!pMonsterInfo)
	{
		pPlayer->SendText("Couldn't find that monster!", 1);
		return false;
	}

	float fScale = 1.0f;
	if (argc >= 2)
	{
		fScale = (float)atof(argv[1]);
		if (fScale < 0.01)
			fScale = 0.01f;
		if (fScale > 1000.0)
			fScale = 1000;
	}

	pPlayer->m_miBaseModel = pMonsterInfo->appearance;
	pPlayer->m_dwModel = pMonsterInfo->physics.setup_id;
	pPlayer->m_dwAnimationSet = pMonsterInfo->physics.mtable_id;
	pPlayer->m_dwSoundSet = pMonsterInfo->physics.stable_id;
	pPlayer->m_dwEffectSet = pMonsterInfo->physics.phstable_id;
	pPlayer->m_fScale = pMonsterInfo->physics.object_scale * fScale;

	pPlayer->UpdateEntity(pPlayer);
	return false;
}

CLIENT_COMMAND(invisible, "", "Go Invisible", BASIC_ACCESS)
{
	float fSpeed = (argc >= 2) ? (float)atof(argv[1]) : 1.0f;
	pPlayer->Animation_PlayPrimary(160, fSpeed, 0.5f);
	return false;
}

CLIENT_COMMAND(visible, "", "Go Visible", BASIC_ACCESS)
{
	float fSpeed = (argc >= 2) ? (float)atof(argv[1]) : 1.0f;
	pPlayer->Animation_PlayPrimary(161, fSpeed, 0.5f);
	return false;
}

CLIENT_COMMAND(motion, "<index> [speed=1]", "Plays a sequenced animation.", BASIC_ACCESS)
{
	if (argc < 1)
		return true;

	WORD wIndex = atoi(argv[0]);
	float fSpeed = (argc >= 2) ? (float)atof(argv[1]) : 1.0f;

	pPlayer->Animation_PlaySimpleAnimation(wIndex, fSpeed);

	return false;
}

void SendDungeonInfo(CBasePlayer* pPlayer, DungeonDesc_t* pInfo)
{
	pPlayer->SendText(csprintf("Information for ID %04X:", pInfo->wBlockID), 1);
	pPlayer->SendText(csprintf("Name: %s\nAuthor: %s\nDescription: %s", pInfo->szDungeonName, pInfo->szAuthor, pInfo->szDescription), 1);
}

void SendDungeonInfo(CBasePlayer* pPlayer, WORD wBlockID)
{
	DungeonDesc_t* pInfo = g_pWorld->GetDungeonDesc(wBlockID);

	if (!pInfo)
	{
		if (!g_pWorld->DungeonExists(wBlockID))
			pPlayer->SendText("Whatchoo' talk bout willis? (That's not a dungeon.)", 1);
		else
			pPlayer->SendText("That dungeon has not been described.", 1);
	}
	else
		SendDungeonInfo(pPlayer, pInfo);
}

CLIENT_COMMAND(dungeon, "<command>", "Dungeon commands.", BASIC_ACCESS)
{
	if (argc < 1)
		return true;

	if (!stricmp(argv[0], "help"))
	{
		if (argc < 2)
		{
			pPlayer->SendText("Dungeon commands:", 1);
			pPlayer->SendText("@dungeon help [command]\n@dungeon info [block/name]\n@dungeon list\n@dungeon random\n@dungeon search <substring>\n@dungeon set <name>\n@dungeon tele <block/name>\n@dungeon unknown", 1);
			return false;
		}

		if (!stricmp(argv[1], "info"))
		{
			pPlayer->SendText("@dungeon info - Retrieves information about your current dungeon.", 1);
			pPlayer->SendText("@dungeon info [block] - Retrieves dungeon information using a hex BlockID.", 1);
			pPlayer->SendText("@dungeon info [name] - Retrieves dungeon information by name.", 1);
		}
		else if (!stricmp(argv[1], "list"))
			pPlayer->SendText("@dungeon list - Displays a list of named dungeons.", 1);
		else if (!stricmp(argv[1], "set"))
			pPlayer->SendText("@dungeon set <name> - Sets your current dungeon's name, and its drop point.", 1);
		else if (!stricmp(argv[1], "tele"))
		{
			pPlayer->SendText("@dungeon tele <block> - Teleports to a dungeon by BlockID.", 1);
			pPlayer->SendText("@dungeon tele <name> - Teleports to a dungeon by name.", 1);
		}
		else if (!stricmp(argv[1], "unknown"))
			pPlayer->SendText("@dungeon unknown - List of unnamed dungeon BlockIDs.", 1);
		else if (!stricmp(argv[1], "random"))
			pPlayer->SendText("@dungeon random - Teleports you to a random dungeon.", 1);
		else if (!stricmp(argv[1], "search"))
			pPlayer->SendText("@dungeon search <substring> - Searchs for dungeon names that contain the substring.", 1);
		else
			pPlayer->SendText("Unknown @dungeon command. Type @dungeon help for a list of valid choices.", 1);

		return false;
	}
	else if (!stricmp(argv[0], "unknown"))
	{
		LocationMap* pDungeons = g_pWorld->GetDungeons();
		LocationMap::iterator i = pDungeons->begin();
		LocationMap::iterator e = pDungeons->end();

		DWORD dwCount = 0;
		std::string UnkList = "Unknown Dungeons:\n";
		while (i != e)
		{
			//DWORD dwCell = i->first;
			WORD wBlockID = BLOCK_WORD(i->first);

			if (!g_pWorld->GetDungeonDesc(wBlockID))
			{
				UnkList += csprintf("%04X, ", wBlockID);
				dwCount++;
			}
			i++;
		}
		UnkList += csprintf("\nEnd of Unknown Dungeons (%lu unknown)", dwCount);

		pPlayer->SendText(UnkList.c_str(), 1);
	}
	else if (!stricmp(argv[0], "tele"))
	{
		if (argc < 2)
			return true;

		if (strlen(argv[1]) < 6)
		{
			//by ID
			WORD wBlockID = (WORD)strtoul(argv[1], NULL, 16);
			bool forceFindDrop = false;

			if (argc >= 3)
			{
				forceFindDrop = atoi(argv[2]) ? true : false;
			}

			if (!wBlockID)
			{
				pPlayer->SendText("Bad block ID. Please use a valid hex value.", 1);
				return false;
			}

			DungeonDesc_t* pInfo = g_pWorld->GetDungeonDesc(wBlockID);

			if (!pInfo || forceFindDrop)
			{
				if (!g_pWorld->DungeonExists(wBlockID))
					pPlayer->SendText("That dungeon doesn't exist!", 1);
				else
				{
					loc_t result = g_pWorld->FindDungeonDrop(wBlockID);
					if (!result.landcell)
						pPlayer->SendText("Couldn't find a drop zone there.", 1);
					else
					{
						pPlayer->SendText(csprintf("Teleporting you to BlockID %04X ..", wBlockID), 1);
						pPlayer->Movement_Teleport(result, pPlayer->m_Angles);
					}
				}
			}
			else
			{
				pPlayer->SendText(csprintf("Teleporting you to %s (%04X) ..", pInfo->szDungeonName, pInfo->wBlockID), 1);
				pPlayer->Movement_Teleport(pInfo->origin, pInfo->angles);
			}
		}
		else
		{
			//by name
			const char* szName = argv[1];

			DungeonDesc_t* pInfo = g_pWorld->GetDungeonDesc(szName);
			if (pInfo)
			{
				pPlayer->SendText(csprintf("Teleporting you to %s (%04X) ..", pInfo->szDungeonName, pInfo->wBlockID), 1);
				pPlayer->Movement_Teleport(pInfo->origin, pInfo->angles);
			}
			else
				pPlayer->SendText("There is no dungeon by that name. Use @dungeon list for named dungeons. Remember to use quotes.", 1);
		}
	}
	else if (!stricmp(argv[0], "set"))
	{
		if (argc < 2)
			return true;

		DWORD dwCell = pPlayer->GetLandcell();

		if (CELL_WORD(dwCell) < 0xFF) {
			pPlayer->SendText("Go inside first, silly!", 1);
			return false;
		}

		if (strlen(argv[1]) < 6) {
			pPlayer->SendText("Please enter a name of at least 6 characters in length.", 1);
			return false;
		}
		if (strlen(argv[1]) > 80) {
			pPlayer->SendText("Please enter a shorter name.", 1);
			return false;
		}

		time_t ltime = time(NULL);
		tm*	tptr = localtime(&ltime);
		char* szTime;

		if (tptr)
		{
			szTime = asctime(tptr);
			szTime[strlen(szTime) - 1] = 0; //Remove \n
		}
		else
			szTime = "???";

		WORD wBlockID = BLOCK_WORD(dwCell);
		const char* szName = argv[1];
		const char* szAuthor = pPlayer->GetName();
		const char* szDescription = csprintf("Recorded %s", szTime);

		g_pWorld->SetDungeonDesc(wBlockID, szName, szAuthor, szDescription, pPlayer->m_Origin, pPlayer->m_Angles);

		pPlayer->SendText(csprintf("Dungeon information set. (%04X = \"%s\")", wBlockID, szName), 1);
	}
	else if (!stricmp(argv[0], "info"))
	{
		if (argc < 2)
		{
			DWORD dwCell = pPlayer->GetLandcell();
			if (CELL_WORD(dwCell) < 0xFF) {
				pPlayer->SendText("Go inside first, silly!", 1);
				return false;
			}

			SendDungeonInfo(pPlayer, BLOCK_WORD(dwCell));

			return false;
		}

		if (strlen(argv[1]) < 6)
		{
			//by ID
			WORD wBlockID = (WORD)strtoul(argv[1], NULL, 16);

			if (!wBlockID)
			{
				pPlayer->SendText("Bad block ID. Please use a valid hex value. Remember to use quotes around names!", 1);
				return false;
			}

			SendDungeonInfo(pPlayer, wBlockID);
		}
		else
		{
			//by name
			const char* szName = argv[1];

			DungeonDesc_t* pInfo = g_pWorld->GetDungeonDesc(szName);
			if (pInfo)
				SendDungeonInfo(pPlayer, pInfo);
			else
				pPlayer->SendText("There is no dungeon by that name. Use @dungeon list for named dungeons. Use quotes around names.", 1);
		}
	}
	else if (!stricmp(argv[0], "list"))
	{
		DungeonDescMap* pDDs = g_pWorld->GetDungeonDescs();
		DungeonDescMap::iterator i = pDDs->begin();
		DungeonDescMap::iterator e = pDDs->end();

		std::string DL = "Dungeon List:";
		while (i != e)
		{
			DL += csprintf("\n%04X - %s", i->second.wBlockID, i->second.szDungeonName);
			i++;
		}
		DL += csprintf("\nEnd of Dungeon List (%lu named dungeons)", (DWORD)pDDs->size());

		pPlayer->SendText(DL.c_str(), 1);
	}
	else if (!stricmp(argv[0], "search"))
	{
		if ((argc < 2) || (strlen(argv[1]) <= 0))
		{
			pPlayer->SendText("You must enter a search string.", 1);
			return 0;
		}

		DungeonDescMap* pDDs = g_pWorld->GetDungeonDescs();
		DungeonDescMap::iterator i = pDDs->begin();
		DungeonDescMap::iterator e = pDDs->end();

		DWORD dwCount = 0;
		std::string DL = "Dungeon Matches:";
		while (i != e)
		{
			static char szDungeonName[100];
			strncpy(szDungeonName, i->second.szDungeonName, 100);

			if (strstr(strlwr(szDungeonName), strlwr(argv[1])))
			{
				DL += csprintf("\n%04X - %s", i->second.wBlockID, i->second.szDungeonName);
				dwCount++;
			}
			i++;
		}
		DL += csprintf("\nEnd of List (%lu matches)", dwCount);

		pPlayer->SendText(DL.c_str(), 1);
	}
	else if (!stricmp(argv[0], "random"))
	{
		loc_t result = g_pWorld->FindDungeonDrop();
		heading_t angles = pPlayer->m_Angles;

		if (!result.landcell)
		{
			pPlayer->SendText("No dungeons to teleport to.", 1);
		}
		else
		{
			WORD wBlockID = BLOCK_WORD(result.landcell);
			DungeonDesc_t* pDD;
			if (pDD = g_pWorld->GetDungeonDesc(wBlockID))
			{
				pPlayer->SendText(csprintf("Teleporting you to %s ..", pDD->szDungeonName), 1);

				result = pDD->origin;
				angles = pDD->angles;
			}
			else
				pPlayer->SendText("Teleporting you to a random dungeon..", 1);

			pPlayer->Movement_Teleport(result, angles);
		}
	}
	else
		return true;

	return false;
}

CLIENT_COMMAND(player, "<command>", "Player commands.", BASIC_ACCESS)
{
	if (argc < 1)
		return 1;

	if (!stricmp(argv[0], "help"))
	{
		if (argc < 2)
		{
			pPlayer->SendText("Player commands:", 1);
			pPlayer->SendText("@player help [command]\n@player info <name>\n@player list\n@player tele <name>", 1);
			return false;
		}

		if (!stricmp(argv[1], "info"))
			pPlayer->SendText("@player info <name> - Retrieves information on a player, by name.", 1);
		else if (!stricmp(argv[1], "list"))
			pPlayer->SendText("@player list - Displays a list of players currently logged in.", 1);
		else if (!stricmp(argv[1], "tele"))
		{
			pPlayer->SendText("@player tele <name> - Teleports to a player by name.", 1);
			pPlayer->SendText("NOTE: This is the same as the @tele command.", 1);
		}
		else
			pPlayer->SendText("Unknown @player command. Type @player help for a list of valid choices.", 1);

		return false;
	}
	else if (!stricmp(argv[0], "tele"))
	{
		pPlayer->SendText("NOTE: Use @tele instead. ;)", 1);

		tele(pPlayer, &argv[1], argc - 1);
	}
	else if (!stricmp(argv[0], "info"))
	{
		if (argc < 2)
			return true;

		CBasePlayer *pPlayer = g_pWorld->FindPlayer(argv[1]);

		if (!pPlayer)
			pPlayer->SendText(csprintf("Couldn't find player \"%s\"", argv[1]), 1);
		else
		{
			const char* info = csprintf(
				"Player Info:\nGUID: 0x%08X\nName: %s\nLocation: %08X %.1f %.1f %.1f",
				pPlayer->m_dwGUID, pPlayer->GetName(), pPlayer->GetLandcell(),
				pPlayer->m_Origin.x, pPlayer->m_Origin.y, pPlayer->m_Origin.z);

			pPlayer->SendText(info, 1);
		}
	}
	else if (!stricmp(argv[0], "list"))
	{
		PlayerMap *pPlayers = g_pWorld->GetPlayers();

		std::string strPlayerList = "Player List:";
		for (PlayerMap::iterator i = pPlayers->begin(); i != pPlayers->end(); i++)
		{
			strPlayerList += "\n";
			strPlayerList += i->second->GetName();
		}
		strPlayerList += csprintf("\nEnd of Player List (%lu)", (DWORD)pPlayers->size());

		pPlayer->SendText(strPlayerList.c_str(), 1);
	}
	else
		return true;

	return false;
}

CLIENT_COMMAND(tabletest, "", "Performs a table test.", BASIC_ACCESS)
{
	if (argc < 1)
		return true;

	long value = atol(argv[0]);

	pPlayer->SendText(csprintf("Level: %lu", GetAttributeXP(value)), 1);

	return false;
}

CLIENT_COMMAND(doomshard, "[palette=0xBF7]", "Spawns a doom shard.", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	WORD palette = 0xBF7;

	if (argc >= 1)
		palette = (unsigned short)strtoul(argv[0], NULL, 16);

	CBaseMonster *pDoomShard = new CBaseMonster();
	pDoomShard->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	pDoomShard->m_dwModel = 0x02000700;
	pDoomShard->m_fScale = 1.6f;
	pDoomShard->m_strName = "Doom Shard";
	pDoomShard->m_Origin = pPlayer->m_Origin;

	pDoomShard->m_miBaseModel.dwBasePalette = 0xBEF;
	pDoomShard->m_miBaseModel.lPalettes.push_back(PaletteRpl(palette, 0x00, 0x00));

	g_pWorld->CreateEntity(pDoomShard);
	return false;
}

CLIENT_COMMAND(spawnaerfalle, "", "Spawn aerfalle for testing if the data is available.", ADMIN_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	g_pGameDatabase->SpawnAerfalle();
	return false;
}

CLIENT_COMMAND(freecam, "", "Allows free camera movement.", BASIC_ACCESS)
{
	pPlayer->ChangeVIS(pPlayer->m_PhysicsState ^ (DWORD)(PARTICLE_EMITTER_PS));
	return false;
}

CLIENT_COMMAND(spawn, "[name] [scale] [animate 0=no 1=yes]", "Spawns something by name (right now works for monsters, NPCs, players.)", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	if (argc < 1)
		return true;

	CCapturedWorldObjectInfo *pMonsterInfo = g_pGameDatabase->GetCapturedMonsterData(argv[0]);

	if (!pMonsterInfo)
	{
		pPlayer->SendText("Couldn't find that to spawn!", 1);
		return false;
	}

	float fScale = 1.0f;
	if (argc >= 2)
	{
		fScale = (float) atof(argv[1]);
		if (fScale < 0.01)
			fScale = 0.01f;
		if (fScale > 1000.0)
			fScale = 1000;
	}

	bool bAnimate = true;
	if (argc >= 3)
	{
		bAnimate = atoi(argv[2]) ? true : false;
	}

	CPhysicsObj *pMonster = g_pGameDatabase->CreateFromCapturedData(pMonsterInfo);

	// Modify these
	pMonster->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	pMonster->m_fScale = pMonsterInfo->physics.object_scale * fScale;
	pMonster->m_Origin = pPlayer->m_Origin;

	// Add and spawn it
	g_pWorld->CreateEntity(pMonster);	
	pMonster->PostSpawn();

	return false;
}

CLIENT_COMMAND(spawnitem2, "[name] [scale", "Spawns something by name (works for most items.)", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	if (argc < 1)
	{
		return true;
	}

	CCapturedWorldObjectInfo *pItemInfo = g_pGameDatabase->GetCapturedItemData(argv[0]);

	if (!pItemInfo)
	{
		pPlayer->SendText("Couldn't find that to spawn!", 1);
		return false;
	}

	float fScale = 1.0f;
	if (argc >= 2)
	{
		fScale = (float)atof(argv[1]);
		if (fScale < 0.01)
			fScale = 0.01f;
		if (fScale > 1000.0)
			fScale = 1000;
	}

	CPhysicsObj *pItem = g_pGameDatabase->CreateFromCapturedData(pItemInfo);

	pItem->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	pItem->m_fScale = pItemInfo->physics.object_scale * fScale;
	pItem->m_Origin = pPlayer->m_Origin;	
	pItem->m_dwCoverage2 = 0; // Not equipped

	g_pWorld->CreateEntity(pItem);
	return false;
}

CLIENT_COMMAND(spawnarmor, "[name]", "Spawns armor by name.", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	if (argc < 1)
	{
		return true;
	}

	CCapturedWorldObjectInfo *pItemInfo = g_pGameDatabase->GetCapturedArmorData(argv[0]);

	if (!pItemInfo)
	{
		pPlayer->SendText("Couldn't find that to spawn!", 1);
		return false;
	}

	float fScale = 1.0f;
	bool bAnimate = true;

	if (argc >= 2)
	{
		fScale = (float)atof(argv[1]);
		if (fScale < 0.01)
			fScale = 0.01f;
		if (fScale > 1000.0)
			fScale = 1000;
	}

	if (argc >= 3)
	{
		bAnimate = atoi(argv[2]) ? true : false;
	}

	CPhysicsObj *pItem = g_pGameDatabase->CreateFromCapturedData(pItemInfo);

	pItem->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	pItem->m_fScale = pItemInfo->physics.object_scale * fScale;
	pItem->m_dwCoverage2 = 0;
	pItem->m_Origin = pPlayer->m_Origin;

	if (!bAnimate)
	{
		pItem->m_AnimOverrideData = NULL;
		pItem->m_AnimOverrideDataLen = 0;
		pItem->m_AutonomousMovement = 0;
	}

	g_pWorld->CreateEntity(pItem);
	return false;
}

CLIENT_COMMAND(spawnlist, "[num to list]", "Lists spawnable objects.", BASIC_ACCESS)
{
	int num = 20;
	if (argc >= 1)
	{
		num = atoi(argv[0]);
		if (num < 0)
			num = 0;
		if (num >= 100)
			num = 100;
	}

	while (num > 0)
	{
		CCapturedWorldObjectInfo *pMonsterInfo = g_pGameDatabase->GetRandomCapturedMonsterData();

		if (!pMonsterInfo)
		{
			pPlayer->SendText("Couldn't find anything to spawn!", 1);
			return false;
		}

		pPlayer->SendText(pMonsterInfo->weenie._name.c_str(), 1);
		num--;
	}
	return false;
}


CLIENT_COMMAND(spawnrandomarmor, "", "Spawns random armor.", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	CCapturedWorldObjectInfo *pItemInfo = g_pGameDatabase->GetRandomCapturedArmorData();

	if (!pItemInfo)
	{
		pPlayer->SendText("Couldn't find anything to spawn!", 1);
		return false;
	}

	CPhysicsObj *pItem = g_pGameDatabase->CreateFromCapturedData(pItemInfo);

	pItem->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
	pItem->m_dwCoverage2 = 0;
	pItem->m_Origin = pPlayer->m_Origin;
	pItem->m_dwEquipSlot = 0; // ???
	pItem->m_dwEquipType = 0; // ???
	pItem->m_AnimOverrideData = NULL;
	pItem->m_AnimOverrideDataLen = 0;
	pItem->m_AutonomousMovement = 0;

	g_pWorld->CreateEntity(pItem);
	return false;
}

CLIENT_COMMAND(spawnrandom, "[num to spawn] [scale]", "Spawns random objects.", BASIC_ACCESS)
{
	if (argc < 1)
	{
		return true;
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	int num = atoi(argv[0]);
	if (num < 0)
		num = 0;
	if (num >= 20)
		num = 20;
	
	float fScale = 1.0f;
	if (argc >= 2)
	{
		fScale = (float) atof(argv[1]);
		if (fScale < 0.01f)
			fScale = 0.01f;
		if (fScale >= 1000.0f)
			fScale = 1000.0f;
	}

	int total = num;

	while (num > 0)
	{
		CCapturedWorldObjectInfo *pMonsterInfo = g_pGameDatabase->GetRandomCapturedMonsterData();

		if (!pMonsterInfo)
		{
			pPlayer->SendText("Couldn't find anything to spawn!", 1);
			return false;
		}

		CPhysicsObj *pMonster = g_pGameDatabase->CreateFromCapturedData(pMonsterInfo);

		pMonster->m_dwGUID = g_pWorld->GenerateGUID(eDynamicGUID);
		pMonster->m_fScale = pMonsterInfo->physics.object_scale * fScale;
		pMonster->m_dwCoverage2 = 0;
		pMonster->m_AnimOverrideData = NULL;
		pMonster->m_AnimOverrideDataLen = 0;
		pMonster->m_AutonomousMovement = 0;

		pMonster->m_Origin = pPlayer->m_Origin;
		pMonster->m_Origin.x += (float)RandomFloat(-2.0f * total, 2.0f * total);
		pMonster->m_Origin.y += (float)RandomFloat(-2.0f * total, 2.0f * total);

		g_pWorld->CreateEntity(pMonster);
		pMonster->PostSpawn();

		num--;
	}
	return false;
}

const char* CommandBase::Info(CommandEntry* pCommand)
{
	const char* szName = pCommand->name;
	const char* szArgs = pCommand->args;
	const char* szHelp = pCommand->help;

	if (strlen(szArgs) <= 0)
		return csprintf("Syntax: @%s - %s", szName, szHelp);
	else
		return csprintf("Syntax: @%s %s - %s", szName, szArgs, szHelp);
}

CommandEntry* CommandBase::FindCommand(const char* szName, int iAccessLevel)
{
	CommandMap::iterator i = m_mCommands.find(szName);

	if (i == m_mCommands.end())
		return NULL;

	if (iAccessLevel == -1 || iAccessLevel >= i->second.access)
		return &i->second;
	else
		return NULL;
}

bool CommandBase::Execute(char* szCommand, CBasePlayer *pPlayer, int iAccessLevel)
{
	char* argv[MAX_ARGUMENTS];
	int	argc = 0;
	bool inquote = false;
	char *argstart = szCommand;
	char *p = szCommand;

	// Quote-delimited arguments.
	for (; p[0] != '\0' && argc < MAX_ARGUMENTS; p++)
	{
		char feed = p[0];

		if (inquote)
		{
			if (feed == '\"')
			{
				inquote = false;

				*p = 0;
				argv[argc++] = argstart;
				argstart = p + 1;
				continue;
			}
		}
		else
		{
			if (feed == ' ')
			{
				if (argstart == p)
					argstart++;
				else
				{
					*p = 0;
					argv[argc++] = argstart;
					argstart = p + 1;
				}
			}

			if (feed == '\"')
			{
				inquote = true;
				argstart = p + 1;
			}
		}
	}

	if (argstart[0] != 0 && argc < MAX_ARGUMENTS)
	{
		argv[argc++] = argstart;
	}

	if (argc > 0)
	{
		char* szCommandName = argv[0];
		if (!stricmp(szCommandName, "help"))
		{
			if (!pPlayer)
				return true;

			if (argc > 1) {
				CommandEntry* pCommand = FindCommand(argv[1], iAccessLevel);

				if (pCommand)
				{
					pPlayer->SendText(Info(pCommand), 1);
					return true;
				}
				else
					pPlayer->SendText("Unknown command. Use !help to receive a list of commands.", 1);
			}
			else
			{
				// List all commands.
				std::string strCommandList = "Command List: \n";

				for (CommandMap::iterator it = m_mCommands.begin(); it != m_mCommands.end(); it++)
				{
					if (it->second.access <= iAccessLevel) {
						strCommandList += Info(&it->second);
						strCommandList += "\n";
					}
				}
				strCommandList += "Use !help <command> to receive command syntax.\n";
				pPlayer->SendText(strCommandList.c_str(), 1);
				return true;
			}
		}
		else
		{
			CommandEntry* pCommand = FindCommand(szCommandName, iAccessLevel);

			if (!pCommand)
			{
				if (pPlayer)
					pPlayer->SendText("Unknown command!", 1);
			}
			else
			{
				if (!pCommand->source || pPlayer)
				{
					// Run the command callback.
					if ((*pCommand->func)(pPlayer, argv + 1, argc - 1))
					{
						//Error
						if (pPlayer)
							pPlayer->SendText(Info(pCommand), 1);
					}
					else
						return true;
				}
			}
		}
	}

	return false;
}

ClientCommand::ClientCommand(const char* szName, const char* szArguments, const char* szHelp, pfnCommandCallback pCallback, int iAccessLevel)
{
	CommandBase::Create(szName, szArguments, szHelp, pCallback, iAccessLevel, true);
}

ServerCommand::ServerCommand(const char* szName, const char* szArguments, const char* szHelp, pfnCommandCallback pCallback, int iAccessLevel)
{
	CommandBase::Create(szName, szArguments, szHelp, pCallback, iAccessLevel, false);
}
