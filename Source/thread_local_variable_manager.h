#pragma once


/**
	@mainpage Thread Local Variable Manager
	@section	INTRO	소개
	- 소개		:	Thread Local Variable Manager (이하 TLVM) 은 스레드 내에서 수시로 생성 / 해제되는 임시 변수들의 부하 절감 등을 목적으로 하는 메모리 풀 입니다.
	@section	CREATEINFO	작성 정보
	- 작성자	:	b4nfter
	- 작성일	:	2020/02/27
	*/


/**
	@file		thread_local_variable_manager.h
	@brief		TLVM 기능에 연관된 클래스들을 선언
	*/


#include <cstdlib>								// std::malloc, std::free, std::aligned_alloc (C++17)
//	#include <cstddef>							// std::size_t, std::ptrdiff_t
#include <new>									// new[], delete[], std::nothrow
#include <map>									// std::map
#include <list>									// std::list
#include <queue>								// std::queue
#include <array>								// std::array
#include <algorithm>							// std::min, std::max
#include <thread>								// std::thread
#include <limits>								// std::numericr_limits
#include <type_traits>							// std::is_fundamental, std::is_array
#include <cassert>								// assert


/**
	@brief		TLVM / 형식 요소 별로 특정 스레드 마다 할당하는 메모리 풀 (기반 클래스)
	@remark		사용자는 본 클래스를 직접적으로 사용하게 될 경우는 없음
	*/
class thread_local_variable_type_pool_base
{
public:
	thread_local_variable_type_pool_base() = default;

public:
	void* construct(size_t pSize, bool pDoCallCtorDtorAlways, size_t pReserve, /*out*/ bool& pIsJustMade);
	void destroy(void* pData, bool pDoCallCtorDtorAlways);

	virtual void clear();

protected:
	std::list<void*> m_List;			//!< 원본 메모리 목록 관리
	std::queue<void*> m_Pool_Obj;		//!< 형식 메모리 목록 관리 : 생성자를 최초 호출 후에 소멸자는 앱 종료 시에만 호출 → 기본 생성자가 호출된 객체 상태
	std::queue<void*> m_Pool_Raw;		//!< 형식 메모리 목록 관리 : 생성자 / 소멸자를 매번 호출 → 메모리 청크 상태

};


/**
	@brief		TLVM / 형식 요소 별로 특정 스레드 마다 할당하는 메모리 풀
	@remark		사용자는 본 클래스를 직접적으로 사용하게 될 경우는 없음
	*/
template<typename TTYPE>
class thread_local_variable_type_pool final
	: public thread_local_variable_type_pool_base
{
	static_assert(sizeof(TTYPE) > 0,	"Invalid!");	// ※ empty type 에는 적용 불가

public:
	using base_type = thread_local_variable_type_pool_base;
	using value_type = TTYPE;

public:
	TTYPE* construct(bool pDoCallCtorDtorAlways, size_t pReserve, /*out*/ bool& pIsJustMade);
	void destroy(TTYPE* pData, bool pDoCallCtorDtorAlways);

	virtual void clear();

};


/**
	@brief		TLVM 에서 특정 형식을 관리하기 위한 형식 요소 클래스 (기반 클래스)
	@details	TLVM 가 특정 형식을 관리하는 기준
	@remark		사용자는 본 클래스를 직접적으로 사용하게 될 경우는 없음
	*/
class thread_local_variable_type_base
{
private:
	friend class thread_local_variable_manager;

public:
	thread_local_variable_type_base() = default;

private:
	void clear();
	
protected:
	std::map<std::thread::id, thread_local_variable_type_pool_base*> m_List;

};


/**
	@brief		TLVM 에서 특정 형식을 관리하기 위한 형식 요소 클래스
	@remark		사용자는 본 클래스를 직접적으로 사용하게 될 경우는 없음
	*/
