
#pragma once

enum RadarEnum {
	Undef_RadarEnum,
	ShowNever_RadarEnum,
	ShowMovement_RadarEnum,
	ShowAttacking_RadarEnum,
	ShowAlways_RadarEnum
};

enum ITEM_USEABLE {
	USEABLE_UNDEF = 0,
	USEABLE_NO = (1 << 0),
	USEABLE_SELF = (1 << 1),
	USEABLE_WIELDED = (1 << 2),
	USEABLE_CONTAINED = (1 << 3),
	USEABLE_VIEWED = (1 << 4),
	USEABLE_REMOTE = (1 << 5),
	USEABLE_NEVER_WALK = (1 << 6),
	USEABLE_OBJSELF = (1 << 7),
	USEABLE_CONTAINED_VIEWED = 24,
	USEABLE_CONTAINED_VIEWED_REMOTE = 56,
	USEABLE_CONTAINED_VIEWED_REMOTE_NEVER_WALK = 120,
	USEABLE_VIEWED_REMOTE = 48,
	USEABLE_VIEWED_REMOTE_NEVER_WALK = 112,
	USEABLE_REMOTE_NEVER_WALK = 96,
	USEABLE_SOURCE_WIELDED_TARGET_WIELDED = 262148,
	USEABLE_SOURCE_WIELDED_TARGET_CONTAINED = 524292,
	USEABLE_SOURCE_WIELDED_TARGET_VIEWED = 1048580,
	USEABLE_SOURCE_WIELDED_TARGET_REMOTE = 2097156,
	USEABLE_SOURCE_WIELDED_TARGET_REMOTE_NEVER_WALK = 6291460,
	USEABLE_SOURCE_CONTAINED_TARGET_WIELDED = 262152,
	USEABLE_SOURCE_CONTAINED_TARGET_CONTAINED = 524296,
	USEABLE_SOURCE_CONTAINED_TARGET_OBJSELF_OR_CONTAINED = 8912904,
	USEABLE_SOURCE_CONTAINED_TARGET_SELF_OR_CONTAINED = 655368,
	USEABLE_SOURCE_CONTAINED_TARGET_VIEWED = 1048584,
	USEABLE_SOURCE_CONTAINED_TARGET_REMOTE = 2097160,
	USEABLE_SOURCE_CONTAINED_TARGET_REMOTE_NEVER_WALK = 6291464,
	USEABLE_SOURCE_CONTAINED_TARGET_REMOTE_OR_SELF = 2228232,
	USEABLE_SOURCE_VIEWED_TARGET_WIELDED = 262160,
	USEABLE_SOURCE_VIEWED_TARGET_CONTAINED = 524304,
	USEABLE_SOURCE_VIEWED_TARGET_VIEWED = 1048592,
	USEABLE_SOURCE_VIEWED_TARGET_REMOTE = 2097168,
	USEABLE_SOURCE_REMOTE_TARGET_WIELDED = 262176,
	USEABLE_SOURCE_REMOTE_TARGET_CONTAINED = 524320,
	USEABLE_SOURCE_REMOTE_TARGET_VIEWED = 1048608,
	USEABLE_SOURCE_REMOTE_TARGET_REMOTE = 2097184,
	USEABLE_SOURCE_REMOTE_TARGET_REMOTE_NEVER_WALK = 6291488,
	USEABLE_SOURCE_MASK = 65535,
	USEABLE_TARGET_MASK = -65536,
};

