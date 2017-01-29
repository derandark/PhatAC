
#include "StdAfx.h"
#include "Polygon.h"
#include "Render.h"
#include "BSPData.h"

CPolygon*    BSPNODE::pack_poly;        // CPolygon* g_pBSPTriangles = NULL;
DWORD        BSPNODE::pack_tree_type;    // int g_iBSPMode = 0;

CSphere::CSphere(const Vector& origin, float radius)
{
	m_origin = origin;
	m_radius = radius;
}

CSphere::CSphere()
{
}

ULONG CSphere::pack_size()
{
	return(m_origin.pack_size() + sizeof(float));
}

BOOL CSphere::UnPack(BYTE** ppData, ULONG iSize)
{
	if (iSize < pack_size())
		return FALSE;

	UNPACK_OBJ(m_origin);
	UNPACK(float, m_radius);

	return TRUE;
}

BOOL CSphere::intersects(CSphere* pSphere)
{
	// The offset of the spheres.
	Vector offset = m_origin - pSphere->m_origin;

	// The sum of the radius of both spheres.
	float reach = m_radius + pSphere->m_radius;

	float dist = offset.dot_product(offset) - (reach * reach);

	// Do they intersect?
	if (F_EPSILON > dist)
		return TRUE;

	return FALSE;
}

BOOL CSphere::sphere_intersects_ray(const Ray& ray)
{
	// The offset of the spheres.
	Vector offset = ray.m_origin - m_origin;

	float off_dir_dp = offset.dot_product(ray.m_direction);
	float off_off_dp = offset.dot_product(offset);
	float dir_dir_dp = ray.m_direction.dot_product(ray.m_direction);

	float compare = dir_dir_dp * (off_off_dp - (m_radius * m_radius));

	if (compare < (off_dir_dp * off_dir_dp))
		return TRUE;

	return FALSE;
}

CCylSphere::CCylSphere()
{
	m_origin = Vector(0, 0, 0);
	m_f0C = 0;
	m_f10 = 0;
}

ULONG CCylSphere::pack_size()
{
	return(m_origin.pack_size() + sizeof(float) * 2);
}

BOOL CCylSphere::UnPack(BYTE** ppData, ULONG iSize)
{
	UNPACK_OBJ(m_origin);
	UNPACK(float, m_f10);
	UNPACK(float, m_f0C);

	return TRUE;
}

CSolid::CSolid(const Vector& Vec)
{
	m_Vec = Vec;
}

CSolid::CSolid()
{
	m_Vec = Vector(0, 0, 0);
}

ULONG CSolid::pack_size()
{
	return m_Vec.pack_size();
}

BOOL CSolid::UnPack(BYTE** ppData, ULONG iSize)
{
	return UNPACK_OBJ(m_Vec);
}

CPortalPoly::CPortalPoly()
{
	Destroy();
}

CPortalPoly::~CPortalPoly()
{
	Destroy();
}

void CPortalPoly::Destroy()
{
	m_pPolygon = NULL;
	m_iWhat = -1;
}

BOOL CPortalPoly::UnPack(BYTE** ppData, ULONG iSize)
{
	short Index;
	short What;
	UNPACK(short, Index);
	UNPACK(short, What);

	m_pPolygon = &BSPNODE::pack_poly[Index];
	m_iWhat = What;

	return TRUE;
}

BSPNODE::BSPNODE()
{
	m_NodeType = '####';
	m_iTriangleCount = 0;
	m_pTriangles = NULL;
	m_pChild30 = NULL;
	m_pChild34 = NULL;
}

BSPNODE::~BSPNODE()
{
	Destroy();
}

void BSPNODE::Destroy()
{
	if (m_pChild30)
	{
		delete m_pChild30;
		m_pChild30 = NULL;
	}

	if (m_pTriangles)
	{
		delete[] m_pTriangles;
		m_pTriangles = NULL;
	}

	m_iTriangleCount = 0;

	if (m_pChild34)
	{
		delete m_pChild34;
		m_pChild34 = NULL;
	}

	m_NodeType = '####';
}

