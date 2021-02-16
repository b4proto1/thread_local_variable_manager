

#pragma once


#include <WINDOWS.H>
#include <TCHAR.H>
#include <STDIO.h>


class CListBox
{
public:
	class CLockAuto
	{
	public:
		CLockAuto(CRITICAL_SECTION& pLock) {
			m_Lock = &pLock;
			::EnterCriticalSection(m_Lock);
		}
		~CLockAuto() {
			::LeaveCriticalSection(m_Lock);
		}
	private:
		LPCRITICAL_SECTION m_Lock;
	};

public:
	static VOID Open();
	static VOID Close();

	static INT AddString(HWND hList, LPCTSTR lpszFormat, ...);
	static INT AddStringLastLine(HWND hList, LPCTSTR lpszFormat, ...);
	static INT SetItemData(HWND hList, INT iListIndex, LPARAM lpItemData);
	static LRESULT GetItemData(HWND hList, INT iListIndex);
	static VOID SelectLastLine(HWND hList);
	static VOID ResetContents(HWND hList);

private:
	static CRITICAL_SECTION m_Lock;

};


CRITICAL_SECTION CListBox::m_Lock;


inline VOID CListBox::Open()
{
	::InitializeCriticalSection(&m_Lock);
}


inline VOID CListBox::Close()
{
	::DeleteCriticalSection(&m_Lock);
}


inline INT CListBox::AddString(HWND hList, LPCTSTR lpszFormat, ...)
{
	if (!hList) {
		return -1;
	}

	CLockAuto aLockAuto(m_Lock);

	TCHAR cBuffer[2048];
	va_list cArgs;
	va_start(cArgs, lpszFormat);
#if (_MSC_VER >= 1400)
	_vstprintf_s(cBuffer, _countof(cBuffer), lpszFormat, cArgs);
#else
	#pragma warning(disable:4995)
	_vstprintf(cBuffer, lpszFormat, cArgs);
	#pragma warning(default:4995)
#endif 
	va_end(cArgs);

	return (INT)::SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)cBuffer);
}


inline INT CListBox::AddStringLastLine(HWND hList, LPCTSTR lpszFormat, ...)
{
	if (!hList) {
		return -1;
	}

	CLockAuto aLockAuto(m_Lock);

	TCHAR cBuffer[2048];
	va_list cArgs;
	va_start(cArgs, lpszFormat);
#if (_MSC_VER >= 1400)
	_vstprintf_s(cBuffer, _countof(cBuffer), lpszFormat, cArgs);
#else
	#pragma warning(disable:4995)
	_vstprintf(cBuffer, lpszFormat, cArgs);
	#pragma warning(default:4995)
#endif 
	va_end(cArgs);

	INT iRetVal = (INT)::SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)cBuffer);
	::SendMessage(hList, LB_SETCURSEL, (WPARAM)iRetVal, 0);
	::SendMessage(hList, LB_SETCURSEL, (WPARAM)-1, 0);
	return iRetVal;
}


inline INT CListBox::SetItemData(HWND hList, INT iListIndex, LPARAM lpItemData)
{
	return (INT)::SendMessage(hList, LB_SETITEMDATA, (WPARAM)iListIndex, lpItemData);
}


inline LRESULT CListBox::GetItemData(HWND hList, INT iListIndex)
{
	return (LRESULT)::SendMessage(hList, LB_GETITEMDATA, (WPARAM)iListIndex, (LPARAM)0);
}


inline VOID CListBox::SelectLastLine(HWND hList)
{
	if (!hList) {
		return;
	}

	CLockAuto aLockAuto(m_Lock);

	INT iPos = (INT)::SendMessage(hList, LB_GETCOUNT, 0, 0);
	::SendMessage(hList, LB_SETCURSEL, (WPARAM)iPos-1, 0);
	::SendMessage(hList, LB_SETCURSEL, (WPARAM)-1, 0);
}


inline VOID CListBox::ResetContents(HWND hList)
{
	if (!hList) {
		return;
	}

	CLockAuto aLockAuto(m_Lock);

	::SendMessage(hList, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	::SendMessage(hList, LB_SETCURSEL, (WPARAM)0, (LPARAM)0);
}

