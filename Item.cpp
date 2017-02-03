
#include "StdAfx.h"
#include "PhysicsObj.h"
#include "Item.h"
#include "LandBlock.h"

#include "BinaryWriter.h"
#include "World.h"

CBaseItem::CBaseItem()
{
	m_dwSoundSet = 0x20000014;
	m_dwEffectSet = 0x3400002B;

	m_wBurden = 0;
	m_dwValue = 1;

	m_bWieldSequence = 0;
	m_pWielder = NULL;

	m_bContainSequence = 0;
	m_pContainer = NULL;

	m_dwEquipSlot = eEquipTypeNone;
	m_dwEquipType = eEquipSlotNone;

	m_bCoverSequence = 0;
	m_dwCoverage1 = NULL; //What it CAN cover?
	m_dwCoverage2 = NULL; //What it IS covering?
	m_dwCoverage3 = NULL; //??

	m_strName = "Item";

	m_PhysicsState = PhysicsState::GRAVITY_PS | PhysicsState::IGNORE_COLLISIONS_PS | PhysicsState::ETHEREAL_PS;
}

CBaseItem::~CBaseItem()
{
}

BOOL CBaseItem::CanEquip()
{
	//...
	return TRUE;
}

DWORD CBaseItem::GetLandcell()
{
	if (m_pContainer)
		return m_pContainer->GetLandcell();
	if (m_pWielder)
		return m_pWielder->GetLandcell();

	return m_Origin.landcell;
}

BOOL CBaseItem::IsContained() {
	return GetContainerID() ? TRUE : FALSE;
}
BOOL CBaseItem::IsWielded() {
	return GetWielderID() ? TRUE : FALSE;
}

DWORD CBaseItem::GetContainerID() {
	return (m_pContainer ? m_pContainer->m_dwGUID : NULL);
}
DWORD CBaseItem::GetWielderID() {
	return (m_pWielder ? m_pWielder->m_dwGUID : NULL);
}

CPhysicsObj* CBaseItem::GetWorldContainer() {
	return m_pContainer;
}
CPhysicsObj* CBaseItem::GetWorldWielder() {
	return m_pWielder;
}

void CBaseItem::SetWorldContainer(DWORD dwCell, CPhysicsObj *pContainer)
{
	BinaryWriter SWC;

	SWC.WriteDWORD(0x22D);
	SWC.WriteBYTE(++m_bContainSequence);
	SWC.WriteDWORD(m_dwGUID);
	SWC.WriteDWORD(2);
	SWC.WriteDWORD(pContainer ? pContainer->m_dwGUID : 0);
	g_pWorld->BroadcastPVS(dwCell, SWC.GetData(), SWC.GetSize(), PRIVATE_MSG);

	if (pContainer) //&& !m_pContainer )
	{
		CLandBlock* pBlock = GetBlock();

		if (pBlock)
			pBlock->Release(this);
	}

	m_pContainer = pContainer;
}

void CBaseItem::SetWorldWielder(DWORD dwCell, CPhysicsObj *pWielder)
{
	if (m_ItemType == TYPE_ARMOR)
	{
		LOG(Object, Warning, "Trying to wield armor!\n");
	}

	BinaryWriter SWW;

	SWW.WriteDWORD(0x22D);
	SWW.WriteBYTE(++m_bWieldSequence);
	SWW.WriteDWORD(m_dwGUID);
	SWW.WriteDWORD(3);
	SWW.WriteDWORD(pWielder ? pWielder->m_dwGUID : 0);
	g_pWorld->BroadcastPVS(dwCell, SWW.GetData(), SWW.GetSize(), PRIVATE_MSG);

	if (pWielder) //&& !m_pWielder )
	{
		CLandBlock* pBlock = GetBlock();

		if (pBlock)
			pBlock->Release(this);
	}
	else
	{
		SetWorldCoverage(dwCell, NULL);
	}

	m_pWielder = pWielder;
}

