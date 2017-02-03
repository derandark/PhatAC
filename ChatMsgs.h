
#pragma once

class BinaryWriter;
class CPhysicsObj;
class CBasePlayer;

extern BinaryWriter *ChannelChat(DWORD dwChannel, const char* szName, const char* szText);
extern BinaryWriter *LocalChat(const char *szText, const char* szName, DWORD dwSourceID, long lColor = 2);
extern BinaryWriter *ActionChat(const char *szText, const char* szName, DWORD dwSourceID);
extern BinaryWriter *EmoteChat(const char *szText, const char* szName, DWORD dwSourceID);
extern BinaryWriter *ServerText(const char *szText, long lColor = 0);
extern BinaryWriter *ServerBroadcast(const char *szSource, const char *szText, long lColor = 0);
extern BinaryWriter *DirectChat(const char* szText, const char* szName, DWORD dwSourceID, DWORD dwDestID, long lColor);
