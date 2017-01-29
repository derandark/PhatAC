
#include "StdAfx.h"
#include "TurbineXPTable.h"

TurbineXPTable::TurbineXPTable(DWORD dwID) : TurbineObject(dwID)
{
	Clear();
};

TurbineXPTable::~TurbineXPTable()
{
	Clear();
}

void TurbineXPTable::Clear(void)
{
	m_mAttributes.clear();
	m_mVitals.clear();
	m_mTrained.clear();
	m_mSpecialized.clear();
	m_mLevels.clear();
}

void TurbineXPTable::Initialize(BYTE *pbData, DWORD dwLength)
{
	// 5 columns: attributes, vitals, trained, specialized, level

	if (!pbData)
		return;

	DWORD *data, *_data;
	_data = data = (DWORD *)pbData;

	if (dwLength < (6 * sizeof(DWORD)))
		return;

	//DWORD fileID = data[0];
	DWORD numAttribs = data[1] + 1;
	DWORD numVitals = data[2] + 1;
	DWORD numTrained = data[3] + 1;
	DWORD numSpec = data[4] + 1;
	DWORD numLevels = data[5] + 1;
	data += 6;

	data = InitializeTable(&m_mAttributes, data, numAttribs);
	data = InitializeTable(&m_mVitals, data, numVitals);
	data = InitializeTable(&m_mTrained, data, numTrained);
	data = InitializeTable(&m_mSpecialized, data, numSpec);
	data = InitializeTable(&m_mLevels, data, numLevels);
}

DWORD* TurbineXPTable::InitializeTable(XPMAP *pMap, DWORD *data, DWORD entries)
{
	for (unsigned int i = 0; i < entries; i++)
	{
		pMap->insert(std::pair< EXPVAL, LEVELVAL >(*data, i));
		data++;
	}
	return data;
}

LEVELVAL TurbineXPTable::GetAttributeLevel(EXPVAL XP)
{
	if (m_mAttributes.empty())
		return 0;

	XPMAP::iterator i = m_mAttributes.lower_bound(XP + 1);

	if (i == m_mAttributes.end())
		return m_mAttributes.rbegin()->second;
	else
		return (i->second - 1);
}

EXPVAL TurbineXPTable::GetAttributeXP(LEVELVAL Level)
{
	if (m_mAttributes.empty())
		return 0;

	XPMAP::iterator i = m_mAttributes.begin();
	XPMAP::iterator end = m_mAttributes.end();

	while (i != end)
	{
		if (i->second == Level)
			return i->first;

		i++;
	}

	return m_mAttributes.rbegin()->first;
}

LEVELVAL TurbineXPTable::GetAttributeMax()
{
	if (m_mAttributes.empty())
		return 0;

	return m_mAttributes.rbegin()->second;
}

EXPVAL TurbineXPTable::GetAttributeMaxXP()
{
	if (m_mAttributes.empty())
		return 0;

	return m_mAttributes.rbegin()->first;
}

LEVELVAL TurbineXPTable::GetVitalLevel(EXPVAL XP)
{
	if (m_mVitals.empty())
		return 0;

	XPMAP::iterator i = m_mVitals.lower_bound(XP + 1);

	if (i == m_mVitals.end())
		return m_mVitals.rbegin()->second;
	else
		return (i->second - 1);
}

EXPVAL TurbineXPTable::GetVitalXP(LEVELVAL Level)
{
	if (m_mVitals.empty())
		return 0;

	XPMAP::iterator i = m_mVitals.begin();
	XPMAP::iterator end = m_mVitals.end();

	while (i != end)
	{
		if (i->second == Level)
			return i->first;

		i++;
	}

	return m_mVitals.rbegin()->first;
}

LEVELVAL TurbineXPTable::GetVitalMax()
{
	if (m_mVitals.empty())
		return 0;

	return m_mVitals.rbegin()->second;
}

EXPVAL TurbineXPTable::GetVitalMaxXP()
{
	if (m_mVitals.empty())
		return 0;

	return m_mVitals.rbegin()->first;
}

LEVELVAL TurbineXPTable::GetTrainedLevel(EXPVAL XP)
{
	if (m_mTrained.empty())
		return 0;

	XPMAP::iterator i = m_mTrained.lower_bound(XP + 1);

	if (i == m_mTrained.end())
		return m_mTrained.rbegin()->second;
	else
		return (i->second - 1);
}

EXPVAL TurbineXPTable::GetTrainedXP(LEVELVAL Level)
{
	if (m_mTrained.empty())
		return 0;

	XPMAP::iterator i = m_mTrained.begin();
	XPMAP::iterator end = m_mTrained.end();

	while (i != end)
	{
		if (i->second == Level)
			return i->first;

		i++;
	}

	return m_mTrained.rbegin()->first;
}

LEVELVAL TurbineXPTable::GetTrainedMax()
{
	if (m_mTrained.empty())
		return 0;

	return m_mTrained.rbegin()->second;
}

EXPVAL TurbineXPTable::GetTrainedMaxXP()
{
	if (m_mTrained.empty())
		return 0;

	return m_mTrained.rbegin()->first;
}

LEVELVAL TurbineXPTable::GetSpecializedLevel(EXPVAL XP)
{
	if (m_mSpecialized.empty())
		return 0;

	XPMAP::iterator i = m_mSpecialized.lower_bound(XP + 1);

	if (i == m_mSpecialized.end())
		return m_mSpecialized.rbegin()->second;
	else
		return (i->second - 1);
}

EXPVAL TurbineXPTable::GetSpecializedXP(LEVELVAL Level)
{
	if (m_mSpecialized.empty())
		return 0;

	XPMAP::iterator i = m_mSpecialized.begin();
	XPMAP::iterator end = m_mSpecialized.end();

	while (i != end)
	{
		if (i->second == Level)
			return i->first;

		i++;
	}

	return m_mSpecialized.rbegin()->first;
}

LEVELVAL TurbineXPTable::GetSpecializedMax()
{
	if (m_mSpecialized.empty())
		return 0;

	return m_mSpecialized.rbegin()->second;
}

EXPVAL TurbineXPTable::GetSpecializedMaxXP()
{
	if (m_mSpecialized.empty())
		return 0;

	return m_mSpecialized.rbegin()->first;
}

LEVELVAL TurbineXPTable::GetLevel(EXPVAL XP)
{
	if (m_mLevels.empty())
		return 0;

	XPMAP::iterator i = m_mLevels.lower_bound(XP + 1);

	if (i == m_mLevels.end())
		return m_mLevels.rbegin()->second;
	else
		return (i->second - 1);
}

EXPVAL TurbineXPTable::GetLevelXP(LEVELVAL Level)
{
	if (m_mLevels.empty())
		return 0;

	XPMAP::iterator i = m_mLevels.begin();
	XPMAP::iterator end = m_mLevels.end();

	while (i != end)
	{
		if (i->second == Level)
			return i->first;

		i++;
	}

	return m_mLevels.rbegin()->first;
}


