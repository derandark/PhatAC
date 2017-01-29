
#pragma once
#include "TurbineObject.h"

class TurbineDungeon;
class DungeonPortal;
class DungeonVertex;
class DungeonTriFan;

typedef TurbineDungeon DUNGEON, *LPDUNGEON;
typedef DungeonPortal DUNGEONPORTAL, *LPDUNGEONPORTAL;
typedef DungeonVertex DUNGEONVERTEX, *LPDUNGEONVERTEX;
typedef DungeonTriFan DUNGEONTRIFAN, *LPDUNGEONTRIFAN;

class DungeonVertex
{
public:
	DungeonVertex()
	{
		x = y = z = nx = ny = nz = 0;
	}

	float x, y, z, nx, ny, nz;
};

class DungeonTriFan
{
public:
	DungeonTriFan()
	{
		m_pVertexIndices = NULL;
		m_cNumVertices = 0;
	}

	~DungeonTriFan()
	{
		SafeDeleteArray(m_pVertexIndices);
	}

	WORD *m_pVertexIndices;
	unsigned char m_cNumVertices;
};

class DungeonPortal
{
public:
	DungeonPortal()
	{
		m_pVertices = NULL;
		m_dwNumVertices = 0;

		m_pTriFans = NULL;
		m_dwNumTriFans = 0;
	}

	~DungeonPortal()
	{
		SafeDeleteArray(m_pVertices);
		SafeDeleteArray(m_pTriFans);
	}

	DUNGEONVERTEX *m_pVertices;
	DWORD m_dwNumVertices;

	DUNGEONTRIFAN *m_pTriFans;
	DWORD m_dwNumTriFans;
};

class TurbineDungeon : public TurbineObject
{
public:
	TurbineDungeon(DWORD dwID);
	~TurbineDungeon();

	void Initialize(BYTE *pbData, DWORD dwLength);

	BOOL IsLandingZone(DUNGEONVERTEX* pVertex1, DUNGEONVERTEX* pVertex2, DUNGEONVERTEX* pVertex3);
	loc_t FindLandingZone(WORD wPortal);

protected:

	DWORD m_dwNumPortals;
	DUNGEONPORTAL *m_pPortals;

};