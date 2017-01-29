
#pragma once

enum eAttribute
{
	eStrength = 0x01,
	eEndurance = 0x02,
	eQuickness = 0x03,
	eCoordination = 0x04,
	eFocus = 0x05,
	eSelf = 0x06,
};

#pragma pack(push, 1)
struct _ATTRIBUTEDATA
{
	DWORD raises;
	DWORD base;
	DWORD exp;
};
#pragma pack(pop)

struct ATTRIBUTE;

extern DWORD GetAttributeXP(DWORD Level);
extern DWORD GetAttributeLevel(DWORD XP);
extern DWORD GetAttributeMax();
extern DWORD GetAttributeMaxXP();
extern const char* GetAttributeName(eAttribute index);
extern DWORD GetAttributeMax(ATTRIBUTE *pAttrib, float fVitae);

struct ATTRIBUTE
{
	ATTRIBUTE()
	{
		data.raises = 0;
		data.base = 10;
		data.exp = 0;
	}
	void SetBase(DWORD dwBase)
	{
		data.base = dwBase;
	}
	void SetRaises(DWORD dwRaises)
	{
		data.raises = dwRaises;
		data.exp = GetAttributeXP(dwRaises);
	}
	void SetExperience(DWORD dwXP)
	{
		data.exp = dwXP;
		data.raises = GetAttributeLevel(dwXP);
	}

	_ATTRIBUTEDATA data;
};






