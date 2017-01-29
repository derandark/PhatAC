
#include "StdAfx.h"
#include "Vertex.h"
#include "Polygon.h"

CVertexArray *CPolygon::pack_verts;

CPolygon::CPolygon()
{
	m_pVertexObjects = NULL;
	m_pVertexIndices = NULL;

	m_iPolyIndex = -1;

	m_iVertexCount = 0;
	m_iPolyType = 0;

	m_iCullType = 0;
	m_Face1_UVIndices = NULL;
	m_Face2_UVIndices = NULL;
	m_Face1_TexIndex = -1;
	m_Face2_TexIndex = -1;

	m_pVertexUnks = NULL;
}

CPolygon::~CPolygon()
{
	Destroy();
}

void CPolygon::Destroy()
{
	if (m_Face1_UVIndices)
	{
		delete[] m_Face1_UVIndices;
		m_Face1_UVIndices = NULL;
	}

	if (m_iCullType == 2)
	{
		if (m_Face2_UVIndices)
		{
			delete[] m_Face2_UVIndices;
			m_Face2_UVIndices = NULL;
		}
	}

	if (m_pVertexObjects)
	{
		delete[] m_pVertexObjects;
		m_pVertexObjects = NULL;
	}

	if (m_pVertexIndices)
	{
		delete[] m_pVertexIndices;
		m_pVertexIndices = NULL;
	}

	if (m_pVertexUnks)
	{
		delete[] m_pVertexUnks;
		m_pVertexUnks = NULL;
	}

	m_iVertexCount = 0;
	m_iPolyType = 0;
	m_Face1_TexIndex = -1;
	m_Face2_TexIndex = -1;
	m_iPolyIndex = -1;

}

void CPolygon::SetPackVerts(CVertexArray *Verts)
{
	pack_verts = Verts;
}

BOOL CPolygon::UnPack(BYTE** ppData, ULONG iSize)
{
	UNPACK(short, m_iPolyIndex);

	UNPACK(BYTE, m_iVertexCount);
	UNPACK(BYTE, m_iPolyType);

	UNPACK(DWORD, m_iCullType);
	UNPACK(short, m_Face1_TexIndex);
	UNPACK(short, m_Face2_TexIndex);

	m_pVertexObjects = new CSWVertex*[m_iVertexCount];
	m_pVertexIndices = new WORD[m_iVertexCount];
	m_pVertexUnks = new CSWVertex*[m_iVertexCount];

	for (DWORD i = 0; i < m_iVertexCount; i++)
	{
		WORD Index;
		UNPACK(WORD, Index);

		m_pVertexIndices[i] = Index;
		m_pVertexObjects[i] = (CSWVertex *)((BYTE *)pack_verts->m_pVertexBuffer + Index * CVertexArray::vertex_size);
	}

	if (!(m_iPolyType & 4))
	{
		m_Face1_UVIndices = new BYTE[m_iVertexCount];

		for (DWORD i = 0; i < m_iVertexCount; i++)
			UNPACK(BYTE, m_Face1_UVIndices[i]);
	}

	if ((m_iCullType == 2) && !(m_iPolyType & 8))
	{
		m_Face2_UVIndices = new BYTE[m_iVertexCount];

		for (DWORD i = 0; i < m_iVertexCount; i++)
			UNPACK(BYTE, m_Face2_UVIndices[i]);
	}

	if (m_iCullType == CullNone)
	{
		m_Face2_TexIndex = m_Face1_TexIndex;
		m_Face2_UVIndices = m_Face1_UVIndices;
	}

#ifdef PRE_TOD
	PACK_ALIGN();
#endif

	make_plane();

	return TRUE;
}

void CPolygon::make_plane()
{
	// Not meant for human eyes.
	int i;
	CSWVertex *pPin;
	CSWVertex **ppSpread;

	Vector Norm(0, 0, 0);

	for (i = m_iVertexCount - 2, pPin = m_pVertexObjects[0], ppSpread = &m_pVertexObjects[1]; i > 0; i--, ppSpread++)
	{
		Vector V1 = ppSpread[0]->origin - pPin->origin;
		Vector V2 = ppSpread[1]->origin - pPin->origin;

		Norm = Norm + cross_product(V1, V2);
	}

	Norm.normalize();

	float distsum = 0;

	for (i = m_iVertexCount, ppSpread = m_pVertexObjects; i > 0; i--, ppSpread++)
		distsum += Norm.dot_product(ppSpread[0]->origin);

	m_plane.m_dist = -(distsum / m_iVertexCount);
	m_plane.m_normal = Norm;
}

