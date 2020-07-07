#include "ntfs.h"
#include "commutil.h"
#include <shlwapi.h>

#pragma comment(lib,"shlwapi.lib")

CNtfsFileSystem::CNtfsFileSystem(IBaseReader *prmReader /* = NULL */)
	:CBaseFileSystem(prmReader)
{

}

CNtfsFileSystem::~CNtfsFileSystem()
{
	this->FreeRunList(m_mftRunList);
}

void CNtfsFileSystem::Init()
{
	UCHAR	szBuf[512] = { 0 };
	this->ReadBuf(szBuf, 0, 512);
	//this->m_vm->ReadSector(this->m_startSecotr, 512, szBuf);
	UINT16 *bytesPerSector = (UINT16*)&szBuf[0xB];
	this->SetBytesPerSector(*bytesPerSector);
	UINT8 *sectorsPerCluster = (UINT8*)&szBuf[0xD];
	this->SetSectorsPerCluster(*sectorsPerCluster);

	m_mftStartCluster = *((UINT64*)&szBuf[0x30]);
	m_clustersPerIndex = *((UINT32*)&szBuf[0x44]);
	this->GetMFTRunList();
}

UINT64	CNtfsFileSystem::ReadFileContent(CBaseFileObject *prmFileObject, UCHAR prmDstBuf[], UINT64 prmByteOff, UINT64 prmByteToRead)
{
	UINT64		tmpResult = this->ReadFileContent(prmDstBuf,prmByteOff,prmByteToRead,
		prmFileObject->GetFileSize(), prmFileObject->GetFileExtent());
	return tmpResult;
}

void CNtfsFileSystem::GetDeletedFiles(vector<CBaseFileObject*> &fileArray)
{
	//this->FreeArray(fileArray);
	UINT32	clusterSize = m_sectorsPerCluster * m_bytesPerSector;
	TCHAR	szFileName[MAX_PATH * 2] = { 0 };
	TCHAR	szAnsiName[MAX_PATH * 2] = { 0 };
	UCHAR	szAttrValue[1024] = { 0 };
	UINT32	nameLenOffset = 0;
	UINT32	nameLen = 0;
	UINT16  usnOffset = 0;
	//FileInfo	fileInfo;
	UCHAR	*szBuf = (UCHAR*)malloc(sizeof(UCHAR)*clusterSize);
	if (szBuf == NULL)
	{
		return;
	}
	Ntfs_Data_Run	*p = m_mftRunList;
	while (p != NULL)
	{
		for (int i = 0; i < p->length; i++)
		{
			UINT64	clusterOffset = p->lcn*m_sectorsPerCluster + i*m_sectorsPerCluster;
			this->ReadBuf(szBuf, clusterOffset, clusterSize);
			usnOffset = *(UINT16*)&szBuf[4];
			for (UINT32 j = 0; j < m_sectorsPerCluster; j++)
			{
				memcpy(szBuf + 0x1FE + j*m_bytesPerSector, szBuf + usnOffset + 2 + j * 2, 2);
			}
			for (UINT32 j = 0; j < clusterSize ; j += 1024)
			{
				if (szBuf[j + 0x16] == 0)
				{
#ifdef _DEBUG
					UINT64 seqNo = p->vcn*m_sectorsPerCluster / 2;
					seqNo += i*m_sectorsPerCluster / 2 + j / 1024;
#endif
					//fileInfo.seqNo = p->vcn*m_ntfsType->sectorsPerCluster / 2;
					//fileInfo.seqNo += i*m_ntfsType->sectorsPerCluster / 2 + j / 1024;
					if (!this->GetAttrValue(ATTR_FILE_NAME, szBuf + j, (UCHAR*)szAttrValue))
					{
						continue;
					}
					nameLenOffset = 0x58;
					nameLen = *(UINT8*)(szAttrValue + nameLenOffset);
		
					memset(szFileName, 0, sizeof(szFileName));
					memcpy(szFileName, szAttrValue + nameLenOffset + 2, nameLen << 1);
#ifndef _UNICODE
					::WideCharToMultiByte(CP_THREAD_ACP, 0, (LPCWSTR)szFileName, -1, szAnsiName, MAX_PATH * 2, 0, 0);
#endif
					
					
					File_Content_Extent_s	*fileExtents = NULL;

					this->GetFileExtent(szBuf + j, clusterOffset + j / 512, &fileExtents);

					CBaseFileObject	*fileObject = new CBaseFileObject;
#ifdef _UNICODE
					fileObject->SetFileName(szFileName);
#else
					fileObject->SetFileName(szAnsiName);
#endif
					fileObject->SetFileSize(this->GetFileSize(szBuf+j));
					
					fileObject->SetFileExtent(fileExtents);
					fileObject->SetAccessTime(this->GetAccessTime(szBuf+j));
					fileObject->SetCreateTime(this->GetCreateTime(szBuf+j));
					fileObject->SetModifyTime(this->GetModifyTime(szBuf+j));
					fileArray.push_back(fileObject);
				}
			}
		}
		p = p->next;
	}
	free(szBuf);
}

