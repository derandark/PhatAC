
#include "StdAfx.h"
#include "TurbineFormats.h"

bool LoadDataFromFile(const char *filepath, BYTE **data, DWORD *length)
{
	*data = NULL;
	*length = 0;

	FILE *fp = fopen(filepath, "rb");

	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		long _fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		if (_fileSize < 0)
			_fileSize = 0;

		DWORD fileSize = (DWORD)_fileSize;

		BYTE *fileData = new BYTE[fileSize];
		fread(fileData, fileSize, 1, fp);
		fclose(fp);

		*data = fileData;
		*length = fileSize;
		return true;
	}

	return false;
}

static char szReadBuffer[1024];
static char szWriteBuffer[600];

char* csprintf(const char *format, ...)
{
	szReadBuffer[0] = 0;

	va_list args;
	va_start(args, format);
	_vsnprintf(szReadBuffer, 1024, format, args);
	va_end(args);

	szReadBuffer[1023] = '\0';

	return szReadBuffer;
}

char *timestamp()
{
	static char result[16];
	tm *beef;
	time_t cake;

	time(&cake);
	beef = localtime(&cake);

	if (beef)
		sprintf(result, "%.2d:%.2d:%.2d", beef->tm_hour, beef->tm_min, beef->tm_sec);
	else
		result[0] = 0;

	return result;
}

unsigned long ResolveIPFromHost(const char *host)
{
	ULONG ipaddr;

	// Check if it's in IP format.
	ipaddr = ::inet_addr(host);

	if (ipaddr && ipaddr != INADDR_NONE)
		return (unsigned long)ipaddr;

	// Try to resolve an IP address.

	LPHOSTENT lphost;
	lphost = gethostbyname(host);

	if (lphost != NULL)
	{
		ipaddr = ((LPIN_ADDR)lphost->h_addr)->s_addr;

		if (ipaddr && ipaddr != INADDR_NONE)
			return (unsigned long)ipaddr;
	}

	return 0;
}

unsigned long GetLocalIP()
{
	char hostname[256];
	gethostname(hostname, 256);
	DWORD hostaddr = *((DWORD *)gethostbyname(hostname)->h_addr);

	return *(unsigned long *)gethostbyname(hostname)->h_addr;
}

std::string GetLocalIPString()
{
	unsigned long localIP = GetLocalIP();
	return inet_ntoa(*(in_addr *)&localIP);
}

std::string DebugBytesToString(void *_data, unsigned int len)
{
	BYTE *data = (BYTE *)_data;

	std::string strBytes;

	for (unsigned int i = 0; i < len; i++)
	{
		char temp[3];
		sprintf(temp, "%02X", data[i]);
		strBytes += temp;

		if (!((i + 1) % 16))
			strBytes += "\n";
		else
			strBytes += " ";
	}

	if (len % 16)
	{
		strBytes += "\n";
	}

	return strBytes;
}

void MsgBox(UINT type, const char *format, ...)
{
	szWriteBuffer[0] = 0;

	va_list args;
	va_start(args, format);
	_vsnprintf(szWriteBuffer, 1024, format, args);
	va_end(args);

	HWND hWnd = g_pGlobals ? g_pGlobals->GetWindowHandle() : NULL;

	MessageBox(hWnd, szWriteBuffer, "PhatAC", MB_OK | type);
}

void MsgBox(const char *format, ...)
{
	szWriteBuffer[0] = 0;

	va_list args;
	va_start(args, format);
	_vsnprintf(szWriteBuffer, 1024, format, args);
	va_end(args);

	MessageBox(NULL, szWriteBuffer, "PhatAC", MB_OK);
}

void MsgBoxError(DWORD dwError, const char* event)
{
	LPVOID lpMsgBuf;
	if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL))
	{
		MsgBox(MB_ICONHAND, "Unknown error #%lu %s", dwError, event);
		return;
	}

	MsgBox(MB_ICONHAND, "Error #%lu %s:\r\n%s", dwError, event, lpMsgBuf);

	LocalFree(lpMsgBuf);
}

