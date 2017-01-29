
#pragma once


#ifdef PRE_TOD_DATA_FILES
#define DAT_HEADER_OFFSET 0x12C
#else
#define DAT_HEADER_OFFSET 0x140
#endif

#ifdef PRE_TOD_DATA_FILES
struct DATHeader
{
	DWORD    FileType;        // 0x00 'TB' !
	DWORD    BlockSize;        // 0x04 0x400 for PORTAL : 0x100 for CELL
	DWORD    FileSize;        // 0x08 Should match file size.
	DWORD    Iteration;        // 0x0C Version iteration.
	DWORD    FreeHead;        // 0x10
	DWORD    FreeTail;        // 0x14
	DWORD    FreeCount;        // 0x18
	DWORD    BTree;            // 0x1C BTree offset
	DWORD    Unknown0;        // 0x20
	DWORD    Unknown1;        // 0x24
	DWORD    Unknown2;        // 0x28
};
#else
struct DATHeader
{
	DWORD    FileType;
	DWORD    BlockSize;
	DWORD    FileSize;
	DWORD    Iteration;
	DWORD    Iteration2;
	DWORD    FreeHead;
	DWORD    FreeTail;
	DWORD    FreeCount;
	DWORD    BTree;
	DWORD    Unknown0;
	DWORD    Unknown1;
	DWORD    Unknown2;
	DWORD    Unknown3;
	DWORD    Unknown4;
	DWORD    Unknown5;
	DWORD    Unknown6;
};
#endif

#ifdef PRE_TOD_DATA_FILES
struct BTreeEntry
{
	DWORD ID;
	DWORD BlockHead;
	DWORD Length;
};
#else
struct BTreeEntry
{
	DWORD Unknown00;
	DWORD ID;
	DWORD BlockHead;
	DWORD Length;
	DWORD Timestamp;
	DWORD Unknown14;
};
#endif

struct BTreeData
{
	DWORD        BlockSpacer;
	DWORD        Branches[0x3E];
	DWORD        EntryCount;
	BTreeEntry    Entries[0x3D];
};

class BlockLoader;

class BTreeNode
{
public:
	BTreeNode(BlockLoader *pBlockLoader);
	virtual ~BTreeNode();

	BOOL LoadData(DWORD BlockHead);
	void LoadChildren();
	void LoadChildrenRecursive();

	void SetFileCallback(void(*)(void *, DWORD, BTreeEntry *));
	void SetProgressCallback(void(*)(void *, float));
	void SetCallbackArg(void *);

	BOOL Lookup(DWORD ID, BTreeEntry *pEntry);

	void FindEntryIDsWithinRange(DWORD Min, DWORD Max, float Progress, float ProgressDelta);

protected:

	DWORD GetBranchCount() const;
	BTreeNode *GetBranch(DWORD index);

	// Using this design, you can't run 2 scans at the same time.
	static void(*m_pfnFileCallback)(void *, DWORD, BTreeEntry *);
	static void(*m_pfnProgressCallback)(void *, float);
	static void *m_pCallbackArg;

	BlockLoader *m_pBlockLoader;

	BTreeData m_TreeData;
	BTreeNode* m_Branches[0x3E];

	BOOL m_bLeaf;
};

class BTree : public BTreeNode
{
public:
	BTree(BlockLoader *pBlockLoader);
	virtual ~BTree();

	BOOL Init();
};

class DiskDev
{
public:
	DiskDev();
	~DiskDev();

	BOOL OpenFile(const char* Path, DATHeader *pHeader);
	void CloseFile();

	BOOL SyncRead(void *pBuffer, DWORD dwLength, DWORD dwPosition);
	BOOL SyncWrite(void *pBuffer, DWORD dwLength, DWORD dwPosition);

private:

	HANDLE m_hFile;
};

class BlockLoader
{
public:
	BlockLoader();
	~BlockLoader();

	BOOL Init(const char *Path, DATHeader *pHeader);

	DWORD GetTreeOrigin();

	BOOL LoadData(DWORD HeadBlock, void *pBuffer, DWORD Length);

private:

	DATHeader *m_pHeader;
	DiskDev    m_DiskDev;

};

struct DATEntry
{
	DWORD ID;
	BYTE* Data;
	DWORD Length;
};

class DATDisk
{
public:
	static BOOL OpenDisks(const char *portalPath, const char *cellPath);
	static void CloseDisks();
	static DATDisk *pPortal;
	static DATDisk *pCell;

	DATDisk(const char *Path);
	~DATDisk();

	BOOL Open();
	BOOL GetData(DWORD ID, DATEntry *pEntry);
	BOOL GetDataEx(DWORD BlockHead, void *Data, DWORD Length);
	void FindFileIDsWithinRange(DWORD Min, DWORD Max, void(*FileCallback)(void *, DWORD, BTreeEntry *), void(*ProgressCallback)(void *, float), void *CallbackArg);

private:

	char *m_FilePath;

	DATHeader m_DATHeader;
	BlockLoader m_BlockLoader;
	BTree m_BTree;
};




