
#pragma once


class CGameMode
{
public:
	CGameMode();
	virtual ~CGameMode();

	virtual const char *GetName() = 0;
	virtual void Think() = 0;

	virtual void OnRemoveEntity(CPhysicsObj *pEntity) = 0;
	virtual void OnTargetAttacked(CPhysicsObj *pTarget, CPhysicsObj *pSource) = 0;
};

class CGameMode_Tag : public CGameMode
{
public:
	CGameMode_Tag();
	virtual ~CGameMode_Tag() override;

	virtual const char *GetName();
	virtual void Think() override;

	virtual void OnRemoveEntity(CPhysicsObj *pEntity) override;
	virtual void OnTargetAttacked(CPhysicsObj *pTarget, CPhysicsObj *pSource) override;

protected:
	void SelectPlayer(CBasePlayer *pPlayer);
	void UnselectPlayer();

	CBasePlayer *m_pSelectedPlayer;
};