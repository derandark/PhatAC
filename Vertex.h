
#pragma once
#include "MathLib.h"

class CVertexArray
{
public:
	CVertexArray();
	~CVertexArray();

	static void SetVertexSize(DWORD Size);

	BOOL UnPack(BYTE** ppData, ULONG iSize);
	BOOL AllocateVertex(DWORD VertexCount, DWORD VertexType);
	void DestroyVertex();

	LPVOID m_pAlloc;            // 0x00
	DWORD m_VertexType;        // 0x04
	DWORD  m_VertexCount;        // 0x08
	LPVOID  m_pVertexBuffer;    // 0x0C

	static DWORD vertex_size;

#define VERTEX_NUM(vertbuffer, vertindex) \
        ((CSWVertex *)((BYTE *)vertbuffer + ((CVertexArray::vertex_size) * (vertindex))))
};

class CSWVertexUV
{
public:
	CSWVertexUV();

	BOOL UnPack(BYTE **ppData, ULONG iSize);

	float u, v;
};

class CSWVertex
{
public:
	void Init();
	void Destroy();
	BOOL UnPack(BYTE **ppData, ULONG iSize);

	Vector origin; // 0x00 0x04 0x08
	short index; // 0x0C Identifier
	short uvcount; // 0x0E
	CSWVertexUV* uvarray; // 0x10 UV Array data
	Vector normal; // 0x14 0x18 0x1C
	float unk20; // 0x20
	float unk24; // 0x24
};

class BBox
{
public:
	BBox();

	void CalcBBox(Vector& Point);

	Vector m_Min, m_Max;
};