template<typename TTYPE>
class thread_local_variable_type_elem final
	: public thread_local_variable_type_base
{
	static_assert(sizeof(TTYPE) > 0,	"Invalid!");	// ※ empty type 에는 적용 불가

private:
	friend class thread_local_variable_manager;

public:
	using base_type = thread_local_variable_type_base;
	using value_type = TTYPE;

private:
	TTYPE* construct(bool pDoCallCtorDtorAlways, size_t pReserve, /*out*/ bool& pIsJustMade);
	void destroy(std::thread::id const& pThreadId, TTYPE* pData, bool pDoCallCtorDtorAlways);

public:
	static constexpr size_t aligned_sizeof()
	{
		return ((sizeof(TTYPE) / alignof(max_align_t) * alignof(max_align_t)) + alignof(max_align_t));
	}

};


/**
	@brief		TLVM 를 통해 사용자가 지역 변수를 대체할 목적으로 사용 (이하 TLVA)
	@details	사용자는 본 클래스 객체 (TLVA 변수) 를 생성 / 사용하는 것으로 TLVM 을 실질적으로 사용
	@tparam		TTYPE	사용자가 (지역 변수 대신) TLVA 로 생성하기를 원하는 데이터 형식
	@tparam		VCTDT	TLVA 변수의 생명 주기의 시작과 끝에 항상 생성자 / 소멸자를 호출하기를 원하는지 여부 (true : 일반적인 지역 변수와 동일하게 동작 / false : 해당 데이터 형식의 소멸자는 프로세스 종료 시에만 호출)
	@tparam		VRSSZ	TLVA 변수 생성 시 내부적으로 관리하는 메모리 풀의 자원이 고갈 시 마다 생성할 메모리 청크의 개수
	*/
template<typename TTYPE, bool VCTDT = false, size_t VRSSZ = 1>
class thread_local_variable_auto
{
	static_assert(sizeof(TTYPE) > 0,						"Invalid!");	// ※ empty type 에는 적용 불가
	static_assert(std::is_array<TTYPE>::value == false,		"Invalid!");	// ※ array type 에는 적용 불가

public:
	using value_type = TTYPE;

public:
	/**
		@brief	TLVM 이 특정 데이터 형식에 대해서 할당한 메모리를 관리하며 사용자가 접근할 수 있도록 중계 기능 제공
		*/
	struct data
	{
	private:
		friend thread_local_variable_auto;

	public:
		data();
		data(std::thread::id const& pThreadId, TTYPE* pData, bool pDoCallCtorDtorAlways);
		data(data&& pRv);

		const TTYPE* get() const
		{
			return m_Data;
		}

		TTYPE* get()
		{
#if defined(DEBUG) || defined(_DEBUG)
			assert(true == is_initialized() && std::this_thread::get_id() == m_ThreadId);
#endif	// DEBUG || _DEBUG
			return const_cast<TTYPE*>(static_cast<const data*>(this)->get());
		}

		bool is_initialized() const noexcept
		{
			return (nullptr != m_Data);
		}

	private:
		void reset()
		{
			m_Data = nullptr;
		}

		void release();

	private:
		std::thread::id const m_ThreadId;			//!< 스레드 식별자
		TTYPE* m_Data;								//!< 형식 인스턴스
		bool m_DoCallCtorDtorAlways;				//!< 변수 회수 시에 해당 변수 형식에 대한 소멸자를 호출할지 여부
	};

public:
	/**
		@brief		생성자
		@tparam		...vargs...		사용자는 TLVA 선언 시 원래 데이터 형식을 선언할 때와 동일하게 생성자 인자를 지정 가능
		*/
	template<typename ... TARGS>
	thread_local_variable_auto(TARGS ... pArgs);
	/**
		@brief		생성자
		@tparam		pArg			데이터 형식의 단일 인자
		@remark		gcc 빌드 시 다음과 같은 컴파일 에러가 발생할 수 있으므로 인해 추가 → error: use of deleted function ‘thread_local_variable_auto<int>::thread_local_variable_auto(const thread_local_variable_auto<int>&)’
		*/
	thread_local_variable_auto(TTYPE&& pArg);
	/**
		@brief		소멸자
		*/
	~thread_local_variable_auto();

public:
	/**
		@brief		TLVA 객체가 유효한지를 확인하는 용도
		@return		TLVA 객체가 유효한지 여부 (true : 유효 / false : 사용 불가)
		*/
	bool operator!() const noexcept
	{
		return !(m_Data.is_initialized());
	}

