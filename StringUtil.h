

#ifndef STRING_UTIL_INCLUDE_H
#define STRING_UTIL_INCLUDE_H

#include <windows.h>
#include <tchar.h>
#include <vector>

using namespace std;

namespace	commutil
{
	class CStringUtil
	{
	public:
		
		CStringUtil();
		
		CStringUtil(LPCTSTR prmStr);

	
		CStringUtil(LPCTSTR prmStr,int prmStart,int prmLen);
	
		CStringUtil(const CStringUtil &prmObj);
		
		CStringUtil & operator=(const CStringUtil &prmObj);
		
		~CStringUtil();

		CStringUtil	&Append(LPCTSTR prmStr);

		CStringUtil	&Append(LPCTSTR prmStr, UINT32 prmStartPos, UINT32 prmLen);
#ifdef _UNICODE
		
		CStringUtil	&Append(char *prmStr);
#else
		
		CStringUtil	&Append(wchar_t *prmStr);
#endif
		
		CStringUtil	&Append(const CStringUtil &prmObj);
		
		CStringUtil	&Append(TCHAR prmChar);
		
		CStringUtil	&Append(int prmNum);

		CStringUtil &operator+=(LPCTSTR prmStr);
#ifdef _UNICODE
		CStringUtil	&operator+=(char *prmStr);
#else
		CStringUtil	&operator+=(wchar_t *prmStr);
#endif
		CStringUtil &operator+=(const CStringUtil &prmObj);
		CStringUtil &operator+=(TCHAR prmChar);
		CStringUtil &operator+=(int prmNum);

		CStringUtil operator+(LPCTSTR prmStr);
#ifdef _UNICODE
		CStringUtil	operator+(char *prmStr);
#else
		CStringUtil	operator+(wchar_t *prmStr);
#endif
		CStringUtil operator+(const CStringUtil &prmObj);
		CStringUtil operator+(TCHAR prmChar);
		CStringUtil operator+(int prmNum);

		void	Empty();
	
		BOOL	IsEmpty();
	
		BOOL	operator==(const CStringUtil &prmObj);
	
		BOOL	operator!=(const CStringUtil &prmObj);
	
		BOOL	operator<(const CStringUtil &prmObj);
	
		BOOL	operator<=(const CStringUtil &prmObj);
	
		BOOL	operator>(const CStringUtil &prmObj);
	
		BOOL	operator>=(const CStringUtil &prmObj);

		
		int		CompareNoCase(const TCHAR *prmStr);
		
		int		CompareNoCase(const CStringUtil &prmObj);
		
		int		CompareNChar(const TCHAR *prmStr,size_t prmLen);
		
		int		CompareNChar(const CStringUtil &prmObj,size_t prmLen);
		
		int		CompareNCharNoCase(const TCHAR *prmStr,size_t prmLen);
		
		int		CompareNCharNoCase(const CStringUtil &prmObj,size_t prmLen);

		
		CStringUtil Left(int prmLen);
	
		CStringUtil	Right(int prmLen);
		
		CStringUtil Mid(int prmStart,int prmEnd);

		
		TCHAR	&operator[](int prmIndex);
		
		TCHAR	GetAt(int prmIndex);

		
		LPCTSTR GetString()const {return m_Buf;}
		
		int		GetLength(){return m_BufLen;}

		
		int		FindString(const TCHAR *prmStr);
		
		int		FindString(const CStringUtil &prmObj);
		
		int		RFindString(const TCHAR *prmStr);
		
		int		RFindString(const CStringUtil &prmObj);
		
		TCHAR	*StrChar(const TCHAR prmChar);
		
		TCHAR	*StrrChar(const TCHAR prmChar);

		
		BOOL	StartWith(const TCHAR *prmStr);
		
		BOOL	StartWith(const CStringUtil &prmObj);
		
		BOOL	EndWith(const TCHAR *prmStr);
		
		BOOL	EndWith(const CStringUtil &prmObj);

		BOOL	StartWithNoCase(const TCHAR *prmStr);
		
		BOOL	StartWithNoCase(const CStringUtil &prmObj);
		
		BOOL	EndWithNoCase(const TCHAR *prmStr);
		
		BOOL	EndWithNoCase(const CStringUtil &prmObj);

		CStringUtil	&Trim();

		void	ToUpper();

		void	ToLower();

		void	SplitString(vector<CStringUtil> &prmArray,LPCTSTR prmSplitStr);

		CStringUtil	ReplaceStr(TCHAR *prmSrc, TCHAR	*prmDst);

		void		Format(TCHAR *prmForamt, ...);
	private:
		
		BOOL	Assign(size_t prmNewSize);
		
		int		Compare(const TCHAR *prmStr1,const TCHAR *prmStr2);
		
		BOOL	IsSpaceChar(int prmCharVal);
	private:
		TCHAR		*m_Buf;
		UINT32		m_BufLen;
		UINT32		m_Capacity;
	};
}

#endif