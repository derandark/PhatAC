
#pragma once

#pragma warning(disable: 4503)

#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma warning( disable : 4503 ) //4503 can safely be ignored

#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <windows.h>
#include <commctrl.h>
#include <time.h>

#include "zlib/zlib.h"
#include "mysql/mysql.h"

//STL
#include <vector>
#include <list>
#include <map>
#include <hash_map>
#include <string>
#include <iterator>
#include <algorithm>
#include <stdint.h>
#include <stdio.h>

#include "resource.h"

#define stricmp _stricmp
#define strlwr _strlwr

class CPhysicsObj;
class CBasePlayer;

#include "Logging.h"
#include "Enums.h"
#include "Common.h"
#include "Math.h"
#include "Util.h"
#include "Globals.h"
#include "Killable.h"

extern class CDatabase *g_pDB;
extern class CMYSQLDatabase *g_pDB2;
extern class CGameDatabase *g_pGameDatabase;
extern class CWorld *g_pWorld;
extern class CNetwork *g_pNetwork;

#include "DATDisk.h"
#include "TurbineData.h"
#include "TurbinePortal.h"
#include "TurbineCell.h"
#include "Rules.h"

extern TURBINEPORTAL* g_pPortal;
extern TURBINECELL* g_pCell;
extern GAMERULES* g_pGameRules;

extern bool g_bDebugToggle;

