//---------------------------------------------------------------------------

#ifndef UnitLogManagerH
#define UnitLogManagerH
//---------------------------------------------------------------------------

#include <System.Classes.hpp>

class TLogManager
{
public:
	TLogManager();
	~TLogManager();

	void OpenLog();
	bool Append(const wchar_t* format, ...);
	String GetEntry(unsigned int uIndex);
	String GetLastEntry();
	bool FlushToDisk();
	void CloseLog();

	__property String LogFile = { read = m_strLogFile, write = m_strLogFile };
	__property unsigned int MaximumEntries = { read = m_uMaxEntries, write = SetMaxEntries };

private:
	static const unsigned int m_uDefaultMaxEntries;
	static const size_t m_nDefaultEntrySize;

	String m_strLogFile;
	unsigned int m_uMaxEntries;
	TStringList* m_pData;

	void SetMaxEntries(unsigned int uNewValue);
};

#endif
