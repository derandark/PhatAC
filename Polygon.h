
#pragma once

#include "MathLib.h"

class CVertexArray;
class CSWVertex;

enum eCullModes
{
	CullCW = 0,
	CullNone = 1,
	CullUnknown = 2
};

class CPolygon
{
public:
	CPolygon();
	~CPolygon();

	static void SetPackVerts(CVertexArray *Verts);

	void Destroy();
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	void make_plane();
	BOOL polygon_hits_ray(const Ray& ray, float *time);
	BOOL point_in_polygon(const Vector& point);

	// !! PeaFIXed
	BOOL peafixed_polygon_hits_ray(const Ray& ray, float *depth);

	static CVertexArray *pack_verts;

	CSWVertex**            m_pVertexObjects;    // 0x00
	WORD*                m_pVertexIndices;    // 0x04
	CSWVertex**            m_pVertexUnks;        // 0x08
	short                m_iPolyIndex;        // 0x0C
	BYTE                m_iVertexCount;        // 0x0E
	BYTE                m_iPolyType;        // 0x0F
	DWORD                m_iCullType;        // 0x10 - 0=CULLCW, 1=CULLNONE (aka: m_iUnknown)
	BYTE*                m_Face1_UVIndices;    // 0x14
	BYTE*                m_Face2_UVIndices;    // 0x18
	short                m_Face1_TexIndex;    // 0x1C
	short                m_Face2_TexIndex;    // 0x1E

	Plane                m_plane;            // 0x20

};
