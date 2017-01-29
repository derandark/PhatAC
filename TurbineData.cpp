

#include "StdAfx.h"
#include "Globals.h"

//
#include "TurbineData.h"
#include "TurbineObject.h"

TurbineData::TurbineData()
{
	m_dwVersion = 0;

	m_mFileInfo.clear();
	m_mObjects.clear();

	m_strPath = csprintf("%s\\Data\\", g_pGlobals->GetGameDirectory());
}

TurbineData::~TurbineData()
{
	CloseFile();
}

void TurbineData::FileFoundCallback(void *This, DWORD dwFileID, BTreeEntry *pEntry)
{
	((TurbineData *)This)->FileFoundCallbackInternal(dwFileID, pEntry);
}

void TurbineData::FileFoundCallbackInternal(DWORD dwFileID, BTreeEntry *pEntry)
{
	FILEINFO info;
	info.dwPosition = pEntry->BlockHead;
	info.dwLength = pEntry->Length;
	m_mFileInfo[dwFileID] = info;
}

void TurbineData::LoadFile(const char* szFile)
{
	OutputConsole("Loading %s.. ", szFile);

	m_strFile = szFile;

	std::string fullpath = m_strPath + "\\" + szFile;
	m_pDATDisk = new DATDisk(fullpath.c_str());

	if (!m_pDATDisk->Open())
	{
		OutputConsole("Error loading file %s!", fullpath.c_str());
	}

	OutputConsole(" mapping.. ");

	m_pDATDisk->FindFileIDsWithinRange(0, (DWORD)-1, FileFoundCallback, NULL, this);

	OutputConsole("done!");

done:
	OutputConsole("\r\n");
}

void TurbineData::CloseFile()
{
	SafeDelete(m_pDATDisk);

	//clean up object pool
	for (OBJECTMAP::iterator i = m_mObjects.begin(); i != m_mObjects.end(); ++i)
		SafeDelete(i->second);

	m_mObjects.clear();
	m_mFileInfo.clear();
}

BOOL TurbineData::FileExists(DWORD dwID)
{
	FILEMAP::iterator i = m_mFileInfo.find(dwID);

	return (i != m_mFileInfo.end()) ? TRUE : FALSE;
}

TURBINEFILE *TurbineData::GetFile(DWORD dwID)
{
	DATEntry entry;
	if (m_pDATDisk->GetData(dwID, &entry))
	{
		return new TurbineFile(dwID, entry.Data, entry.Length);
	}

	return NULL;
}

DWORD TurbineData::GetFileCount()
{
	return (DWORD)m_mFileInfo.size();
}

DWORD TurbineData::GetVersion()
{
	return m_dwVersion;
}

FILEMAP *TurbineData::GetFiles()
{
	return &m_mFileInfo;
}

void TurbineData::Think()
{
}

void TurbineData::InsertObject(DWORD dwID, TURBINEOBJECT* pObject)
{
	m_mObjects[dwID] = pObject;
}

TURBINEOBJECT *TurbineData::FindObject(DWORD dwFileID)
{
	OBJECTMAP::iterator it = m_mObjects.find(dwFileID);

	if (it != m_mObjects.end())
		return it->second;
	else
		return NULL;
}