enum ITEM_TYPE {
	TYPE_UNDEF = 0,
	TYPE_MELEE_WEAPON = (1 << 0), // 1
	TYPE_ARMOR = (1 << 1), // 2
	TYPE_CLOTHING = (1 << 2), // 4
	TYPE_JEWELRY = (1 << 3), // 8
	TYPE_CREATURE = (1 << 4), // 0x10
	TYPE_FOOD = (1 << 5), // 0x20
	TYPE_MONEY = (1 << 6), // x40
	TYPE_MISC = (1 << 7), // 0x80
	TYPE_MISSILE_WEAPON = (1 << 8), // 0x100
	TYPE_CONTAINER = (1 << 9),
	TYPE_USELESS = (1 << 10),
	TYPE_GEM = (1 << 11),
	TYPE_SPELL_COMPONENTS = (1 << 12), // 0x1000
	TYPE_WRITABLE = (1 << 13),
	TYPE_KEY = (1 << 14),
	TYPE_CASTER = (1 << 15),
	TYPE_PORTAL = (1 << 16), // 0x10000
	TYPE_LOCKABLE = (1 << 17),
	TYPE_PROMISSORY_NOTE = (1 << 18),
	TYPE_MANASTONE = (1 << 19),
	TYPE_SERVICE = (1 << 20), // 0x100000
	TYPE_MAGIC_WIELDABLE = (1 << 21),
	TYPE_CRAFT_COOKING_BASE = (1 << 22),
	TYPE_CRAFT_ALCHEMY_BASE = (1 << 23),
	// NOTE: Skip 1
	TYPE_CRAFT_FLETCHING_BASE = (1 << 25), // 0x2000000
	TYPE_CRAFT_ALCHEMY_INTERMEDIATE = (1 << 26),
	TYPE_CRAFT_FLETCHING_INTERMEDIATE = (1 << 27),
	TYPE_LIFESTONE = (1 << 28), // 0x10000000
	TYPE_TINKERING_TOOL = (1 << 29),
	TYPE_TINKERING_MATERIAL = (1 << 30),
	TYPE_GAMEBOARD = (1 << 31),
	TYPE_PORTAL_MAGIC_TARGET = 268500992,
	TYPE_LOCKABLE_MAGIC_TARGET = 640,
	TYPE_VESTEMENTS = 6,
	TYPE_WEAPON = 257,
	TYPE_WEAPON_OR_CASTER = 33025,
	TYPE_ITEM = 3013615,
	TYPE_REDIRECTABLE_ITEM_ENCHANTMENT_TARGET = 33031,
	TYPE_ITEM_ENCHANTABLE_TARGET = 560015,
	TYPE_SELF = 0,
	TYPE_VENDOR_SHOPKEEP = 1208248231,
	TYPE_VENDOR_GROCER = 4481568
};

enum PhysicsState {
	STATIC_PS = (1 << 0),
	UNUSED1_PS = (1 << 1),
	ETHEREAL_PS = (1 << 2),
	REPORT_COLLISIONS_PS = (1 << 3),
	IGNORE_COLLISIONS_PS = (1 << 4), // 0x10
	NODRAW_PS = (1 << 5),
	MISSILE_PS = (1 << 6),
	PUSHABLE_PS = (1 << 7),
	ALIGNPATH_PS = (1 << 8), // 0x100
	PATHCLIPPED_PS = (1 << 9), // 0x200
	GRAVITY_PS = (1 << 10), // 0x400
	LIGHTING_ON_PS = (1 << 11), // 0x800
	PARTICLE_EMITTER_PS = (1 << 12), // 0x1000
	UNNUSED2_PS = (1 << 13),
	HIDDEN_PS = (1 << 14),
	SCRIPTED_COLLISION_PS = (1 << 15),
	HAS_PHYSICS_BSP_PS = (1 << 16), // 0x10000
	INELASTIC_PS = (1 << 17),
	HAS_DEFAULT_ANIM_PS = (1 << 18),
	HAS_DEFAULT_SCRIPT_PS = (1 << 19),
	CLOAKED_PS = (1 << 20), // 0x100000
	REPORT_COLLISIONS_AS_ENVIRONMENT_PS = (1 << 21),
	EDGE_SLIDE_PS = (1 << 22),
	SLEDDING_PS = (1 << 23),
	FROZEN_PS = (1 << 24)
};

enum BitfieldIndex {
	BF_OPENABLE = (1 << 0), // 1
	BF_INSCRIBABLE = (1 << 1), // 2
	BF_STUCK = (1 << 2), // 4
	BF_PLAYER = (1 << 3), // 8
	BF_ATTACKABLE = (1 << 4), // 0x10
	BF_PLAYER_KILLER = (1 << 5),
	BF_HIDDEN_ADMIN = (1 << 6),
	BF_UI_HIDDEN = (1 << 7),
	BF_BOOK = (1 << 8), // 0x100
	BF_VENDOR = (1 << 9),
	BF_PKSWITCH = (1 << 10),
	BF_NPKSWITCH = (1 << 11),
	BF_DOOR = (1 << 12), // 0x1000
	BF_CORPSE = (1 << 13),
	BF_LIFESTONE = (1 << 14), // 0x4000
	BF_FOOD = (1 << 15),
	BF_HEALER = (1 << 16), // 0x10000
	BF_LOCKPICK = (1 << 17),
	BF_PORTAL = (1 << 18),
	// NOTE: Skip 1
	BF_ADMIN = (1 << 20), // 0x100000
	BF_FREE_PKSTATUS = (1 << 21),
	BF_IMMUNE_CELL_RESTRICTIONS = (1 << 22),
	BF_REQUIRES_PACKSLOT = (1 << 23),
	BF_RETAINED = (1 << 24), // 0x1000000
	BF_PKLITE_PKSTATUS = (1 << 25),
	BF_INCLUDES_SECOND_HEADER = (1 << 26),
	BF_BINDSTONE = (1 << 27),
	BF_VOLATILE_RARE = (1 << 28), // 0x10000000
	BF_WIELD_ON_USE = (1 << 29),
	BF_WIELD_LEFT = (1 << 30),
};

