#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#ifdef _MSC_VER
# pragma warning (push)
# pragma warning (disable: 4127 4503 4702 4786)
#endif

#include <cstring>
#include <cstdlib>
#include <string>
#include <map>
#include <list>
#include <algorithm>
#include <stdio.h>

#ifdef _DEBUG
# ifndef assert
#  include <cassert>
# endif
# define SI_ASSERT(x)   assert(x)
#else
# define SI_ASSERT(x)
#endif

enum SI_Error {
	SI_OK = 0,     
	SI_UPDATED = 1,        
	SI_INSERTED = 2,        

	SI_FAIL = -1,     
	SI_NOMEM = -2,       
	SI_FILE = -3           
};

#define SI_UTF8_SIGNATURE     "\xEF\xBB\xBF"

#define SI_NEWLINE_A   "\n"
#define SI_NEWLINE_W   L"\n"

#define SI_HAS_WIDE_FILE
#define SI_WCHAR_T     wchar_t

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
class CSimpleIniTempl
{
public:
	typedef SI_CHAR SI_CHAR_T;

	struct Entry {
		const SI_CHAR * pItem;
		const SI_CHAR * pComment;
		int             nOrder;

		Entry(const SI_CHAR * a_pszItem = NULL, int a_nOrder = 0)
			: pItem(a_pszItem)
			, pComment(NULL)
			, nOrder(a_nOrder)
		{ }
		Entry(const SI_CHAR * a_pszItem, const SI_CHAR * a_pszComment, int a_nOrder)
			: pItem(a_pszItem)
			, pComment(a_pszComment)
			, nOrder(a_nOrder)
		{ }
		Entry(const Entry & rhs) { operator=(rhs); }
		Entry & operator=(const Entry & rhs) {
			pItem = rhs.pItem;
			pComment = rhs.pComment;
			nOrder = rhs.nOrder;
			return *this;
		}

#if defined(_MSC_VER) && _MSC_VER <= 1200
		bool operator<(const Entry & rhs) const { return LoadOrder()(*this, rhs); }
		bool operator>(const Entry & rhs) const { return LoadOrder()(rhs, *this); }
#endif

		struct KeyOrder {
			bool operator()(const Entry & lhs, const Entry & rhs) const {
				const static SI_STRLESS isLess = SI_STRLESS();
				return isLess(lhs.pItem, rhs.pItem);
			}
		};

		struct LoadOrder {
			bool operator()(const Entry & lhs, const Entry & rhs) const {
				if (lhs.nOrder != rhs.nOrder) {
					return lhs.nOrder < rhs.nOrder;
				}
				return KeyOrder()(lhs.pItem, rhs.pItem);
			}
		};
	};

	typedef std::multimap<Entry, const SI_CHAR *, typename Entry::KeyOrder> TKeyVal;

	typedef std::map<Entry, TKeyVal, typename Entry::KeyOrder> TSection;

	typedef std::list<Entry> TNamesDepend;

	class OutputWriter {
	public:
		OutputWriter() { }
		virtual ~OutputWriter() { }
		virtual void Write(const char * a_pBuf) = 0;
	private:
		OutputWriter(const OutputWriter &);              
		OutputWriter & operator=(const OutputWriter &);  
	};

	class FileWriter : public OutputWriter {
		FILE * m_file;
	public:
		FileWriter(FILE * a_file) : m_file(a_file) { }
		void Write(const char * a_pBuf) {
			fputs(a_pBuf, m_file);
		}
	private:
		FileWriter(const FileWriter &);              
		FileWriter & operator=(const FileWriter &);  
	};

	class StringWriter : public OutputWriter {
		std::string & m_string;
	public:
		StringWriter(std::string & a_string) : m_string(a_string) { }
		void Write(const char * a_pBuf) {
			m_string.append(a_pBuf);
		}
	private:
		StringWriter(const StringWriter &);              
		StringWriter & operator=(const StringWriter &);  
	};

	class Converter : private SI_CONVERTER {
	public:
		Converter(bool a_bStoreIsUtf8) : SI_CONVERTER(a_bStoreIsUtf8) {
			m_scratch.resize(1024);
		}
		Converter(const Converter & rhs) { operator=(rhs); }
		Converter & operator=(const Converter & rhs) {
			m_scratch = rhs.m_scratch;
			return *this;
		}
		bool ConvertToStore(const SI_CHAR * a_pszString) {
			size_t uLen = SI_CONVERTER::SizeToStore(a_pszString);
			if (uLen == (size_t)(-1)) {
				return false;
			}
			while (uLen > m_scratch.size()) {
				m_scratch.resize(m_scratch.size() * 2);
			}
			return SI_CONVERTER::ConvertToStore(
				a_pszString,
				const_cast<char*>(m_scratch.data()),
				m_scratch.size());
		}
		const char * Data() { return m_scratch.data(); }
	private:
		std::string m_scratch;
	};

public:
	CSimpleIniTempl(
		bool a_bIsUtf8 = false,
		bool a_bMultiKey = false,
		bool a_bMultiLine = false
	);

	~CSimpleIniTempl();

	void Reset();

	bool IsEmpty() const { return m_data.empty(); }

	void SetUnicode(bool a_bIsUtf8 = true) {
		if (!m_pData) m_bStoreIsUtf8 = a_bIsUtf8;
	}

	bool IsUnicode() const { return m_bStoreIsUtf8; }

	void SetMultiKey(bool a_bAllowMultiKey = true) {
		m_bAllowMultiKey = a_bAllowMultiKey;
	}

	bool IsMultiKey() const { return m_bAllowMultiKey; }

	void SetMultiLine(bool a_bAllowMultiLine = true) {
		m_bAllowMultiLine = a_bAllowMultiLine;
	}

	bool IsMultiLine() const { return m_bAllowMultiLine; }

	void SetSpaces(bool a_bSpaces = true) {
		m_bSpaces = a_bSpaces;
	}

	bool UsingSpaces() const { return m_bSpaces; }

	SI_Error LoadFile(
		const char * a_pszFile
	);

#ifdef SI_HAS_WIDE_FILE
	SI_Error LoadFile(
		const SI_WCHAR_T * a_pwszFile
	);
#endif  

	SI_Error LoadFile(
		FILE * a_fpFile
	);

	SI_Error LoadData(const std::string & a_strData) {
		return LoadData(a_strData.c_str(), a_strData.size());
	}

	SI_Error LoadData(
		const char *    a_pData,
		size_t          a_uDataLen
	);

