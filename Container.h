


void Container_Init();
void Container_InitContents(char MaxEquipment, char MaxInventory, char MaxContainers);
void Container_Shutdown();

BOOL Container_HasContents();

CPhysicsObj* Container_FindContainer(DWORD dwContainer);
CBaseItem* Container_FindItem(DWORD dwEID);
void Container_GetEquippedArmor(ItemVector* pVector);
void Container_GetWieldedItems(ItemVector* pVector);
DWORD Container_GetWeaponID();

BOOL Container_CanEquip(CBaseItem* pItem, DWORD dwCoverage);
BOOL Container_CanStore(CBaseItem* pItem);

long Container_DropItem(DWORD dwItem);
void Container_EquipItem(DWORD dwCell, CBaseItem* pItem, DWORD dwCoverage);
char Container_InsertInventoryItem(DWORD dwCell, CBaseItem* pItem, char cSlot);

char Container_MaxEquipmentSlots() { return m_cMaxEquipment; }
char Container_MaxInventorySlots() { return m_cMaxInventory; }
char Container_MaxContainerSlots() { return m_cMaxContainers; }

void Container_ReleaseItem(CBaseItem* pItem, BOOL bDirect);

CBaseItem** m_pEquipment;
char m_cMaxEquipment;

CBaseItem** m_pInventory;
char m_cInventoryCount;
char m_cMaxInventory;

CBaseItem** m_pContainers;
char m_cContainerCount;
char m_cMaxContainers;

