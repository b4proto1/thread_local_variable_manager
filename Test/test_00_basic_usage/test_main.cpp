// Test_00_BasicUsage.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "../../Source/thread_local_variable_manager.h"


struct CTest1
{
public:
	CTest1(int pVal1, double pVal2)
	{
		m_Val1 = pVal1;
		m_Val2 = pVal2;
		std::cout << "CTest1 : ctor : 01 : " << m_Val1 << std::endl;
	}
	~CTest1()
	{
		std::cout << "CTest1 : dtor : " << m_Val1 << std::endl;
	}
public:
	int m_Val1;
	double m_Val2;
};


struct CTest2
{
public:
	CTest2()
		: m_Val5(0)
	{
		std::cout << "CTest2 : ctor : 00" << std::endl;
	}
	CTest2(int pVal5)
	{
		m_Val5 = pVal5;
		std::cout << "CTest2 : ctor : 01 : " << m_Val5 << std::endl;
	}
	~CTest2()
	{
		std::cout << "CTest2 : dtor : " << m_Val5 << std::endl;
	}
public:
	int m_Val5;
};


struct CTest3
{
public:
	CTest3()
		: m_Val6(0)
	{
		std::cout << "CTest3 : ctor : 00" << std::endl;
	}
	~CTest3()
	{
		std::cout << "CTest3 : dtor : " << m_Val6 << std::endl;
	}
public:
	int m_Val6;
};


struct CTest4
{
public:
	CTest4()
	{
		std::cout << "CTest4 : ctor" << std::endl;
	}
	~CTest4()
	{
		std::cout << "CTest4 : dtor : " << m_Test3.m_Val6 << std::endl;
	}
public:
	CTest3 m_Test3;		// ※ 멤버 변수의 생성자도 호출되는지 확인 필요
};


struct CTest5
{
public:
	CTest5()
	{
		std::cout << "CTest5 : ctor" << std::endl;
	}
	~CTest5()
	{
		std::cout << "CTest5 : dtor" << std::endl;
	}
public:
	CTest3 m_Test3[10];		// ※ 멤버 변수의 생성자도 호출되는지 확인 필요
};