void CBaseItem::SetWorldCoverage(DWORD dwCell, DWORD dwCoverage)
{
	BinaryWriter SC;

	SC.WriteDWORD(0x229);
	SC.WriteBYTE(++m_bCoverSequence);
	SC.WriteDWORD(m_dwGUID);
	SC.WriteDWORD(10);
	SC.WriteDWORD(dwCoverage);
	g_pWorld->BroadcastPVS(dwCell, SC.GetData(), SC.GetSize(), PRIVATE_MSG);

	m_dwCoverage2 = dwCoverage;
}

BOOL CBaseItem::HasCoverage()
{
	return ((m_dwCoverage1 || m_dwCoverage2 || m_dwCoverage3) ? TRUE : FALSE);
}

CBaseWand::CBaseWand()
{
	//We'll be a drudge wand by default =)
	m_dwModel = 0x02000B79;
	m_fScale = 1.2f;

	m_wBurden = 0;
	m_dwValue = 1337;
	m_strName = "Wand of Death";

	m_wTypeID = 0x2FBD;
	m_wIcon = 0x22B3;
	m_ItemType = TYPE_CASTER;

	m_dwEquipSlot = eEquipSlotMelee;
	m_dwCoverage1 = CA_WEAPON_FOCUS;
	//m_dwCoverage2		= CA_WEAPON_FOCUS;
	//m_dwCoverage3		= CA_WEAPON_FOCUS;
}

CEnvoyShield::CEnvoyShield()
{
	//We'll be a drudge wand by default =)
	m_dwModel = 0x02001035;

	m_wBurden = 0;
	m_dwValue = 1;
	m_strName = "Envoy's Shield";

	m_wTypeID = 0x65CA;
	m_wIcon = 0x13FF;
	m_ItemType = TYPE_ARMOR;

	m_dwEquipSlot = eEquipSlotShield;
	m_dwEquipType = eEquipTypeShield;

	m_dwCoverage1 = CA_SHIELD;
}

CBaseArmor::CBaseArmor()
{
	//We'll be a drudge wand by default =)
	m_strName = "Armor";
	m_ItemType = TYPE_ARMOR;
	m_dwEquipSlot = eEquipSlotNone;

	//m_dwCoverage1		= ..;
	//m_dwCoverage2		= ..;
	//m_dwCoverage3		= ..;
}

ModelInfo* CBaseArmor::GetArmorModel()
{
	return &m_miArmorModel;
}

CAcademyCoat::CAcademyCoat()
{
	m_strName = "Academy Coat";
	m_dwModel = 0x020000D4;

	m_wTypeID = 0x339A;
	m_wIcon = 0x1F14;

	m_dwCoverage1 = CA_LOWERARMS | CA_UPPERARMS | CA_GIRTH | CA_CHEST;
	m_dwCoverage3 = 0x3C;

	m_miBaseModel.dwBasePalette = 0x7E;
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x1083, 0x50, 0x0C));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x1083, 0x60, 0x0C));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x1083, 0x74, 0x0C));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x1083, 0xD8, 0x18));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x108F, 0x48, 0x08));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x108F, 0x6C, 0x08));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x108F, 0xAE, 0x0C));

	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0x3D5, 0x1AFE));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0x3D4, 0x1AFC));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0xBB0, 0x1AF9));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0xCBE, 0x1AFD));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0x2C4, 0x1AFA));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0x2CC, 0x1AFB));

	m_miBaseModel.lModels.push_back(ModelRpl(0x00, 0x474));

	//m_miArmorModel.lModels.push_back( ModelRpl(0x10, 0x449) );
}

