
#ifndef NTFS_INCLUDE_H
#define NTFS_INCLUDE_H
#include "FileSystem.h"
#include "commutil.h"



class CNtfsFileSystem:public CBaseFileSystem
{
public:
	CNtfsFileSystem(IBaseReader *prmReader = NULL);

	~CNtfsFileSystem();
	
	virtual		void	Init();

	
	virtual UINT64	ReadFileContent(CBaseFileObject *prmFileObject,
		UCHAR prmDstBuf[], UINT64 prmByteOff, UINT64 prmByteToRead);

	virtual void	GetDeletedFiles(vector<CBaseFileObject*> &fileArray);

private:

	void	GetMFTRunList();

	UINT32	GetAttrValue(NTFS_ATTRDEF prmAttrTitle,UCHAR prmBuf[],UCHAR *prmAttrValue);

	UINT32	GetAttrFromAttributeList(NTFS_ATTRDEF prmAttrType,UINT32 prmOffset,UCHAR *prmAttrList,UCHAR *prmAttrValue);

	void	GetDataRunList(UCHAR *prmBuf,UINT16 prmRunListOffset,Ntfs_Data_Run **prmList);

	void	FreeRunList(Ntfs_Data_Run	*prmList);

	UINT32	GetExtendMFTAttrValue(UINT64 prmSeqNum,NTFS_ATTRDEF prmAttrType,UCHAR *prmAttrValue);

	UINT64	GetOffsetByMFTRef(UINT64 prmSeqNo);

	void	GetFileFromIndexRoot(UCHAR *prmAttrValue,vector<CBaseFileObject*> *prmFileArray);

	void	GetFileFromAllocIndex(UCHAR *prmAttrValue,vector<CBaseFileObject*> *prmFileArray);
	
	void	ParseFileFromIndex(UCHAR *prmBuf,UINT16 prmOffset,UINT32 prmBufLen,vector<CBaseFileObject*> *prmFileArray);

	CStringUtil	GetFileWin32Name(UINT64 prmOffset);

	BOOL	IsFileExists(UINT64 prmStartSector,vector<CBaseFileObject*> *prmFileArray);
	
	void	GetFileExtent(UCHAR *prmBuf,UINT64 prmMftSector,File_Content_Extent_s **prmFileExtent);

	UINT64	ReadFileContent(UCHAR prmDstBuf[],UINT64 prmByteOff,UINT64 prmByteToRead,
		UINT64 prmFileSize,File_Content_Extent_s *prmFileExtent);

	void  FreeFileExtent(File_Content_Extent_s *prmFileExtent);

	UINT64	GetOffsetByFileName(UINT64 prmParentFileOffset,const CStringUtil	&prmFileName);

	UINT64	GetOffsetFromRootByFileName(UCHAR *prmAttrValue,const CStringUtil &prmFileName);

	UINT64	GetOffsetFromAllocByFileName(UCHAR *prmAttrValue,const CStringUtil &prmFileName);

	UINT64	GetOffsetByFileNameInIndex(UCHAR *prmBuf,UINT16 prmOffset,UINT32 prmBufLen,const CStringUtil &prmFileName);

	UINT64	GetFileSize(UCHAR *prmMFTRecord);

	CStringUtil	GetAccessTime(UCHAR szBuf[]);

	CStringUtil GetCreateTime(UCHAR szBuf[]);

	CStringUtil GetModifyTime(UCHAR szBuf[]);

	CStringUtil FileTimeToString(UINT64 prmFileTime);

	FILE_OBJECT_TYPE	GetFileType(UCHAR *prmMFTRecord);
private:
	UINT64		m_mftStartCluster;
	UINT64		m_clustersPerIndex;
	Ntfs_Data_Run	*m_mftRunList;
};
#endif