int main()
{
	{	// 테스트 : 기본 형식 사용
//		thread_local_variable_auto<int> aVal01;				// ok
//		thread_local_variable_auto<int> aVal01(100);		// ok
		thread_local_variable_auto<int> aVal01 = 100;		// ok

		if (!aVal01) {	// TLVA 변수 자체의 유효성 검사
			std::cout << "aVal01 is invalid!" << std::endl;
		}
		else {
			std::cout << "aVal01 : " << (*aVal01) << std::endl;

			(*aVal01) += 256;

			std::cout << "aVal01 : " << (*aVal01) << std::endl;
		}
	}
	std::cout << "---------- line feed : " << __LINE__ << std::endl << std::endl;

	{	// 테스트 : 사용자 정의 형식 사용
//		thread_local_variable_auto<CTest1> aVal02(6345, 1524.324f);				// error	// ※ 기본 형식이 아니고 생성자에 인자(들)를 받는 경우에는 TLVA 변수가 매번 생성자 / 소멸자를 호출하는 형태로 선언 필요 (→ VCTDT = true)
		thread_local_variable_auto<CTest1, true> aVal02(6345, 1524.324f);		// ok
		assert(false == !aVal02);

		std::cout << "aVal02 : " << aVal02->m_Val1 << " / " << aVal02->m_Val2 << std::endl;

		aVal02->m_Val1 += 10000;

		std::cout << "aVal02 : " << aVal02->m_Val1 << " / " << aVal02->m_Val2 << std::endl;
	}
	std::cout << "---------- line feed : " << __LINE__ << std::endl << std::endl;

	{	// 테스트 : 사용자 정의 형식 사용
		thread_local_variable_auto<CTest2> aVal03;					// ok
//		thread_local_variable_auto<CTest2> aVal03(1000);			// error	// ※ 기본 형식이 아니고 생성자에 인자(들)를 받는 경우에는 TLVA 변수가 매번 생성자 / 소멸자를 호출하는 형태로 선언 필요 (→ VCTDT = true)
//		thread_local_variable_auto<CTest2, true> aVal03(1000);		// ok
		assert(false == !aVal03);

		std::cout << "aVal03 : " << (*aVal03).m_Val5 << std::endl;

		(*aVal03).m_Val5 += 12355;

		std::cout << "aVal03 : " << (*aVal03).m_Val5 << std::endl;
	}
	std::cout << "---------- line feed : " << __LINE__ << std::endl << std::endl;

	{	// 테스트 : std::vector 사용
		thread_local_variable_auto<std::vector<CTest2>> aVal04;				// ok
		assert(false == !aVal04);

		aVal04->reserve(64);	// ※ std::vector 에 원소 추가 시 저장 공간 고갈로 인한 재할당을 방지하기 위함

		for (auto idx = 0 ; 3 > idx ; ++idx) {
//		for (auto idx = 0 ; 100 > idx ; ++idx) {
			aVal04->emplace_back(CTest2(idx + 3));
		}

//		thread_local_variable_auto<std::vector<CTest2>> aVal05(*aVal04);					// error	// ※ 기본 형식이 아니고 생성자에 인자(들)를 받는 경우에는 TLVA 변수가 매번 생성자 / 소멸자를 호출하는 형태로 선언 필요 (→ VCTDT = true)
//		thread_local_variable_auto<std::vector<CTest2>, true> aVal05(*aVal04);				// ok		// ※ std::vector 가 파라메터로 넘겨질 때 임시 변수가 생성되므로 지양 필요
		thread_local_variable_auto<std::vector<CTest2>, true> aVal05(std::cref(*aVal04));	// ok
		assert(false == !aVal05);

		for (const auto& elem : (*aVal05)) {
			std::cout << "aVal05 : elem : " << elem.m_Val5 << std::endl;
		}

		aVal04->clear();
//		aVal05->clear();	// ※ TLVA 변수를 'VCTDT = true' 로 생성했으므로 이렇게 별도로 초기화를 하지 않아도 해당 TLVA 변수의 생명 주기가 끝나는 시점에 소멸자 호출로 인해 할당받은 자원(들)도 해제됨
	}
	std::cout << "---------- line feed : " << __LINE__ << std::endl << std::endl;

	{	// 테스트 : std::map 사용
		thread_local_variable_auto<std::map<int, CTest3>> aVal06;		// ok
		assert(false == !aVal06);

		for (auto idx = 0 ; 100 > idx ; idx += 3) {
			auto aResult = aVal06->insert(std::make_pair(idx, CTest3()));
			if (aResult.second == true) {
				(*aResult.first).second.m_Val6 = idx + 1000;
			}
		}

		for (auto idx = 0 ; 100 > idx ; idx += 2) {
			auto aIt = aVal06->find(idx);
			if (aVal06->end() != aIt) {
				std::cout << "aVal06 : elem : " << idx << " / " << aIt->second.m_Val6 << std::endl;
			}
		}
	}
	std::cout << "---------- line feed : " << __LINE__ << std::endl << std::endl;

	{	// 테스트 : 사용자 정의 형식 내의 다른 사용자 형식 멤버 변수들의 초기화 여부
		thread_local_variable_auto<CTest4> aVal07;
		assert(false == !aVal07);

		aVal07->m_Test3.m_Val6 = 100;
		std::cout << "aVal07 : m_Test3 : " << (*aVal07).m_Test3.m_Val6 << std::endl;
	}
	std::cout << "---------- line feed : " << __LINE__ << std::endl << std::endl;

	{	// 테스트 : 배열 형식 사용
//		thread_local_variable_auto<int[10]> aVal08;					// error		// ※ 배열 형식은 사용할 수 없으며, 대신 std::array (혹은 std::vector) 등으로 교체를 권장
		thread_local_variable_auto<std::array<int, 10>> aVal08;		// ok
		assert(false == !aVal08);

		for (auto idx = 0; static_cast<int>(aVal08->size()) > idx; ++idx) {
			(*aVal08)[idx] = idx * 30;
		}

		for (auto elem : (*aVal08)) {
			std::cout << "aVal08 : elem : " << elem << std::endl;
		}
	}
	std::cout << "---------- line feed : " << __LINE__ << std::endl << std::endl;

	{	// 테스트 : 사용자 형식 내의 배열 멤버 선언 / 접근
		thread_local_variable_auto<CTest5> aVal09;
		assert(false == !aVal09);

		int idx = 4;

		aVal09->m_Test3[idx].m_Val6 = 5143597;

		std::cout << "aVal09 : m_Test3 [" << idx << "] : " << aVal09->m_Test3[idx].m_Val6 << std::endl;
	}
	std::cout << "---------- line feed : " << __LINE__ << std::endl << std::endl;
	
	std::cout << "---------- test over : " << __LINE__ << std::endl << std::endl;
	
//	thread_local_variable_manager::clear();		// 최종 호출	// ※ 명시적인 자원 해제

    return 0;
}

