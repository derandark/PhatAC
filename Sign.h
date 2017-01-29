
#pragma once

class CBaseSign : public CPhysicsObj
{
public:
	CBaseSign();

	virtual void Precache();
	virtual void Use(CPhysicsObj *);
};