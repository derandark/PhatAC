
#pragma once

enum eVital
{
	eHealth	= 0x01,
	eStamina = 0x03,
	eMana = 0x05,
};

#pragma pack(push, 1)
struct _VITALDATA
{
	DWORD raises;
	DWORD unk;
	DWORD exp;
	DWORD current;
};
#pragma pack(pop)

struct VITAL;

extern DWORD GetVitalMaxXP();
extern const char* GetVitalName( eVital index );

extern DWORD GetVitalXP( DWORD Level );
extern DWORD GetVitalLevel( DWORD XP );

extern float GetVitalPercent( VITAL *pVital, float fVitae );
extern DWORD GetVitalCurrent( VITAL *pVital, float fVitae );
extern DWORD GetVitalMin( VITAL *pVital, float fVitae );
extern DWORD GetVitalMax( VITAL *pVital, float fVitae );

struct VITAL
{
	VITAL()
	{
		data.raises	= 0;
		data.unk = 0;
		data.exp = 0;
		data.current = 100;
	}
	void SetRaises( DWORD dwRaises )
	{
		data.raises	= dwRaises;
		data.exp = GetVitalXP(dwRaises);
	}
	void SetExperience( DWORD dwXP )
	{
		data.exp = dwXP;
		data.raises	= GetVitalLevel(dwXP);
	}

	_VITALDATA data;
};