enum PhysicsDescInfo {
	CSetup = (1 << 0), // 1
	MTABLE = (1 << 1), // 2
	VELOCITY = (1 << 2), // 4
	ACCELERATION = (1 << 3), // 8
	OMEGA = (1 << 4), // 0x10
	PARENT = (1 << 5), // 0x20
	CHILDREN = (1 << 6), // 0x40
	OBJSCALE = (1 << 7), // 0x80
	FRICTION = (1 << 8), // 0x100
	ELASTICITY = (1 << 9), // 0x200
	TIMESTAMPS = (1 << 10), // 0x400
	STABLE = (1 << 11), // 0x800
	PETABLE = (1 << 12), // 0x1000
	DEFAULT_SCRIPT = (1 << 13), // 0x2000
	DEFAULT_SCRIPT_INTENSITY = (1 << 14), // 0x4000
	POSITION = (1 << 15), // 0x8000
	MOVEMENT = (1 << 16), // 0x10000
	ANIMFRAME_ID = (1 << 17), // 0x20000
	TRANSLUCENCY = (1 << 18) // 0x40000
};

enum PublicWeenieDescPackHeader {
	PWD_Packed_None = 0,
	PWD_Packed_PluralName = (1 << 0), // 1
	PWD_Packed_ItemsCapacity = (1 << 1), // 2
	PWD_Packed_ContainersCapacity = (1 << 2), // 4
	PWD_Packed_Value = (1 << 3), // 8
	PWD_Packed_Useability = (1 << 4), // 0x10
	PWD_Packed_UseRadius = (1 << 5), // 0x20
	PWD_Packed_Monarch = (1 << 6), // 0x40
	PWD_Packed_UIEffects = (1 << 7), // 0x80
	PWD_Packed_AmmoType = (1 << 8),  // 0x100
	PWD_Packed_CombatUse = (1 << 9), // 0x200
	PWD_Packed_Structure = (1 << 10), // 0x400
	PWD_Packed_MaxStructure = (1 << 11), // 0x800
	PWD_Packed_StackSize = (1 << 12),  // 0x1000
	PWD_Packed_MaxStackSize = (1 << 13), // 0x2000
	PWD_Packed_ContainerID = (1 << 14), // 0x4000
	PWD_Packed_WielderID = (1 << 15), // 0x8000
	PWD_Packed_ValidLocations = (1 << 16),  // 0x10000
	PWD_Packed_Location = (1 << 17), // 0x20000
	PWD_Packed_Priority = (1 << 18), // 0x40000
	PWD_Packed_TargetType = (1 << 19), // 0x80000
	PWD_Packed_BlipColor = (1 << 20),  // 0x100000
	PWD_Packed_Burden = (1 << 21), // 0x200000  // NOTE: May be PWD_Packed_VendorClassID
	PWD_Packed_SpellID = (1 << 22), // 0x400000
	PWD_Packed_RadarEnum = (1 << 23), // 0x800000 // NOTE: May be PWD_Packed_RadarDistance
	PWD_Packed_Workmanship = (1 << 24), // 0x1000000
	PWD_Packed_HouseOwner = (1 << 25), // 0x2000000
	PWD_Packed_HouseRestrictions = (1 << 26), // 0x4000000
	PWD_Packed_PScript = (1 << 27), // 0x8000000
	PWD_Packed_HookType = (1 << 28), // 0x10000000
	PWD_Packed_HookItemTypes = (1 << 29), // 0x20000000
	PWD_Packed_IconOverlay = (1 << 30), // 0x40000000
	PWD_Packed_MaterialType = (1 << 31) // 0x80000000
};

enum PublicWeenieDescPackHeader2 {
	PWD2_Packed_None = 0,
	PWD2_Packed_IconUnderlay = (1 << 0),
	PWD2_Packed_CooldownID = (1 << 1),
	PWD2_Packed_CooldownDuration = (1 << 2),
	PWD2_Packed_PetOwner = (1 << 3),
};


