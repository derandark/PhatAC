

#pragma once

#pragma pack(push, 1)
struct PaletteRpl
{
	PaletteRpl(WORD _wPaletteID, BYTE _bOffset, BYTE _bLength)
	{
		wPaletteID = _wPaletteID;
		bOffset = _bOffset;
		bLength = _bLength;
	}

	PaletteRpl()
	{
		wPaletteID = 0;
		bOffset = 0;
		bLength = 0;
	}

	WORD wPaletteID;
	BYTE bOffset;
	BYTE bLength;
};
#pragma pack(pop)
typedef std::list< PaletteRpl >		PaletteRList;

#pragma pack(push, 1)
struct TextureRpl
{
	TextureRpl(BYTE _bIndex, WORD _wOriginID, WORD _wTextureID)
	{
		bIndex = _bIndex;
		wOriginID = _wOriginID;
		wTextureID = _wTextureID;
	}
	TextureRpl()
	{
		bIndex = 0;
		wOriginID = 0;
		wTextureID = 0;
	}

	BYTE bIndex;
	WORD wOriginID;
	WORD wTextureID;
};
#pragma pack(pop)
typedef std::list< TextureRpl >		TextureRList;

#pragma pack(push, 1)
struct ModelRpl
{
	ModelRpl(BYTE _bIndex, WORD _wModelID)
	{
		bIndex = _bIndex;
		wModelID = _wModelID;
	}

	ModelRpl()
	{
		bIndex = 0;
		wModelID = 0;
	}

	BYTE bIndex;
	WORD wModelID;
};
#pragma pack(pop)
typedef std::list<ModelRpl> ModelRList;

struct ModelInfo
{
	ModelInfo()
	{
		bUnknown = 0x11;
		wBasePalette = 0;
	}
	~ModelInfo()
	{
		ClearInfo();
	}

	void ClearInfo()
	{
		wBasePalette = 0;

		lPalettes.clear();
		lTextures.clear();
		lModels.clear();
	}

	void MergeData(ModelInfo* pSrc, DWORD dwLayer);
	NetFood *NetData();

	BYTE bUnknown;
	WORD wBasePalette;

	PaletteRList lPalettes;
	TextureRList lTextures;
	ModelRList lModels;

	inline void SetBasePalette(WORD _wBasePalette)
	{
		wBasePalette = _wBasePalette;
	}
	inline void ReplacePalette(WORD wPaletteID, BYTE bOffset, BYTE bLength)
	{
		lPalettes.push_back(PaletteRpl(wPaletteID, bOffset, bLength));
	}
	inline void ReplaceTexture(BYTE bIndex, WORD wOriginID, WORD wTextureID)
	{
		lTextures.push_back(TextureRpl(bIndex, wOriginID, wTextureID));
	}
	inline void ReplaceModel(BYTE bIndex, WORD wModelID)
	{
		lModels.push_back(ModelRpl(bIndex, wModelID));
	}
};