#define REGLOC HKEY_LOCAL_MACHINE, "Software\\PhatAC"

BOOL SaveConfigKey(const char* Key, DWORD value)
{
	BOOL bReturn = FALSE;
	HKEY hKey;
	if (RegCreateKeyEx(REGLOC, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
	{
		if (SUCCEEDED(RegSetValueEx(hKey, Key, NULL, REG_DWORD, (BYTE *)&value, sizeof(DWORD))))
			bReturn = TRUE;
		RegCloseKey(hKey);
	}
	return bReturn;
}

BOOL SaveConfigKey(const char* Key, const char* value)
{
	BOOL bReturn = FALSE;
	HKEY hKey;
	if (RegCreateKeyEx(REGLOC, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
	{
		if (SUCCEEDED(RegSetValueEx(hKey, Key, NULL, REG_SZ, (BYTE*)value, (DWORD)strlen(value) + 1)))
			bReturn = TRUE;
		RegCloseKey(hKey);
	}
	return bReturn;
}

BOOL ReadConfigKey(const char* Key, DWORD* value)
{
	DWORD type = REG_DWORD;
	DWORD size = sizeof(DWORD);
	BOOL bReturn = FALSE;
	HKEY hKey;
	if (RegOpenKeyEx(REGLOC, NULL, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		if (!RegQueryValueEx(hKey, Key, NULL, &type, (BYTE*)value, &size))
			bReturn = TRUE;
		RegCloseKey(hKey);
	}
	return bReturn;
}

BOOL ReadConfigKey(const char* Key, char* value, DWORD size)
{
	DWORD type = REG_SZ;
	BOOL bReturn = FALSE;
	HKEY hKey;
	if (RegOpenKeyEx(REGLOC, NULL, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
		if (!RegQueryValueEx(hKey, Key, NULL, &type, (BYTE*)value, &size))
			bReturn = TRUE;
		RegCloseKey(hKey);
	}
	return bReturn;
}

bool FileExists(const char *filePath)
{
	DWORD dwAttrib = GetFileAttributes(filePath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void strtrim(char *szText)
{
	//how many left spaces are leading?
	int leading = 0;
	while (szText[leading] == ' ')
		leading++;

	if (leading) { //shift left
		int i = leading;
		while (1)
		{
			szText[i - leading] = szText[i];
			if (szText[i] == 0)
				break;
			i++;
		}
	}

	int pos = (int)strlen(szText) - 1; //remove trailing spaces
	while ((pos >= 0) && szText[pos] == ' ')
		szText[pos--] = 0;
}

BOOL strmask(const char* szTest, const char* szMask)
{
	//We need to someday create a function for validating a string mask.
	return FALSE;
}

long fsize(FILE* fileptr)
{
	long lOld, lSize;

	lOld = ftell(fileptr);
	fseek(fileptr, 0, SEEK_END);
	lSize = ftell(fileptr);
	fseek(fileptr, lOld, SEEK_SET);

	return lSize;
}



float NorthSouth(char *szCoord)
{
	strlwr(szCoord);
	strtrim(szCoord);

	int len = (int)strlen(szCoord);
	if (len < 1)
		return 0;

	char *end = &szCoord[len - 1];
	char NS = *end;
	*end = 0;

	strtrim(szCoord);

	float dir = 0.0f;
	if (NS == 'n')
		dir = 1.0f;
	else if (NS == 's')
		dir = -1.0f;

	float coord = 0.0f;
	sscanf(szCoord, "%f", &coord);

	return coord * dir;
}

float EastWest(char *szCoord)
{
	strlwr(szCoord);
	strtrim(szCoord);

	int len = (int)strlen(szCoord);
	if (len < 1)
		return 0;

	char *end = &szCoord[len - 1];
	char EW = *end;
	*end = 0;
	strtrim(szCoord);

	float dir = 0.0f;
	if (EW == 'e')
		dir = 1.0f;
	else if (EW == 'w')
		dir = -1.0f;

	float coord = 0.0f;
	sscanf(szCoord, "%f", &coord);

	return coord * dir;
}

DWORD GetCellFromBase(DWORD BaseX, DWORD BaseY)
{
	BYTE blockx = (BYTE)(BaseX >> 3);
	BYTE blocky = (BYTE)(BaseY >> 3);
	BYTE cellx = (BYTE)(BaseX & 7);
	BYTE celly = (BYTE)(BaseY & 7);

	WORD block = (blockx << 8) | (blocky);
	WORD cell = (cellx << 3) | (celly);

	DWORD dwCell = (block << 16) | (cell + 1);
	return dwCell;
}

loc_t GetLocation(double NS, double EW)
{
	NS -= 0.5f;
	EW -= 0.5f;
	NS *= 10.0f;
	EW *= 10.0f;

	DWORD basex = (DWORD)(EW + 0x400);
	DWORD basey = (DWORD)(NS + 0x400);

	if (long(basex) < 0 || long(basey) < 0 || basex >= 0x7F8 || basey >= 0x7F8)
	{
		//Out of bounds.
		return loc_t();
	}

	DWORD dwCell = GetCellFromBase(basex, basey);
	float xOffset = ((basex & 7) * 24.0f) + 12;
	float yOffset = ((basey & 7) * 24.0f) + 12;

	loc_t Result;
	Result.landcell = dwCell;
	Result.x = xOffset;
	Result.y = yOffset;
	Result.z = CalcSurfaceZ(dwCell, xOffset, yOffset);

	return Result;
}

// Water blocks are not passable!
BOOL IsWaterBlock(BlockData *pBlock)
{
	for (unsigned int x = 0; x < 9; x++) {
		for (unsigned int y = 0; y < 9; y++) {
			if ((pBlock->wSurface[x][y] & 0x50) != 0x50)
				return FALSE;
		}
	}

	return TRUE;
}

//
BOOL IsWaterBlock(DWORD dwCell)
{
	dwCell &= 0xFFFF0000;
	dwCell |= 0x0000FFFF;

	TURBINEFILE *pFile = g_pCell->GetFile(dwCell);

	if (!pFile)
		return FALSE;

	BOOL bResult = IsWaterBlock((BlockData*)pFile->GetData());
	delete pFile;

	return bResult;
}

BOOL HasObjectBlock(BlockData *pBlock)
{
	return ((pBlock->bObject) ? TRUE : FALSE);
}

BOOL HasObjectBlock(DWORD dwCell)
{
	dwCell &= 0xFFFF0000;
	dwCell |= 0x0000FFFF;

	TURBINEFILE *pFile = g_pCell->GetFile(dwCell);

	if (!pFile)
		return FALSE;

	BOOL bResult = HasObjectBlock((BlockData*)pFile->GetData());
	delete pFile;

	return bResult;
}

float CalcSurfaceZ(DWORD dwCell, float xOffset, float yOffset)
{
	TURBINEFILE *pFile = g_pCell->GetFile((dwCell & 0xFFFF0000) | 0xFFFF);
	if (!pFile)
		return 0;

	WORD cell = CELL_WORD(dwCell);
	int minix = (cell >> 3) & 7;
	int miniy = (cell >> 0) & 7;
	xOffset -= (24 * minix);
	yOffset -= (24 * miniy);

	BlockData *pBlock = (BlockData*)pFile->GetData();

	Vector P1(0, 24, pBlock->bHeight[minix][miniy + 1] * 2.0f);
	Vector P2;
	Vector P3(24, 0, pBlock->bHeight[minix + 1][miniy] * 2.0f);

	if ((xOffset + yOffset) < 1.0f)
		P2 = Vector(0, 0, pBlock->bHeight[minix][miniy] * 2.0f);
	else
		P2 = Vector(24, 24, pBlock->bHeight[minix + 1][miniy + 1] * 2.0f);

	delete pFile;

	return FindVectorZ(P1, P2, P3, xOffset, yOffset);
}