enum PScriptType {
	PS_Invalid,
	PS_Test1,
	PS_Test2,
	PS_Test3,
	PS_Launch,
	PS_Explode,
	PS_AttribUpRed,
	PS_AttribDownRed,
	PS_AttribUpOrange,
	PS_AttribDownOrange,
	PS_AttribUpYellow,
	PS_AttribDownYellow,
	PS_AttribUpGreen,
	PS_AttribDownGreen,
	PS_AttribUpBlue,
	PS_AttribDownBlue,
	PS_AttribUpPurple,
	PS_AttribDownPurple,
	PS_SkillUpRed,
	PS_SkillDownRed,
	PS_SkillUpOrange,
	PS_SkillDownOrange,
	PS_SkillUpYellow,
	PS_SkillDownYellow,
	PS_SkillUpGreen,
	PS_SkillDownGreen,
	PS_SkillUpBlue,
	PS_SkillDownBlue,
	PS_SkillUpPurple,
	PS_SkillDownPurple,
	PS_SkillDownBlack,
	PS_HealthUpRed,
	PS_HealthDownRed,
	PS_HealthUpBlue,
	PS_HealthDownBlue,
	PS_HealthUpYellow,
	PS_HealthDownYellow,
	PS_RegenUpRed,
	PS_RegenDownREd,
	PS_RegenUpBlue,
	PS_RegenDownBlue,
	PS_RegenUpYellow,
	PS_RegenDownYellow,
	PS_ShieldUpRed,
	PS_ShieldDownRed,
	PS_ShieldUpOrange,
	PS_ShieldDownOrange,
	PS_ShieldUpYellow,
	PS_ShieldDownYellow,
	PS_ShieldUpGreen,
	PS_ShieldDownGreen,
	PS_ShieldUpBlue,
	PS_ShieldDownBlue,
	PS_ShieldUpPurple,
	PS_ShieldDownPurple,
	PS_ShieldUpGrey,
	PS_ShieldDownGrey,
	PS_EnchantUpRed,
	PS_EnchantDownRed,
	PS_EnchantUpOrange,
	PS_EnchantDownOrange,
	PS_EnchantUpYellow,
	PS_EnchantDownYellow,
	PS_EnchantUpGreen,
	PS_EnchantDownGreen,
	PS_EnchantUpBlue,
	PS_EnchantDownBlue,
	PS_EnchantUpPurple,
	PS_EnchantDownPurple,
	PS_VitaeUpWhite,
	PS_VitaeDownBlack,
	PS_VisionUpWhite,
	PS_VisionDownBlack,
	PS_SwapHealth_Red_To_Yellow,
	PS_SwapHealth_Red_To_Blue,
	PS_SwapHealth_Yellow_To_Red,
	PS_SwapHealth_Yellow_To_Blue,
	PS_SwapHealth_Blue_To_Red,
	PS_SwapHealth_Blue_To_Yellow,
	PS_TransUpWhite,
	PS_TransDownBlack,
	PS_Fizzle,
	PS_PortalEntry,
	PS_PortalExit,
	PS_BreatheFlame,
	PS_BreatheFrost,
	PS_BreatheAcid,
	PS_BreatheLightning,
	PS_Create,
	PS_Destroy,
	PS_ProjectileCollision,
	PS_SplatterLowLeftBack,
	PS_SplatterLowLeftFront,
	PS_SplatterLowRightBack,
	PS_SplatterLowRightFront,
	PS_SplatterMidLeftBack,
	PS_SplatterMidLeftFront,
	PS_SplatterMidRightBack,
	PS_SplatterMidRightFront,
	PS_SplatterUpLeftBack,
	PS_SplatterUpLeftFront,
	PS_SplatterUpRightBack,
	PS_SplatterUpRightFront,
	PS_SparkLowLeftBack,
	PS_SparkLowLeftFront,
	PS_SparkLowRightBack,
	PS_SparkLowRightFront,
	PS_SparkMidLeftBack,
	PS_SparkMidLeftFront,
	PS_SparkMidRightBack,
	PS_SparkMidRightFront,
	PS_SparkUpLeftBack,
	PS_SparkUpLeftFront,
	PS_SparkUpRightBack,
	PS_SparkUpRightFront,
	PS_PortalStorm,
	PS_Hide,
	PS_UnHide,
	PS_Hidden,
	PS_DisappearDestroy,
	SpecialState1,
	SpecialState2,
	SpecialState3,
	SpecialState4,
	SpecialState5,
	SpecialState6,
	SpecialState7,
	SpecialState8,
	SpecialState9,
	SpecialState0,
	SpecialStateRed,
	SpecialStateOrange,
	SpecialStateYellow,
	SpecialStateGreen,
	SpecialStateBlue,
	SpecialStatePurple,
	SpecialStateWhite,
	SpecialStateBlack,
	PS_LevelUp,
	PS_EnchantUpGrey,
	PS_EnchantDownGrey,
	PS_WeddingBliss,
	PS_EnchantUpWhite,
	PS_EnchantDownWhite,
	PS_CampingMastery,
	PS_CampingIneptitude,
	PS_DispelLife,
	PS_DispelCreature,
	PS_DispelAll,
	PS_BunnySmite,
	PS_BaelZharonSmite,
	PS_WeddingSteele,
	PS_RestrictionEffectBlue,
	PS_RestrictionEffectGreen,
	PS_RestrictionEffectGold,
	PS_LayingofHands,
	PS_AugmentationUseAttribute,
	PS_AugmentationUseSkill,
	PS_AugmentationUseResistances,
	PS_AugmentationUseOther,
	PS_BlackMadness,
	PS_AetheriaLevelUp,
	PS_AetheriaSurgeDestruction,
	PS_AetheriaSurgeProtection,
	PS_AetheriaSurgeRegeneration,
	PS_AetheriaSurgeAffliction,
	PS_AetheriaSurgeFestering,
	PS_HealthDownVoid,
	PS_RegenDownVoid,
	PS_SkillDownVoid,
	PS_DirtyFightingHealDebuff,
	PS_DirtyFightingAttackDebuff,
	PS_DirtyFightingDefenseDebuff,
	PS_DirtyFightingDamageOverTime,
	NUM_PSCRIPT_TYPES
};

