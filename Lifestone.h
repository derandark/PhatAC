#pragma once

#define LIFESTONE_MAX_BIND_DISTANCE 4.0f

class CBaseLifestone : public CPhysicsObj
{
public:
	CBaseLifestone();
	~CBaseLifestone();

	virtual void Precache();
	virtual void Use(CPhysicsObj *);

protected:
};