BOOL BSPNODE::UnPack(BYTE** ppData, ULONG iSize)
{
	BYTE *pOld = *ppData;

	UNPACK_OBJ(m_plane);

	switch (m_NodeType)
	{
	case 'BPnn':
	case 'BPIn':
		if (!BSPNODE::UnPackChild(&m_pChild30, ppData, iSize))
			return FALSE;

		break;
	case 'BpIN':
	case 'BpnN':
		if (!BSPNODE::UnPackChild(&m_pChild34, ppData, iSize))
			return FALSE;

		break;
	case 'BPIN':
	case 'BPnN':
		if (!BSPNODE::UnPackChild(&m_pChild30, ppData, iSize))
			return FALSE;
		if (!BSPNODE::UnPackChild(&m_pChild34, ppData, iSize))
			return FALSE;

		break;
	}

	if (pack_tree_type) /* 5e4818 in august? */
	{
		if (pack_tree_type == 1)
			UNPACK_OBJ(m_bounds);

		return TRUE;
	}

	UNPACK_OBJ(m_bounds);
	UNPACK(DWORD, m_iTriangleCount);

	if (m_iTriangleCount)
	{
		m_pTriangles = new CPolygon*[m_iTriangleCount];

		for (DWORD i = 0; i < m_iTriangleCount; i++)
		{
			WORD Index;
			UNPACK(WORD, Index);

			m_pTriangles[i] = pack_poly + Index;
		}
	}

#ifdef PRE_TOD
	PACK_ALIGN();
#endif

	return TRUE;
}

BOOL BSPNODE::UnPackChild(BSPNODE** pOut, BYTE** ppData, ULONG iSize)
{
	DWORD NodeType;
	UNPACK(DWORD, NodeType);

	if (NodeType == 'PORT')
	{
		BSPPORTAL* pPortal = new BSPPORTAL();

		*pOut = pPortal;
		pPortal->m_NodeType = 'PORT';

		return pPortal->UnPackPortal(ppData, iSize);
	}
	else if (NodeType == 'LEAF')
	{
		BSPLEAF* pLeaf = new BSPLEAF();

		*pOut = pLeaf;
		pLeaf->m_NodeType = 'LEAF';

		return pLeaf->UnPackLeaf(ppData, iSize);
	}
	else
	{
		BSPNODE* pNode = new BSPNODE();

		*pOut = pNode;
		pNode->m_NodeType = NodeType;

		return pNode->UnPack(ppData, iSize);
	}
}

