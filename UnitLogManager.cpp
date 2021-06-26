//---------------------------------------------------------------------------
#include "URAIDAPCH.h"
#include <IOUtils.hpp>
#pragma hdrstop

#include "UnitLogManager.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

const unsigned int TLogManager::m_uDefaultMaxEntries = 1000;
const size_t TLogManager::m_nDefaultEntrySize = 1024;

TLogManager::TLogManager() : m_uMaxEntries(m_uDefaultMaxEntries), m_pData(nullptr)
{
	//
}
//---------------------------------------------------------------------------
TLogManager::~TLogManager()
{
	if (m_pData)
		delete m_pData;
}
//---------------------------------------------------------------------------
void TLogManager::OpenLog()
{
	if (m_pData)
		delete m_pData;

	m_pData = new TStringList();
	if (!m_strLogFile.IsEmpty() && TFile::Exists(m_strLogFile))
	{
		m_pData->LoadFromFile(m_strLogFile);
	}
}
//---------------------------------------------------------------------------
bool TLogManager::Append(const wchar_t* format, ...)
{
	if (!m_pData)
		return false;

	if (m_pData->Count >= m_uMaxEntries)
		m_pData->Clear();

	va_list args;
	va_start(args, format);

	String strNewEntry, strInput;
	strInput.SetLength(m_nDefaultEntrySize);
	wvsprintfW(strInput.c_str(), format, args);
	strNewEntry.sprintf(L"[%s] %s", FormatDateTime(L"dd mmmm hh:mm:ss", Now()).c_str(),
		strInput.c_str());

	m_pData->Append(strNewEntry);

	va_end(args);

	return true;
}
//---------------------------------------------------------------------------
String TLogManager::GetEntry(unsigned int uIndex)
{
	if (!m_pData || (m_pData && (uIndex > m_pData->Count)))
		return L"";

	return m_pData->Strings[uIndex];
}
//---------------------------------------------------------------------------
String TLogManager::GetLastEntry()
{
	if (!m_pData || !m_pData->Count)
		return L"";

	return m_pData->Strings[m_pData->Count - 1];
}
//---------------------------------------------------------------------------
bool TLogManager::FlushToDisk()
{
	if (!m_pData || !m_pData->Count)
		return false;
	if (m_strLogFile.IsEmpty())
		return false;

	if (TFile::Exists(m_strLogFile))
		TFile::Delete(m_strLogFile);

	m_pData->SaveToFile(m_strLogFile);

	return true;
}
//---------------------------------------------------------------------------
void TLogManager::CloseLog()
{
	if (m_pData)
	{
		this->FlushToDisk();
		delete m_pData;
		m_pData = nullptr;
	}
}
//---------------------------------------------------------------------------
void TLogManager::SetMaxEntries(unsigned int uNewValue)
{
	m_uMaxEntries = std::max(1, static_cast<int>(uNewValue));

	if (!m_pData || !m_pData->Count)
		return;

	if (m_pData->Count >= m_uMaxEntries)
		m_pData->Clear();
}
//---------------------------------------------------------------------------