	SI_Error SaveFile(
		const char *    a_pszFile,
		bool            a_bAddSignature = true
	) const;

#ifdef SI_HAS_WIDE_FILE
	SI_Error SaveFile(
		const SI_WCHAR_T *  a_pwszFile,
		bool                a_bAddSignature = true
	) const;
#endif  

	SI_Error SaveFile(
		FILE *  a_pFile,
		bool    a_bAddSignature = false
	) const;

	SI_Error Save(
		OutputWriter &  a_oOutput,
		bool            a_bAddSignature = false
	) const;

	SI_Error Save(
		std::string &   a_sBuffer,
		bool            a_bAddSignature = false
	) const
	{
		StringWriter writer(a_sBuffer);
		return Save(writer, a_bAddSignature);
	}

	void GetAllSections(
		TNamesDepend & a_names
	) const;

	bool GetAllKeys(
		const SI_CHAR * a_pSection,
		TNamesDepend &  a_names
	) const;

	bool GetAllValues(
		const SI_CHAR * a_pSection,
		const SI_CHAR * a_pKey,
		TNamesDepend &  a_values
	) const;

	int GetSectionSize(
		const SI_CHAR * a_pSection
	) const;

	const TKeyVal * GetSection(
		const SI_CHAR * a_pSection
	) const;

	const SI_CHAR * GetValue(
		const SI_CHAR * a_pSection,
		const SI_CHAR * a_pKey,
		const SI_CHAR * a_pDefault = NULL,
		bool *          a_pHasMultiple = NULL
	) const;

	long GetLongValue(
		const SI_CHAR * a_pSection,
		const SI_CHAR * a_pKey,
		long            a_nDefault = 0,
		bool *          a_pHasMultiple = NULL
	) const;

	double GetDoubleValue(
		const SI_CHAR * a_pSection,
		const SI_CHAR * a_pKey,
		double          a_nDefault = 0,
		bool *          a_pHasMultiple = NULL
	) const;

	bool GetBoolValue(
		const SI_CHAR * a_pSection,
		const SI_CHAR * a_pKey,
		bool            a_bDefault = false,
		bool *          a_pHasMultiple = NULL
	) const;

	SI_Error SetValue(
		const SI_CHAR * a_pSection,
		const SI_CHAR * a_pKey,
		const SI_CHAR * a_pValue,
		const SI_CHAR * a_pComment = NULL,
		bool            a_bForceReplace = false
	)
	{
		return AddEntry(a_pSection, a_pKey, a_pValue, a_pComment, a_bForceReplace, true);
	}

	SI_Error SetLongValue(
		const SI_CHAR * a_pSection,
		const SI_CHAR * a_pKey,
		long            a_nValue,
		const SI_CHAR * a_pComment = NULL,
		bool            a_bUseHex = false,
		bool            a_bForceReplace = false
	);

	SI_Error SetDoubleValue(
		const SI_CHAR * a_pSection,
		const SI_CHAR * a_pKey,
		double          a_nValue,
		const SI_CHAR * a_pComment = NULL,
		bool            a_bForceReplace = false
	);

	SI_Error SetBoolValue(
		const SI_CHAR * a_pSection,
		const SI_CHAR * a_pKey,
		bool            a_bValue,
		const SI_CHAR * a_pComment = NULL,
		bool            a_bForceReplace = false
	);

	bool Delete(
		const SI_CHAR * a_pSection,
		const SI_CHAR * a_pKey,
		bool            a_bRemoveEmpty = false
	);

	bool DeleteValue(
		const SI_CHAR * a_pSection,
		const SI_CHAR * a_pKey,
		const SI_CHAR * a_pValue,
		bool            a_bRemoveEmpty = false
	);

	Converter GetConverter() const {
		return Converter(m_bStoreIsUtf8);
	}

private:
	CSimpleIniTempl(const CSimpleIniTempl &);  
	CSimpleIniTempl & operator=(const CSimpleIniTempl &);  

	SI_Error FindFileComment(
		SI_CHAR *&      a_pData,
		bool            a_bCopyStrings
	);

	bool FindEntry(
		SI_CHAR *&  a_pData,
		const SI_CHAR *&  a_pSection,
		const SI_CHAR *&  a_pKey,
		const SI_CHAR *&  a_pVal,
		const SI_CHAR *&  a_pComment
	) const;

	SI_Error AddEntry(
		const SI_CHAR * a_pSection,
		const SI_CHAR * a_pKey,
		const SI_CHAR * a_pValue,
		const SI_CHAR * a_pComment,
		bool            a_bForceReplace,
		bool            a_bCopyStrings
	);

	inline bool IsSpace(SI_CHAR ch) const {
		return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
	}

	inline bool IsComment(SI_CHAR ch) const {
		return (ch == ';' || ch == '#');
	}


	inline void SkipNewLine(SI_CHAR *& a_pData) const {
		a_pData += (*a_pData == '\r' && *(a_pData + 1) == '\n') ? 2 : 1;
	}

	SI_Error CopyString(const SI_CHAR *& a_pString);

	void DeleteString(const SI_CHAR * a_pString);

	bool IsLess(const SI_CHAR * a_pLeft, const SI_CHAR * a_pRight) const {
		const static SI_STRLESS isLess = SI_STRLESS();
		return isLess(a_pLeft, a_pRight);
	}

	bool IsMultiLineTag(const SI_CHAR * a_pData) const;
	bool IsMultiLineData(const SI_CHAR * a_pData) const;
	bool LoadMultiLineText(
		SI_CHAR *&          a_pData,
		const SI_CHAR *&    a_pVal,
		const SI_CHAR *     a_pTagName,
		bool                a_bAllowBlankLinesInComment = false
	) const;
	bool IsNewLineChar(SI_CHAR a_c) const;

	bool OutputMultiLineText(
		OutputWriter &  a_oOutput,
		Converter &     a_oConverter,
		const SI_CHAR * a_pText
	) const;

private:
	SI_CHAR * m_pData;

	size_t m_uDataLen;

	const SI_CHAR * m_pFileComment;

	TSection m_data;

	TNamesDepend m_strings;

	bool m_bStoreIsUtf8;

	bool m_bAllowMultiKey;

	bool m_bAllowMultiLine;

	bool m_bSpaces;

