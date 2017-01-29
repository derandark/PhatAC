
#pragma once

class TurbineData;
class TurbineFile;
class TurbineObject;

typedef TurbineData	TURBINEDATA;
typedef TurbineFile	TURBINEFILE;
typedef TurbineObject TURBINEOBJECT;

typedef struct FileInfo
{
	DWORD dwPosition;
	DWORD dwLength;
} FILEINFO;

typedef	std::map<DWORD, FILEINFO> FILEMAP;
typedef std::map<DWORD, TURBINEOBJECT *> OBJECTMAP;

#include "TurbineFile.h"

class TurbineData
{
public:
	TurbineData();
	virtual ~TurbineData();

	void Think();

	DWORD GetVersion();
	DWORD GetFileCount();

	FILEMAP *GetFiles();
	TURBINEFILE *GetFile(DWORD dwID);
	TURBINEOBJECT *GetObject(DWORD dwID);

	BOOL FileExists(DWORD dwID);

protected:
	void LoadFile(const char* szFile);
	void CloseFile();

	void InsertObject(DWORD dwID, TURBINEOBJECT* pObject);
	TURBINEOBJECT *FindObject(DWORD dwID);

private:
	std::string	m_strPath;
	std::string	m_strFile;

	static void FileFoundCallback(void *This, DWORD dwFileID, BTreeEntry *pEntry);
	void FileFoundCallbackInternal(DWORD dwFileID, BTreeEntry *pEntry);

	DATDisk *m_pDATDisk;

	DWORD m_dwVersion;

	FILEMAP	m_mFileInfo;
	OBJECTMAP m_mObjects;
};
