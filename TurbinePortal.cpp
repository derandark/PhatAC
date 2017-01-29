

#include "StdAfx.h"
#include "TurbinePortal.h"

TurbinePortal::TurbinePortal()
{
	LoadFile("portal.dat");
}

ANIMATION* TurbinePortal::GetAnimation(DWORD dwAnimID)
{
	ANIMATION* pAnimation = (ANIMATION *)FindObject(dwAnimID);

	if (!pAnimation)
	{
		TURBINEFILE* pFile = GetFile(dwAnimID);

		if (pFile)
		{
			pAnimation = new ANIMATION(dwAnimID);
			pAnimation->Initialize(pFile->GetData(), pFile->GetLength());

			InsertObject(dwAnimID, pAnimation);
			delete pFile;
		}
	}

	return pAnimation;
}

ANIMATIONSET* TurbinePortal::GetAnimationSet(DWORD dwAnimSetID)
{
	ANIMATIONSET* pAnimationSet = (ANIMATIONSET *)FindObject(dwAnimSetID);

	if (!pAnimationSet)
	{
		TURBINEFILE* pFile = GetFile(dwAnimSetID);

		if (pFile)
		{
			pAnimationSet = new ANIMATIONSET(dwAnimSetID);
			pAnimationSet->Initialize(pFile->GetData(), pFile->GetLength());

			InsertObject(dwAnimSetID, pAnimationSet);
			delete pFile;
		}
	}

	return pAnimationSet;
}

DUNGEON* TurbinePortal::GetDungeon(DWORD dwBlockID)
{
	DUNGEON* pDungeon = (DUNGEON *)FindObject(dwBlockID);

	if (!pDungeon)
	{
		TURBINEFILE* pFile = GetFile(dwBlockID);

		if (pFile)
		{
			pDungeon = new DUNGEON(dwBlockID);
			pDungeon->Initialize(pFile->GetData(), pFile->GetLength());

			InsertObject(dwBlockID, pDungeon);
			delete pFile;
		}
	}

	return pDungeon;
}

XPTABLE* TurbinePortal::GetXPTable(DWORD dwTableID)
{
	XPTABLE* pXPTable = (XPTABLE *)FindObject(dwTableID);

	if (!pXPTable)
	{
		TURBINEFILE* pFile = GetFile(dwTableID);

		if (pFile)
		{
			pXPTable = new XPTABLE(dwTableID);
			pXPTable->Initialize(pFile->GetData(), pFile->GetLength());

			InsertObject(dwTableID, pXPTable);
			delete pFile;
		}
	}

	return pXPTable;
}