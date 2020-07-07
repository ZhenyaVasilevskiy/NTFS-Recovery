
#ifndef FILE_MXL_INCLUDE_H
#define FILE_MXL_INCLUDE_H

#include "StringUtil.h"
#include "commutil.h"

using namespace commutil;

enum	FILE_OBJECT_TYPE
{
	FILE_OBJECT_TYPE_FILE,
	FILE_OBJECT_TYPE_DIRECTORY,
	FILE_OBJECT_TYPE_ROOT,
	FILE_OBJECT_TYPE_UNKNOWN
};

class CBaseFileObject
{
public:

	CBaseFileObject();

	void		SetFileStartSector(UINT64  prmSector);

	UINT64		GetFileStartSector();

	void		SetFileName(const CStringUtil &prmFileName);

	const CStringUtil	&GetFileName();

	void		SetFileType(FILE_OBJECT_TYPE prmType);

	FILE_OBJECT_TYPE  GetFileType();

	void		SetFileSize(UINT64 prmFileSize);

	UINT64		GetFileSize();

	void		SetAccessTime(const CStringUtil &accessTime){ m_accessTime = accessTime; }

	CStringUtil	&GetAccessTime(){ return m_accessTime; }

	void		SetModifyTime(const CStringUtil &modifyTime){ m_modifyTime = modifyTime; }

	CStringUtil	&GetModifyTime(){ return m_modifyTime; }

	void		SetCreateTime(const CStringUtil &createTime){ m_createTime = createTime; }

	CStringUtil &GetCreateTime(){ return m_createTime; }

	void		SetFileExtent(File_Content_Extent_s *fileExtent){ m_fileExtent = fileExtent; }

	File_Content_Extent_s *GetFileExtent(){ return m_fileExtent; }

	void		Destroy();
private:
	CStringUtil		m_fileName;
	CStringUtil		m_accessTime;
	CStringUtil		m_modifyTime;
	CStringUtil		m_createTime;
	UINT64			m_startSector;
	UINT64			m_fileSize;
	FILE_OBJECT_TYPE	m_objectType;
	File_Content_Extent_s	*m_fileExtent;
private:

	virtual ~CBaseFileObject();
};

#endif