#if 0
void BSPNODE::draw_no_check()
{
#define BSP_GETFRAMEFUNC Render::GetFrameCurrent

	// Step through the binary tree, beginning with me.
	BSPNODE *pNode = this;

	// Side represents the visual plane this polygon lies on.
	int side;

next_node:
	switch (pNode->m_NodeType)
	{
	case 'BPIn':
		side = pNode->m_plane.which_side(BSP_GETFRAMEFUNC()->m_LocalView.m_origin, F_EPSILON);

		switch (side)
		{
		case 0:
			for (UINT i = 0; i < pNode->m_iTriangleCount; i++)
				Render::polyDraw(pNode->m_pTriangles[i]);
		case 2:
			pNode = pNode->m_pChild30;
			goto next_node;
		case 1:
			pNode->m_pChild30->draw_no_check();

			for (UINT i = 0; i < pNode->m_iTriangleCount; i++)
				Render::polyDraw(pNode->m_pTriangles[i]);
		}
		break;
	case 'BpIN':
		side = pNode->m_plane.which_side(BSP_GETFRAMEFUNC()->m_LocalView.m_origin, F_EPSILON);

		switch (side)
		{
		case 1:
			for (UINT i = 0; i < pNode->m_iTriangleCount; i++)
				Render::polyDraw(pNode->m_pTriangles[i]);
		case 2:
			pNode = pNode->m_pChild34;
			goto next_node;
		case 0:
			pNode->m_pChild34->draw_no_check();

			for (UINT i = 0; i < pNode->m_iTriangleCount; i++)
				Render::polyDraw(pNode->m_pTriangles[i]);
		default:
			// return
			;
		}
		break;
	case 'BPnn':
		pNode = pNode->m_pChild30;
		goto next_node;
	case 'BpnN':
		pNode = pNode->m_pChild34;
		goto next_node;
	case 'BPOL':
		Render::polyDraw(pNode->m_pTriangles[0]);
		break; // return
	case 'BPIN':
	case 'PORT':
		side = pNode->m_plane.which_side(BSP_GETFRAMEFUNC()->m_LocalView.m_origin, F_EPSILON);

		switch (side)
		{
		case 0:
			pNode->m_pChild34->draw_no_check();

			for (UINT i = 0; i < pNode->m_iTriangleCount; i++)
				Render::polyDraw(pNode->m_pTriangles[i]);

			pNode = pNode->m_pChild30;
			goto next_node;
		case 1:
			pNode->m_pChild30->draw_no_check();

			for (UINT i = 0; i < pNode->m_iTriangleCount; i++)
				Render::polyDraw(pNode->m_pTriangles[i]);

			pNode = pNode->m_pChild34;
			goto next_node;
		case 2:
			pNode->m_pChild30->draw_no_check();

			pNode = pNode->m_pChild34;
			goto next_node;
		default:
			// return
			;
		}
		break;
	case 'BPnN':
		side = pNode->m_plane.which_side(BSP_GETFRAMEFUNC()->m_LocalView.m_origin, F_EPSILON);

		switch (side)
		{
		case 0:
			pNode->m_pChild34->draw_no_check();

			pNode = pNode->m_pChild30;
			goto next_node;
		case 1:
			pNode->m_pChild30->draw_no_check();

			pNode = pNode->m_pChild34;
			goto next_node;
		case 2:
			pNode->m_pChild30->draw_no_check();

			pNode = pNode->m_pChild34;
			goto next_node;
		default:
			// return
			;
		}
		break;
	case 'BpIn':
		for (UINT i = 0; i < pNode->m_iTriangleCount; i++)
			Render::polyDraw(pNode->m_pTriangles[i]);
		break;
	default:
		// OutputDebug("BSP ID: %X\n", pNode->m_dwID);
		// __asm int 3;
		break;
	}
}
#endif

void BSPNODE::draw_no_check()
{
	// Side represents the visual plane that this polygon lies on.
	int side;

	switch (m_NodeType)
	{
	case 'BPIn':
		side = m_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON);

		switch (side)
		{
		case 0:
			for (UINT i = 0; i < m_iTriangleCount; i++)
				Render::polyDraw(m_pTriangles[i]);
		case 2:
			m_pChild30->draw_no_check();
			break;
		case 1:
			m_pChild30->draw_no_check();

			for (UINT i = 0; i < m_iTriangleCount; i++)
				Render::polyDraw(m_pTriangles[i]);
		}
		break;
	case 'BpIN':
		side = m_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON);

		switch (side)
		{
		case 1:
			for (UINT i = 0; i < m_iTriangleCount; i++)
				Render::polyDraw(m_pTriangles[i]);
		case 2:
			m_pChild34->draw_no_check();
			break;
		case 0:
			m_pChild34->draw_no_check();

			for (UINT i = 0; i < m_iTriangleCount; i++)
				Render::polyDraw(m_pTriangles[i]);
		}
		break;
	case 'BPnn':
		m_pChild30->draw_no_check();
		break;
	case 'BpnN':
		m_pChild34->draw_no_check();
		break;
	case 'BPOL':
		Render::polyDraw(m_pTriangles[0]);
		break;
	case 'BPIN':
	case 'PORT':
		side = m_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON);

		switch (side)
		{
		case 0:
			m_pChild34->draw_no_check();

			for (UINT i = 0; i < m_iTriangleCount; i++)
				Render::polyDraw(m_pTriangles[i]);

			m_pChild30->draw_no_check();
			break;
		case 1:
			m_pChild30->draw_no_check();

			for (UINT i = 0; i < m_iTriangleCount; i++)
				Render::polyDraw(m_pTriangles[i]);

			m_pChild34->draw_no_check();
			break;
		case 2:
			m_pChild30->draw_no_check();
			m_pChild34->draw_no_check();
			break;
		}
		break;
	case 'BPnN':
		side = m_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON);

		switch (side)
		{
		case 0:
			m_pChild34->draw_no_check();
			m_pChild30->draw_no_check();
			break;
		case 1:
			m_pChild30->draw_no_check();
			m_pChild34->draw_no_check();
			break;
		case 2:
			m_pChild30->draw_no_check();
			m_pChild34->draw_no_check();
			break;
		}
		break;
	case 'BpIn':
		for (UINT i = 0; i < m_iTriangleCount; i++)
			Render::polyDraw(m_pTriangles[i]);

		break;
	default:
		// OutputDebug("BSP ID: %X\n", pNode->m_dwID);
		// __asm int 3;
		break;
	}
}