BOOL CPolygon::polygon_hits_ray(const Ray& ray, float *time)
{
	// return peafixed_polygon_hits_ray(ray, time);

	if (!m_iCullType && (m_plane.m_normal.dot_product(ray.m_direction) > 0))
		return FALSE;

	if (!m_plane.compute_time_of_intersection(ray, time))
		return FALSE;

	return point_in_polygon(ray.m_origin + (ray.m_direction * (*time)));
}

BOOL CPolygon::peafixed_polygon_hits_ray(const Ray& ray, float *depth)
{
	if (!m_iCullType && (m_plane.m_normal.dot_product(ray.m_direction) > 0))
		return FALSE;

	float u, v;

	for (int i = 1; i < m_iVertexCount - 1;)
	{
		CSWVertex* CurrVertex1 = m_pVertexObjects[0];
		CSWVertex* CurrVertex2 = m_pVertexObjects[i];
		CSWVertex* CurrVertex3 = m_pVertexObjects[++i];

		if (D3DXIntersectTri(
			(D3DXVECTOR3 *)&CurrVertex1->origin,
			(D3DXVECTOR3 *)&CurrVertex2->origin,
			(D3DXVECTOR3 *)&CurrVertex3->origin,
			(D3DXVECTOR3 *)&ray.m_origin,
			(D3DXVECTOR3 *)&ray.m_direction,
			&u, &v, depth))
			return TRUE;
	}

	return FALSE;
}

BOOL CPolygon::point_in_polygon(const Vector& point)
{
	CSWVertex* LastVertex = m_pVertexObjects[m_iVertexCount - 1];

	for (int i = 0; i < m_iVertexCount; i++)
	{
		CSWVertex* CurrVertex = m_pVertexObjects[i];

		Vector crossp =
			cross_product(m_plane.m_normal, CurrVertex->origin - LastVertex->origin);

		float dotp = crossp.dot_product(point - LastVertex->origin);

		if (dotp < 0.0)
			return FALSE;

		LastVertex = CurrVertex;
	}

	return TRUE;
}

//
// For ray-tracing vertices -- something not in the client.
//

CSWVertex *PickVertexFromPolygon(CPolygon *pPoly, const Ray& ray)
{
	if (!pPoly->m_iCullType && (pPoly->m_plane.m_normal.dot_product(ray.m_direction) > 0))
		return FALSE;

	float curr_depth = FLT_MAX;

	CSWVertex*
		result = NULL;

	for (int i = 1; i < pPoly->m_iVertexCount - 1;)
	{
		CSWVertex* CurrVertex1 = pPoly->m_pVertexObjects[0];
		CSWVertex* CurrVertex2 = pPoly->m_pVertexObjects[i];
		CSWVertex* CurrVertex3 = pPoly->m_pVertexObjects[++i];

		float u, v, depth;
		if (D3DXIntersectTri(
			(D3DXVECTOR3 *)&CurrVertex1->origin,
			(D3DXVECTOR3 *)&CurrVertex2->origin,
			(D3DXVECTOR3 *)&CurrVertex3->origin,
			(D3DXVECTOR3 *)&ray.m_origin,
			(D3DXVECTOR3 *)&ray.m_direction,
			&u, &v, &depth) && (depth < curr_depth))
		{
			Vector u_scalar = (CurrVertex2->origin - CurrVertex1->origin);
			Vector v_scalar = (CurrVertex3->origin - CurrVertex1->origin);
			Vector face_point = CurrVertex1->origin + (u_scalar * u) + (v_scalar * v);

			float vdist1 = (face_point - CurrVertex1->origin).magnitude();
			float vdist2 = (face_point - CurrVertex2->origin).magnitude();
			float vdist3 = (face_point - CurrVertex3->origin).magnitude();

			if (vdist1 <= vdist2)
				result = ((vdist1 <= vdist3) ? CurrVertex1 : CurrVertex3);
			else
				result = ((vdist2 <= vdist3) ? CurrVertex2 : CurrVertex3);
		}
	}

	return result;
}


