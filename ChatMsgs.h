
#pragma once

class NetFood;
class CPhysicsObj;
class CBasePlayer;

extern NetFood *ChannelChat(DWORD dwChannel, const char* szName, const char* szText);
extern NetFood *LocalChat(const char *szText, const char* szName, DWORD dwSourceID, long lColor = 2);
extern NetFood *ActionChat(const char *szText, const char* szName, DWORD dwSourceID);
extern NetFood *EmoteChat(const char *szText, const char* szName, DWORD dwSourceID);
extern NetFood *ServerText(const char *szText, long lColor = 0);
extern NetFood *ServerBroadcast(const char *szSource, const char *szText, long lColor = 0);
extern NetFood *DirectChat(const char* szText, const char* szName, DWORD dwSourceID, DWORD dwDestID, long lColor);
