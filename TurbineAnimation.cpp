
#include "StdAfx.h"
#include "TurbineAnimation.h"

TurbineAnimationFrame::TurbineAnimationFrame()
{
	m_pTranslations = NULL;
	m_pEvents = NULL;
}

TurbineAnimationFrame::~TurbineAnimationFrame()
{
	SafeDeleteArray(m_pTranslations);
	SafeDeleteArray(m_pEvents);
}

BYTE* TurbineAnimationFrame::InitializeTranslations(BYTE *pbFrame, int iPartCount)
{
	m_pTranslations = new PartOrientation[iPartCount];
	for (int i = 0; i < iPartCount; i++)
		pbFrame = m_pTranslations[i].ReadData(pbFrame);

	return pbFrame;
}

BYTE* TurbineAnimationFrame::InitializeEvents(BYTE *pbFrame)
{
	int iEventCount = *((DWORD *)pbFrame);
	pbFrame += 4;

	if (iEventCount)
	{
		m_pEvents = new PartEvent[iEventCount];
		for (int i = 0; i < iEventCount; i++)
			pbFrame = m_pEvents[i].ReadData(pbFrame);
	}

	return pbFrame;
}

void TurbineAnimationFrame::ExecuteFrame(CPhysicsObj* pWeenie, AnimationPackage* pAnimation)
{

}

TurbineAnimation::TurbineAnimation(DWORD dwID) : TurbineObject(dwID)
{
	m_iType = 0;
	m_iPartCount = 0;
	m_iFrameCount = 0;
}

TurbineAnimation::~TurbineAnimation()
{
	SafeDeleteArray(m_pFrames);
}

BYTE* TurbineAnimation::InitializeFrames(BYTE *pbFrames, unsigned int iType, unsigned int iPartCount, unsigned int iFrameCount)
{
	m_iType = iType;
	m_iPartCount = iPartCount;
	m_iFrameCount = iFrameCount;

	m_pFrames = new ANIMATIONFRAME[m_iFrameCount];

	if (iType == 1 || iType == 3)
		pbFrames += sizeof(PartOrientation) * iPartCount;

	for (unsigned int i = 0; i < iFrameCount; i++)
	{
		pbFrames = m_pFrames[i].InitializeTranslations(pbFrames, iPartCount);
		pbFrames = m_pFrames[i].InitializeEvents(pbFrames);
	}

	return pbFrames;
}

void TurbineAnimation::Initialize(BYTE *pbData, DWORD dwLength)
{
	if (!pbData)
		return;

	DWORD *header = (DWORD *)pbData;

	// Animation Header (4 dwords, ie: 16 bytes)
	DWORD dwFileID = header[0];
	int iType = header[1];
	int iPartCount = header[2];
	int iFrameCount = header[3];

	BYTE* frames = pbData + 16;

	InitializeFrames(frames, iType, iPartCount, iFrameCount);
}

unsigned int TurbineAnimation::GetFrameCount()
{
	return m_iFrameCount;
}

ANIMATIONFRAME* TurbineAnimation::GetFrame(int iFrame)
{
	// Returns a frame by index from the array.
	return &m_pFrames[iFrame];
}

long TurbineAnimation::GetFrameByTime(float fTime, float fSpeed)
{
	if (!fSpeed)
		fSpeed = 30.0f;

	return (long)(fTime * fSpeed);
}

bool TurbineAnimation::Execute(CPhysicsObj* pWeenie, AnimationPackage* pAnimation)
{
	// The is the workhorse for animations.
	if (!pAnimation)
		return false;

	if (!GetFrameCount())
		return false;

	// Calculate the number of frames intended to be executed.
	DWORD dwFrameCount =
		min(GetFrameCount() - 1, pAnimation->m_dwEndFrame) -
		min(GetFrameCount() - 1, pAnimation->m_dwStartFrame);

	//The offset from the starting frame.
	float fTime = (float)(g_pGlobals->Time() - pAnimation->m_fStartTime);
	long lFrame = GetFrameByTime(fTime, pAnimation->m_fSpeed);

	//The offset extends beyond the desired frames.
	if (unsigned(abs(lFrame)) >= dwFrameCount)
		return false;

	DWORD dwCurrentFrame = pAnimation->GetBaseFrame() + lFrame;

	if (dwCurrentFrame != pAnimation->m_dwCurrentFrame)
	{
		//Time to launch some events!
		if (lFrame > 0) {
			while (1)
			{
				//Execute all frames until we are up to date.
				if (0x80000000 == pAnimation->m_dwCurrentFrame) //Null frame.
				{
					pAnimation->m_dwCurrentFrame = pAnimation->GetBaseFrame();
					GetFrame(pAnimation->m_dwCurrentFrame)->ExecuteFrame(pWeenie, pAnimation);

				}
				else if (dwCurrentFrame > pAnimation->m_dwCurrentFrame)
				{
					GetFrame(++pAnimation->m_dwCurrentFrame)->ExecuteFrame(pWeenie, pAnimation);
				}
				else
					break;
			}
		}
		else if (lFrame < 0)
		{
			while (1)
			{
				if (0x80000000 == pAnimation->m_dwCurrentFrame) //Null frame.
				{
					pAnimation->m_dwCurrentFrame = pAnimation->GetBaseFrame();
					GetFrame(pAnimation->m_dwCurrentFrame)->ExecuteFrame(pWeenie, pAnimation);
				}
				else if (dwCurrentFrame < pAnimation->m_dwCurrentFrame)
				{
					GetFrame(--pAnimation->m_dwCurrentFrame)->ExecuteFrame(pWeenie, pAnimation);
				}
				else
					break;
			}
		}
	}

	return true;
}









