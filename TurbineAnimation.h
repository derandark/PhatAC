

#pragma once

#include "TurbineObject.h"
#include "AnimationPackage.h"

// The guts of the frame data are lists:
struct PartOrientation
{
	inline BYTE* ReadData(BYTE* pbData) {
		return (BYTE *)ReadData((float *)pbData);
	}
	inline float* ReadData(float* pData) {
		tx = pData[0];
		ty = pData[1];
		tz = pData[2];
		rw = pData[3];
		rx = pData[4];
		ry = pData[5];
		rz = pData[6];

		return (pData + 7);
	}
	float tx, ty, tz;		// X Y Z translation
	float rw, rx, ry, rz;	// W X Y Z rotation
};

// The guts of the frame data are lists:
struct PartEvent
{
	inline BYTE* ReadData(BYTE *pbData) {

		EventID = ((DWORD *)pbData)[0];
		pbData += 4;

		switch (EventID)
		{
		case 0x01: memcpy(EventData, pbData, 8);	pbData += 8;	break; // 0x00, soundref
		case 0x02: memcpy(EventData, pbData, 8);	pbData += 8;	break; // 0x00, 0x0C
		case 0x03: memcpy(EventData, pbData, 32);	pbData += 32;	break; // 0x00, 0x14, 4 floats, 0x77D45F0E, 0xDCBAABCD?
		case 0x05: memcpy(EventData, pbData, 8);	pbData += 8;	break; // 0x00, 0xBB401
		case 0x06: memcpy(EventData, pbData, 8);	pbData += 8;	break; // 0x01, 0x01
		case 0x07: memcpy(EventData, pbData, 20);	pbData += 20;	break; // 0x00, 0x0A, 1.0, 1.0, 0x00
		case 0x0D: memcpy(EventData, pbData, 44);	pbData += 44;	break;
		case 0x0F: memcpy(EventData, pbData, 8);	pbData += 8;	break; // 0x00,0x01
		case 0x11: memcpy(EventData, pbData, 4);	pbData += 4;	break; // 0x00
		case 0x13: memcpy(EventData, pbData, 12);	pbData += 12;	break; // 0x00, someREF, 0x00
		case 0x14: memcpy(EventData, pbData, 16);	pbData += 16;	break; // 0x00, 0x00, 0x00, 0x00
		case 0x15: memcpy(EventData, pbData, 20);	pbData += 20;	break; // 0x00, soundref, 3 floats
		case 0x16: memcpy(EventData, pbData, 16);	pbData += 16;	break; // 0x00, 0x00, 2floats
		default:
			LOG(Animation, Warning, "Unknown animation event %08X!\n", EventID);
		}

		return pbData;
	}
	DWORD EventID;
	DWORD EventData[15]; // To avoid using more dynamic memory, static max size of 15.
};


class TurbineAnimationFrame;
class TurbineAnimation;

class TurbineAnimationFrame
{
public:
	TurbineAnimationFrame();
	~TurbineAnimationFrame();

	BYTE*	InitializeTranslations(BYTE *pbFrame, int iPartCount);
	BYTE*	InitializeEvents(BYTE *pbFrame);

	void ExecuteFrame(CPhysicsObj* pWeenie, AnimationPackage* pAnimation);

private:
	PartOrientation*	m_pTranslations;
	PartEvent*			m_pEvents;
};
typedef TurbineAnimationFrame	ANIMATIONFRAME;

class TurbineAnimation : public TurbineObject
{
public:
	TurbineAnimation(DWORD dwID);
	~TurbineAnimation();

	void	Initialize(BYTE *pbData, DWORD dwLength);
	BYTE*	InitializeFrames(BYTE *pbFrames, unsigned int iType, unsigned int iPartCount, unsigned int iFrameCount);

	inline unsigned int GetFrameCount();
	inline ANIMATIONFRAME* GetFrame(int iFrame);

	long GetFrameByTime(float fTime, float fSpeed);

	bool Execute(CPhysicsObj* pWeenie, AnimationPackage* pAnimation);

private:
	ANIMATIONFRAME* m_pFrames;

	unsigned int m_iType;
	unsigned int m_iPartCount;
	unsigned int m_iFrameCount;

};

typedef TurbineAnimation	ANIMATION;