void BSPNODE::draw_check(DWORD unknown)
{
	switch (m_NodeType)
	{
	case 'BPIn':

		switch (Render::checkplanes(m_bounds, &unknown))
		{
		case 1:

			switch (m_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON))
			{
			case 0:
				for (UINT i = 0; i < m_iTriangleCount; i++)
					Render::polyDrawClip(m_pTriangles[i], unknown);

				m_pChild30->draw_check(unknown);
				break;
			case 1:
				m_pChild30->draw_check(unknown);

				for (UINT i = 0; i < m_iTriangleCount; i++)
					Render::polyDrawClip(m_pTriangles[i], unknown);
				break;
			case 2:
				m_pChild30->draw_check(unknown);
				break;
			}
			break;
		case 2:

			switch (m_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON))
			{
			case 0:
				for (UINT i = 0; i < m_iTriangleCount; i++)
					Render::polyDraw(m_pTriangles[i]);

				m_pChild30->draw_no_check();
				break;
			case 1:
				m_pChild30->draw_no_check();

				for (UINT i = 0; i < m_iTriangleCount; i++)
					Render::polyDraw(m_pTriangles[i]);
				break;
			}
			break;
		}

		break;

	case 'BpIN':

		switch (Render::checkplanes(m_bounds, &unknown))
		{
		case 1:

			switch (m_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON))
			{
			case 0:
				m_pChild34->draw_check(unknown);

				for (UINT i = 0; i < m_iTriangleCount; i++)
					Render::polyDrawClip(m_pTriangles[i], unknown);
				break;
			case 1:
				for (UINT i = 0; i < m_iTriangleCount; i++)
					Render::polyDrawClip(m_pTriangles[i], unknown);

				m_pChild34->draw_check(unknown);
				break;
			case 2:
				m_pChild34->draw_check(unknown);
				break;
			}
			break;
		case 2:

			switch (m_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON))
			{
			case 0:
				m_pChild34->draw_no_check();

				for (UINT i = 0; i < m_iTriangleCount; i++)
					Render::polyDraw(m_pTriangles[i]);
				break;
			case 1:
				for (UINT i = 0; i < m_iTriangleCount; i++)
					Render::polyDraw(m_pTriangles[i]);

				m_pChild34->draw_no_check();
				break;
			case 2:
				m_pChild34->draw_no_check();
				break;
			}
			break;
		}

		break;
	case 'BPnn':
		switch (Render::checkplanes(m_bounds, &unknown))
		{
		case 1:
			m_pChild30->draw_check(unknown);
			break;
		case 2:
			m_pChild30->draw_no_check();
			break;
		}

		break;
	case 'BpnN':
		switch (Render::checkplanes(m_bounds, &unknown))
		{
		case 1:
			m_pChild34->draw_check(unknown);
			break;
		case 2:
			m_pChild34->draw_no_check();
			break;
		}

		break;
	case 'BPOL':
		switch (Render::checkplanes(m_bounds, &unknown))
		{
		case 1:
			Render::polyDrawClip(m_pTriangles[0], unknown);
			break;
		case 2:
			Render::polyDraw(m_pTriangles[0]);
			break;
		}

		break;
	case 'BPIN':
	case 'PORT':

		switch (Render::checkplanes(m_bounds, &unknown))
		{
		case 1:

			switch (m_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON))
			{
			case 0:
				m_pChild34->draw_check(unknown);

				for (UINT i = 0; i < m_iTriangleCount; i++)
					Render::polyDrawClip(m_pTriangles[i], unknown);

				m_pChild30->draw_check(unknown);
				break;
			case 1:
				m_pChild30->draw_check(unknown);

				for (UINT i = 0; i < m_iTriangleCount; i++)
					Render::polyDrawClip(m_pTriangles[i], unknown);

				m_pChild34->draw_check(unknown);
				break;
			case 2:
				m_pChild30->draw_check(unknown);
				m_pChild34->draw_check(unknown);
				break;
			}
			break;
		case 2:

			switch (m_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON))
			{
			case 0:
				m_pChild34->draw_no_check();

				for (UINT i = 0; i < m_iTriangleCount; i++)
					Render::polyDraw(m_pTriangles[i]);

				m_pChild30->draw_no_check();
				break;
			case 1:
				m_pChild30->draw_no_check();

				for (UINT i = 0; i < m_iTriangleCount; i++)
					Render::polyDraw(m_pTriangles[i]);

				m_pChild34->draw_no_check();
				break;
			case 2:
				m_pChild30->draw_no_check();
				m_pChild34->draw_no_check();
				break;
			}
			break;
		}

		break;
	case 'BPnN':

		switch (Render::checkplanes(m_bounds, &unknown))
		{
		case 1:

			switch (m_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON))
			{
			case 0:
				m_pChild34->draw_check(unknown);
				m_pChild30->draw_check(unknown);
				break;
			case 1:
				m_pChild30->draw_check(unknown);
				m_pChild34->draw_check(unknown);
				break;
			case 2:
				m_pChild30->draw_check(unknown);
				m_pChild34->draw_check(unknown);
				break;
			}
			break;
		case 2:

			switch (m_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON))
			{
			case 0:
				m_pChild34->draw_no_check();
				m_pChild30->draw_no_check();
				break;
			case 1:
				m_pChild30->draw_no_check();
				m_pChild34->draw_no_check();
				break;
			case 2:
				m_pChild30->draw_no_check();
				m_pChild34->draw_no_check();
				break;
			}
			break;
		}

		break;
	case 'BpIn':

		switch (Render::checkplanes(m_bounds, &unknown))
		{
		case 1:
			for (UINT i = 0; i < m_iTriangleCount; i++)
				Render::polyDrawClip(m_pTriangles[i], unknown);
			break;
		case 2:
			for (UINT i = 0; i < m_iTriangleCount; i++)
				Render::polyDraw(m_pTriangles[i]);
			break;
		}

		break;
	default:
		// OutputDebug("BSP ID: %X\n", pNode->m_dwID);
		// __asm int 3;
		break;
	}
}