void CNtfsFileSystem::GetMFTRunList()
{
	UCHAR	tmpBuf[1024] = { 0 };
	UCHAR	tmpAttrValue[1024] = { 0 };
	this->ReadBuf(tmpBuf, m_mftStartCluster*m_sectorsPerCluster,1024);
	UINT16	usnOffset = *(UINT16*)&tmpBuf[4];
	memcpy(tmpBuf + 0x1FE, tmpBuf + usnOffset + 2, 2);
	memcpy(tmpBuf + 0x3FE, tmpBuf + usnOffset + 4, 2);
	if (this->GetAttrValue(ATTR_DATA, tmpBuf, tmpAttrValue))
	{
		UINT16	dataRunOffset = *(UINT16*)&tmpAttrValue[0x20];
		this->GetDataRunList(tmpAttrValue, dataRunOffset, &m_mftRunList);
	}
}

UINT32 CNtfsFileSystem::GetAttrValue(NTFS_ATTRDEF prmAttrTitle,UCHAR prmBuf[],UCHAR *prmAttrValue)
{
	UINT32	fileSign = *(UINT32*)prmBuf;
	if (fileSign != 0x454c4946)
	{
		return 0;
	}
	UINT16 flag = *(UINT16*)&prmBuf[0x16];
	/*if (flag == 0 || flag == 2)
	{
		return 0;
	}*/
	
	UINT32	attrOffset = *(UINT16*)&prmBuf[20];

	UINT32	attrLen = 0;
	while (attrOffset < 1024)
	{
		if (prmBuf[attrOffset] == 0xff)
		{
			return 0;
		}
		attrLen = *(UINT16*)&prmBuf[attrOffset + 4];
		
		if (prmBuf[attrOffset] == prmAttrTitle)
		{
			memcpy(prmAttrValue, prmBuf + attrOffset, attrLen);
			return 1;
		}
		attrOffset += attrLen;
	}
	return 0;
}

UINT32 CNtfsFileSystem::GetAttrFromAttributeList(NTFS_ATTRDEF prmAttrType,UINT32 prmOffset,UCHAR *prmAttrList,UCHAR *prmAttrValue)
{
	UINT32	*tmpAttrLen = (UINT32*)(prmAttrList+4);
	while(prmOffset+4<*tmpAttrLen)
	{
		UINT16	*tmpLen = (UINT16*)(prmAttrList+prmOffset+4); 
		if(prmAttrList[prmOffset]==prmAttrType)
		{
			memcpy(prmAttrValue,prmAttrList+prmOffset,*tmpLen);
			return prmOffset;
		}
		prmOffset += *tmpLen;
	}
	return 0;
}

void CNtfsFileSystem::GetDataRunList(UCHAR *prmBuf,UINT16 prmRunListOffset,Ntfs_Data_Run **prmList)
{
	UINT64 index_alloc_size = 0;
	UINT64 lcn = 0;
	int j;
	UINT16	bufOffset = prmRunListOffset;
	UINT32	temp = 0;
	*prmList = NULL;
	Ntfs_Data_Run *p = NULL;

	
	for (;;)
	{
		index_alloc_size = 0;
		for (j = 0; j < prmBuf[bufOffset] % 16; j++)
		{
			index_alloc_size = index_alloc_size + prmBuf[bufOffset + 1 + j] * (UINT64)pow((long double)256, j);
		}

		
		if (prmBuf[bufOffset + prmBuf[bufOffset] % 16 + prmBuf[bufOffset] / 16] > 127)  
		{
			for (j = 0; j < prmBuf[bufOffset] / 16; j++)
			{

				temp = ~prmBuf[bufOffset + prmBuf[bufOffset] % 16 + 1 + j];
				temp = temp & 255;
				lcn = lcn - temp * (UINT64)pow((long double)256, j);
			}
			lcn = lcn - 1;
		}
		else
		{
			for (j = 0; j < prmBuf[bufOffset] / 16; j++)
			{
				lcn = lcn + prmBuf[bufOffset + prmBuf[bufOffset] % 16 + 1 + j] * (UINT64)pow((long double)256, j);
			}
		}
		Ntfs_Data_Run *datarun = new Ntfs_Data_Run;
		if (*prmList == NULL)
		{
			*prmList = datarun;
		}
		datarun->lcn = lcn;
		datarun->vcn = 0;
		datarun->length = index_alloc_size;

		if (p != NULL)
		{
			datarun->vcn += p->length;
			p->next = datarun;
		}
		p = datarun;

		
		bufOffset = bufOffset + prmBuf[bufOffset] / 16 + prmBuf[bufOffset] % 16 + 1;
		if (0 == prmBuf[bufOffset])
		{
			
			break;
		}
	}
}

void CNtfsFileSystem::FreeRunList(Ntfs_Data_Run *prmList)
{
	Ntfs_Data_Run	*p = prmList;
	Ntfs_Data_Run	*q;
	while (p)
	{
		q = p->next;
		delete p;
		p = q;
	}
}

UINT32	CNtfsFileSystem::GetExtendMFTAttrValue(UINT64 prmSeqNum,NTFS_ATTRDEF prmAttrType,UCHAR *prmAttrValue)
{
	UCHAR	tmpBuf[1024] = {0};
	prmSeqNum = prmSeqNum & MFTREFMASK;
	UINT64	tmpOffset = this->GetOffsetByMFTRef(prmSeqNum);
	this->ReadBuf(tmpBuf,tmpOffset/512,1024);
	UINT16	tmpUsnOffset = *(UINT16*)&tmpBuf[4];
	memcpy(tmpBuf+0x1FE,tmpBuf+tmpUsnOffset+2,2);
	memcpy(tmpBuf+0x3FE,tmpBuf+tmpUsnOffset+4,2);
	if(this->GetAttrValue(prmAttrType,tmpBuf,prmAttrValue))
	{
		return 1;
	}
	return 0;
}

