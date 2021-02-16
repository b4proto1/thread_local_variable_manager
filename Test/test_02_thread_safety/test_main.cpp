// Test.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Test.h"
#include "ListBox.hpp"

#include "../../Source/thread_local_variable_manager.h"


#define	DF_DEFAULT_SCREENPADDING			(15)
#define	DF_DEFAULT_SCREENPADDING_LIST_Y		(95)

#define	DF_DEFAULT_TEST_THREAD_COUNT		(8)			// (기본) 테스트 스레드 개수
#define	DF_DEFAULT_TEST_LOOP_COUNT_01		(1000)		// (기본) 테스트 루프 횟수
#define	DF_DEFAULT_TEST_LOOP_COUNT_02		(1000)		// (기본) 테스트 루프 횟수


HINSTANCE g_hInstance;
HWND g_hMainDlg;


struct CTest1
{
public:
	CTest1(int pVal2) : m_Val2(pVal2) { }
private:
	char m_Val1[450];
	int m_Val2;
};

struct CTest2
{
public:
	CTest2(int pVal2) : m_Val2(pVal2) { }
private:
	char m_Val1[550];
	int m_Val2;
};


INT_PTR CALLBACK MainDlgProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM /*lParam*/);

bool FetchParam(__out int& pThreadCount, __out int& pFuncCallCount);

void TestFunc01(int pThreadCount, int pFuncCallCount);	// std::vector - TLVM 사용
void TestFunc02(int pThreadCount, int pFuncCallCount);	// std::vector - 기존


int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow
)
{
	g_hInstance = hInstance;

	DialogBoxW(hInstance, MAKEINTRESOURCE(IDD_DLG_MAIN), HWND_DESKTOP, MainDlgProc);

	return 0;
}


INT_PTR CALLBACK MainDlgProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM /*lParam*/)
{
	switch(uMessage) {
	case WM_INITDIALOG:
		{
			g_hMainDlg = hWnd;

			CListBox::Open();

			::SetDlgItemInt(hWnd, IDC_EDIT_THREAD_COUNT, DF_DEFAULT_TEST_THREAD_COUNT, FALSE);
			::SetDlgItemInt(hWnd, IDC_EDIT_FUNC_CALL_COUNT, DF_DEFAULT_TEST_LOOP_COUNT_01, FALSE);

			::SendMessage(hWnd, WM_SIZE, (WPARAM)(0), (LPARAM)(0));
		}
		return TRUE;
	case WM_CLOSE:
		{
//			thread_local_variable_manager::clear();		// 최종 호출	// ※ 명시적인 자원 해제

			CListBox::Close();

			::EndDialog(hWnd, 0);
		}
		return TRUE;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam)) {
			case IDOK:
			case IDCANCEL:
				return TRUE;
			case IDC_BUTTON_TEST1:		// 테스트 : TLVM
				{
					CListBox::AddStringLastLine(GetDlgItem(g_hMainDlg, IDC_LIST_MAIN), _T("Test : TLVM"));

					int aThreadCount = 0;
					int aFuncCallCount = 0;
					if (false == FetchParam(__out aThreadCount, __out aFuncCallCount)) {
						::MessageBoxW(hWnd, L"Invalid Parameter(s)", L"ERROR", MB_OK | MB_ICONERROR);
						return TRUE;
					}

					auto stx_time = std::chrono::high_resolution_clock::now();

					TestFunc01(aThreadCount, aFuncCallCount);

					auto dur_time = std::chrono::high_resolution_clock::now() - stx_time;

					CListBox::AddStringLastLine(GetDlgItem(g_hMainDlg, IDC_LIST_MAIN), _T("elapsed time : %I64d"), (dur_time.count() / 1000000));
				}
				return TRUE;
			case IDC_BUTTON_TEST2:		// 테스트 : 기존
				{
					CListBox::AddStringLastLine(GetDlgItem(g_hMainDlg, IDC_LIST_MAIN), _T("Test : Common"));
					
					int aThreadCount = 0;
					int aFuncCallCount = 0;
					if (false == FetchParam(__out aThreadCount, __out aFuncCallCount)) {
						::MessageBoxW(hWnd, L"Invalid Parameter(s)", L"ERROR", MB_OK | MB_ICONERROR);
						return TRUE;
					}

					auto stx_time = std::chrono::high_resolution_clock::now();

					TestFunc02(aThreadCount, aFuncCallCount);

					auto dur_time = std::chrono::high_resolution_clock::now() - stx_time;

					CListBox::AddStringLastLine(GetDlgItem(g_hMainDlg, IDC_LIST_MAIN), _T("elapsed time : %I64d"), (dur_time.count() / 1000000));
				}
				return TRUE;
			case IDC_BUTTON_LIST_CLEAR:
				{
					CListBox::ResetContents(GetDlgItem(g_hMainDlg, IDC_LIST_MAIN));
				}
				return TRUE;
			}
		}
		break;
	case WM_SIZE:
		{
			RECT stRect;
			::GetClientRect(hWnd, &stRect);
			::MoveWindow(
				::GetDlgItem(hWnd, IDC_LIST_MAIN),
				stRect.left+DF_DEFAULT_SCREENPADDING, 
				stRect.top+DF_DEFAULT_SCREENPADDING+DF_DEFAULT_SCREENPADDING_LIST_Y,
				stRect.right-DF_DEFAULT_SCREENPADDING*2,
				stRect.bottom-DF_DEFAULT_SCREENPADDING*2- DF_DEFAULT_SCREENPADDING_LIST_Y,
				TRUE
			);
		}
		break;
	}

	return FALSE;
}


