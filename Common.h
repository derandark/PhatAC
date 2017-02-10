
#pragma once

#pragma warning(disable: 4200)
#pragma pack(push, 1)

#if _DEBUG
#define DEBUG_BREAK() DebugBreak()
#else
#define DEBUG_BREAK()
#endif

#define SafeDelete(x) { if (x) { delete x; x = 0; } }
#define SafeDeleteArray(x) { if (x) { delete [] x; x = 0; } }

#include "NetworkDefs.h"
#include "Enums.h"
#include "Logging.h"
#include "Killable.h"
#include "Math.h"
#include "Util.h"
#include "Globals.h"
