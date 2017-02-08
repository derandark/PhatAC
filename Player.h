
#pragma once

class CClient;
class BinaryWriter;

class CBasePlayer : public CBaseMonster
{
public:
	CBasePlayer(CClient *, DWORD dwGUID);
	~CBasePlayer();

	BOOL PlayerThink();

	//base virtuals
	virtual void Precache();
	virtual void Save();
	virtual void RemoveMe()
	{
		m_pClient = NULL;
		CBaseMonster::RemoveMe();
	}

	//General Player Statistics.
	virtual const char *GetGenderString();
	virtual const char *GetRaceString();
	virtual const char *GetTitleString();
	virtual BinaryWriter *GetModelData();

	virtual DWORD GiveAttributeXP(eAttribute index, DWORD dwXP);
	virtual DWORD GiveVitalXP(eVital index, DWORD dwXP);
	virtual DWORD GiveSkillXP(eSkill index, DWORD dwXP);

	//Movement overrides
	//...

	//Animation overrides
	//...

	//Control events.
	virtual void MakeAware(CPhysicsObj *);
	virtual void ChangeVIS(DWORD);

	void AddSpellByID(DWORD id);

	//Network events.
	virtual void SendNetMessage(void *_data, DWORD _len, WORD _group, BOOL _event = 0) override;
	virtual void SendNetMessage(BinaryWriter *_food, WORD _group, BOOL _event = 0, BOOL del = 1) override;
	virtual void EnterPortal();
	virtual void ExitPortal();
	virtual void LoginCharacter();
	virtual void UpdateEntity(CPhysicsObj *);

	virtual BOOL IsPlayer() { return TRUE; }

	virtual DWORD GetObjectStat(eObjectStat index);
	virtual DWORD SetObjectStat(eObjectStat index, DWORD value);

	//cmoski -- remove last assessed item
	void SetLastAssessed(DWORD guid);
	std::string RemoveLastAssessed();
	//

	void EmitSoundUI(DWORD dwIndex, float fSpeed);

	CClient *GetClient() { return m_pClient; }

	void SpawnCows();
	void SendText(const char* szText, long lColor);

	BOOL IsHuman();

	DWORD m_LastAssessed;

protected:
	CClient *m_pClient;
	BYTE m_bStatSequence;
};