	int m_nOrder;
};

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::CSimpleIniTempl(
	bool a_bIsUtf8,
	bool a_bAllowMultiKey,
	bool a_bAllowMultiLine
)
	: m_pData(0)
	, m_uDataLen(0)
	, m_pFileComment(NULL)
	, m_bStoreIsUtf8(a_bIsUtf8)
	, m_bAllowMultiKey(a_bAllowMultiKey)
	, m_bAllowMultiLine(a_bAllowMultiLine)
	, m_bSpaces(true)
	, m_nOrder(0)
{ }

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::~CSimpleIniTempl()
{
	Reset();
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
void
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::Reset()
{
	delete[] m_pData;
	m_pData = NULL;
	m_uDataLen = 0;
	m_pFileComment = NULL;
	if (!m_data.empty()) {
		m_data.erase(m_data.begin(), m_data.end());
	}

	if (!m_strings.empty()) {
		typename TNamesDepend::iterator i = m_strings.begin();
		for (; i != m_strings.end(); ++i) {
			delete[] const_cast<SI_CHAR*>(i->pItem);
		}
		m_strings.erase(m_strings.begin(), m_strings.end());
	}
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
SI_Error
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::LoadFile(
	const char * a_pszFile
)
{
	FILE * fp = NULL;
#if __STDC_WANT_SECURE_LIB__ && !_WIN32_WCE
	fopen_s(&fp, a_pszFile, "rb");
#else  
	fp = fopen(a_pszFile, "rb");
#endif  
	if (!fp) {
		return SI_FILE;
	}
	SI_Error rc = LoadFile(fp);
	fclose(fp);
	return rc;
}

#ifdef SI_HAS_WIDE_FILE
template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
SI_Error
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::LoadFile(
	const SI_WCHAR_T * a_pwszFile
)
{
#ifdef _WIN32
	FILE * fp = NULL;
#if __STDC_WANT_SECURE_LIB__ && !_WIN32_WCE
	_wfopen_s(&fp, a_pwszFile, L"rb");
#else  
	fp = _wfopen(a_pwszFile, L"rb");
#endif  
	if (!fp) return SI_FILE;
	SI_Error rc = LoadFile(fp);
	fclose(fp);
	return rc;
#else    
	char szFile[256];
	u_austrncpy(szFile, a_pwszFile, sizeof(szFile));
	return LoadFile(szFile);
#endif  
}
#endif  

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
SI_Error
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::LoadFile(
	FILE * a_fpFile
)
{
	int retval = fseek(a_fpFile, 0, SEEK_END);
	if (retval != 0) {
		return SI_FILE;
	}
	long lSize = ftell(a_fpFile);
	if (lSize < 0) {
		return SI_FILE;
	}
	if (lSize == 0) {
		return SI_OK;
	}

	char * pData = new(std::nothrow) char[lSize + 1];
	if (!pData) {
		return SI_NOMEM;
	}
	pData[lSize] = 0;

	fseek(a_fpFile, 0, SEEK_SET);
	size_t uRead = fread(pData, sizeof(char), lSize, a_fpFile);
	if (uRead != (size_t)lSize) {
		delete[] pData;
		return SI_FILE;
	}

	SI_Error rc = LoadData(pData, uRead);
	delete[] pData;
	return rc;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
SI_Error
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::LoadData(
	const char *    a_pData,
	size_t          a_uDataLen
)
{
	if (!a_pData) {
		return SI_OK;
	}

	if (a_uDataLen >= 3 && memcmp(a_pData, SI_UTF8_SIGNATURE, 3) == 0) {
		a_pData += 3;
		a_uDataLen -= 3;
		SI_ASSERT(m_bStoreIsUtf8 || !m_pData);       
		SetUnicode();
	}

	if (a_uDataLen == 0) {
		return SI_OK;
	}

	SI_CONVERTER converter(m_bStoreIsUtf8);
	size_t uLen = converter.SizeFromStore(a_pData, a_uDataLen);
	if (uLen == (size_t)(-1)) {
		return SI_FAIL;
	}

	SI_CHAR * pData = new(std::nothrow) SI_CHAR[uLen + 1];
	if (!pData) {
		return SI_NOMEM;
	}
	memset(pData, 0, sizeof(SI_CHAR)*(uLen + 1));

	if (!converter.ConvertFromStore(a_pData, a_uDataLen, pData, uLen)) {
		delete[] pData;
		return SI_FAIL;
	}

	const static SI_CHAR empty = 0;
	SI_CHAR * pWork = pData;
	const SI_CHAR * pSection = &empty;
	const SI_CHAR * pItem = NULL;
	const SI_CHAR * pVal = NULL;
	const SI_CHAR * pComment = NULL;

	bool bCopyStrings = (m_pData != NULL);

	SI_Error rc = FindFileComment(pWork, bCopyStrings);
	if (rc < 0) return rc;

	while (FindEntry(pWork, pSection, pItem, pVal, pComment)) {
		rc = AddEntry(pSection, pItem, pVal, pComment, false, bCopyStrings);
		if (rc < 0) return rc;
	}

	if (bCopyStrings) {
		delete[] pData;
	}
	else {
		m_pData = pData;
		m_uDataLen = uLen + 1;
	}

	return SI_OK;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
SI_Error
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::FindFileComment(
	SI_CHAR *&      a_pData,
	bool            a_bCopyStrings
)
{
	if (m_pFileComment) {
		return SI_OK;
	}

	if (!LoadMultiLineText(a_pData, m_pFileComment, NULL, false)) {
		return SI_OK;
	}

	if (a_bCopyStrings) {
		SI_Error rc = CopyString(m_pFileComment);
		if (rc < 0) return rc;
	}

	return SI_OK;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
bool
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::FindEntry(
	SI_CHAR *&        a_pData,
	const SI_CHAR *&  a_pSection,
	const SI_CHAR *&  a_pKey,
	const SI_CHAR *&  a_pVal,
	const SI_CHAR *&  a_pComment
) const
{
	a_pComment = NULL;

	SI_CHAR * pTrail = NULL;
	while (*a_pData) {
		while (*a_pData && IsSpace(*a_pData)) {
			++a_pData;
		}
		if (!*a_pData) {
			break;
		}

		if (IsComment(*a_pData)) {
			LoadMultiLineText(a_pData, a_pComment, NULL, true);
			continue;
		}

		if (*a_pData == '[') {
			++a_pData;
			while (*a_pData && IsSpace(*a_pData)) {
				++a_pData;
			}

			a_pSection = a_pData;
			while (*a_pData && *a_pData != ']' && !IsNewLineChar(*a_pData)) {
				++a_pData;
			}

			if (*a_pData != ']') {
				continue;
			}

			pTrail = a_pData - 1;
			while (pTrail >= a_pSection && IsSpace(*pTrail)) {
				--pTrail;
			}
			++pTrail;
			*pTrail = 0;

			++a_pData;          
			while (*a_pData && !IsNewLineChar(*a_pData)) {
				++a_pData;
			}

			a_pKey = NULL;
			a_pVal = NULL;
			return true;
		}

		a_pKey = a_pData;
		while (*a_pData && *a_pData != '=' && !IsNewLineChar(*a_pData)) {
			++a_pData;
		}

		if (*a_pData != '=') {
			continue;
		}

		if (a_pKey == a_pData) {
			while (*a_pData && !IsNewLineChar(*a_pData)) {
				++a_pData;
			}
			continue;
		}

		pTrail = a_pData - 1;
		while (pTrail >= a_pKey && IsSpace(*pTrail)) {
			--pTrail;
		}
		++pTrail;
		*pTrail = 0;

		++a_pData;          
		while (*a_pData && !IsNewLineChar(*a_pData) && IsSpace(*a_pData)) {
			++a_pData;
		}

		a_pVal = a_pData;
		while (*a_pData && !IsNewLineChar(*a_pData)) {
			++a_pData;
		}

		pTrail = a_pData - 1;
		if (*a_pData) {      
			SkipNewLine(a_pData);
		}
		while (pTrail >= a_pVal && IsSpace(*pTrail)) {
			--pTrail;
		}
		++pTrail;
		*pTrail = 0;

		if (m_bAllowMultiLine && IsMultiLineTag(a_pVal)) {
			const SI_CHAR * pTagName = a_pVal + 3;
			return LoadMultiLineText(a_pData, a_pVal, pTagName);
		}

		return true;
	}

	return false;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
bool
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::IsMultiLineTag(
	const SI_CHAR * a_pVal
) const
{
	if (*a_pVal++ != '<') return false;
	if (*a_pVal++ != '<') return false;
	if (*a_pVal++ != '<') return false;
	return true;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
bool
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::IsMultiLineData(
	const SI_CHAR * a_pData
) const
{
	if (!*a_pData) {
		return false;
	}

	if (IsSpace(*a_pData)) {
		return true;
	}

	while (*a_pData) {
		if (IsNewLineChar(*a_pData)) {
			return true;
		}
		++a_pData;
	}

	if (IsSpace(*--a_pData)) {
		return true;
	}

	return false;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
bool
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::IsNewLineChar(
	SI_CHAR a_c
) const
{
	return (a_c == '\n' || a_c == '\r');
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
bool
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::LoadMultiLineText(
	SI_CHAR *&          a_pData,
	const SI_CHAR *&    a_pVal,
	const SI_CHAR *     a_pTagName,
	bool                a_bAllowBlankLinesInComment
) const
{
	SI_CHAR * pDataLine = a_pData;
	SI_CHAR * pCurrLine;

	a_pVal = a_pData;

	SI_CHAR cEndOfLineChar = *a_pData;
	for (;;) {
		if (!a_pTagName && !IsComment(*a_pData)) {
			if (!a_bAllowBlankLinesInComment) {
				break;
			}

			SI_CHAR * pCurr = a_pData;
			int nNewLines = 0;
			while (IsSpace(*pCurr)) {
				if (IsNewLineChar(*pCurr)) {
					++nNewLines;
					SkipNewLine(pCurr);
				}
				else {
					++pCurr;
				}
			}

			if (IsComment(*pCurr)) {
				for (; nNewLines > 0; --nNewLines) *pDataLine++ = '\n';
				a_pData = pCurr;
				continue;
			}

			break;
		}

		pCurrLine = a_pData;
		while (*a_pData && !IsNewLineChar(*a_pData)) ++a_pData;

		if (pDataLine < pCurrLine) {
			size_t nLen = (size_t)(a_pData - pCurrLine);
			memmove(pDataLine, pCurrLine, nLen * sizeof(SI_CHAR));
			pDataLine[nLen] = '\0';
		}

		cEndOfLineChar = *a_pData;
		*a_pData = 0;

		if (a_pTagName &&
			(!IsLess(pDataLine, a_pTagName) && !IsLess(a_pTagName, pDataLine)))
		{
			break;
		}

		if (!cEndOfLineChar) {
			return true;
		}

		pDataLine += (a_pData - pCurrLine);
		*a_pData = cEndOfLineChar;
		SkipNewLine(a_pData);
		*pDataLine++ = '\n';
	}

	if (a_pVal == a_pData) {
		a_pVal = NULL;
		return false;
	}

	*--pDataLine = '\0';

	if (a_pTagName && cEndOfLineChar) {
		SI_ASSERT(IsNewLineChar(cEndOfLineChar));
		*a_pData = cEndOfLineChar;
		SkipNewLine(a_pData);
	}

	return true;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
SI_Error
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::CopyString(
	const SI_CHAR *& a_pString
)
{
	size_t uLen = 0;
	if (sizeof(SI_CHAR) == sizeof(char)) {
		uLen = strlen((const char *)a_pString);
	}
	else if (sizeof(SI_CHAR) == sizeof(wchar_t)) {
		uLen = wcslen((const wchar_t *)a_pString);
	}
	else {
		for (; a_pString[uLen]; ++uLen) ;
	}
	++uLen;   
	SI_CHAR * pCopy = new(std::nothrow) SI_CHAR[uLen];
	if (!pCopy) {
		return SI_NOMEM;
	}
	memcpy(pCopy, a_pString, sizeof(SI_CHAR)*uLen);
	m_strings.push_back(pCopy);
	a_pString = pCopy;
	return SI_OK;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
SI_Error
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::AddEntry(
	const SI_CHAR * a_pSection,
	const SI_CHAR * a_pKey,
	const SI_CHAR * a_pValue,
	const SI_CHAR * a_pComment,
	bool            a_bForceReplace,
	bool            a_bCopyStrings
)
{
	SI_Error rc;
	bool bInserted = false;

	SI_ASSERT(!a_pComment || IsComment(*a_pComment));

	if (a_bCopyStrings && a_pComment) {
		rc = CopyString(a_pComment);
		if (rc < 0) return rc;
	}

	typename TSection::iterator iSection = m_data.find(a_pSection);
	if (iSection == m_data.end()) {
		if (a_bCopyStrings) {
			rc = CopyString(a_pSection);
			if (rc < 0) return rc;
		}

		Entry oSection(a_pSection, ++m_nOrder);
		if (a_pComment && (!a_pKey || !a_pValue)) {
			oSection.pComment = a_pComment;
		}

		typename TSection::value_type oEntry(oSection, TKeyVal());
		typedef typename TSection::iterator SectionIterator;
		std::pair<SectionIterator, bool> i = m_data.insert(oEntry);
		iSection = i.first;
		bInserted = true;
	}
	if (!a_pKey || !a_pValue) {
		return bInserted ? SI_INSERTED : SI_UPDATED;
	}

	TKeyVal & keyval = iSection->second;
	typename TKeyVal::iterator iKey = keyval.find(a_pKey);

	int nLoadOrder = ++m_nOrder;
	if (iKey != keyval.end() && m_bAllowMultiKey && a_bForceReplace) {
		const SI_CHAR * pComment = NULL;
		while (iKey != keyval.end() && !IsLess(a_pKey, iKey->first.pItem)) {
			if (iKey->first.nOrder < nLoadOrder) {
				nLoadOrder = iKey->first.nOrder;
				pComment = iKey->first.pComment;
			}
			++iKey;
		}
		if (pComment) {
			DeleteString(a_pComment);
			a_pComment = pComment;
			CopyString(a_pComment);
		}
		Delete(a_pSection, a_pKey);
		iKey = keyval.end();
	}

	bool bForceCreateNewKey = m_bAllowMultiKey && !a_bForceReplace;
	if (a_bCopyStrings) {
		if (bForceCreateNewKey || iKey == keyval.end()) {
			rc = CopyString(a_pKey);
			if (rc < 0) return rc;
		}

		rc = CopyString(a_pValue);
		if (rc < 0) return rc;
	}

	if (iKey == keyval.end() || bForceCreateNewKey) {
		Entry oKey(a_pKey, nLoadOrder);
		if (a_pComment) {
			oKey.pComment = a_pComment;
		}
		typename TKeyVal::value_type oEntry(oKey, static_cast<const SI_CHAR *>(NULL));
		iKey = keyval.insert(oEntry);
		bInserted = true;
	}
	iKey->second = a_pValue;
	return bInserted ? SI_INSERTED : SI_UPDATED;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
const SI_CHAR *
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::GetValue(
	const SI_CHAR * a_pSection,
	const SI_CHAR * a_pKey,
	const SI_CHAR * a_pDefault,
	bool *          a_pHasMultiple
) const
{
	if (a_pHasMultiple) {
		*a_pHasMultiple = false;
	}
	if (!a_pSection || !a_pKey) {
		return a_pDefault;
	}
	typename TSection::const_iterator iSection = m_data.find(a_pSection);
	if (iSection == m_data.end()) {
		return a_pDefault;
	}
	typename TKeyVal::const_iterator iKeyVal = iSection->second.find(a_pKey);
	if (iKeyVal == iSection->second.end()) {
		return a_pDefault;
	}

	if (m_bAllowMultiKey && a_pHasMultiple) {
		typename TKeyVal::const_iterator iTemp = iKeyVal;
		if (++iTemp != iSection->second.end()) {
			if (!IsLess(a_pKey, iTemp->first.pItem)) {
				*a_pHasMultiple = true;
			}
		}
	}

	return iKeyVal->second;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
long
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::GetLongValue(
	const SI_CHAR * a_pSection,
	const SI_CHAR * a_pKey,
	long            a_nDefault,
	bool *          a_pHasMultiple
) const
{
	const SI_CHAR * pszValue = GetValue(a_pSection, a_pKey, NULL, a_pHasMultiple);
	if (!pszValue || !*pszValue) return a_nDefault;

	char szValue[64] = { 0 };
	SI_CONVERTER c(m_bStoreIsUtf8);
	if (!c.ConvertToStore(pszValue, szValue, sizeof(szValue))) {
		return a_nDefault;
	}

	long nValue = a_nDefault;
	char * pszSuffix = szValue;
	if (szValue[0] == '0' && (szValue[1] == 'x' || szValue[1] == 'X')) {
		if (!szValue[2]) return a_nDefault;
		nValue = strtol(&szValue[2], &pszSuffix, 16);
	}
	else {
		nValue = strtol(szValue, &pszSuffix, 10);
	}

	if (*pszSuffix) {
		return a_nDefault;
	}

	return nValue;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
SI_Error
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::SetLongValue(
	const SI_CHAR * a_pSection,
	const SI_CHAR * a_pKey,
	long            a_nValue,
	const SI_CHAR * a_pComment,
	bool            a_bUseHex,
	bool            a_bForceReplace
)
{
	if (!a_pSection || !a_pKey) return SI_FAIL;

	char szInput[64];
#if __STDC_WANT_SECURE_LIB__ && !_WIN32_WCE
	sprintf_s(szInput, a_bUseHex ? "0x%lx" : "%ld", a_nValue);
#else  
	sprintf(szInput, a_bUseHex ? "0x%lx" : "%ld", a_nValue);
#endif  

	SI_CHAR szOutput[64];
	SI_CONVERTER c(m_bStoreIsUtf8);
	c.ConvertFromStore(szInput, strlen(szInput) + 1,
		szOutput, sizeof(szOutput) / sizeof(SI_CHAR));

	return AddEntry(a_pSection, a_pKey, szOutput, a_pComment, a_bForceReplace, true);
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
double
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::GetDoubleValue(
	const SI_CHAR * a_pSection,
	const SI_CHAR * a_pKey,
	double          a_nDefault,
	bool *          a_pHasMultiple
) const
{
	const SI_CHAR * pszValue = GetValue(a_pSection, a_pKey, NULL, a_pHasMultiple);
	if (!pszValue || !*pszValue) return a_nDefault;

	char szValue[64] = { 0 };
	SI_CONVERTER c(m_bStoreIsUtf8);
	if (!c.ConvertToStore(pszValue, szValue, sizeof(szValue))) {
		return a_nDefault;
	}

	char * pszSuffix = NULL;
	double nValue = strtod(szValue, &pszSuffix);

	if (!pszSuffix || *pszSuffix) {
		return a_nDefault;
	}

	return nValue;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
SI_Error
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::SetDoubleValue(
	const SI_CHAR * a_pSection,
	const SI_CHAR * a_pKey,
	double          a_nValue,
	const SI_CHAR * a_pComment,
	bool            a_bForceReplace
)
{
	if (!a_pSection || !a_pKey) return SI_FAIL;

	char szInput[64];
#if __STDC_WANT_SECURE_LIB__ && !_WIN32_WCE
	sprintf_s(szInput, "%f", a_nValue);
#else  
	sprintf(szInput, "%f", a_nValue);
#endif  

	SI_CHAR szOutput[64];
	SI_CONVERTER c(m_bStoreIsUtf8);
	c.ConvertFromStore(szInput, strlen(szInput) + 1,
		szOutput, sizeof(szOutput) / sizeof(SI_CHAR));

	return AddEntry(a_pSection, a_pKey, szOutput, a_pComment, a_bForceReplace, true);
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
bool
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::GetBoolValue(
	const SI_CHAR * a_pSection,
	const SI_CHAR * a_pKey,
	bool            a_bDefault,
	bool *          a_pHasMultiple
) const
{
	const SI_CHAR * pszValue = GetValue(a_pSection, a_pKey, NULL, a_pHasMultiple);
	if (!pszValue || !*pszValue) return a_bDefault;

	switch (pszValue[0]) {
	case 't': case 'T':  
	case 'y': case 'Y':  
	case '1':             
		return true;

	case 'f': case 'F':  
	case 'n': case 'N':  
	case '0':             
		return false;

	case 'o': case 'O':
		if (pszValue[1] == 'n' || pszValue[1] == 'N') return true;   
		if (pszValue[1] == 'f' || pszValue[1] == 'F') return false;  
		break;
	}

	return a_bDefault;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
SI_Error
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::SetBoolValue(
	const SI_CHAR * a_pSection,
	const SI_CHAR * a_pKey,
	bool            a_bValue,
	const SI_CHAR * a_pComment,
	bool            a_bForceReplace
)
{
	if (!a_pSection || !a_pKey) return SI_FAIL;

	const char * pszInput = a_bValue ? "true" : "false";

	SI_CHAR szOutput[64];
	SI_CONVERTER c(m_bStoreIsUtf8);
	c.ConvertFromStore(pszInput, strlen(pszInput) + 1,
		szOutput, sizeof(szOutput) / sizeof(SI_CHAR));

	return AddEntry(a_pSection, a_pKey, szOutput, a_pComment, a_bForceReplace, true);
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
bool
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::GetAllValues(
	const SI_CHAR * a_pSection,
	const SI_CHAR * a_pKey,
	TNamesDepend &  a_values
) const
{
	a_values.clear();

	if (!a_pSection || !a_pKey) {
		return false;
	}
	typename TSection::const_iterator iSection = m_data.find(a_pSection);
	if (iSection == m_data.end()) {
		return false;
	}
	typename TKeyVal::const_iterator iKeyVal = iSection->second.find(a_pKey);
	if (iKeyVal == iSection->second.end()) {
		return false;
	}

	a_values.push_back(Entry(iKeyVal->second, iKeyVal->first.pComment, iKeyVal->first.nOrder));
	if (m_bAllowMultiKey) {
		++iKeyVal;
		while (iKeyVal != iSection->second.end() && !IsLess(a_pKey, iKeyVal->first.pItem)) {
			a_values.push_back(Entry(iKeyVal->second, iKeyVal->first.pComment, iKeyVal->first.nOrder));
			++iKeyVal;
		}
	}

	return true;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
int
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::GetSectionSize(
	const SI_CHAR * a_pSection
) const
{
	if (!a_pSection) {
		return -1;
	}

	typename TSection::const_iterator iSection = m_data.find(a_pSection);
	if (iSection == m_data.end()) {
		return -1;
	}
	const TKeyVal & section = iSection->second;

	if (!m_bAllowMultiKey || section.empty()) {
		return (int)section.size();
	}

	int nCount = 0;
	const SI_CHAR * pLastKey = NULL;
	typename TKeyVal::const_iterator iKeyVal = section.begin();
	for (int n = 0; iKeyVal != section.end(); ++iKeyVal, ++n) {
		if (!pLastKey || IsLess(pLastKey, iKeyVal->first.pItem)) {
			++nCount;
			pLastKey = iKeyVal->first.pItem;
		}
	}
	return nCount;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
const typename CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::TKeyVal *
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::GetSection(
	const SI_CHAR * a_pSection
) const
{
	if (a_pSection) {
		typename TSection::const_iterator i = m_data.find(a_pSection);
		if (i != m_data.end()) {
			return &(i->second);
		}
	}
	return 0;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
void
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::GetAllSections(
	TNamesDepend & a_names
) const
{
	a_names.clear();
	typename TSection::const_iterator i = m_data.begin();
	for (int n = 0; i != m_data.end(); ++i, ++n) {
		a_names.push_back(i->first);
	}
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
bool
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::GetAllKeys(
	const SI_CHAR * a_pSection,
	TNamesDepend &  a_names
) const
{
	a_names.clear();

	if (!a_pSection) {
		return false;
	}

	typename TSection::const_iterator iSection = m_data.find(a_pSection);
	if (iSection == m_data.end()) {
		return false;
	}

	const TKeyVal & section = iSection->second;
	const SI_CHAR * pLastKey = NULL;
	typename TKeyVal::const_iterator iKeyVal = section.begin();
	for (int n = 0; iKeyVal != section.end(); ++iKeyVal, ++n) {
		if (!pLastKey || IsLess(pLastKey, iKeyVal->first.pItem)) {
			a_names.push_back(iKeyVal->first);
			pLastKey = iKeyVal->first.pItem;
		}
	}

	return true;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
SI_Error
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::SaveFile(
	const char *    a_pszFile,
	bool            a_bAddSignature
) const
{
	FILE * fp = NULL;
#if __STDC_WANT_SECURE_LIB__ && !_WIN32_WCE
	fopen_s(&fp, a_pszFile, "wb");
#else  
	fp = fopen(a_pszFile, "wb");
#endif  
	if (!fp) return SI_FILE;
	SI_Error rc = SaveFile(fp, a_bAddSignature);
	fclose(fp);
	return rc;
}

#ifdef SI_HAS_WIDE_FILE
template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
SI_Error
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::SaveFile(
	const SI_WCHAR_T *  a_pwszFile,
	bool                a_bAddSignature
) const
{
#ifdef _WIN32
	FILE * fp = NULL;
#if __STDC_WANT_SECURE_LIB__ && !_WIN32_WCE
	_wfopen_s(&fp, a_pwszFile, L"wb");
#else  
	fp = _wfopen(a_pwszFile, L"wb");
#endif  
	if (!fp) return SI_FILE;
	SI_Error rc = SaveFile(fp, a_bAddSignature);
	fclose(fp);
	return rc;
#else    
	char szFile[256];
	u_austrncpy(szFile, a_pwszFile, sizeof(szFile));
	return SaveFile(szFile, a_bAddSignature);
#endif  
}
#endif  

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
SI_Error
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::SaveFile(
	FILE *  a_pFile,
	bool    a_bAddSignature
) const
{
	FileWriter writer(a_pFile);
	return Save(writer, a_bAddSignature);
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
SI_Error
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::Save(
	OutputWriter &  a_oOutput,
	bool            a_bAddSignature
) const
{
	Converter convert(m_bStoreIsUtf8);

	if (m_bStoreIsUtf8 && a_bAddSignature) {
		a_oOutput.Write(SI_UTF8_SIGNATURE);
	}

	TNamesDepend oSections;
	GetAllSections(oSections);
#if defined(_MSC_VER) && _MSC_VER <= 1200
	oSections.sort();
#elif defined(__BORLANDC__)
	oSections.sort(Entry::LoadOrder());
#else
	oSections.sort(typename Entry::LoadOrder());
#endif

	bool bNeedNewLine = false;
	if (m_pFileComment) {
		if (!OutputMultiLineText(a_oOutput, convert, m_pFileComment)) {
			return SI_FAIL;
		}
		bNeedNewLine = true;
	}

	typename TNamesDepend::const_iterator iSection = oSections.begin();
	for (; iSection != oSections.end(); ++iSection) {
		if (iSection->pComment) {
			if (bNeedNewLine) {
				a_oOutput.Write(SI_NEWLINE_A);
			}
			if (!OutputMultiLineText(a_oOutput, convert, iSection->pComment)) {
				return SI_FAIL;
			}
			bNeedNewLine = false;
		}

		if (bNeedNewLine) {
			bNeedNewLine = false;
		}

		if (*iSection->pItem) {
			if (!convert.ConvertToStore(iSection->pItem)) {
				return SI_FAIL;
			}
			a_oOutput.Write("[");
			a_oOutput.Write(convert.Data());
			a_oOutput.Write("]");
			a_oOutput.Write(SI_NEWLINE_A);
		}

		TNamesDepend oKeys;
		GetAllKeys(iSection->pItem, oKeys);
#if defined(_MSC_VER) && _MSC_VER <= 1200
		oKeys.sort();
#elif defined(__BORLANDC__)
		oKeys.sort(Entry::LoadOrder());
#else
		oKeys.sort(typename Entry::LoadOrder());
#endif

		typename TNamesDepend::const_iterator iKey = oKeys.begin();
		for (; iKey != oKeys.end(); ++iKey) {
			TNamesDepend oValues;
			GetAllValues(iSection->pItem, iKey->pItem, oValues);

			typename TNamesDepend::const_iterator iValue = oValues.begin();
			for (; iValue != oValues.end(); ++iValue) {
				if (iValue->pComment) {
					a_oOutput.Write(SI_NEWLINE_A);
					if (!OutputMultiLineText(a_oOutput, convert, iValue->pComment)) {
						return SI_FAIL;
					}
				}

				if (!convert.ConvertToStore(iKey->pItem)) {
					return SI_FAIL;
				}
				a_oOutput.Write(convert.Data());

				if (!convert.ConvertToStore(iValue->pItem)) {
					return SI_FAIL;
				}
				a_oOutput.Write(m_bSpaces ? " = " : "=");
				if (m_bAllowMultiLine && IsMultiLineData(iValue->pItem)) {
					a_oOutput.Write("<<<END_OF_TEXT" SI_NEWLINE_A);
					if (!OutputMultiLineText(a_oOutput, convert, iValue->pItem)) {
						return SI_FAIL;
					}
					a_oOutput.Write("END_OF_TEXT");
				}
				else {
					a_oOutput.Write(convert.Data());
				}
				a_oOutput.Write(SI_NEWLINE_A);
			}
		}

		bNeedNewLine = true;
	}

	return SI_OK;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
bool
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::OutputMultiLineText(
	OutputWriter &  a_oOutput,
	Converter &     a_oConverter,
	const SI_CHAR * a_pText
) const
{
	const SI_CHAR * pEndOfLine;
	SI_CHAR cEndOfLineChar = *a_pText;
	while (cEndOfLineChar) {
		pEndOfLine = a_pText;
		for (; *pEndOfLine && *pEndOfLine != '\n'; ++pEndOfLine) ;
		cEndOfLineChar = *pEndOfLine;

		*const_cast<SI_CHAR*>(pEndOfLine) = 0;
		if (!a_oConverter.ConvertToStore(a_pText)) {
			return false;
		}
		*const_cast<SI_CHAR*>(pEndOfLine) = cEndOfLineChar;
		a_pText += (pEndOfLine - a_pText) + 1;
		a_oOutput.Write(a_oConverter.Data());
		a_oOutput.Write(SI_NEWLINE_A);
	}
	return true;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
bool
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::Delete(
	const SI_CHAR * a_pSection,
	const SI_CHAR * a_pKey,
	bool            a_bRemoveEmpty
)
{
	return DeleteValue(a_pSection, a_pKey, NULL, a_bRemoveEmpty);
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
bool
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::DeleteValue(
	const SI_CHAR * a_pSection,
	const SI_CHAR * a_pKey,
	const SI_CHAR * a_pValue,
	bool            a_bRemoveEmpty
)
{
	if (!a_pSection) {
		return false;
	}

	typename TSection::iterator iSection = m_data.find(a_pSection);
	if (iSection == m_data.end()) {
		return false;
	}

	if (a_pKey) {
		typename TKeyVal::iterator iKeyVal = iSection->second.find(a_pKey);
		if (iKeyVal == iSection->second.end()) {
			return false;
		}

		const static SI_STRLESS isLess = SI_STRLESS();

		typename TKeyVal::iterator iDelete;
		bool bDeleted = false;
		do {
			iDelete = iKeyVal++;

			if (a_pValue == NULL ||
				(isLess(a_pValue, iDelete->second) == false &&
					isLess(iDelete->second, a_pValue) == false)) {
				DeleteString(iDelete->first.pItem);
				DeleteString(iDelete->second);
				iSection->second.erase(iDelete);
				bDeleted = true;
			}
		} while (iKeyVal != iSection->second.end()
			&& !IsLess(a_pKey, iKeyVal->first.pItem));

		if (!bDeleted) {
			return false;
		}

		if (!a_bRemoveEmpty || !iSection->second.empty()) {
			return true;
		}
	}
	else {
		typename TKeyVal::iterator iKeyVal = iSection->second.begin();
		for (; iKeyVal != iSection->second.end(); ++iKeyVal) {
			DeleteString(iKeyVal->first.pItem);
			DeleteString(iKeyVal->second);
		}
	}

	DeleteString(iSection->first.pItem);
	m_data.erase(iSection);

	return true;
}

template<class SI_CHAR, class SI_STRLESS, class SI_CONVERTER>
void
CSimpleIniTempl<SI_CHAR, SI_STRLESS, SI_CONVERTER>::DeleteString(
	const SI_CHAR * a_pString
)
{
	if (a_pString < m_pData || a_pString >= m_pData + m_uDataLen) {
		typename TNamesDepend::iterator i = m_strings.begin();
		for (; i != m_strings.end(); ++i) {
			if (a_pString == i->pItem) {
				delete[] const_cast<SI_CHAR*>(i->pItem);
				m_strings.erase(i);
				break;
			}
		}
	}
}

template<class SI_CHAR>
struct SI_GenericCase {
	bool operator()(const SI_CHAR * pLeft, const SI_CHAR * pRight) const {
		long cmp;
		for (; *pLeft && *pRight; ++pLeft, ++pRight) {
			cmp = (long)*pLeft - (long)*pRight;
			if (cmp != 0) {
				return cmp < 0;
			}
		}
		return *pRight != 0;
	}
};

template<class SI_CHAR>
struct SI_GenericNoCase {
	inline SI_CHAR locase(SI_CHAR ch) const {
		return (ch < 'A' || ch > 'Z') ? ch : (ch - 'A' + 'a');
	}
	bool operator()(const SI_CHAR * pLeft, const SI_CHAR * pRight) const {
		long cmp;
		for (; *pLeft && *pRight; ++pLeft, ++pRight) {
			cmp = (long)locase(*pLeft) - (long)locase(*pRight);
			if (cmp != 0) {
				return cmp < 0;
			}
		}
		return *pRight != 0;
	}
};

template<class SI_CHAR>
class SI_ConvertA {
	bool m_bStoreIsUtf8;
protected:
	SI_ConvertA() { }
public:
	SI_ConvertA(bool a_bStoreIsUtf8) : m_bStoreIsUtf8(a_bStoreIsUtf8) { }

	SI_ConvertA(const SI_ConvertA & rhs) { operator=(rhs); }
	SI_ConvertA & operator=(const SI_ConvertA & rhs) {
		m_bStoreIsUtf8 = rhs.m_bStoreIsUtf8;
		return *this;
	}

	size_t SizeFromStore(
		const char *    a_pInputData,
		size_t          a_uInputDataLen)
	{
		(void)a_pInputData;
		SI_ASSERT(a_uInputDataLen != (size_t)-1);

		return a_uInputDataLen;
	}

	bool ConvertFromStore(
		const char *    a_pInputData,
		size_t          a_uInputDataLen,
		SI_CHAR *       a_pOutputData,
		size_t          a_uOutputDataSize)
	{
		if (a_uInputDataLen > a_uOutputDataSize) {
			return false;
		}
		memcpy(a_pOutputData, a_pInputData, a_uInputDataLen);
		return true;
	}

	size_t SizeToStore(
		const SI_CHAR * a_pInputData)
	{
		return strlen((const char *)a_pInputData) + 1;
	}

	bool ConvertToStore(
		const SI_CHAR * a_pInputData,
		char *          a_pOutputData,
		size_t          a_uOutputDataSize)
	{
		size_t uInputLen = strlen((const char *)a_pInputData) + 1;
		if (uInputLen > a_uOutputDataSize) {
			return false;
		}

		memcpy(a_pOutputData, a_pInputData, uInputLen);
		return true;
	}
};

#include <windows.h>
#include <mbstring.h>
template<class SI_CHAR>
struct SI_NoCase {
	bool operator()(const SI_CHAR * pLeft, const SI_CHAR * pRight) const {
		if (sizeof(SI_CHAR) == sizeof(char)) {
			return _mbsicmp((const unsigned char *)pLeft,
				(const unsigned char *)pRight) < 0;
		}
		if (sizeof(SI_CHAR) == sizeof(wchar_t)) {
			return _wcsicmp((const wchar_t *)pLeft,
				(const wchar_t *)pRight) < 0;
		}
		return SI_GenericNoCase<SI_CHAR>()(pLeft, pRight);
	}
};

template<class SI_CHAR>
class SI_ConvertW {
	UINT m_uCodePage;
protected:
	SI_ConvertW() { }
public:
	SI_ConvertW(bool a_bStoreIsUtf8) {
		m_uCodePage = a_bStoreIsUtf8 ? CP_UTF8 : CP_ACP;
	}

	SI_ConvertW(const SI_ConvertW & rhs) { operator=(rhs); }
	SI_ConvertW & operator=(const SI_ConvertW & rhs) {
		m_uCodePage = rhs.m_uCodePage;
		return *this;
	}

	size_t SizeFromStore(
		const char *    a_pInputData,
		size_t          a_uInputDataLen)
	{
		SI_ASSERT(a_uInputDataLen != (size_t)-1);

		int retval = MultiByteToWideChar(
			m_uCodePage, 0,
			a_pInputData, (int)a_uInputDataLen,
			0, 0);
		return (size_t)(retval > 0 ? retval : -1);
	}

	bool ConvertFromStore(
		const char *    a_pInputData,
		size_t          a_uInputDataLen,
		SI_CHAR *       a_pOutputData,
		size_t          a_uOutputDataSize)
	{
		int nSize = MultiByteToWideChar(
			m_uCodePage, 0,
			a_pInputData, (int)a_uInputDataLen,
			(wchar_t *)a_pOutputData, (int)a_uOutputDataSize);
		return (nSize > 0);
	}

	size_t SizeToStore(
		const SI_CHAR * a_pInputData)
	{
		int retval = WideCharToMultiByte(
			m_uCodePage, 0,
			(const wchar_t *)a_pInputData, -1,
			0, 0, 0, 0);
		return (size_t)(retval > 0 ? retval : -1);
	}

	bool ConvertToStore(
		const SI_CHAR * a_pInputData,
		char *          a_pOutputData,
		size_t          a_uOutputDataSize)
	{
		int retval = WideCharToMultiByte(
			m_uCodePage, 0,
			(const wchar_t *)a_pInputData, -1,
			a_pOutputData, (int)a_uOutputDataSize, 0, 0);
		return retval > 0;
	}
};

typedef CSimpleIniTempl<char,
	SI_NoCase<char>, SI_ConvertA<char> >                 CSimpleIniA;
typedef CSimpleIniTempl<char,
	SI_GenericCase<char>, SI_ConvertA<char> >                   CSimpleIniCaseA;

typedef CSimpleIniTempl<wchar_t,
	SI_NoCase<wchar_t>, SI_ConvertW<wchar_t> >           CSimpleIniW;
typedef CSimpleIniTempl<wchar_t,
	SI_GenericCase<wchar_t>, SI_ConvertW<wchar_t> >             CSimpleIniCaseW;

#ifdef _UNICODE
# define CSimpleIni      CSimpleIniW
# define CSimpleIniCase  CSimpleIniCaseW
# define SI_NEWLINE      SI_NEWLINE_W
#else  
# define CSimpleIni      CSimpleIniA
# define CSimpleIniCase  CSimpleIniCaseA
# define SI_NEWLINE      SI_NEWLINE_A
#endif  

#ifdef _MSC_VER
# pragma warning (pop)
#endif