

#include "StdAfx.h"
#include "TurbineDungeon.h"

TurbineDungeon::TurbineDungeon(DWORD dwID) : TurbineObject(dwID)
{
	m_dwNumPortals = 0;
	m_pPortals = NULL;
}

TurbineDungeon::~TurbineDungeon()
{
	SafeDeleteArray(m_pPortals);
}

void TurbineDungeon::Initialize(BYTE *pbData, DWORD dwLength)
{
	return; // Fix the BSP data

	BYTE *_data = pbData;
	BYTE* _dataend = pbData + dwLength;

	//fileID
	DWORD dwFileID = *((DWORD *)pbData);
	pbData += sizeof(DWORD);

	//numParts
	m_dwNumPortals = *((DWORD *)pbData);
	pbData += sizeof(DWORD);

	m_pPortals = new DUNGEONPORTAL[m_dwNumPortals];

	//if ( m_dwNumPortals == 1 ) __asm int 3;
	for (unsigned int i = 0; i < m_dwNumPortals; i++)
	{
		DWORD dwIndex = *((DWORD *)pbData);
		pbData += sizeof(DWORD);
		DUNGEONPORTAL* pPortal = &m_pPortals[dwIndex];

		if (dwIndex != i)
			__asm int 3;

		DWORD dwNumTriFans = *((DWORD *)pbData);
		if (dwNumTriFans > 1000)
			__asm int 3;
		pbData += sizeof(DWORD);

		//if (pPortal->m_dwNumTriFans > 100000) .. owwww...
		DWORD dwNumPads = *((DWORD *)pbData);
		pbData += sizeof(DWORD);
		DWORD dwUnk1 = *((DWORD *)pbData);
		pbData += sizeof(DWORD);
		DWORD dwUnk2 = *((DWORD *)pbData);
		pbData += sizeof(DWORD);

		DWORD dwNumVertices = *((DWORD *)pbData);
		if (dwNumVertices > 1000)
			__asm int 3;

		pbData += sizeof(DWORD);

		pPortal->m_dwNumVertices = dwNumVertices;
		pPortal->m_pVertices = new DUNGEONVERTEX[dwNumVertices];
		for (unsigned int j = 0; j < dwNumVertices; j++)
		{
			DUNGEONVERTEX* pVertex = &pPortal->m_pVertices[*((WORD *)pbData)];
			pbData += sizeof(WORD);

			WORD wNumSTs = *((WORD *)pbData);
			pbData += sizeof(WORD);

			pVertex->x = *((float *)pbData);
			pbData += sizeof(float);
			pVertex->y = *((float *)pbData);
			pbData += sizeof(float);
			pVertex->z = *((float *)pbData);
			pbData += sizeof(float);
			pVertex->nx = *((float *)pbData);
			pbData += sizeof(float);
			pVertex->ny = *((float *)pbData);
			pbData += sizeof(float);
			pVertex->nz = *((float *)pbData);
			pbData += sizeof(float);

			for (unsigned int k = 0; k < wNumSTs; k++) {
				//S
				pbData += sizeof(float);
				//T
				pbData += sizeof(float);
			}
		}

		//if ( dwFileID == 218103889 && i == 0 )
		//	__asm int 3;

		pPortal->m_pTriFans = new DUNGEONTRIFAN[dwNumTriFans];
		pPortal->m_dwNumTriFans = dwNumTriFans;
		for (unsigned int j = 0; j < dwNumTriFans; j++)
		{
			//polyNum
			WORD polyNum = *((WORD *)pbData);
			if (polyNum != j)
			{
				// DebugMe();
			}

			DUNGEONTRIFAN*	pTriFan = &pPortal->m_pTriFans[polyNum];
			pbData += sizeof(WORD);

			unsigned char cNumVerts = *((unsigned char *)pbData);
			pbData += sizeof(unsigned char);

			unsigned char cType = *((unsigned char *)pbData);
			pbData += sizeof(unsigned char);

			//??
			pbData += sizeof(DWORD);

			WORD numTex = *((WORD *)pbData);
			pbData += sizeof(WORD);

			//??
			pbData += sizeof(WORD);

			pTriFan->m_pVertexIndices = new WORD[cNumVerts];
			pTriFan->m_cNumVertices = cNumVerts;
			for (unsigned int k = 0; k < cNumVerts; k++) {
				// vertNum
				pTriFan->m_pVertexIndices[k] = *((WORD *)pbData);
				pbData += sizeof(WORD);
			}

			if (cType == 4) {
				if (cNumVerts & 1)
					pbData += sizeof(WORD);

				continue;
			}

			for (unsigned int k = 0; k < cNumVerts; k++)
			{
				//stNum index for each vertex..
				pbData += sizeof(unsigned char);
			}

			//if ( (cNumVerts * 4 + cNumVerts * 2) % 8 != 0 )
			//{
			//	pbData += sizeof(unsigned char) * ((8 - ((cNumVerts * 4 + cNumVerts * 2) % 8)) / 2);
			//}
			if ((pbData - _data) & 3) //DWORD-align
				pbData += (sizeof(DWORD) - ((pbData - _data) & 3));
		}

		if ((i + 1) == m_dwNumPortals)
			break;

		// 0x0006 0x0008 0x0007 0x0000

		pbData += dwUnk1 * sizeof(WORD);

		if ((pbData - _data) & 3) //DWORD-align
			pbData += (4 - ((pbData - _data) & 3));

		DWORD* obj = (DWORD *)pbData;
		unsigned int k = 0;
		while (k < 1/*dwNumPads*/) {
			// BPnn = 0x42506E6E = 4 floats?
			// BPnN = 0x42506E4E = 4 floats?
			// BPLF = 0x4250.... = 4 floats?
			// BPIn = 0x4250496E = 4 floats?
			// 

			/*

			LEAF (0x4C454146):
				DWORD- Index
				DWORD- Zero
				DWORDx4 - CDCDCDCDh
				DWORD- NodeCount?
				WORD[NodeCount] Indices?
				--dword align--
				floatX4[NodeCount] who knows
			*/
			DWORD part = obj[0]; obj++;

			if (part == 0)
				__asm int 3;
			else if (part == 0x42506E6E) //BPnn
				obj += 4;
			else if (part == 0x42506E4E) //BPnN
				obj += 4;
			else if (part == 0x4250464C) //BPFL ... not BPLF?
			{
				obj += 4;
				break;
			}
			else
			{
				_DebugMe();
			}

			//k++;
		}
		pbData = (BYTE *)obj;

		for (unsigned int j = 0; j < dwNumPads; j++)
		{
			//polyNum
			pbData += sizeof(WORD);

			unsigned char cNumVerts = *((unsigned char *)pbData);
			pbData += sizeof(unsigned char);

			unsigned char cType = *((unsigned char *)pbData);
			pbData += sizeof(unsigned char);

			//??
			pbData += sizeof(DWORD);

			WORD numTex = *((WORD *)pbData);
			pbData += sizeof(WORD);

			//??
			pbData += sizeof(WORD);

			for (unsigned int k = 0; k < cNumVerts; k++) {
				// vertNum
				pbData += sizeof(WORD);
			}

			if (cType == 4) {
				if (cNumVerts & 1)
					pbData += sizeof(WORD);

				continue;
			}

			for (unsigned int k = 0; k < cNumVerts; k++)
			{
				//stNum index for each vertex..
				pbData += sizeof(unsigned char);
			}

			//if ( (cNumVerts * 4 + cNumVerts * 2) % 8 != 0 )
			//{
			//	pbData += sizeof(unsigned char) * ((8 - ((cNumVerts * 4 + cNumVerts * 2) % 8)) / 2);
			//}
			if ((pbData - _data) & 3) //DWORD-align
				pbData += (sizeof(DWORD) - ((pbData - _data) & 3));
		}

		obj = (DWORD *)pbData;
		k = 0;
		bool hasdata = false;
		while (k < dwNumPads || k < dwNumTriFans) {
			// BPnn = 0x42506E6E = 4 floats?
			// BPnN = 0x42506E4E = 4 floats?
			// BPLF = 0x4250.... = 4 floats?
			// BPIn = 0x4250496E = 4 floats?
			// 

			/*

			LEAF (0x4C454146):
				DWORD- Index
				DWORD- Zero
				DWORDx4 - CDCDCDCDh
				DWORD- NodeCount?
				WORD[NodeCount] Indices?
				--dword align--
				floatX4[NodeCount] who knows
			*/
			DWORD part = obj[0]; obj++;

			if (part == 0x42506E6E) //BPnn
				obj += 4;
			else if (part == 0x42506E4E) //BPnN
				obj += 4;
			else if (part == 0x42504C46) //BPLF
				obj += 4;
			else if (part == 0x4250496E) //BPIn
				obj += 4;
			else if (part == 0x4C454146) //LEAF
			{
				obj += 1; //index
				obj += 1; //zero
				obj += 4; //CDCDCDCDh
				DWORD nodecount = *obj;
				obj += 1;
				obj += (nodecount + 1) / 2;
				if (nodecount)
					obj += 4; //buncha floats
			}
			else if (part == 0x4270496E || part == 0x42504F4C) //BpIn? and BPOL?
			{
				obj += 4;

				while ((!obj[0]) || (obj[0] > m_dwNumPortals)) {
					obj += 4;
					k += obj[0];
					obj += 1 + ((obj[0] + 1) / 2);
				}
				hasdata = true;
				break;
			}
			else if (obj[0] < m_dwNumPortals && hasdata)
			{
				//if ( ((BYTE *)(obj + 2))
				break;
			}
		}
		pbData = (BYTE *)obj;
	}
}

