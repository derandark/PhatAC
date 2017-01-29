
#pragma once

#include "MathLib.h"

// #define COLLISION_ROUTINES

class CPolygon;
class CUIFucker;

class CSphere
{
public:
	CSphere(const Vector& origin, float radius);
	CSphere();

	ULONG pack_size();
	BOOL UnPack(BYTE** ppData, ULONG iSize);
	BOOL intersects(CSphere* pSphere);
	BOOL sphere_intersects_ray(const Ray& ray);

	Vector    m_origin;
	float    m_radius;
};

class CCylSphere
{
public:
	CCylSphere();

	ULONG pack_size();
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	Vector    m_origin;
	float    m_f0C;
	float    m_f10;
};

class CSolid
{
public:
	CSolid(const Vector& Vec);
	CSolid();

	ULONG pack_size();
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	// Alternatively, could utilize CVector class.
	Vector m_Vec;
};

class CPortalPoly
{
public:
	CPortalPoly();
	~CPortalPoly();

	BOOL UnPack(BYTE** ppData, ULONG iSize);
	void Destroy();

	long            m_iWhat;    //0x00
	CPolygon*        m_pPolygon; //0x04
};

class BSPNODE
{
public:
	BSPNODE();
	virtual ~BSPNODE(); //0x00

	void Destroy();

	static BOOL        UnPackChild(BSPNODE** pOut, BYTE** ppData, ULONG iSize);
	BOOL            UnPack(BYTE** ppData, ULONG iSize);

	// Draw routines
	void draw_no_check();
	void draw_check(DWORD unknown);

#ifdef COLLISION_ROUTINES
	virtual bool    sphere_intersects_poly(CSphere* pSphere, Vector*, CPolygon**, Vector*);            // 0x04    
	virtual bool    sphere_intersects_solid(CSphere* pSphere, BOOL);                                    // 0x08
	virtual bool    point_intersects_solid(Vector* pPoint);                                            // 0x0C
	virtual bool    sphere_intersects_solid_poly(CSphere* pSphere, float, BOOL*, CPolygon**, BOOL);    // 0x10
	virtual void    find_walkable(void* HolyFucker, CSphere*, CPolygon**, Vector*, Vector*, BOOL*);    // 0x14
	virtual bool    hits_walkable(void* HolyFucker, CSphere*, Vector*);                                // 0x18
#endif

	BOOL            point_inside_cell_bsp(const Vector& point);

	CSphere        m_bounds;                //0x04 (size: 0x10)
	Plane        m_plane;                //0x14 (size: 0x10)
	DWORD        m_NodeType;                //0x24
	DWORD        m_iTriangleCount;        //0x28
	CPolygon**    m_pTriangles;            //0x2C
	BSPNODE*    m_pChild30;
	BSPNODE*    m_pChild34;

	static DWORD        pack_tree_type;
	static CPolygon*    pack_poly;
};

class BSPLEAF : public BSPNODE
{
public:
	BSPLEAF();
	virtual ~BSPLEAF();

	BOOL            UnPackLeaf(BYTE** ppData, ULONG iSize);

#ifdef COLLISION_ROUTINES
	virtual bool    sphere_intersects_poly(CSphere* pSphere, Vector*, CPolygon**, Vector*);            // 0x04
	virtual bool    sphere_intersects_solid(CSphere* pSphere, BOOL);                                    // 0x08
	virtual bool    point_intersects_solid(Vector* pPoint);                                            // 0x0C
	virtual bool    sphere_intersects_solid_poly(CSphere* pSphere, float, BOOL*, CPolygon**, BOOL);    // 0x10
	virtual void    find_walkable(CUIFucker* HolyFucker, CSphere*, CPolygon**, Vector*, Vector*, BOOL*);    // 0x14
	virtual bool    hits_walkable(CUIFucker* HolyFucker, CSphere*, Vector*);                                // 0x18
#endif

	DWORD m_dwLeaf38; //0x38
	DWORD m_dwLeaf3C; //0x3C
};

class BSPPORTAL : public BSPNODE
{
public:
	BSPPORTAL();
	virtual ~BSPPORTAL();

	void    Destroy();
	BOOL    UnPackPortal(BYTE** ppData, ULONG iSize);

	DWORD            m_dwPolyCount;    //0x38
	CPortalPoly**    m_pPortalPoly;    //0x3C
};

class BSPTREE // size: 0x04
{
public:
	BSPTREE();
	virtual ~BSPTREE();

	void Destroy();

	BOOL UnPack(BYTE** ppData, ULONG iSize);
	CSphere *GetSphere();

	// Draw routines
	void draw_no_check();

	// Cell routines
	BOOL point_inside_cell_bsp(const Vector& point);

private:
	BSPNODE* m_pHead;
};