bool FetchParam(__out int& pThreadCount, __out int& pFuncCallCount)
{
	BOOL aIsSuccess = FALSE;

	pThreadCount = static_cast<int>(::GetDlgItemInt(g_hMainDlg, IDC_EDIT_THREAD_COUNT, &aIsSuccess, TRUE));
	if (FALSE == aIsSuccess || 0 >= pThreadCount) {
		return false;
	}

	pFuncCallCount = static_cast<int>(::GetDlgItemInt(g_hMainDlg, IDC_EDIT_FUNC_CALL_COUNT, &aIsSuccess, TRUE));
	if (FALSE == aIsSuccess || 0 >= pFuncCallCount)	{
		return false;
	}

	return true;
}


void TestFunc01(int pThreadCount, int pFuncCallCount)	// std::vector - TLVM 사용
{
	for (auto idx0 = 0; pThreadCount > idx0; ++idx0) {
		std::thread aThread(
			[pFuncCallCount]()
			{
				thread_local_variable_auto<std::vector<CTest1>> aVec1;
				if (0 >= aVec1->capacity()) {
					aVec1->reserve(pFuncCallCount);
				}
				aVec1->clear();

				thread_local_variable_auto<std::vector<CTest2>> aVec2;
				if (0 >= aVec2->capacity()) {
					aVec2->reserve(pFuncCallCount);
				}
				aVec2->clear();

				for (auto idx1 = 0; pFuncCallCount > idx1; ++idx1) {
					{	// 상단에서 선언된 변수
						aVec1->push_back(CTest1(idx1));
						aVec2->push_back(CTest2(idx1));
					}

					thread_local_variable_auto<std::vector<CTest1>> aVec1;
//					auto aVec1 = thread_local_variable_manager::construct<std::vector<CTest1>>();
					if (0 >= aVec1->capacity()) {
						aVec1->reserve(DF_DEFAULT_TEST_LOOP_COUNT_02);
					}
					aVec1->clear();

					thread_local_variable_auto<std::vector<CTest2>> aVec2;
//					auto aVec2 = thread_local_variable_manager::construct<std::vector<CTest2>>();
					if (0 >= aVec2->capacity()) {
						aVec2->reserve(DF_DEFAULT_TEST_LOOP_COUNT_02);
					}
					aVec2->clear();

					for (auto idx2 = 0; DF_DEFAULT_TEST_LOOP_COUNT_02 > idx2; ++idx2) {
						aVec1->push_back(CTest1(idx2));
						aVec2->push_back(CTest2(idx2));
					}
				}
			}
		);

		aThread.join();
	}
}


void TestFunc02(int pThreadCount, int pFuncCallCount)	// std::vector - 기존
{
	for (auto idx0 = 0; pThreadCount > idx0; ++idx0) {
		std::thread aThread(
			[pFuncCallCount]()
			{
				std::vector<CTest1> aVec1;
				aVec1.reserve(pFuncCallCount);

				std::vector<CTest2> aVec2;
				aVec2.reserve(pFuncCallCount);

				for (auto idx1 = 0; pFuncCallCount > idx1; ++idx1) {
					{	// 상단에서 선언된 변수
						aVec1.push_back(CTest1(idx1));
						aVec2.push_back(CTest2(idx1));
					}

					std::vector<CTest1> aVec1;
					aVec1.reserve(DF_DEFAULT_TEST_LOOP_COUNT_02);

					std::vector<CTest2> aVec2;
					aVec2.reserve(DF_DEFAULT_TEST_LOOP_COUNT_02);

					for (auto idx2 = 0; DF_DEFAULT_TEST_LOOP_COUNT_02 > idx2; ++idx2) {
						aVec1.push_back(CTest1(idx2));
						aVec2.push_back(CTest2(idx2));
					}
				}
			}
		);

		aThread.join();
	}
}