//#define LANDING_NORMAL( v ) \
//	( ((v->nx == 0.0f) && (v->ny == 0.0f) && (v->nz == 1.0f)) ? TRUE : FALSE )

BOOL TurbineDungeon::IsLandingZone(DUNGEONVERTEX* pVertex1, DUNGEONVERTEX* pVertex2, DUNGEONVERTEX* pVertex3)
{
	//if ( BLOCK_WORD( m_dwID ) == 0x2FA )
	//	break;

	//3EA = broke (part1)

	/*if ( m_dwID == 0x0D0003EA )
	{
		LOG(Temp, Normal, "--\n%.01f %.01f %.01f, %.01f %.01f %.01f\n",
			pVertex1->x, pVertex1->y, pVertex1->z,
			pVertex1->nx, pVertex1->ny, pVertex1->nz );

		LOG(Temp, Normal, "%.01f %.01f %.01f, %.01f %.01f %.01f\n",
			pVertex2->x, pVertex2->y, pVertex2->z,
			pVertex2->nx, pVertex2->ny, pVertex2->nz );

		LOG(Temp, Normal, "%.01f %.01f %.01f, %.01f %.01f %.01f\n",
			pVertex3->x, pVertex3->y, pVertex3->z,
			pVertex3->nx, pVertex3->ny, pVertex3->nz );
	}*/

	const float width_tolerance = 0.8f;
	const float normal_tolerance = 1.0f;
	const float height_tolerance = 0.5f;

	if (pVertex1->nz < normal_tolerance &&
		pVertex2->nz < normal_tolerance &&
		pVertex3->nz < normal_tolerance)
		return FALSE;

	if ((fabs(pVertex3->x - pVertex1->x) < width_tolerance) &&
		(fabs(pVertex2->x - pVertex1->x) < width_tolerance) &&
		(fabs(pVertex3->x - pVertex2->x) < width_tolerance))
		return FALSE;

	if ((fabs(pVertex3->y - pVertex1->y) < width_tolerance) &&
		(fabs(pVertex2->y - pVertex1->y) < width_tolerance) &&
		(fabs(pVertex3->y - pVertex2->y) < width_tolerance))
		return FALSE;

	if ((fabs(pVertex3->z - pVertex1->z) > height_tolerance) ||
		(fabs(pVertex2->z - pVertex1->z) > height_tolerance) ||
		(fabs(pVertex3->z - pVertex2->z) > height_tolerance))
		return FALSE;

	/*if ( m_dwID == 0x0D00011B )
	{
		LOG(Temp, Normal, "--\n%.01f %.01f %.01f, %.01f %.01f %.01f\n",
			pVertex1->x, pVertex1->y, pVertex1->z,
			pVertex1->nx, pVertex1->ny, pVertex1->nz );

		LOG(Temp, Normal, "%.01f %.01f %.01f, %.01f %.01f %.01f\n",
			pVertex2->x, pVertex2->y, pVertex2->z,
			pVertex2->nx, pVertex2->ny, pVertex2->nz );

		LOG(Temp, Normal, "%.01f %.01f %.01f, %.01f %.01f %.01f\n",
			pVertex3->x, pVertex3->y, pVertex3->z,
			pVertex3->nx, pVertex3->ny, pVertex3->nz );
	}*/

	return TRUE;
}