CTuskerHelm::CTuskerHelm()
{
	m_strName = "Tusker Helm";
	m_dwModel = 0x020000D4;

	m_wTypeID = 0x339A;
	m_wIcon = 0x1F14;

	m_dwCoverage1 = CA_HEAD;
	m_dwCoverage3 = 0x3C;

	m_miBaseModel.dwBasePalette = 0x7E;
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x1083, 0x50, 0x0C));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x1083, 0x60, 0x0C));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x1083, 0x74, 0x0C));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x1083, 0xD8, 0x18));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x108F, 0x48, 0x08));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x108F, 0x6C, 0x08));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x108F, 0xAE, 0x0C));

	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0x3D5, 0x1AFE));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0x3D4, 0x1AFC));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0xBB0, 0x1AF9));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0xCBE, 0x1AFD));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0x2C4, 0x1AFA));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0x2CC, 0x1AFB));

	m_miBaseModel.lModels.push_back(ModelRpl(0x00, 0x474));

	m_miArmorModel.dwBasePalette = 0x7E;
	m_miArmorModel.lPalettes.push_back(PaletteRpl(0x485, 0xF0, 0x0A));
	m_miArmorModel.lPalettes.push_back(PaletteRpl(0x5EA, 0xFA, 0x06));

	m_miArmorModel.lTextures.push_back(TextureRpl(0x10, 0x3C8, 0x3C8));
	m_miArmorModel.lTextures.push_back(TextureRpl(0x10, 0x3C9, 0x3C9));

	m_miArmorModel.lModels.push_back(ModelRpl(0x10, 0x449));
}

CBoboHelm::CBoboHelm()
{
	m_strName = "Helm of Bobo";
	m_dwModel = 0x020000D4;

	m_wTypeID = 0x339B;
	m_wIcon = 0x2957;

	m_dwCoverage1 = CA_HEAD;
	m_dwCoverage3 = 0x3C;

	m_miBaseModel.dwBasePalette = 0x7E;
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x1083, 0x50, 0x0C));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x1083, 0x60, 0x0C));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x1083, 0x74, 0x0C));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x1083, 0xD8, 0x18));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x108F, 0x48, 0x08));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x108F, 0x6C, 0x08));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x108F, 0xAE, 0x0C));

	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0x3D5, 0x1AFE));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0x3D4, 0x1AFC));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0xBB0, 0x1AF9));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0xCBE, 0x1AFD));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0x2C4, 0x1AFA));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0x2CC, 0x1AFB));

	m_miBaseModel.lModels.push_back(ModelRpl(0x00, 0x474));

	//m_miArmorModel.wBasePalette	= 0x7E;
	//m_miArmorModel.lPalettes.push_back( PaletteRpl(0x485, 0xF0, 0x0A) );
	//m_miArmorModel.lPalettes.push_back( PaletteRpl(0x5EA, 0xFA, 0x06) );

	//m_miArmorModel.lTextures.push_back( TextureRpl(0x10, 0x3C8, 0x3C8) );
	//m_miArmorModel.lTextures.push_back( TextureRpl(0x10, 0x3C9, 0x3C9) );

	m_miArmorModel.lModels.push_back(ModelRpl(0x10, 0x2CF7));
}