UINT64	CNtfsFileSystem::GetOffsetByMFTRef(UINT64 prmSeqNo)
{
	prmSeqNo = prmSeqNo & MFTREFMASK;
	UINT64	tmpOffset = prmSeqNo<<1;
	tmpOffset = tmpOffset * m_bytesPerSector;
	UINT64	tmpVCN = tmpOffset/m_sectorsPerCluster/m_bytesPerSector;
	Ntfs_Data_Run	*p = m_mftRunList;
	while(p!=NULL)
	{
		if(tmpVCN > p->vcn && tmpVCN < p->vcn+p->length)
		{
			break;
		}
		p = p->next;
	}
	if(p==NULL)
	{
		return 0;
	}
	UINT64 tmpOfs = tmpOffset-p->vcn*m_sectorsPerCluster*m_bytesPerSector;
	return tmpOfs + p->lcn*m_sectorsPerCluster*m_bytesPerSector;
}

void CNtfsFileSystem::GetFileFromIndexRoot(UCHAR *prmAttrValue,vector<CBaseFileObject*> *prmFileArray)
{
	if(prmAttrValue[0] != ATTR_INDEX_ROOT)
	{
		return;
	}
	
	UINT32   tmpAttrLen = *(UINT32*)(prmAttrValue+4);
	
	UINT32	 tmpAttrOffset = *(UINT32*)(prmAttrValue+0x30);
	
	UINT32	tmpTotalSize = *(UINT32*)(prmAttrValue+0x34);
	
	tmpTotalSize -= tmpAttrOffset;
	tmpAttrOffset += 0x30;
	UINT8	tmpFlags = *(UINT8*)(prmAttrValue+0x3C);
	if(tmpFlags==1)
	{
		
		return;
	}
	if(tmpAttrOffset>= tmpAttrLen || tmpAttrOffset+tmpTotalSize > tmpAttrLen)
	{
		return;
	}
	this->ParseFileFromIndex(prmAttrValue,tmpAttrOffset,tmpTotalSize,prmFileArray);
}

void CNtfsFileSystem::GetFileFromAllocIndex(UCHAR *prmAttrValue,vector<CBaseFileObject*> *prmFileArray)
{
	if(prmAttrValue[0]!=ATTR_INDEX_ALLOCATION)
	{
		return;
	}
	UINT32	tmpAttrLen = *(UINT32*)(prmAttrValue+4);
	UINT16	tmpIndexOffset = *(UINT16*)(prmAttrValue+0x20);
	if(tmpIndexOffset>=tmpAttrLen)
	{
		return;
	}
	UINT32		tmpClusterSize = m_sectorsPerCluster * m_bytesPerSector;
	UCHAR		*tmpClusterBuf = (UCHAR*)malloc(tmpClusterSize);
	Ntfs_Data_Run	*tmpDataRunList = NULL;
	this->GetDataRunList(prmAttrValue,tmpIndexOffset,&tmpDataRunList);
	Ntfs_Data_Run *p = tmpDataRunList;
	
	while(p!=NULL)
	{
		
		for(int i=0;i<p->length;i++)
		{
			this->ReadBuf(tmpClusterBuf,p->lcn*m_sectorsPerCluster + i*m_sectorsPerCluster,tmpClusterSize);
			
			UINT16 usnOffset = *(UINT16*)&tmpClusterBuf[4];
			for (UINT32 j = 0; j < m_sectorsPerCluster; j++)
			{
				memcpy(tmpClusterBuf + 0x1FE + j*m_bytesPerSector, tmpClusterBuf + usnOffset + 2 + j * 2, 2);
			}
			UINT32	tmpFirstIndex = *(UINT32*)(tmpClusterBuf+0x18);
			tmpFirstIndex += 0x18;
			UINT32	tmpLastIndex = *(UINT32*)(tmpClusterBuf+0x1C);
			this->ParseFileFromIndex(tmpClusterBuf,tmpFirstIndex,tmpLastIndex,prmFileArray);
		}
		p = p->next;
	}
	this->FreeRunList(tmpDataRunList);
	free(tmpClusterBuf);
}