enum AMMO_TYPE {
	AMMO_NONE,
	AMMO_ARROW,
	AMMO_BOLT,
	AMMO_ATLATL,
	AMMO_ARROW_CRYSTAL,
	AMMO_BOLT_CRYSTAL,
	AMMO_ATLATL_CRYSTAL,
	AMMO_ARROW_CHORIZITE,
	AMMO_BOLT_CHORIZITE,
	AMMO_ATLATL_CHORIZITE
};


enum MaterialType {
	Undef_MaterialType,
	Ceramic_MaterialType,
	Porcelain_MaterialType,
	Cloth_MaterialType,
	Linen_MaterialType,
	Satin_MaterialType,
	Silk_MaterialType,
	Velvet_MaterialType,
	Wool_MaterialType,
	Gem_MaterialType,
	Agate_MaterialType,
	Amber_MaterialType,
	Amethyst_MaterialType,
	Aquamarine_MaterialType,
	Azurite_MaterialType,
	Black_Garnet_MaterialType,
	Black_Opal_MaterialType,
	Bloodstone_MaterialType,
	Carnelian_MaterialType,
	Citrine_MaterialType,
	Diamond_MaterialType,
	Emerald_MaterialType,
	Fire_Opal_MaterialType,
	Green_Garnet_MaterialType,
	Green_Jade_MaterialType,
	Hematite_MaterialType,
	Imperial_Topaz_MaterialType,
	Jet_MaterialType,
	Lapis_Lazuli_MaterialType,
	Lavender_Jade_MaterialType,
	Malachite_MaterialType,
	Moonstone_MaterialType,
	Onyx_MaterialType,
	Opal_MaterialType,
	Peridot_MaterialType,
	Red_Garnet_MaterialType,
	Red_Jade_MaterialType,
	Rose_Quartz_MaterialType,
	Ruby_MaterialType,
	Sapphire_MaterialType,
	Smoky_Quartz_MaterialType,
	Sunstone_MaterialType,
	Tiger_Eye_MaterialType,
	Tourmaline_MaterialType,
	Turquoise_MaterialType,
	White_Jade_MaterialType,
	White_Quartz_MaterialType,
	White_Sapphire_MaterialType,
	Yellow_Garnet_MaterialType,
	Yellow_Topaz_MaterialType,
	Zircon_MaterialType,
	Ivory_MaterialType,
	Leather_MaterialType,
	Armoredillo_Hide_MaterialType,
	Gromnie_Hide_MaterialType,
	Reed_Shark_Hide_MaterialType,
	Metal_MaterialType,
	Brass_MaterialType,
	Bronze_MaterialType,
	Copper_MaterialType,
	Gold_MaterialType,
	Iron_MaterialType,
	Pyreal_MaterialType,
	Silver_MaterialType,
	Steel_MaterialType,
	Stone_MaterialType,
	Alabaster_MaterialType,
	Granite_MaterialType,
	Marble_MaterialType,
	Obsidian_MaterialType,
	Sandstone_MaterialType,
	Serpentine_MaterialType,
	Wood_MaterialType,
	Ebony_MaterialType,
	Mahogany_MaterialType,
	Oak_MaterialType,
	Pine_MaterialType,
	Teak_MaterialType,
	Number_MaterialType = Teak_MaterialType,
	NumMaterialTypes_MaterialType = 238
};