CPhatRobe::CPhatRobe()
{
	m_strName = "Phat Robe";
	m_dwModel = 0x020000D4;

	m_wTypeID = 0x1337;
	m_wIcon = 0x1B93;

	m_dwCoverage1 = CA_HEAD | CA_CHEST | CA_GIRTH | CA_UPPERARMS | CA_LOWERARMS | CA_UPPERLEGS | CA_LOWERLEGS | CA_FEET;
	m_dwCoverage3 = 0x3C;

	m_wBurden = 200;
	m_dwStats[eArmorLevel] = 1337;

	m_miBaseModel.dwBasePalette = 0x7E;
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x1083, 0x50, 0x0C));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x1083, 0x60, 0x0C));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x1083, 0x74, 0x0C));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x1083, 0xD8, 0x18));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x108F, 0x48, 0x08));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x108F, 0x6C, 0x08));
	m_miBaseModel.lPalettes.push_back(PaletteRpl(0x108F, 0xAE, 0x0C));

	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0x3D5, 0x1AFE));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0x3D4, 0x1AFC));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0xBB0, 0x1AF9));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0xCBE, 0x1AFD));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0x2C4, 0x1AFA));
	m_miBaseModel.lTextures.push_back(TextureRpl(0x00, 0x2CC, 0x1AFB));

	m_miBaseModel.lModels.push_back(ModelRpl(0x00, 0x474));

	m_miArmorModel.dwBasePalette = 0x7E;
	//0x1080 = white robe
	m_miArmorModel.lPalettes.push_back(PaletteRpl(0xFFD, 0x28, 0x28));
	m_miArmorModel.lPalettes.push_back(PaletteRpl(0xFFD, 0x50, 0x0C));
	m_miArmorModel.lPalettes.push_back(PaletteRpl(0xFFD, 0x60, 0x0C));
	m_miArmorModel.lPalettes.push_back(PaletteRpl(0xFFD, 0x74, 0x0C));

	m_miArmorModel.lTextures.push_back(TextureRpl(0x00, 0x1879, 0x1879));
	m_miArmorModel.lTextures.push_back(TextureRpl(0x00, 0x1878, 0x1878));
	m_miArmorModel.lTextures.push_back(TextureRpl(0x01, 0x1880, 0x1880));
	m_miArmorModel.lTextures.push_back(TextureRpl(0x02, 0x187F, 0x187F));
	m_miArmorModel.lTextures.push_back(TextureRpl(0x05, 0x1880, 0x1880));
	m_miArmorModel.lTextures.push_back(TextureRpl(0x06, 0x187F, 0x187F));
	m_miArmorModel.lTextures.push_back(TextureRpl(0x09, 0x3D5, 0x187C));
	m_miArmorModel.lTextures.push_back(TextureRpl(0x09, 0x3D4, 0x187D));
	m_miArmorModel.lTextures.push_back(TextureRpl(0x0A, 0x187B, 0x187B));
	m_miArmorModel.lTextures.push_back(TextureRpl(0x0B, 0x187A, 0x187A));
	m_miArmorModel.lTextures.push_back(TextureRpl(0x0D, 0x187B, 0x187B));
	m_miArmorModel.lTextures.push_back(TextureRpl(0x0E, 0x187A, 0x187A));
	//m_miArmorModel.lTextures.push_back( TextureRpl(0x10, 0x187E, 0x187E) ); - Face Texture

	m_miArmorModel.lModels.push_back(ModelRpl(0x00, 0x1A16));
	m_miArmorModel.lModels.push_back(ModelRpl(0x01, 0x1A0D));
	m_miArmorModel.lModels.push_back(ModelRpl(0x02, 0x1A06));
	m_miArmorModel.lModels.push_back(ModelRpl(0x03, 0x1EC));
	m_miArmorModel.lModels.push_back(ModelRpl(0x04, 0x1EC));
	m_miArmorModel.lModels.push_back(ModelRpl(0x05, 0x1A11));
	m_miArmorModel.lModels.push_back(ModelRpl(0x06, 0x1A09));
	m_miArmorModel.lModels.push_back(ModelRpl(0x07, 0x1EC));
	m_miArmorModel.lModels.push_back(ModelRpl(0x08, 0x1EC));
	m_miArmorModel.lModels.push_back(ModelRpl(0x09, 0x120D));
	m_miArmorModel.lModels.push_back(ModelRpl(0x0A, 0x19F7));
	m_miArmorModel.lModels.push_back(ModelRpl(0x0B, 0x19EF));
	m_miArmorModel.lModels.push_back(ModelRpl(0x0D, 0x19FF));
	m_miArmorModel.lModels.push_back(ModelRpl(0x0E, 0x19EF));
	m_miArmorModel.lModels.push_back(ModelRpl(0x10, 0x1A13));
}

