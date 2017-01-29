
#pragma once

#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <time.h>
#include "zlib/zlib.h"

//STL
#include <vector>
#include <list>
#include <map>
#include <hash_map>
#include <string>
#include <iterator>

#include "resource.h"

#define stricmp _stricmp
#define strlwr _strlwr

class CPhysicsObj;
class CBasePlayer;

#include "Common.h"
#include "math.h"
#include "Util.h"
#include "Globals.h"
#include "Killable.h"

extern class CDatabase *g_pDB;
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