loc_t TurbineDungeon::FindLandingZone(WORD wPortal)
{
	loc_t LZ;
	LZ.landcell = FALSE;

	if (wPortal > m_dwNumPortals)
	{
		_DebugMe();
		return LZ;
	}

	DUNGEONPORTAL* pPortal = &m_pPortals[wPortal];

	if (!pPortal)
	{
		LOG(Temp, Normal, "Missing portal %u in %08X.\n", wPortal, GetID());
	}
	else
	{
		/*for ( unsigned int j = 0; j < pPortal->m_dwNumVertices; j++ )
		{
			DUNGEONVERTEX* pVertex = &pPortal->m_pVertices[ j ];

			if ( !pVertex )
			{
				LOG(Temp, Normal, "Missing vertex zone!\n");
				continue;
			}

			if ( pVertex->nx == 0.0f && pVertex->ny == 0.0f && pVertex->nz == 1.0f )
			{
				LZ.landcell	= TRUE;
				LZ.x	= pVertex->x;
				LZ.y	= pVertex->y;
				LZ.z	= pVertex->z;
				//LOG(Temp, Normal, "Found landing zone.\n");
				//return LZ;
			}
		}*/

		for (unsigned int i = 0; i < pPortal->m_dwNumTriFans; i++)
		{
			DUNGEONTRIFAN* pTriFan = &pPortal->m_pTriFans[i];
			if (!pTriFan)
			{
				_DebugMe();
				continue;
			}
			if (!pTriFan->m_cNumVertices)
			{
				_DebugMe();
				continue;
			}

			DUNGEONVERTEX* pBaseVertex = &pPortal->m_pVertices[pTriFan->m_pVertexIndices[0]];

			for (unsigned int j = 1; j < (unsigned)(pTriFan->m_cNumVertices - 1); j++)
			{
				DUNGEONVERTEX* pVertex2 = &pPortal->m_pVertices[pTriFan->m_pVertexIndices[j]];
				DUNGEONVERTEX* pVertex3 = &pPortal->m_pVertices[pTriFan->m_pVertexIndices[j + 1]];

				if (!IsLandingZone(pBaseVertex, pVertex2, pVertex3))
					continue;

				//LOG(Temp, Normal, "Found landing zone! %f %f %f %f %f %f\n",
				//	pVertex2->nx, pVertex2->ny, pVertex2->nz,
				//	pVertex3->nx, pVertex3->ny, pVertex3->nz);
				LZ.landcell = TRUE;
				//LZ.x	= (pBaseVertex->x + pVertex2->x) / 2;
				//LZ.y	= (pBaseVertex->y + pVertex2->y) / 2;
				//LZ.z	= (pBaseVertex->z + pVertex2->z) / 2;
				LZ.x = (pBaseVertex->x + pVertex2->x + pVertex3->x) / 3;
				LZ.y = (pBaseVertex->y + pVertex2->y + pVertex3->y) / 3;
				LZ.z = (pBaseVertex->z + pVertex2->z + pVertex3->z) / 3;
				return LZ;
			}
		}
		//LOG(Temp, Normal, "Couldn't find landing zone.\n");
	}

	return LZ;
}






