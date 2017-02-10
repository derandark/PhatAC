

#include "StdAfx.h"
#include "TurbineAnimationSet.h"

TurbineAnimationSet::TurbineAnimationSet(DWORD dwID) : TurbineObject(dwID)
{
}

TurbineAnimationSet::~TurbineAnimationSet()
{
}

void TurbineAnimationSet::Initialize(BYTE *pbData, DWORD dwLength)
{
	BYTE* pbGoodEnd = pbData + dwLength;

	DWORD dwFileID = *((DWORD *)pbData);
	pbData += sizeof(DWORD);

	pbData = InitializeCombatStances(pbData);
	pbData = InitializeAnimations(pbData);
	pbData = InitializeSequences(pbData);

	OutputDebugString(csprintf("GoodEnd: %08X Result: %08X\r\n", pbGoodEnd, pbData));
}

BYTE* TurbineAnimationSet::InitializeCombatStances(BYTE *pbData)
{
	WORD wDefaultIndex = *((WORD *)pbData); pbData += 2;
	WORD wDefaultSeq = *((WORD *)pbData); pbData += 2;
	m_csDefault = CombatStance(wDefaultIndex, wDefaultSeq);

	DWORD dwStanceCount = *((DWORD *)pbData); pbData += 4;
	for (unsigned int i = 0; i < dwStanceCount; i++)
	{
		WORD wStanceIndex = *((WORD *)pbData);	pbData += 2;
		WORD wStanceSeq = *((WORD *)pbData);	pbData += 2;
		DWORD dwType = *((DWORD *)pbData);	pbData += 4;

		m_mCombatStances[wStanceIndex] = CombatStance(wStanceIndex, wStanceSeq, dwType);
	}
	OutputDebugString(csprintf("EndS: %08X\r\n", pbData));

	return pbData;
}

BYTE* TurbineAnimationSet::InitializeAnimations(BYTE *pbData)
{
	DWORD dwAnimCount = *((DWORD *)pbData); pbData += 4;
	for (unsigned int i = 0; i < dwAnimCount; i++)
	{
		WORD wIndex = *((WORD *)pbData);	pbData += 2;
		WORD wStance = *((WORD *)pbData);	pbData += 2;
		DWORD dwFlags = *((DWORD *)pbData);	pbData += 4;
		DWORD dwAnimID = *((DWORD *)pbData);	pbData += 4;
		DWORD dwStartFrame = *((DWORD *)pbData);	pbData += 4;
		DWORD dwEndFrame = *((DWORD *)pbData);	pbData += 4;
		float fSpeed = *((float *)pbData);	pbData += 4;
		Vector vTranslate(0, 0, 0), vRotate(0, 0, 0);

		if (dwFlags & 0x10000) {
			vTranslate.x = *((float *)pbData);	pbData += 4;
			vTranslate.y = *((float *)pbData);	pbData += 4;
			vTranslate.z = *((float *)pbData);	pbData += 4;
		}
		if (dwFlags & 0x20000) {
			vRotate.x = *((float *)pbData);	pbData += 4;
			vRotate.y = *((float *)pbData);	pbData += 4;
			vRotate.z = *((float *)pbData);	pbData += 4;
		}

		m_mAnimations[wStance][wIndex] = AnimationEntry(dwAnimID, dwStartFrame, dwEndFrame, fSpeed, vTranslate, vRotate);
	}
	OutputDebugString(csprintf("End1: %08X\r\n", pbData));

	//Pivot Rotations? 100% guess.
	DWORD dwPivotCount = *((DWORD *)pbData); pbData += 4;
	for (unsigned int i = 0; i < dwPivotCount; i++)
	{
		WORD wIndex = *((WORD *)pbData);	pbData += 2;
		WORD wStance = *((WORD *)pbData);	pbData += 2;
		DWORD dwFlags = *((DWORD *)pbData);	pbData += 4;
		Vector vTranslate(0, 0, 0), vRotate(0, 0, 0);

		if (dwFlags & 0x10000) {
			vTranslate.x = *((float *)pbData);	pbData += 4;
			vTranslate.y = *((float *)pbData);	pbData += 4;
			vTranslate.z = *((float *)pbData);	pbData += 4;
		}
		if (dwFlags & 0x20000) {
			vRotate.x = *((float *)pbData);	pbData += 4;
			vRotate.y = *((float *)pbData);	pbData += 4;
			vRotate.z = *((float *)pbData);	pbData += 4;
		}

		m_mPivotMotions[wStance][wIndex] = PivotMotionEntry(vTranslate, vRotate);
	}
	OutputDebugString(csprintf("End2: %08X\r\n", pbData));

	return pbData;
}

BYTE* TurbineAnimationSet::InitializeSequences(BYTE *pbData)
{
	DWORD dwSequenceCount = *((DWORD *)pbData); pbData += 4;
	OutputDebugString(csprintf("SequenceCount: %08X\r\n", dwSequenceCount));
	for (unsigned int i = 0; i < dwSequenceCount; i++)
	{
		WORD wIndex = *((WORD *)pbData);	pbData += 2;
		WORD wStance = *((WORD *)pbData);	pbData += 2;
		DWORD dwTypes = *((DWORD *)pbData);	pbData += 4;
		for (unsigned int j = 0; j < dwTypes; j++)
		{
			DWORD dwType = *((DWORD *)pbData);	pbData += 4;
			DWORD dwNumAnims = *((DWORD *)pbData);	pbData += 4;

			AnimationSequence& lSequence = m_mSequences[wStance][wIndex][dwType];
			lSequence.resize(dwNumAnims);

			for (unsigned int k = 0; k < dwNumAnims; k++)
			{
				DWORD dwAnimID = *((DWORD *)pbData);	pbData += 4;
				DWORD dwStartFrame = *((DWORD *)pbData);	pbData += 4;
				DWORD dwEndFrame = *((DWORD *)pbData);	pbData += 4;
				float fSpeed = *((float *)pbData);	pbData += 4;

				lSequence.push_back(AnimationEntry(dwAnimID, dwStartFrame, dwEndFrame, fSpeed));
			}
		}
	}
	OutputDebugString(csprintf("End3: %08X\r\n", pbData));

	return pbData;
}

AnimationEntry* TurbineAnimationSet::LookupAnimation(WORD wStance, WORD wIndex)
{
	LOG(Temp, Normal, "LookupAnimation %04X %04X", wStance, wIndex);
	AnimationMap::iterator i = m_mAnimations.find(wStance);

	if (i == m_mAnimations.end())
		return NULL;

	std::map< WORD, AnimationEntry >::iterator j = i->second.find(wIndex);

	if (j == i->second.end())
		return NULL;

	return &j->second;
}