// BSPNODE collision routines
#ifdef COLLISION_ROUTINES

bool BSPNODE::sphere_intersects_poly(CSphere* pSphere, Vector* pVector, CPolygon** pHit, Vector* pOut)
{
	if (!m_bounds.intersects(pSphere))
		return false;

	float fDist = m_plane.dot_product(pSphere->m_origin);
	float fReach = pSphere->m_radius - F_EPSILON;

	if (fDist >= fReach)
		return m_pChild30->sphere_intersects_poly(pSphere, pVector, pHit, pOut);
	if (fDist <= -fReach)
		return m_pChild34->sphere_intersects_poly(pSphere, pVector, pHit, pOut);

	if (m_pChild30->sphere_intersects_poly(pSphere, pVector, pHit, pOut))
		return true;
	if (m_pChild34->sphere_intersects_poly(pSphere, pVector, pHit, pOut))
		return true;

	return false;
}

bool BSPNODE::point_intersects_solid(Vector* pPoint)
{
	if (m_plane.dot_product(*pPoint) >= 0)
		return m_pChild30->point_intersects_solid(pPoint);
	else
		return m_pChild34->point_intersects_solid(pPoint);
}

bool BSPNODE::sphere_intersects_solid(CSphere* pSphere, BOOL bArg4)
{
	// Is this sphere even within our bounds?
	if (!m_bounds.intersects(pSphere))
		return false;

	float fDist = m_plane.dot_product(pSphere->m_origin);
	float fReach = pSphere->m_radius - F_EPSILON;

	// Is this sphere inside our plane?
	if ((fDist) >= (fReach))
		return m_pChild30->sphere_intersects_solid(pSphere, bArg4);

	if ((fDist) <= (-fReach))
		return m_pChild34->sphere_intersects_solid(pSphere, bArg4);

	if (fDist < 0.0)
	{
		if (m_pChild30->sphere_intersects_solid(pSphere, bArg4))
			return true;

		return m_pChild34->sphere_intersects_solid(pSphere, FALSE);
	}
	else
	{
		if (m_pChild30->sphere_intersects_solid(pSphere, FALSE))
			return true;

		return m_pChild34->sphere_intersects_solid(pSphere, bArg4);
	}
}

