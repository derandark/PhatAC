
#pragma once

enum eEquipSlot
{
	eEquipSlotNone = 0,
	eEquipSlotMelee = 1,
	eEquipSlotMissile = 2,
	eEquipSlotShield = 3
};

enum eEquipType
{
	eEquipTypeNone = 0,
	eEquipTypeMelee = 1,
	eEquipTypeMissile = 2,
	eEquipTypeAmmo = 3,
	eEquipTypeShield = 4
};

enum eSlotType
{
	eSlotItem = 0,
	eSlotContainer = 1,
	eSlotFoci = 2
};

#define CA_HEAD 0x00000001
#define CA_CHEST_UNDERWEAR 0x00000002
#define CA_GIRTH_UNDERWEAR 0x00000004
#define CA_UPPERARMS_UNDERWEAR 0x00000008
#define CA_LOWERARMS_UNDERWEAR 0x00000010
#define CA_HANDS 0x00000020
#define CA_UPPERLEGS_UNDERWEAR 0x00000040
#define CA_LOWERLEGS_UNDERWEAR 0x00000080
#define CA_FEET 0x00000100
#define CA_CHEST 0x00000200
#define CA_GIRTH 0x00000400
#define CA_UPPERARMS 0x00000800
#define CA_LOWERARMS 0x00001000
#define CA_UPPERLEGS 0x00002000
#define CA_LOWERLEGS 0x00004000
#define CA_WEAPON_MELEE 0x00100000
#define CA_SHIELD 0x00200000
#define CA_WEAPON_PROJECTILE 0x00400000
#define CA_WEAPON_AMMUNITION 0x00800000
#define CA_WEAPON_FOCUS 0x01000000

class CBaseItem : public CPhysicsObj
{
public:
	CBaseItem();
	~CBaseItem();

	virtual BOOL IsItem() { return TRUE; }

	BOOL IsContained();
	BOOL IsWielded();

	virtual DWORD GetLandcell();
	virtual DWORD GetContainerID();
	virtual DWORD GetWielderID();
	CPhysicsObj* GetWorldContainer();
	CPhysicsObj* GetWorldWielder();
	virtual void SetWorldContainer(DWORD dwCell, CPhysicsObj *pContainer);
	virtual void SetWorldWielder(DWORD dwCell, CPhysicsObj *pWielder);
	virtual void SetWorldCoverage(DWORD dwCell, DWORD dwCoverage);

	virtual DWORD GetEquipSlot() { return m_dwEquipSlot; }
	virtual DWORD GetEquipType() { return m_dwEquipType; }
	virtual DWORD GetCoverage1() { return m_dwCoverage1; }
	virtual DWORD GetCoverage2() { return m_dwCoverage2; }
	virtual DWORD GetCoverage3() { return m_dwCoverage3; }
	virtual DWORD GetSlotType() { return eSlotItem; }

	virtual BOOL HasBurden() { return TRUE; }
	virtual BOOL HasValue() { return TRUE; }
	virtual BOOL HasCoverage();

	BOOL CanEquip();

// protected:
	BYTE m_bWieldSequence;
	BYTE m_bContainSequence;
	BYTE m_bCoverSequence;

	CPhysicsObj* m_pWielder;
	CPhysicsObj* m_pContainer;
};

class CBaseWand : public CBaseItem
{
public:
	CBaseWand();
};

class CBaseArmor : public CBaseItem
{
public:
	CBaseArmor();
};

class CAcademyCoat : public CBaseArmor
{
public:
	CAcademyCoat();
};

class CTuskerHelm : public CBaseArmor
{
public:
	CTuskerHelm();
};

class CBoboHelm : public CBaseArmor
{
public:
	CBoboHelm();
};

class CPhatRobe : public CBaseArmor
{
public:
	CPhatRobe();
};

class CEnvoyShield : public CBaseItem
{
public:
	CEnvoyShield();
};