void CNtfsFileSystem::ParseFileFromIndex(UCHAR *prmBuf,UINT16 prmOffset,UINT32 prmBufLen,vector<CBaseFileObject*> *prmFileArray)
{
	wchar_t		tmpUnicodeFileName[MAX_PATH] = {0};
	char		tmpAnsiName[MAX_PATH*2] = {0};
	
	UINT8		tmpNameSpace = 0;
	UINT8		tmpFileNameLen = 0;
	UINT32	tmpAttrOffset = prmOffset;
	UINT64	tmpMFTNum = 0;

	while(tmpAttrOffset<prmBufLen)
	{
		UINT16 tmpIndexItemLen = *(UINT16*)(prmBuf+tmpAttrOffset+8);
		if(prmBuf[tmpAttrOffset+6]==0 && prmBuf[tmpAttrOffset+7]==0)
		{
			tmpAttrOffset += tmpIndexItemLen;
			continue;
		}
		
		tmpNameSpace = *(UINT8*)(prmBuf+tmpAttrOffset+0x51);
		tmpFileNameLen = *(UINT8*)(prmBuf+tmpAttrOffset + 0x50);
		tmpMFTNum = *(UINT64*)(prmBuf+tmpAttrOffset);
		tmpMFTNum = tmpMFTNum & MFTREFMASK;
		if(tmpMFTNum<=25)
		{
			
			tmpAttrOffset += tmpIndexItemLen;
			continue;
		}
		UINT64	tmpFileOffset = this->GetOffsetByMFTRef(tmpMFTNum);
		
		if(!IsFileExists(tmpFileOffset/m_bytesPerSector,prmFileArray))
		{
			CBaseFileObject	*tmpFileObject = new CBaseFileObject();
			
			if(tmpNameSpace==2)
			{
				
				
				CStringUtil	tmpFileNameStr =  this->GetFileWin32Name(tmpFileOffset);
				tmpFileObject->SetFileName(tmpFileNameStr);
				if(tmpFileNameStr.GetLength()==0)
				{
					
					tmpFileObject->Destroy();
					tmpAttrOffset += tmpIndexItemLen;
					continue;
				}
			}
			else
			{
				memset(tmpUnicodeFileName,0,sizeof(tmpUnicodeFileName));
				memcpy(tmpUnicodeFileName,prmBuf+tmpAttrOffset+0x52,tmpFileNameLen<<1);
#ifdef _UNICODE
				tmpFileObject->SetFileName(tmpUnicodeFileName);
#else
				memset(tmpAnsiName,0,sizeof(tmpAnsiName));
				::WideCharToMultiByte(CP_ACP,0,tmpUnicodeFileName,-1,tmpAnsiName,MAX_PATH*2,0,0);
				tmpFileObject->SetFileName(tmpAnsiName);
#endif
			}
			tmpFileObject->SetFileStartSector(tmpFileOffset/m_bytesPerSector);
			UINT32	tmpFileFlags = *(UINT32*)(prmBuf+tmpAttrOffset+0x48);
			if(tmpFileFlags & 0x10000000)
			{
				tmpFileObject->SetFileType(FILE_OBJECT_TYPE_DIRECTORY);
			}
			else
			{
				tmpFileObject->SetFileType(FILE_OBJECT_TYPE_FILE);
			}
			UINT64	tmpFileSize = *(UINT64*)(prmBuf+tmpAttrOffset+0x40);
			tmpFileObject->SetFileSize(tmpFileSize);
			prmFileArray->push_back(tmpFileObject);
			
		}
		tmpAttrOffset += tmpIndexItemLen;
	}
}

CStringUtil CNtfsFileSystem::GetFileWin32Name(UINT64 prmOffset)
{
	UCHAR		tmpBuf[1024] = {0};
	wchar_t		tmpFileName[MAX_PATH] = {0};
	char		tmpAnsiName[MAX_PATH*2] = {0};
	this->ReadBuf(tmpBuf,prmOffset/m_bytesPerSector,1024);
	UINT16	tmpUsnOffset = *(UINT16*)&tmpBuf[4];
	memcpy(tmpBuf+0x1FE,tmpBuf+tmpUsnOffset+2,2);
	memcpy(tmpBuf+0x3FE,tmpBuf+tmpUsnOffset+4,2);
	UINT32	fileSign = *(UINT32*)tmpBuf;
	if (fileSign != 0x454c4946)
	{
		return FALSE;
	}
	UINT16 flag = *(UINT16*)&tmpBuf[0x16];
	if (flag == 0 || flag == 2)
	{
		return FALSE;
	}
	
	UINT32	attrOffset = *(UINT16*)&tmpBuf[20];

	UINT32	attrLen = 0;
	while (attrOffset < 1024)
	{
		if (tmpBuf[attrOffset] == 0xff)
		{
			return 0;
		}
		attrLen = *(UINT16*)&tmpBuf[attrOffset + 4];
		
		if (tmpBuf[attrOffset] == ATTR_FILE_NAME && tmpBuf[attrOffset+0x59]!=2)
		{
			UINT8	tmpFileLen = tmpBuf[attrOffset + 0x58];
			memset(tmpFileName,0,sizeof(tmpFileName));
			memcpy(tmpFileName,tmpBuf+attrOffset+0x5A,tmpFileLen<<1);
#ifdef _UNICODE
			return tmpFileName;
#else
			::WideCharToMultiByte(CP_ACP,0,tmpFileName,-1,tmpAnsiName,MAX_PATH*2,0,0);
			return tmpAnsiName;
#endif
			
		}
		attrOffset += attrLen;
	}
	return TEXT("");
}

BOOL CNtfsFileSystem::IsFileExists(UINT64 prmStartSector,vector<CBaseFileObject*> *prmFileArray)
{
	for(size_t i=0;i<prmFileArray->size();i++)
	{
		if(prmStartSector == prmFileArray->at(i)->GetFileStartSector())
		{
			return TRUE;
		}
	}
	return FALSE;
}

