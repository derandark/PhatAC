
#pragma once

class BinaryWriter;
class CPhysicsObj;
class CBaseMonster;
class CBasePlayer;

extern BinaryWriter *CreateObject(CPhysicsObj *pEntity);	//0x0000F745
extern BinaryWriter *IdentifyObject(CPhysicsObj *pEntity);	//0x000000C9
extern BinaryWriter *LoginCharacter(CBasePlayer *pPlayer);	//0x00000013
extern BinaryWriter *HealthUpdate(CBaseMonster *pMonster);	//0x000001C0
extern BinaryWriter *InventoryEquip(DWORD dwItemID, DWORD dwCoverage); //0x00000023
extern BinaryWriter *InventoryMove(DWORD dwItemID, DWORD dwContainerID, DWORD dwSlot, DWORD dwType); //0x00000022
extern BinaryWriter *InventoryDrop(DWORD dwItemID); //0x0000019A
extern BinaryWriter *MoveUpdate(CPhysicsObj *pEntity);		//0x0000F748