bool BSPNODE::sphere_intersects_solid_poly(CSphere* pSphere, float arg4, BOOL* bArg8, CPolygon** pHit, BOOL bArg10)
{
	if (!m_bounds.intersects(pSphere))
		return NULL;

	float fDist = m_plane.dot_product(pSphere->m_origin);
	float fReach = arg4 - F_EPSILON;

	if (fDist >= (fReach))
		return m_pChild30->sphere_intersects_solid_poly(pSphere, arg4, bArg8, pHit, bArg10);
	if (fDist <= -(fReach))
		return m_pChild34->sphere_intersects_solid_poly(pSphere, arg4, bArg8, pHit, bArg10);

	if (fDist < 0.0)
	{
		m_pChild34->sphere_intersects_solid_poly(pSphere, arg4, bArg8, pHit, bArg10);

		if (*pHit)
			return *bArg8;

		return m_pChild30->sphere_intersects_solid_poly(pSphere, arg4, bArg8, pHit, NULL);
	}
	else
	{
		m_pChild30->sphere_intersects_solid_poly(pSphere, arg4, bArg8, pHit, bArg10);

		if (*pHit)
			return *bArg8;

		return m_pChild34->sphere_intersects_solid_poly(pSphere, arg4, bArg8, pHit, NULL);
	}
}

void BSPNODE::find_walkable(void* HolyFucker, CSphere* pSphere, CPolygon** pHit, Vector* pVector1, Vector* pVector2, BOOL* bArg14)
{
	if (!m_bounds.intersects(pSphere))
		return; // would return false

	float fDist = m_plane.dot_product(pSphere->m_origin);
	float fReach = pSphere->m_radius - F_EPSILON;

	if (fDist >= fReach) {
		m_pChild30->find_walkable(HolyFucker, pSphere, pHit, pVector1, pVector2, bArg14);
		return;
	}
	if (fDist <= -fReach) {
		m_pChild34->find_walkable(HolyFucker, pSphere, pHit, pVector1, pVector2, bArg14);
		return;
	}

	m_pChild30->find_walkable(HolyFucker, pSphere, pHit, pVector1, pVector2, bArg14);
	m_pChild34->find_walkable(HolyFucker, pSphere, pHit, pVector1, pVector2, bArg14);

	return;
}