void CNtfsFileSystem::GetFileExtent(UCHAR *prmBuf,UINT64 prmMftSector,File_Content_Extent_s **prmFileExtent)
{
	UCHAR	szAttrValue[1024] = { 0 };
	UCHAR	szAttrList[1024] = {0};
	UCHAR	szExtentAttrValue[1024] = {0};
	UINT32 readBytes = 0;
	Ntfs_Data_Run		*runList = NULL;
	Ntfs_Data_Run		*q = NULL;
	UINT32		attrLen = 0;
	UINT32		attrOff = 0;
	UINT32		result = 0;


	if(this->GetAttrValue(ATTR_ATTRIBUTE_LIST,prmBuf,szAttrList))
	{
		UINT32	tmpOffset = 0x18;
		while(tmpOffset=this->GetAttrFromAttributeList(ATTR_DATA,tmpOffset,szAttrList,szAttrValue))
		{
			UINT16	tmpLen = *(UINT16*)(szAttrValue+4);
			UINT64		seqNum =   *(UINT64*)(szAttrValue + 0x10);
			if(this->GetExtendMFTAttrValue(seqNum,ATTR_DATA,szExtentAttrValue))
			{
				UINT16	runlistOffset = *(UINT16*)&szExtentAttrValue[0x20];
				Ntfs_Data_Run *p = NULL;
				this->GetDataRunList(szExtentAttrValue,runlistOffset,&p);
				if (runList == NULL)
				{
					runList = p;
					q = p;
				}
				else
				{
					while (q->next)
					{
						q = q->next;
					}
					q->next = p;
				}
			}
			tmpOffset += tmpLen;
		}
	}
	if(runList!=NULL)
	{
		goto END;
	}
	
	if (this->GetAttrValue(ATTR_DATA, prmBuf, szAttrValue))
	{
		attrLen = *(UINT32*)&szAttrValue[4];
		if (szAttrValue[8] == 0)
		{
			UINT32 fileLength = *(UINT32*)&szAttrValue[0x10];
			*prmFileExtent = new File_Content_Extent_s;
			(*prmFileExtent)->startSector = prmMftSector;
			(*prmFileExtent)->totalSector = 2;
			(*prmFileExtent)->isPersist = 1;
		}
		else 
		{
			UINT16	runlistOffset = *(UINT16*)&szAttrValue[0x20];
			this->GetDataRunList(szAttrValue, runlistOffset, &runList);
			goto END;
		}
	}
END:
	Ntfs_Data_Run *p = runList;
	File_Content_Extent_s *tmpExtent = *prmFileExtent;
	while (p != NULL && p->lcn != 0 && p->length != 0 && p->length < m_totalSector / m_sectorsPerCluster)
	{
		File_Content_Extent_s *t = new File_Content_Extent_s;
		t->totalSector = p->length*m_sectorsPerCluster;
		t->startSector = p->lcn*m_sectorsPerCluster;
		
		if (tmpExtent == NULL)
		{
			tmpExtent = t;
			*prmFileExtent = t;
		}
		else
		{
			tmpExtent->next = t;
			tmpExtent = t;
		}
		p = p->next;
	}
	this->FreeRunList(runList);
}

UINT64	CNtfsFileSystem::ReadFileContent(UCHAR prmDstBuf[],UINT64 prmByteOff,UINT64 prmByteToRead, 
	UINT64 prmFileSize,File_Content_Extent_s *prmFileExtent)
{
	UINT64		tmpResult = 0;
	UINT64		tmpByteRead = 0;
	if (prmByteOff >= prmFileSize)
	{
		return 0;
	}
	if(prmByteOff+prmByteToRead > prmFileSize)
	{
		prmByteToRead = prmFileSize - prmByteOff;
	}
	
	
	if(prmFileExtent->isPersist)
	{
		UCHAR	szBuf[1024] = { 0 };
		UCHAR	szAttr[1024] = { 0 };
		this->ReadBuf(szBuf, prmFileExtent->startSector, 1024);
		UINT16	usnOffset = *(UINT16*)&szBuf[4];
		memcpy(szBuf + 0x1FE, szBuf + usnOffset + 2, 2);
		memcpy(szBuf + 0x3FE, szBuf + usnOffset + 4, 2);
		if (this->GetAttrValue(ATTR_DATA, szBuf, szAttr))
		{
			memcpy(prmDstBuf, szAttr + 0x18 + (UINT32)prmByteOff, (UINT32)prmByteToRead);
			return (UINT32)prmByteToRead;
		}
		return 0;
	}

	File_Content_Extent_s *p = prmFileExtent;
	
	while(p && prmByteOff >= p->totalSector*m_bytesPerSector)
	{
		prmByteOff -= p->totalSector*m_bytesPerSector;
		p = p->next;
	}

	if(p==NULL)
	{
		return 0;
	}

	
	if(prmByteOff %512 != 0)
	{
		UCHAR		tmpBuf[512] = { 0 };
		tmpByteRead = 512 - prmByteOff%512;
		if (prmByteToRead <= tmpByteRead)
		{
			tmpByteRead = prmByteToRead;
		}
		
		this->ReadBuf(tmpBuf,p->startSector + prmByteOff/512,512);
		memcpy(prmDstBuf,tmpBuf+prmByteOff%512,(UINT32)tmpByteRead);
		tmpResult = tmpByteRead;
		if (tmpByteRead == prmByteToRead)
		{
			return tmpResult;
		}
		
		prmByteOff += tmpByteRead;
		
		if(prmByteOff >= p->totalSector*m_bytesPerSector)
		{
			prmByteOff = prmByteOff - p->totalSector*m_bytesPerSector;
			p = p->next;
			if(p==NULL)
			{
				return tmpResult;
			}
		}
	}

	
	UINT64	tmpRunListRemainBytes = p->totalSector * m_bytesPerSector - prmByteOff;
	
	UINT64	tmpOff = p->startSector + prmByteOff/512;
	while(p && (tmpRunListRemainBytes < (prmByteToRead-tmpResult)) )
	{
		
		tmpByteRead = this->ReadBuf(prmDstBuf+tmpResult,tmpOff,tmpRunListRemainBytes);
		tmpResult += tmpByteRead;
		if (tmpByteRead != tmpRunListRemainBytes)
		{
			return tmpResult;
		}
		p = p->next;
		if (p == NULL)
		{
			return tmpResult;
		}
		tmpRunListRemainBytes = p->totalSector*m_bytesPerSector;
		tmpOff = p->startSector;
	}

	
	if(prmByteToRead!=tmpResult)
	{
		tmpByteRead  = this->ReadBuf(prmDstBuf + tmpResult, tmpOff, prmByteToRead-tmpResult);
		tmpResult += tmpByteRead;
	}
	return tmpResult;
#if 0
	
	UINT64 runListRemainSize = p->totalSector*m_bytesPerSector - prmByteOff;
	UINT64	fileOffset = p->startSector*m_bytesPerSector+prmByteOff;
	while (p && (runListRemainSize < (prmByteToRead - tmpResult)))
	{
		tmpByteRead = this->ReadBuf(prmDstBuf + tmpResult, fileOffset / m_bytesPerSector, (UINT32)runListRemainSize);
		tmpResult += tmpByteRead;
		if (tmpByteRead != runListRemainSize)
		{
			goto END;
		}
		p = p->next;
		if(p==NULL)
		{
			goto END;
		}
		runListRemainSize = p->totalSector*m_bytesPerSector;
		fileOffset = p->startSector*m_bytesPerSector;

		
	}
	
	if (p == NULL)
	{
		goto END;
	}

	
	if(prmByteToRead!=tmpResult)
	{
		tmpByteRead  = this->ReadBuf(prmDstBuf + tmpResult, fileOffset  / m_bytesPerSector, prmByteToRead-tmpResult);
		tmpResult += tmpByteRead;
	}
END:
	return tmpResult;
#endif
}

