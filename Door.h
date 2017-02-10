
#pragma once

class CBaseDoor : public CPhysicsObj
{
public:
	CBaseDoor();
	~CBaseDoor();

	virtual void Precache();
	virtual void Use(CPhysicsObj *);

	BOOL CloseThink();
	void CloseDoor();
	void OpenDoor();

protected:
	BOOL m_bOpen;

	WORD m_wOpenAnim;
	WORD m_wCloseAnim;
};