	/**
		@brief		TLVA 객체가 가리키는 특정 데이터 형식에 대한 인스턴스에 접근하기 위한 역참조 연산자
		@return		TLVA 객체가 가리키는 특정 데이터 형식의 인스턴스의 참조 반환
		*/
	const TTYPE& operator*() const
	{
		return *(m_Data.get());
	}

	/**
		@brief		TLVA 객체가 가리키는 특정 데이터 형식에 대한 인스턴스에 접근하기 위한 역참조 연산자
		@return		TLVA 객체가 가리키는 특정 데이터 형식의 인스턴스의 참조 반환
		*/
	TTYPE& operator*()
	{
		return const_cast<TTYPE&>(static_cast<const thread_local_variable_auto*>(this)->operator*());
	}

	/**
		@brief		TLVA 객체가 가리키는 특정 데이터 형식에 대한 인스턴스에 접근하기 위한 포인터 연산자
		@return		TLVA 객체가 가리키는 특정 데이터 형식의 인스턴스의 포인터 반환
		*/
	const TTYPE* operator->() const
	{
		return m_Data.get();
	}

	/**
		@brief		TLVA 객체가 가리키는 특정 데이터 형식에 대한 인스턴스에 접근하기 위한 포인터 연산자
		@return		TLVA 객체가 가리키는 특정 데이터 형식의 인스턴스의 포인터 반환
		*/
	TTYPE* operator->()
	{
		return const_cast<TTYPE*>(static_cast<const thread_local_variable_auto*>(this)->operator->());
	}

private:
	data m_Data;	//!< TLVA 가 거리키는 데이터 형식의 인스턴스를 관리

};


/**
	@brief		사용자가 TLS 방식으로 지역 변수를 대체할 수 있는 기능들을 제공 / 관리  (이하 TLVM)
	@remark		사용자는 본 클래스를 직접적으로 사용하게 될 경우는 없음 (→ TLVM 대신 TLVA 를 사용)
	*/
class thread_local_variable_manager
{
public:
	enum EF_CONST
	{
		EV_CONST_RESERVE_SIZE_MIN
			= 1,
		EV_CONST_RESERVE_SIZE_MAX
			= std::numeric_limits<int>::max(),
	};

public:
	/**
		@brief		특정 데이터 형식에 대한 객체 생성 기능
		@warning	사용자는 이 함수를 직접 호출하지 말고, TLVA 객체를 대신 사용 필요
		*/
	template<typename TTYPE, bool VCTDT = false, size_t VRSSZ = 1, typename ... TARGS>
	inline static typename thread_local_variable_auto<TTYPE, VCTDT, VRSSZ>::data _construct(TARGS ... pArgs);

	/**
		@brief		특정 데이터 형식에 대한 객체 사용한 후에 반환
		@warning	사용자는 이 함수를 직접 호출하지 말고, TLVA 객체를 대신 사용 필요
		*/
	template<typename TTYPE>
	inline static void _destroy(std::thread::id const& pThreadId, TTYPE* pData, bool pDoCallCtorDtorAlways /*= false*/);

private:
	template<typename TTYPE>
	inline static thread_local_variable_type_elem<TTYPE>* _get_type();

public:
	/**
		@brief		TLVM 이 관리하는 모든 자원을 반환
		@details	자원 반환 시 데이터 형식 인스턴스에 대한 메모리 해제를 진행하기 전에 소멸자 호출이 필요하다면 호출 수행
		@warning	(가급적) 프로세스 종료 직전에 호출 필요
		*/
	static void clear();

};


#include "thread_local_variable_manager.inl"

