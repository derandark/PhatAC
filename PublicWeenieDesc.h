#pragma once

#include "BinaryReader.h"

class RestrictionDB
{
public:
	RestrictionDB() { }
};

class PublicWeenieDesc
{
public:
	uint32_t header;
	uint32_t header2;
	std::string _name;
	uint32_t _wcid;
	uint32_t _iconID;
	ITEM_TYPE _type;
	uint32_t _bitfield;
	std::string _plural_name;
	uint8_t _itemsCapacity;
	uint8_t _containersCapacity;
	AMMO_TYPE _ammoType;
	uint32_t _value;
	ITEM_USEABLE _useability;
	float _useRadius;
	ITEM_TYPE _targetType;
	uint32_t _effects;
	uint8_t _combatUse;
	uint16_t _structure;
	uint16_t _maxStructure;
	uint16_t _stackSize;
	uint16_t _maxStackSize;
	uint32_t _containerID;
	uint32_t _wielderID;
	uint32_t _valid_locations;
	uint32_t _location;
	uint32_t _priority;
	uint8_t _blipColor;
	RadarEnum _radar_enum;
	uint16_t _pscript;
	float _workmanship;
	uint16_t _burden;
	uint16_t _spellID;
	uint32_t _house_owner_iid;
	RestrictionDB _db;
	uint32_t _hook_item_types;
	uint32_t _monarch;
	ITEM_TYPE _hook_type;
	uint32_t _iconOverlayID;
	uint32_t _iconUnderlayID;
	MaterialType _material_type;
	uint32_t _cooldown_id;
	double _cooldown_duration;
	uint32_t _pet_owner;

	void Unpack(BinaryReader &binaryReader) {
		PublicWeenieDesc &newObj = *this;
		newObj.header = binaryReader.ReadUInt32();
		newObj._name = binaryReader.ReadString();
		newObj._wcid = binaryReader.ReadPackedDWORD();
		newObj._iconID = 0x6000000 | binaryReader.ReadPackedDWORD();
		newObj._type = (ITEM_TYPE)binaryReader.ReadUInt32();
		newObj._bitfield = binaryReader.ReadUInt32();
		newObj.header2 = 0;

		binaryReader.ReadAlign();

		if ((newObj._bitfield & (uint32_t)BitfieldIndex::BF_INCLUDES_SECOND_HEADER) != 0) {
			newObj.header2 = binaryReader.ReadUInt32();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_PluralName) != 0) {
			newObj._plural_name = binaryReader.ReadString();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_ItemsCapacity) != 0) {
			newObj._itemsCapacity = binaryReader.ReadByte();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_ContainersCapacity) != 0) {
			newObj._containersCapacity = binaryReader.ReadByte();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_AmmoType) != 0) {
			newObj._ammoType = (AMMO_TYPE)binaryReader.ReadUInt16();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_Value) != 0) {
			newObj._value = binaryReader.ReadUInt32();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_Useability) != 0) {
			newObj._useability = (ITEM_USEABLE)binaryReader.ReadUInt32();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_UseRadius) != 0) {
			newObj._useRadius = binaryReader.ReadSingle();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_TargetType) != 0) {
			newObj._targetType = (ITEM_TYPE)binaryReader.ReadUInt32();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_UIEffects) != 0) {
			newObj._effects = binaryReader.ReadUInt32();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_CombatUse) != 0) {
			newObj._combatUse = binaryReader.ReadByte();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_Structure) != 0) {
			newObj._structure = binaryReader.ReadUInt16();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_MaxStructure) != 0) {
			newObj._maxStructure = binaryReader.ReadUInt16();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_StackSize) != 0) {
			newObj._stackSize = binaryReader.ReadUInt16();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_MaxStackSize) != 0) {
			newObj._maxStackSize = binaryReader.ReadUInt16();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_ContainerID) != 0) {
			newObj._containerID = binaryReader.ReadUInt32();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_WielderID) != 0) {
			newObj._wielderID = binaryReader.ReadUInt32();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_ValidLocations) != 0) {
			newObj._valid_locations = binaryReader.ReadUInt32();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_Location) != 0) {
			newObj._location = binaryReader.ReadUInt32();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_Priority) != 0) {
			newObj._priority = binaryReader.ReadUInt32();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_BlipColor) != 0) {
			newObj._blipColor = binaryReader.ReadByte();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_RadarEnum) != 0) {
			newObj._radar_enum = (RadarEnum)binaryReader.ReadByte();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_PScript) != 0) {
			newObj._pscript = binaryReader.ReadUInt16();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_Workmanship) != 0) {
			newObj._workmanship = binaryReader.ReadSingle();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_Burden) != 0) {
			newObj._burden = binaryReader.ReadUInt16();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_SpellID) != 0) {
			newObj._spellID = binaryReader.ReadUInt16();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_HouseOwner) != 0) {
			newObj._house_owner_iid = binaryReader.ReadUInt32();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_HouseRestrictions) != 0) {
			// TODO: Read here once you get RestrictedDB read finished
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_HookItemTypes) != 0) {
			newObj._hook_item_types = binaryReader.ReadUInt32();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_Monarch) != 0) {
			newObj._monarch = binaryReader.ReadUInt32();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_HookType) != 0) {
			newObj._hook_type = (ITEM_TYPE)binaryReader.ReadUInt16();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_IconOverlay) != 0) {
			newObj._iconOverlayID = 0x6000000 | binaryReader.ReadPackedDWORD();
		}

		if ((newObj.header2 & (uint32_t)PublicWeenieDescPackHeader2::PWD2_Packed_IconUnderlay) != 0) {
			newObj._iconUnderlayID = 0x6000000 | binaryReader.ReadPackedDWORD();
		}

		if ((newObj.header & (uint32_t)PublicWeenieDescPackHeader::PWD_Packed_MaterialType) != 0) {
			newObj._material_type = (MaterialType)binaryReader.ReadUInt32();
		}

		if ((newObj.header2 & (uint32_t)PublicWeenieDescPackHeader2::PWD2_Packed_CooldownID) != 0) {
			newObj._cooldown_id = binaryReader.ReadUInt32();
		}

		if ((newObj.header2 & (uint32_t)PublicWeenieDescPackHeader2::PWD2_Packed_CooldownDuration) != 0) {
			newObj._cooldown_duration = binaryReader.ReadDouble();
		}

		if ((newObj.header2 & (uint32_t)PublicWeenieDescPackHeader2::PWD2_Packed_PetOwner) != 0) {
			newObj._pet_owner = binaryReader.ReadUInt32();
		}

		binaryReader.ReadAlign();
	}
};

