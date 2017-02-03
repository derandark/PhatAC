
#pragma once

class NetFood;
class CPhysicsObj;
class CBaseMonster;
class CBasePlayer;

extern NetFood *CreateObject(CPhysicsObj *pEntity);	//0x0000F745
extern NetFood *IdentifyObject(CPhysicsObj *pEntity);	//0x000000C9
extern NetFood *LoginCharacter(CBasePlayer *pPlayer);	//0x00000013
extern NetFood *HealthUpdate(CBaseMonster *pMonster);	//0x000001C0
extern NetFood *InventoryEquip(DWORD dwItemID, DWORD dwCoverage); //0x00000023
extern NetFood *InventoryMove(DWORD dwItemID, DWORD dwContainerID, DWORD dwSlot, DWORD dwType); //0x00000022
extern NetFood *InventoryDrop(DWORD dwItemID); //0x0000019A
extern NetFood *MoveUpdate(CPhysicsObj *pEntity);		//0x0000F748

