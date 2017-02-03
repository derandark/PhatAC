

#pragma once

#pragma pack(push, 1)
struct PaletteRpl
{
	PaletteRpl(DWORD _dwPaletteID, BYTE _bOffset, BYTE _bLength)
	{
		dwPaletteID = _dwPaletteID;
		bOffset = _bOffset;
		bLength = _bLength;
	}

	PaletteRpl()
	{
		dwPaletteID = 0;
		bOffset = 0;
		bLength = 0;
	}

	DWORD dwPaletteID;
	BYTE bOffset;
	BYTE bLength;
};
#pragma pack(pop)
typedef std::list< PaletteRpl >		PaletteRList;

#pragma pack(push, 1)
struct TextureRpl
{
	TextureRpl(BYTE _bIndex, DWORD _dwOriginID, DWORD _dwTextureID)
	{
		bIndex = _bIndex;
		dwOriginID = _dwOriginID;
		dwTextureID = _dwTextureID;
	}
	TextureRpl()
	{
		bIndex = 0;
		dwOriginID = 0;
		dwTextureID = 0;
	}

	BYTE bIndex;
	DWORD dwOriginID;
	DWORD dwTextureID;
};
#pragma pack(pop)
typedef std::list< TextureRpl >		TextureRList;

#pragma pack(push, 1)
struct ModelRpl
{
	ModelRpl(BYTE _bIndex, DWORD _dwModelID)
	{
		bIndex = _bIndex;
		dwModelID = _dwModelID;
	}

	ModelRpl()
	{
		bIndex = 0;
		dwModelID = 0;
	}

	BYTE bIndex;
	DWORD dwModelID;
};
#pragma pack(pop)
typedef std::list<ModelRpl> ModelRList;

struct ModelInfo
{
	ModelInfo()
	{
		bUnknown = 0x11;
		dwBasePalette = 0;
	}
	~ModelInfo()
	{
		ClearInfo();
	}

	void ClearInfo()
	{
		dwBasePalette = 0;

		lPalettes.clear();
		lTextures.clear();
		lModels.clear();
	}

	void MergeData(ModelInfo* pSrc, DWORD dwLayer);
	BinaryWriter *NetData();

	BYTE bUnknown;
	WORD dwBasePalette;

	PaletteRList lPalettes;
	TextureRList lTextures;
	ModelRList lModels;

	inline void SetBasePalette(WORD _dwBasePalette)
	{
		dwBasePalette = _dwBasePalette;
	}
	inline void ReplacePalette(DWORD dwPaletteID, BYTE bOffset, BYTE bLength)
	{
		lPalettes.push_back(PaletteRpl(dwPaletteID, bOffset, bLength));
	}
	inline void ReplaceTexture(BYTE bIndex, DWORD dwOriginID, DWORD dwTextureID)
	{
		lTextures.push_back(TextureRpl(bIndex, dwOriginID, dwTextureID));
	}
	inline void ReplaceModel(BYTE bIndex, DWORD dwModelID)
	{
		lModels.push_back(ModelRpl(bIndex, dwModelID));
	}
};
