
#pragma once

#include "TurbineObject.h"

struct CombatStance
{
	CombatStance()
	{
	}

	CombatStance(WORD wIndex, WORD wSeq, DWORD dwType = 0x41000003)
	{
		m_wIndex = wIndex;
		m_wSeq = wSeq;
		m_dwType = dwType;
	}

	WORD m_wIndex;
	WORD m_wSeq;
	DWORD m_dwType;
};

struct AnimationEntry
{
	AnimationEntry()
	{
	}

	AnimationEntry(DWORD dwID, DWORD dwSF, DWORD dwEF, float fSpeed, Vector vTranslation = Vector(0, 0, 0), Vector vRotation = Vector(0, 0, 0))
	{
		m_dwAnimID = dwID;
		m_dwStartFrame = dwSF;
		m_dwEndFrame = dwEF;
		m_fSpeed = fSpeed;
		m_vTranslation = vTranslation;
		m_vRotation = vRotation;
	};

	DWORD m_dwAnimID;
	DWORD m_dwStartFrame;
	DWORD m_dwEndFrame;
	float m_fSpeed;

	Vector m_vTranslation;
	Vector m_vRotation;
};

typedef std::list<AnimationEntry> AnimationSequence;

struct PivotMotionEntry
{
	PivotMotionEntry(Vector vTranslate = Vector(0, 0, 0), Vector vRotate = Vector(0, 0, 0))
	{
		m_vTranslation = vTranslate;
		m_vRotation = vRotate;
	};

	Vector m_vTranslation;
	Vector m_vRotation;
};

typedef std::map<WORD, CombatStance> StanceMap;
typedef std::map<WORD, std::map<WORD, AnimationEntry>>AnimationMap;
typedef std::map<WORD, std::map<WORD, PivotMotionEntry>>PivotMotionMap;
typedef std::map<WORD, std::map<WORD, std::map<DWORD, AnimationSequence>>> SequenceMap;

class TurbineAnimationSet : public TurbineObject
{
public:
	TurbineAnimationSet(DWORD dwID);
	~TurbineAnimationSet();

	void Initialize(BYTE *pbData, DWORD dwLength);
	BYTE *InitializeCombatStances(BYTE *pbData);
	BYTE *InitializeAnimations(BYTE *pbData);
	BYTE *InitializeSequences(BYTE *pbData);

	AnimationEntry *LookupAnimation(WORD wStance, WORD wIndex);

private:
	CombatStance m_csDefault;
	StanceMap m_mCombatStances;

	AnimationMap m_mAnimations;
	PivotMotionMap m_mPivotMotions;
	SequenceMap	m_mSequences;
};

typedef TurbineAnimationSet	ANIMATIONSET;

