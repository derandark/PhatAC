
#include "StdAfx.h"
#include "PhysicsObj.h"
#include "Item.h"
#include "World.h"

void CPhysicsObj::Container_Init()
{
	m_cMaxEquipment = -1;
	m_cMaxInventory = -1;
	m_cMaxContainers = -1;

	m_pEquipment = NULL;
	m_pInventory = NULL;
	m_pContainers = NULL;

	m_cInventoryCount = -1;
	m_cContainerCount = -1;
}

void CPhysicsObj::Container_InitContents(char MaxEquipment, char MaxInventory, char MaxContainers)
{
	m_cMaxEquipment = MaxEquipment;
	m_cMaxInventory = MaxInventory;
	m_cMaxContainers = MaxContainers;

	m_pEquipment = new CBaseItem*[m_cMaxEquipment];
	m_pInventory = new CBaseItem*[m_cMaxInventory];
	m_pContainers = new CBaseItem*[m_cMaxContainers];

	memset(m_pEquipment, 0, sizeof(CBaseItem*) * m_cMaxEquipment);
	memset(m_pInventory, 0, sizeof(CBaseItem*) * m_cMaxInventory);
	memset(m_pContainers, 0, sizeof(CBaseItem*) * m_cMaxContainers);

	m_cInventoryCount = 0;
	m_cContainerCount = 0;
}

BOOL CPhysicsObj::Container_HasContents()
{
	if (m_cMaxEquipment > 0 ||
		m_cMaxInventory > 0 ||
		m_cMaxContainers > 0)	 return TRUE;

	return FALSE;
}

DWORD CPhysicsObj::Container_GetWeaponID()
{
	if (m_pEquipment && m_cMaxEquipment >= 32)
	{
		//24 = 0x01
		//25 = 0x02
		//26 = 0x04
		//27 = 0x08
		//28 = 0x10
		//29 = 0x20
		//30 = 0x40
		//31 = 0x80

		if (m_pEquipment[24]) return m_pEquipment[24]->m_dwGUID; //Magic Weapon
		if (m_pEquipment[22]) return m_pEquipment[22]->m_dwGUID; //Missile Weapon
		if (m_pEquipment[20]) return m_pEquipment[20]->m_dwGUID; //Melee Weapon
	}

	return 0;
}

CPhysicsObj* CPhysicsObj::Container_FindContainer(DWORD dwContainer)
{
	if (m_dwGUID == dwContainer)
		return this;

	for (unsigned char i = 0; i < m_cContainerCount; i++)
	{
		if (m_pContainers[i]->m_dwGUID == dwContainer)
			return m_pContainers[i];
	}

	return NULL;
}

void CPhysicsObj::Container_GetEquippedArmor(ItemVector *pVector)
{
	if (!m_pEquipment)
		return;

	for (char index = 0; index < m_cMaxEquipment; index++)
	{
		CBaseItem* pItem = m_pEquipment[index];

		if (!pItem)
			continue;

		if (pItem->m_ItemType == TYPE_ARMOR)
			pVector->push_back(pItem);
	}
}

void CPhysicsObj::Container_GetWieldedItems(ItemVector *pVector)
{
	if (!m_pEquipment)
		return;

	for (char index = 0; index < m_cMaxEquipment; index++)
	{
		CBaseItem* pItem = m_pEquipment[index];

		if (!pItem)
			continue;

		if (pItem->IsWielded())
			pVector->push_back(pItem);
	}
}

void CPhysicsObj::Container_ReleaseItem(CBaseItem* pItem, BOOL bDirect)
{
	if (!pItem)
		return;

	for (int i = 0; i < m_cMaxEquipment; i++)
	{
		if (m_pEquipment[i] == pItem)
			m_pEquipment[i] = NULL;
	}
	for (int i = 0; i < m_cInventoryCount; i++)
	{
		if (m_pInventory[i] == pItem)
		{
			m_cInventoryCount--;

			for (int j = i; j < m_cInventoryCount; j++)
				m_pInventory[j] = m_pInventory[j + 1];

			i--;
		}
	}
	for (int i = 0; i < m_cContainerCount; i++)
	{
		if (m_pContainers[i] == pItem)
		{
			m_cContainerCount--;

			for (int j = i; j < m_cContainerCount; j++)
				m_pContainers[j] = m_pContainers[j + 1];

			i--;
		}
		else
		{
			if (!bDirect)
				m_pContainers[i]->Container_ReleaseItem(pItem, FALSE);
		}
	}
}

