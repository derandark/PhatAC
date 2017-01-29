

#pragma once

#include "TurbineObject.h"

typedef DWORD EXPVAL, LEVELVAL;

typedef	std::map<EXPVAL, LEVELVAL> XPMAP;

class TurbineXPTable : public TurbineObject
{
public:
	TurbineXPTable(DWORD dwID);
	~TurbineXPTable();

	void Clear();

	void Initialize(BYTE *pbData, DWORD dwLength);
	DWORD *InitializeTable(XPMAP *pMap, DWORD *data, DWORD entries);

	LEVELVAL GetAttributeLevel(EXPVAL XP);
	EXPVAL GetAttributeXP(LEVELVAL Level);
	LEVELVAL GetAttributeMax();
	EXPVAL GetAttributeMaxXP();

	LEVELVAL GetVitalLevel(EXPVAL XP);
	EXPVAL GetVitalXP(LEVELVAL Level);
	LEVELVAL GetVitalMax();
	EXPVAL GetVitalMaxXP();

	LEVELVAL GetTrainedLevel(EXPVAL XP);
	EXPVAL GetTrainedXP(LEVELVAL Level);
	LEVELVAL GetTrainedMax();
	EXPVAL GetTrainedMaxXP();

	LEVELVAL GetSpecializedLevel(EXPVAL XP);
	EXPVAL GetSpecializedXP(LEVELVAL Level);
	LEVELVAL GetSpecializedMax();
	EXPVAL GetSpecializedMaxXP();

	LEVELVAL GetLevel(EXPVAL XP);
	EXPVAL GetLevelXP(LEVELVAL Level);

private:

	XPMAP m_mAttributes;
	XPMAP m_mVitals;
	XPMAP m_mTrained;
	XPMAP m_mSpecialized;
	XPMAP m_mLevels;
};

typedef TurbineXPTable XPTABLE;