void CNtfsFileSystem::FreeFileExtent(File_Content_Extent_s *prmFileExtent)
{
	File_Content_Extent_s *p = prmFileExtent;
	File_Content_Extent_s *q = NULL;
	while (p != NULL)
	{
		q = p;
		p = p->next;
		delete q;
	}
}

UINT64	CNtfsFileSystem::GetOffsetByFileName(UINT64 prmParentFileOffset,const CStringUtil &prmFileName)
{
	UCHAR		tmpBuf[1024] = {0};
	UCHAR		tmpAttrValue[1024] = {0};
	UCHAR		tmpAttrList[1024] = {0};
	UINT64		tmpFileOffset = 0;
	if(prmParentFileOffset==0)
	{
		return NULL;
	}
	this->ReadBuf(tmpBuf,prmParentFileOffset/m_bytesPerSector,1024);
	UINT16	tmpUsnOffset = *(UINT16*)&tmpBuf[4];
	memcpy(tmpBuf+0x1FE,tmpBuf+tmpUsnOffset+2,2);
	memcpy(tmpBuf+0x3FE,tmpBuf+tmpUsnOffset+4,2);
	
	if(this->GetAttrValue(ATTR_ATTRIBUTE_LIST,tmpBuf,tmpAttrList))
	{
		UCHAR		tmpExtentMFTValue[1024] = { 0 };
		UINT32	tmpOffset = 0x18;
		while(tmpOffset=this->GetAttrFromAttributeList(ATTR_INDEX_ROOT,tmpOffset,tmpAttrList,tmpAttrValue))
		{
			UINT16	*tmpLen = (UINT16*)(tmpAttrValue+4);
			tmpOffset += *tmpLen;
			UINT64	seqNum = *(UINT64*)&tmpAttrValue[0x10];
			seqNum = seqNum & MFTREFMASK;
			this->GetExtendMFTAttrValue(seqNum,ATTR_INDEX_ROOT,tmpExtentMFTValue);
			tmpFileOffset = this->GetOffsetFromRootByFileName(tmpExtentMFTValue,prmFileName);
			if(tmpFileOffset!=0)
			{
				return tmpFileOffset;
			}
		}

		tmpOffset = 0x18;
		while(tmpOffset=this->GetAttrFromAttributeList(ATTR_INDEX_ALLOCATION,tmpOffset,tmpAttrList,tmpAttrValue))
		{
			UINT16	*tmpLen = (UINT16*)(tmpAttrValue+4);
			tmpOffset += *tmpLen;
			UINT64	seqNum = *(UINT64*)&tmpAttrValue[0x10];
			seqNum = seqNum & MFTREFMASK;
			this->GetExtendMFTAttrValue(seqNum,ATTR_INDEX_ALLOCATION,tmpExtentMFTValue);
			tmpFileOffset = this->GetOffsetFromAllocByFileName(tmpExtentMFTValue,prmFileName);
			if(tmpFileOffset!=0)
			{
				return tmpFileOffset;
			}
		}
	}
	if(this->GetAttrValue(ATTR_INDEX_ROOT,tmpBuf,tmpAttrValue))
	{
		tmpFileOffset = this->GetOffsetFromRootByFileName(tmpAttrValue,prmFileName);
		if(tmpFileOffset!=0)
		{
			return tmpFileOffset;
		}
	}
	if(this->GetAttrValue(ATTR_INDEX_ALLOCATION,tmpBuf,tmpAttrValue))
	{
		tmpFileOffset = this->GetOffsetFromAllocByFileName(tmpAttrValue,prmFileName);
	}
	return tmpFileOffset;
}