BOOL CPhysicsObj::Container_CanEquip(CBaseItem* pItem, DWORD dwCoverage)
{
	if (!m_pEquipment)
		return FALSE;
	if (!pItem)
		return FALSE;

	if (pItem->GetCoverage1() != dwCoverage)
		LOG(Temp, Normal, "Debug: GC1()==%08X and C==%08X\n", pItem->GetCoverage1(), dwCoverage);

	char index = 0;
	while (index < (sizeof(dwCoverage) << 3))
	{
		if ((dwCoverage >> index) & 1)
		{
			//It will need this slot.

			//The slot is out of range!
			if (index >= m_cMaxEquipment)
				return FALSE;

			//The slot is already in use!
			if (m_pEquipment[index] != NULL)
				return FALSE;
		}
		index++;
	}

	return TRUE;
}

void CPhysicsObj::Container_EquipItem(DWORD dwCell, CBaseItem* pItem, DWORD dwCoverage)
{
	for (char index = 0; index < m_cMaxEquipment; index++)
	{
		if (dwCoverage & 1)
			m_pEquipment[index] = pItem;

		dwCoverage = dwCoverage >> 1;
	}

	if (pItem->m_ItemType != TYPE_ARMOR)
	{
		BinaryWriter Blah;
		Blah.WriteDWORD(0xF749);
		Blah.WriteDWORD(m_dwGUID);
		Blah.WriteDWORD(pItem->m_dwGUID);
		Blah.WriteDWORD(1);
		Blah.WriteDWORD(1);
		Blah.WriteWORD(m_wInstance);
		Blah.WriteWORD(++pItem->m_wNumMovements);
		g_pWorld->BroadcastPVS(dwCell, Blah.GetData(), Blah.GetSize());
	}
}

CBaseItem* CPhysicsObj::Container_FindItem(DWORD dwEID)
{
	for (char i = 0; i < m_cMaxEquipment; i++)
	{
		CBaseItem* pItem = m_pEquipment[i];

		if (pItem)
		{
			if (pItem->m_dwGUID == dwEID)
				return m_pEquipment[i];
		}
	}
	for (char i = 0; i < m_cInventoryCount; i++)
	{
		if (m_pInventory[i]->m_dwGUID == dwEID)
			return m_pInventory[i];
	}
	for (char i = 0; i < m_cContainerCount; i++)
	{
		if (m_pContainers[i]->m_dwGUID == dwEID)
			return m_pContainers[i];

		CBaseItem *pItem;

		if (pItem = m_pContainers[i]->Container_FindItem(dwEID))
			return pItem;
	}

	return NULL;
}

BOOL CPhysicsObj::Container_CanStore(CBaseItem* pItem)
{
	DWORD dwSlot = pItem->GetSlotType();

	switch (dwSlot)
	{
	case 0:
		if (m_cInventoryCount >= m_cMaxInventory)
		{
			for (char i = 0; i < m_cInventoryCount; i++)
			{
				if (m_pInventory[i] == pItem)
					return TRUE;
			}
			return FALSE;
		}

		return TRUE;
	case 1:
		if (m_cContainerCount >= m_cMaxContainers)
		{
			for (char i = 0; i < m_cContainerCount; i++)
			{
				if (m_pContainers[i] == pItem)
					return TRUE;
			}
			return FALSE;
		}

		return TRUE;
	case 2:
		//Should check if player already has this foci!
		return TRUE;
	default:
		return FALSE;
	}
}

char CPhysicsObj::Container_InsertInventoryItem(DWORD dwCell, CBaseItem* pItem, char cSlot)
{
	// You should check if the inventory is full before calling this.

	if (cSlot >= m_cInventoryCount)
		cSlot = m_cInventoryCount;
	else
	{
		for (unsigned char i = m_cInventoryCount; i > cSlot; i--)
		{
			CBaseItem *pSlideItem = m_pInventory[i - 1];
			m_pInventory[i] = pSlideItem;
		}
	}

	m_pInventory[cSlot] = pItem;
	m_cInventoryCount++;

	pItem->m_wNumMovements++;

	BinaryWriter Blah;

	Blah.WriteDWORD(0xF74A);
	Blah.WriteDWORD(pItem->m_dwGUID);
	Blah.WriteWORD(pItem->m_wInstance);
	Blah.WriteWORD(pItem->m_wNumMovements);
	g_pWorld->BroadcastPVS(dwCell, Blah.GetData(), Blah.GetSize());

	return cSlot;
}

void CPhysicsObj::Container_Shutdown()
{
	SafeDeleteArray(m_pEquipment);
	SafeDeleteArray(m_pInventory);
	SafeDeleteArray(m_pContainers);
}