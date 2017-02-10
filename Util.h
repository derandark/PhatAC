
#pragma once

///////////////////////////////////////
//
//UTIL.H - PhatAC - 02/28/2004
//
//A collection of random helper functions.
//
///////////////////////////////////////

#include <math.h>

#include "BinaryWriter.h"
#include "BinaryReader.h"

class CClient;
struct BlockData;

bool LoadDataFromFile(const char *filepath, BYTE **data, DWORD *length);

extern void MsgBox(const char* format, ...);
extern void MsgBox(UINT iType, const char* format, ...);
extern void MsgBoxError(DWORD dwError, const char* event);
extern void UpdateFramesHUD(UINT64 frames);
extern void UpdateClientsHUD(CClient **clients, WORD slotrange);

#define cs(x) csprintf(x)
extern char* csprintf(const char* format, ...); //static buffer
extern char* timestamp(); //static buffer
extern void strtrim(char *szText); //specified buffer
extern BOOL strmask(const char* szTest, const char* szMask);
extern long fsize(FILE* fp); //returns a FILE* size

unsigned long ResolveIPFromHost(const char *host);

unsigned long GetLocalIP();
std::string GetLocalIPString();

extern std::string DebugBytesToString(void *data, unsigned int len);
extern void _OutputConsole(const char* format, ...);
#define _DebugMe() LOG(Temp, Normal, "Debug me: %s %u\n", __FUNCTION__, __LINE__);

extern BOOL SaveConfigKey(const char* Key, DWORD value);
extern BOOL SaveConfigKey(const char* Key, const char* value);
extern BOOL ReadConfigKey(const char* Key, DWORD* value);
extern BOOL ReadConfigKey(const char* Key, char* value, DWORD size);

extern bool FileExists(const char *filePath);

extern float NorthSouth(char *szCoord);
extern float EastWest(char *szCoord);
extern loc_t GetLocation(double NS, double EW);
extern BOOL IsWaterBlock(BlockData*);
extern BOOL IsWaterBlock(DWORD dwCell);
extern BOOL HasObjectBlock(BlockData*);
extern BOOL HasObjectBlock(DWORD dwCell);
extern float CalcSurfaceZ(DWORD dwCell, float xOffset, float yOffset);
extern float CalcSurfaceZ2(DWORD dwCell, float xOffset, float yOffset);

// Because the land system sucks.
#define BLOCK_WORD(x) ((WORD)((x & 0xFFFF0000) >> 16))
#define BLOCK_X(x) ((BYTE)((x >> 8) & 0xFF))
#define BLOCK_Y(x) ((BYTE)((x >> 0) & 0xFF))
#define CELL_WORD(x) ((WORD)((x & 0xFFFF)-1))
#define CELL_X(x) ((BYTE)((x >> 3) & 7))
#define CELL_Y(x) ((BYTE)((x >> 0) & 7))

// Used for mapping out the above data to offsets from (0, 0).
#define BASE_OFFSET(X, x) (((DWORD)X << 3) | x)
#define BLOCK_OFFSET(x) ((WORD)((DWORD)x >> 3))

// Potential view cell-range.
#define PVC_RANGE 16

// Land boundaries.
const DWORD dwMinimumCellX = 0x0;
const DWORD dwMinimumCellY = 0x0;
const DWORD dwMaximumCellX = (0x800 - 1);
const DWORD dwMaximumCellY = (0x800 - 1);