UINT64  CNtfsFileSystem::GetOffsetFromRootByFileName(UCHAR *prmAttrValue,const CStringUtil &prmFileName)
{
	if(prmAttrValue[0] != ATTR_INDEX_ROOT)
	{
		return 0;
	}
	
	UINT32   tmpAttrLen = *(UINT32*)(prmAttrValue+4);
	
	UINT32	 tmpAttrOffset = *(UINT32*)(prmAttrValue+0x30);
	
	UINT32	tmpTotalSize = *(UINT32*)(prmAttrValue+0x34);
	
	tmpTotalSize -= tmpAttrOffset;
	tmpAttrOffset += 0x30;
	UINT8	tmpFlags = *(UINT8*)(prmAttrValue+0x3C);
	if(tmpFlags==1)
	{
		
		return 0;
	}
	if(tmpAttrOffset>= tmpAttrLen || tmpAttrOffset+tmpTotalSize > tmpAttrLen)
	{
		return 0;
	}
	return this->GetOffsetByFileNameInIndex(prmAttrValue,tmpAttrOffset,tmpTotalSize,prmFileName);
}

UINT64	CNtfsFileSystem::GetOffsetFromAllocByFileName(UCHAR *prmAttrValue,const CStringUtil &prmFileName)
{
	UINT64	tmpFileOffset = 0;
	if(prmAttrValue[0]!=ATTR_INDEX_ALLOCATION)
	{
		return 0;
	}
	UINT32	tmpAttrLen = *(UINT32*)(prmAttrValue+4);
	UINT16	tmpIndexOffset = *(UINT16*)(prmAttrValue+0x20);
	if(tmpIndexOffset>=tmpAttrLen)
	{
		return 0;
	}
	UINT32		tmpClusterSize = m_sectorsPerCluster * m_bytesPerSector;
	UCHAR		*tmpClusterBuf = (UCHAR*)malloc(tmpClusterSize);
	Ntfs_Data_Run	*tmpDataRunList = NULL;
	this->GetDataRunList(prmAttrValue,tmpIndexOffset,&tmpDataRunList);
	Ntfs_Data_Run *p = tmpDataRunList;

	while(p!=NULL)
	{
		
		for(int i=0;i<p->length;i++)
		{
			this->ReadBuf(tmpClusterBuf,p->lcn*m_sectorsPerCluster + i*m_sectorsPerCluster,tmpClusterSize);
			
			UINT16 usnOffset = *(UINT16*)&tmpClusterBuf[4];
			for (UINT32 j = 0; j < m_sectorsPerCluster; j++)
			{
				memcpy(tmpClusterBuf + 0x1FE + j*m_bytesPerSector, tmpClusterBuf + usnOffset + 2 + j * 2, 2);
			}
			UINT32	tmpFirstIndex = *(UINT32*)(tmpClusterBuf+0x18);
			tmpFirstIndex += 0x18;
			UINT32	tmpLastIndex = *(UINT32*)(tmpClusterBuf+0x1C);
			tmpFileOffset = this->GetOffsetByFileNameInIndex(tmpClusterBuf,tmpFirstIndex,tmpLastIndex,prmFileName);
			if(tmpFileOffset!=0)
			{
				break;
			}
		}
		if(tmpFileOffset!=0)
		{
			break;
		}
		p = p->next;
	}
	this->FreeRunList(tmpDataRunList);
	free(tmpClusterBuf);
	return tmpFileOffset;
}

UINT64	CNtfsFileSystem::GetOffsetByFileNameInIndex(UCHAR *prmBuf,UINT16 prmOffset,UINT32 prmBufLen,const CStringUtil &prmFileName)
{
	UINT64		tmpResult = 0;
	wchar_t		tmpUnicodeFileName[MAX_PATH] = {0};
	char		tmpAnsiName[MAX_PATH*2] = {0};
	CStringUtil	tmpFileName;

	UINT8		tmpNameSpace = 0;
	UINT8		tmpFileNameLen = 0;
	UINT32	tmpAttrOffset = prmOffset;
	UINT64	tmpMFTNum = 0;

	while(tmpAttrOffset<prmBufLen)
	{
		UINT16 tmpIndexItemLen = *(UINT16*)(prmBuf+tmpAttrOffset+8);
		if(prmBuf[tmpAttrOffset+6]==0 && prmBuf[tmpAttrOffset+7]==0)
		{
			tmpAttrOffset += tmpIndexItemLen;
			continue;
		}
		
		tmpNameSpace = *(UINT8*)(prmBuf+tmpAttrOffset+0x51);
		tmpFileNameLen = *(UINT8*)(prmBuf+tmpAttrOffset + 0x50);
		tmpMFTNum = *(UINT64*)(prmBuf+tmpAttrOffset);
		tmpMFTNum = tmpMFTNum & MFTREFMASK;
		if(tmpMFTNum<=25)
		{
			
			tmpAttrOffset += tmpIndexItemLen;
			continue;
		}
		UINT64	tmpFileOffset = this->GetOffsetByMFTRef(tmpMFTNum);
		
		if(tmpNameSpace==2)
		{
			tmpFileName = this->GetFileWin32Name(tmpFileOffset);
		}
		else
		{
			memset(tmpUnicodeFileName,0,sizeof(tmpUnicodeFileName));
			memcpy(tmpUnicodeFileName,prmBuf+tmpAttrOffset+0x52,tmpFileNameLen<<1);
#ifdef _UNICODE
			
			tmpFileName = tmpUnicodeFileName;
#else
			memset(tmpAnsiName,0,sizeof(tmpAnsiName));
			::WideCharToMultiByte(CP_ACP,0,tmpUnicodeFileName,-1,tmpAnsiName,MAX_PATH*2,0,0);
			tmpFileName = tmpAnsiName;
#endif
		}
		if(tmpFileName.CompareNoCase(prmFileName)==0)
		{
			tmpResult = tmpFileOffset;
			break;
		}
		tmpAttrOffset += tmpIndexItemLen;
	}
	return tmpResult; 
}