bool BSPNODE::hits_walkable(void* HolyFucker, CSphere* pSphere, Vector* pVector)
{
	if (!m_bounds.intersects(pSphere))
		return false;

	float fDist = m_plane.dot_product(pSphere->m_origin);
	float fReach = pSphere->m_radius - F_EPSILON;

	if (fDist >= fReach)
		return m_pChild30->hits_walkable(HolyFucker, pSphere, pVector);
	if (fDist <= -fReach)
		return m_pChild34->hits_walkable(HolyFucker, pSphere, pVector);

	if (m_pChild30->hits_walkable(HolyFucker, pSphere, pVector))
		return true;
	if (m_pChild34->hits_walkable(HolyFucker, pSphere, pVector))
		return true;

	return false;
}

#endif
// End of BSPNODE collision routines

BSPLEAF::BSPLEAF()
{
	m_dwLeaf38 = (DWORD)-1;
	m_dwLeaf3C = 0;
}

BSPLEAF::~BSPLEAF()
{
}

BOOL BSPLEAF::UnPackLeaf(BYTE** ppData, ULONG iSize)
{
	UNPACK(DWORD, m_dwLeaf38);

	if (pack_tree_type != 1)
		return TRUE;

	UNPACK(DWORD, m_dwLeaf3C);
	UNPACK_OBJ(m_bounds);
	UNPACK(DWORD, m_iTriangleCount);

	if (m_iTriangleCount)
	{
		m_pTriangles = new CPolygon*[m_iTriangleCount];

		for (DWORD i = 0; i < m_iTriangleCount; i++)
		{
			WORD Index;
			UNPACK(WORD, Index);

			m_pTriangles[i] = &pack_poly[Index];
		}
	}

#ifdef PRE_TOD
	PACK_ALIGN();
#endif

	return TRUE;
}

// BSPLEAF collision routines
#ifdef COLLISION_ROUTINES

bool BSPLEAF::sphere_intersects_poly(CSphere* pSphere, Vector* pVector, CPolygon** pHit, Vector* pOut)
{
	if (!m_iTriangleCount)
		return false;
	if (!m_bounds.intersects(pSphere))
		return false;

	for (UINT i = 0; i < m_iTriangleCount; i++)
	{
		if (m_pTriangles[i]->pos_hits_sphere(pSphere, pVector, pOut, pHit))
			return true;
	}

	return false;
}

bool BSPLEAF::sphere_intersects_solid(CSphere* pSphere, BOOL bArg4)
{
	if (!m_iTriangleCount)
		return false;

	if (m_dwLeaf3C && bArg4)
		return true;

	if (!m_bounds.intersects(pSphere))
		return false;

	for (UINT i = 0; i < m_iTriangleCount; i++)
	{
		if (m_pTriangles[i]->hits_sphere(pSphere))
			return true;
	}

	return false;
}

bool BSPLEAF::point_intersects_solid(Vector* pPoon)
{
	if (!m_iTriangleCount)
		return false;

	return true;
}

bool BSPLEAF::sphere_intersects_solid_poly(CSphere* pSphere, float arg4, BOOL* bArg8, CPolygon** pHit, BOOL bArg10)
{
	if (!m_iTriangleCount)
		return NULL;

	if (bArg10 && m_dwLeaf3C)
		*bArg8 = TRUE;

	if (!m_bounds.intersects(pSphere))
		return *bArg8;

	for (UINT i = 0; i < m_iTriangleCount; i++)
	{
		if (m_pTriangles[i]->hits_sphere(pSphere))
		{
			*pHit = m_pTriangles[i];
			return true;
		}
	}

	return *bArg8;
}

void BSPLEAF::find_walkable(CUIFucker* HolyFucker, CSphere* pSphere, CPolygon** pHit, Vector* pVector1, Vector* pVector2, BOOL* bArg14)
{
	if (!m_iTriangleCount)
		return;
	if (!m_bounds.intersects(pSphere))
		return;

	for (UINT i = 0; i < m_iTriangleCount; i++)
	{
		if (m_pTriangles[i]->walkable_hits_sphere(HolyFucker, pSphere, pVector2))
		{
			if (m_pTriangles[i]->adjust_sphere_to_plane(HolyFucker, pSphere, pVector1))
			{
				*bArg14 = TRUE;
				*pHit = m_pTriangles[i];
			}
		}
	}

	return;
}

