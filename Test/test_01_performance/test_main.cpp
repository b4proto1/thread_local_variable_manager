// Test_Performance.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "../../Source/thread_local_variable_manager.h"


#define	DF_TEST_LOOP_FOR_AVG			(10)				// 테스트 루프 횟수 : 평균 처리 시간 산출
#define	DF_TEST_LOOP_COUNT_01			(100)				// 테스트 루프 횟수 : 01
//#define	DF_TEST_LOOP_COUNT_01		(1000)				// 테스트 루프 횟수 : 01
#define	DF_TEST_LOOP_COUNT_02			(1000)				// 테스트 루프 횟수 : 02
//#define	DF_TEST_BUFFER_SIZE_01		(100000)			// 테스트 버퍼 크기 : 01
//#define	DF_TEST_BUFFER_SIZE_01		(10000)				// 테스트 버퍼 크기 : 01
#define	DF_TEST_BUFFER_SIZE_01			(1000)				// 테스트 버퍼 크기 : 01
//#define	DF_TEST_BUFFER_SIZE_02		(1024*1024*10)		// 테스트 버퍼 크기 : 02		// ※ 기존의 지역 변수 방식 (thread stack 사용) 에서는 Stack overflow 예외 발생
#define	DF_TEST_BUFFER_SIZE_02			(1024*512)			// 테스트 버퍼 크기 : 02
#define	DF_TEST_BUFFER_SIZE_03			(1024*1024*10)		// 테스트 버퍼 크기 : 03


struct CTest1
{
public:
	CTest1(int pVal2) : m_Val2(pVal2) { }
private:
	char m_Val1[DF_TEST_BUFFER_SIZE_01];
	int m_Val2;
};


struct CTest2
{
public:
	CTest2() { Reset(); }
	void Reset() { m_Val2 = m_Val1[0] = 0; }
	void SetVal2(int pVal) { m_Val2 = pVal; }
	int GetVal2() const { return m_Val2;  }
private:
	char m_Val1[DF_TEST_BUFFER_SIZE_02];
	int m_Val2;
};


struct CTest3
{
public:
	CTest3() { m_Val1 = new char[DF_TEST_BUFFER_SIZE_03]; }
	~CTest3() { if (!m_Val1) return; delete[] m_Val1; }
	void Reset() { m_Val2 = m_Val1[0] = 0; }
	void SetVal2(int pVal) { m_Val2 = pVal; }
	int GetVal2() const { return m_Val2;  }
private:
	char* m_Val1 = nullptr;
	int m_Val2;
};


void TestFunc01_01()	// std::vector - TLVM 사용
{
	thread_local_variable_auto<std::vector<CTest1>> aVec1;
	if (0 >= aVec1->capacity()) {
		aVec1->reserve(DF_TEST_LOOP_COUNT_02);
		std::cout << "TestFunc01 - reserve" << std::endl;
	}
	aVec1->clear();

	for (auto idx = 0 ; DF_TEST_LOOP_COUNT_02 > idx ; ++idx) {
		aVec1->emplace_back(CTest1(idx));
//		aVec1->push_back(CTest1(idx));
	}
}

void TestFunc01_02()	// std::vector - 기존
{
	std::vector<CTest1> aVec1;
	aVec1.reserve(DF_TEST_LOOP_COUNT_02);

	for (auto idx = 0 ; DF_TEST_LOOP_COUNT_02 > idx ; ++idx) {
		aVec1.emplace_back(CTest1(idx));
//		aVec1.push_back(CTest1(idx));
	}
}


void TestFunc02_01()	// std::map - TLVM 사용
{
	thread_local_variable_auto<std::map<int, CTest1>> aMap1;
	aMap1->clear();

	for (auto idx = 0; DF_TEST_LOOP_COUNT_02 > idx; ++idx) {
		aMap1->insert(std::make_pair(idx, CTest1(idx)));
	}
}

void TestFunc02_02()	// std::map - 기존
{
	std::map<int, CTest1> aMap1;

	for (auto idx = 0; DF_TEST_LOOP_COUNT_02 > idx; ++idx) {
		aMap1.insert(std::make_pair(idx, CTest1(idx)));
	}
}


int64_t TestFunc03_01()		// CTest2 지역 변수 - TLVM 사용
{
	int64_t aRetVal = 0;

	for (auto idx = 0; DF_TEST_LOOP_COUNT_02 > idx; ++idx) {
		thread_local_variable_auto<CTest2> aTest2;
		aTest2->Reset();
		aTest2->SetVal2(idx + 25);
		aRetVal += aTest2->GetVal2();
	}
	
	return aRetVal;
}

int64_t TestFunc03_02()		// CTest2 지역 변수 - 기존
{
	int64_t aRetVal = 0;

	for (auto idx = 0; DF_TEST_LOOP_COUNT_02 > idx; ++idx) {
		CTest2 aTest2;
		aTest2.Reset();
		aTest2.SetVal2(idx + 25);
		aRetVal += aTest2.GetVal2();
	}
	
	return aRetVal;
}


int64_t TestFunc04_01()		// CTest3 지역 변수 - TLVM 사용
{
	int64_t aRetVal = 0;

	for (auto idx = 0; DF_TEST_LOOP_COUNT_02 > idx; ++idx) {
		thread_local_variable_auto<CTest3> aTest3;
		aTest3->Reset();
		aTest3->SetVal2(idx + 25);
		aRetVal += aTest3->GetVal2();
	}
	
	return aRetVal;
}

int64_t TestFunc04_02()		// CTest3 지역 변수 - 기존
{
	int64_t aRetVal = 0;

	for (auto idx = 0; DF_TEST_LOOP_COUNT_02 > idx; ++idx) {
		CTest3 aTest3;
		aTest3.Reset();
		aTest3.SetVal2(idx + 25);
		aRetVal += aTest3.GetVal2();
	}
	
	return aRetVal;
}


int main()
{
	{	// 테스트
		uint64_t dur_time_sum = 0;

		for (auto idx1 = 0; DF_TEST_LOOP_FOR_AVG > idx1; ++idx1) {
			auto stx_time = std::chrono::high_resolution_clock::now();

			for (auto idx2 = 0; DF_TEST_LOOP_COUNT_01 > idx2; ++idx2) {
// 				TestFunc01_01();	// std::vector - TLVM 사용
// 				TestFunc01_02();	// std::vector - 기존

//				TestFunc02_01();	// std::map - TLVM 사용
//				TestFunc02_02();	// std::map - 기존

//				TestFunc03_01();	// CTest2 지역 변수 - TLVM 사용
//				TestFunc03_02();	// CTest2 지역 변수 - 기존

				TestFunc04_01();	// CTest3 지역 변수 - TLVM 사용
//				TestFunc04_02();	// CTest3 지역 변수 - 기존
			}

			auto dur_time = std::chrono::high_resolution_clock::now() - stx_time;
			uint64_t dur_time_gap = (dur_time.count() / 1000000);	// ns → ms 로 변환
			dur_time_sum += dur_time_gap;

			std::cout << "elapsed time : " << dur_time_gap << std::endl;
		}

		std::cout << "avarage elapsed time : " << (dur_time_sum / DF_TEST_LOOP_FOR_AVG) << std::endl;
	}

//	thread_local_variable_manager::clear();		// 최종 호출	// ※ 명시적인 자원 해제

	return 0;
}