UINT64	CNtfsFileSystem::GetFileSize(UCHAR *prmMFTRecord)
{
	UCHAR	szAttrValue[1024] = { 0 };
	UCHAR	szAttrList[1024] = {0};
	UCHAR	szExtendMFTAttrValue[1024] = {0};
	UINT64	tmpResult = 0;

	
	if(this->GetAttrValue(ATTR_ATTRIBUTE_LIST,prmMFTRecord,szAttrList))
	{
		UINT32	tmpOffset = 0x18;
		while(tmpOffset=this->GetAttrFromAttributeList(ATTR_DATA,tmpOffset,szAttrList,szAttrValue))
		{
			UINT16	*tmpLen = (UINT16*)(szAttrValue+4);
			UINT64		seqNum =   *(UINT64*)(szAttrValue + 0x10);
			if(this->GetExtendMFTAttrValue(seqNum,ATTR_DATA,szExtendMFTAttrValue))
			{
				tmpResult += *(UINT64*)&szExtendMFTAttrValue[0x30];
			}
		}
	}
	if(tmpResult != 0)
	{
		return tmpResult;
	}

	if (this->GetAttrValue(ATTR_DATA, prmMFTRecord, szAttrValue))
	{
		if (szAttrValue[8] == 0)
		{
			tmpResult = *(UINT32*)&szAttrValue[0x10];
		}
		else
		{
			tmpResult = *(UINT64*)&szAttrValue[0x30];
		}
		return tmpResult;
	}
	return 0;
}

CStringUtil	CNtfsFileSystem::GetAccessTime(UCHAR szBuf[])
{
	UCHAR	szAttrValue[1024] = { 0 };
	if (this->GetAttrValue(ATTR_STANDARD, szBuf, szAttrValue))
	{
		UINT64 accessTime = *(UINT64*)(szAttrValue + 0x30);
		return this->FileTimeToString(accessTime);
	}
	return TEXT("");
}

CStringUtil CNtfsFileSystem::GetModifyTime(UCHAR szBuf[])
{
	UCHAR	szAttrValue[1024] = { 0 };
	if (this->GetAttrValue(ATTR_STANDARD, szBuf, szAttrValue))
	{
		UINT64 modifyTime = *(UINT64*)(szAttrValue + 0x20);
		return this->FileTimeToString(modifyTime);
	}
	return TEXT("");
}

CStringUtil CNtfsFileSystem::GetCreateTime(UCHAR szBuf[])
{
	UCHAR	szAttrValue[1024] = { 0 };
	if (this->GetAttrValue(ATTR_STANDARD, szBuf, szAttrValue))
	{
		//UINT16 offset = *(UINT16*)(szAttrValue + 0x0A);
		UINT64 createTime = *(UINT64*)(szAttrValue + 0x18);
		return this->FileTimeToString(createTime);
	}
	return TEXT("");
}

CStringUtil	CNtfsFileSystem::FileTimeToString(UINT64 prmFileTime)
{
	//TCHAR	szBuf[128] = { 0 };
	FILETIME fileTime;
	SYSTEMTIME	systemTime;
	TIME_ZONE_INFORMATION tz;
	fileTime.dwLowDateTime = prmFileTime & 0xFFFFFFFF;
	fileTime.dwHighDateTime = (prmFileTime & 0xFFFFFFFF00000000) >> 32;
	::FileTimeToSystemTime(&fileTime, &systemTime);
	::GetTimeZoneInformation(&tz);
	long lTime = systemTime.wHour * 60 + systemTime.wMinute;
	lTime -= tz.Bias;
	systemTime.wHour = (WORD)lTime / 60;
	systemTime.wMinute = lTime % 60;
	CStringUtil	tmpResult;
	tmpResult.Format(TEXT("%4d-%02d-%02d %02d:%02d:%02d"), systemTime.wYear, systemTime.wMonth, systemTime.wDay,
		systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
	return tmpResult;
}

FILE_OBJECT_TYPE CNtfsFileSystem::GetFileType(UCHAR *prmMFTRecord)
{
	UINT16	tmpFlag = *(UINT16*)(prmMFTRecord+0x16);
	if(tmpFlag==1)
	{
		return FILE_OBJECT_TYPE_FILE;
	}
	else if(tmpFlag==2)
	{
		return FILE_OBJECT_TYPE_DIRECTORY;
	}
	return FILE_OBJECT_TYPE_UNKNOWN;
}