bool BSPLEAF::hits_walkable(CUIFucker* HolyFucker, CSphere* pSphere, Vector* pVector)
{
	if (!m_iTriangleCount)
		return false;
	if (!m_bounds.intersects(pSphere))
		return false;

	for (UINT i = 0; i < m_iTriangleCount; i++)
	{
		if (m_pTriangles[i]->walkable_hits_sphere(HolyFucker, pSphere, pVector))
		{
			if (m_pTriangles[i]->check_small_walkable(pSphere, pVector))
				return true;
		}
	}

	return false;
}

#endif
// End of BSPLEAF collision routines

BSPPORTAL::BSPPORTAL()
{
	m_dwPolyCount = 0;
	m_pPortalPoly = NULL;
}

BSPPORTAL::~BSPPORTAL()
{
	Destroy();
}

void BSPPORTAL::Destroy()
{
	if (m_pPortalPoly)
	{
		for (DWORD i = 0; i < m_dwPolyCount; i++)
		{
			if (m_pPortalPoly[i])
				delete m_pPortalPoly[i];
		}

		delete[] m_pPortalPoly;
		m_pPortalPoly = NULL;
	}

	m_dwPolyCount = 0;

	BSPNODE::Destroy();
}

BOOL BSPPORTAL::UnPackPortal(BYTE** ppData, ULONG iSize)
{
	BYTE *pOld = *ppData;

	if (!UNPACK_OBJ(m_plane))
		return FALSE;

	if (!BSPNODE::UnPackChild(&m_pChild30, ppData, iSize) ||
		!BSPNODE::UnPackChild(&m_pChild34, ppData, iSize))
	{
		*ppData = pOld;
		return FALSE;
	}

	if (pack_tree_type)
		return TRUE;

	UNPACK_OBJ(m_bounds);

	UNPACK(DWORD, m_iTriangleCount);
	UNPACK(DWORD, m_dwPolyCount);

	if (m_iTriangleCount)
	{
		m_pTriangles = new CPolygon*[m_iTriangleCount];

		for (DWORD i = 0; i < m_iTriangleCount; i++)
		{
			WORD Index;
			UNPACK(WORD, Index);

			m_pTriangles[i] = &pack_poly[Index];
		}
	}

	if (m_dwPolyCount)
	{
		m_pPortalPoly = new CPortalPoly*[m_dwPolyCount];

		for (DWORD i = 0; i < m_dwPolyCount; i++)
		{
			m_pPortalPoly[i] = new CPortalPoly();
			m_pPortalPoly[i]->UnPack(ppData, iSize);
		}
	}

#ifdef PRE_TOD
	PACK_ALIGN();
#endif

	return TRUE;
}

BSPTREE::BSPTREE()
{
	m_pHead = NULL;
}

BSPTREE::~BSPTREE()
{
	Destroy();
}

void BSPTREE::Destroy()
{
	if (m_pHead)
	{
		delete m_pHead;
		m_pHead = NULL;
	}
}

BOOL BSPTREE::UnPack(BYTE** ppData, ULONG iSize)
{
	Destroy();

	return BSPNODE::UnPackChild(&m_pHead, ppData, iSize);
}

CSphere *BSPTREE::GetSphere()
{
	return &m_pHead->m_bounds;
}

void BSPTREE::draw_no_check()
{
	m_pHead->draw_no_check();

	Render::polyListFinishInternal();
	Render::polyListClear();
}

BOOL BSPTREE::point_inside_cell_bsp(const Vector& point)
{
	return m_pHead->point_inside_cell_bsp(point);
}

BOOL BSPNODE::point_inside_cell_bsp(const Vector& point)
{
	int side = m_plane.which_side(point, F_EPSILON);

	switch (side)
	{
	case 0: // FRONT
	case 2: // CLOSE

		if (!m_pChild30)
			return TRUE;

		return m_pChild30->point_inside_cell_bsp(point);
	case 1: // BEHIND
	default:
		return FALSE;
	}
}











