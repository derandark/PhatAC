


#pragma once

#include "TurbineData.h"
#include "TurbineAnimation.h"
#include "TurbineAnimationSet.h"
#include "TurbineDungeon.h"
#include "TurbineXPTable.h"

class TurbinePortal : public TurbineData
{
public:
	TurbinePortal();

	ANIMATION*		GetAnimation(DWORD dwAnimID);
	ANIMATIONSET*	GetAnimationSet(DWORD dwAnimSetID);
	DUNGEON*		GetDungeon(DWORD dwBlockID);
	XPTABLE*		GetXPTable(DWORD dwTableID);
};

typedef TurbinePortal TURBINEPORTAL;