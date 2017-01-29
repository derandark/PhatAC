
#pragma once

class CBaseDoor : public CPhysicsObj
{
public:
	CBaseDoor();
	~CBaseDoor();

	virtual void Precache();
	virtual void Use(CPhysicsObj *);

	virtual BOOL CanMove() { return FALSE; }
	virtual DWORD GetDescFlags();
	virtual float GetApproachDist() { return m_fApproachDist; }

	BOOL CloseThink();
	void CloseDoor();
	void OpenDoor();

protected:
	float m_fApproachDist;
	BOOL m_bOpen;

	WORD m_wOpenAnim;
	WORD m_wCloseAnim;
};