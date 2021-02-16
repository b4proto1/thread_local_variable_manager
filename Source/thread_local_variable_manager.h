#pragma once


/**
	@mainpage Thread Local Variable Manager
	@section	INTRO	�Ұ�
	- �Ұ�		:	Thread Local Variable Manager (���� TLVM) �� ������ ������ ���÷� ���� / �����Ǵ� �ӽ� �������� ���� ���� ���� �������� �ϴ� �޸� Ǯ �Դϴ�.
	@section	CREATEINFO	�ۼ� ����
	- �ۼ���	:	b4nfter
	- �ۼ���	:	2020/02/27
	*/


/**
	@file		thread_local_variable_manager.h
	@brief		TLVM ��ɿ� ������ Ŭ�������� ����
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
	@brief		TLVM / ���� ��� ���� Ư�� ������ ���� �Ҵ��ϴ� �޸� Ǯ (��� Ŭ����)
	@remark		����ڴ� �� Ŭ������ ���������� ����ϰ� �� ���� ����
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
	std::list<void*> m_List;			//!< ���� �޸� ��� ����
	std::queue<void*> m_Pool_Obj;		//!< ���� �޸� ��� ���� : �����ڸ� ���� ȣ�� �Ŀ� �Ҹ��ڴ� �� ���� �ÿ��� ȣ�� �� �⺻ �����ڰ� ȣ��� ��ü ����
	std::queue<void*> m_Pool_Raw;		//!< ���� �޸� ��� ���� : ������ / �Ҹ��ڸ� �Ź� ȣ�� �� �޸� ûũ ����

};


/**
	@brief		TLVM / ���� ��� ���� Ư�� ������ ���� �Ҵ��ϴ� �޸� Ǯ
	@remark		����ڴ� �� Ŭ������ ���������� ����ϰ� �� ���� ����
	*/
template<typename TTYPE>
class thread_local_variable_type_pool final
	: public thread_local_variable_type_pool_base
{
	static_assert(sizeof(TTYPE) > 0,	"Invalid!");	// �� empty type ���� ���� �Ұ�

public:
	using base_type = thread_local_variable_type_pool_base;
	using value_type = TTYPE;

public:
	TTYPE* construct(bool pDoCallCtorDtorAlways, size_t pReserve, /*out*/ bool& pIsJustMade);
	void destroy(TTYPE* pData, bool pDoCallCtorDtorAlways);

	virtual void clear();

};


/**
	@brief		TLVM ���� Ư�� ������ �����ϱ� ���� ���� ��� Ŭ���� (��� Ŭ����)
	@details	TLVM �� Ư�� ������ �����ϴ� ����
	@remark		����ڴ� �� Ŭ������ ���������� ����ϰ� �� ���� ����
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
	@brief		TLVM ���� Ư�� ������ �����ϱ� ���� ���� ��� Ŭ����
	@remark		����ڴ� �� Ŭ������ ���������� ����ϰ� �� ���� ����
	*/
template<typename TTYPE>
class thread_local_variable_type_elem final
	: public thread_local_variable_type_base
{
	static_assert(sizeof(TTYPE) > 0,	"Invalid!");	// �� empty type ���� ���� �Ұ�

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
	@brief		TLVM �� ���� ����ڰ� ���� ������ ��ü�� �������� ��� (���� TLVA)
	@details	����ڴ� �� Ŭ���� ��ü (TLVA ����) �� ���� / ����ϴ� ������ TLVM �� ���������� ���
	@tparam		TTYPE	����ڰ� (���� ���� ���) TLVA �� �����ϱ⸦ ���ϴ� ������ ����
	@tparam		VCTDT	TLVA ������ ���� �ֱ��� ���۰� ���� �׻� ������ / �Ҹ��ڸ� ȣ���ϱ⸦ ���ϴ��� ���� (true : �Ϲ����� ���� ������ �����ϰ� ���� / false : �ش� ������ ������ �Ҹ��ڴ� ���μ��� ���� �ÿ��� ȣ��)
	@tparam		VRSSZ	TLVA ���� ���� �� ���������� �����ϴ� �޸� Ǯ�� �ڿ��� �� �� ���� ������ �޸� ûũ�� ����
	*/
template<typename TTYPE, bool VCTDT = false, size_t VRSSZ = 1>
class thread_local_variable_auto
{
	static_assert(sizeof(TTYPE) > 0,						"Invalid!");	// �� empty type ���� ���� �Ұ�
	static_assert(std::is_array<TTYPE>::value == false,		"Invalid!");	// �� array type ���� ���� �Ұ�

public:
	using value_type = TTYPE;

public:
	/**
		@brief	TLVM �� Ư�� ������ ���Ŀ� ���ؼ� �Ҵ��� �޸𸮸� �����ϸ� ����ڰ� ������ �� �ֵ��� �߰� ��� ����
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
		std::thread::id const m_ThreadId;			//!< ������ �ĺ���
		TTYPE* m_Data;								//!< ���� �ν��Ͻ�
		bool m_DoCallCtorDtorAlways;				//!< ���� ȸ�� �ÿ� �ش� ���� ���Ŀ� ���� �Ҹ��ڸ� ȣ������ ����
	};

public:
	/**
		@brief		������
		@tparam		...vargs...		����ڴ� TLVA ���� �� ���� ������ ������ ������ ���� �����ϰ� ������ ���ڸ� ���� ����
		*/
	template<typename ... TARGS>
	thread_local_variable_auto(TARGS ... pArgs);
	/**
		@brief		������
		@tparam		pArg			������ ������ ���� ����
		@remark		gcc ���� �� ������ ���� ������ ������ �߻��� �� �����Ƿ� ���� �߰� �� error: use of deleted function ��thread_local_variable_auto<int>::thread_local_variable_auto(const thread_local_variable_auto<int>&)��
		*/
	thread_local_variable_auto(TTYPE&& pArg);
	/**
		@brief		�Ҹ���
		*/
	~thread_local_variable_auto();

public:
	/**
		@brief		TLVA ��ü�� ��ȿ������ Ȯ���ϴ� �뵵
		@return		TLVA ��ü�� ��ȿ���� ���� (true : ��ȿ / false : ��� �Ұ�)
		*/
	bool operator!() const noexcept
	{
		return !(m_Data.is_initialized());
	}

	/**
		@brief		TLVA ��ü�� ����Ű�� Ư�� ������ ���Ŀ� ���� �ν��Ͻ��� �����ϱ� ���� ������ ������
		@return		TLVA ��ü�� ����Ű�� Ư�� ������ ������ �ν��Ͻ��� ���� ��ȯ
		*/
	const TTYPE& operator*() const
	{
		return *(m_Data.get());
	}

	/**
		@brief		TLVA ��ü�� ����Ű�� Ư�� ������ ���Ŀ� ���� �ν��Ͻ��� �����ϱ� ���� ������ ������
		@return		TLVA ��ü�� ����Ű�� Ư�� ������ ������ �ν��Ͻ��� ���� ��ȯ
		*/
	TTYPE& operator*()
	{
		return const_cast<TTYPE&>(static_cast<const thread_local_variable_auto*>(this)->operator*());
	}

	/**
		@brief		TLVA ��ü�� ����Ű�� Ư�� ������ ���Ŀ� ���� �ν��Ͻ��� �����ϱ� ���� ������ ������
		@return		TLVA ��ü�� ����Ű�� Ư�� ������ ������ �ν��Ͻ��� ������ ��ȯ
		*/
	const TTYPE* operator->() const
	{
		return m_Data.get();
	}

	/**
		@brief		TLVA ��ü�� ����Ű�� Ư�� ������ ���Ŀ� ���� �ν��Ͻ��� �����ϱ� ���� ������ ������
		@return		TLVA ��ü�� ����Ű�� Ư�� ������ ������ �ν��Ͻ��� ������ ��ȯ
		*/
	TTYPE* operator->()
	{
		return const_cast<TTYPE*>(static_cast<const thread_local_variable_auto*>(this)->operator->());
	}

private:
	data m_Data;	//!< TLVA �� �Ÿ�Ű�� ������ ������ �ν��Ͻ��� ����

};


/**
	@brief		����ڰ� TLS ������� ���� ������ ��ü�� �� �ִ� ��ɵ��� ���� / ����  (���� TLVM)
	@remark		����ڴ� �� Ŭ������ ���������� ����ϰ� �� ���� ���� (�� TLVM ��� TLVA �� ���)
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
		@brief		Ư�� ������ ���Ŀ� ���� ��ü ���� ���
		@warning	����ڴ� �� �Լ��� ���� ȣ������ ����, TLVA ��ü�� ��� ��� �ʿ�
		*/
	template<typename TTYPE, bool VCTDT = false, size_t VRSSZ = 1, typename ... TARGS>
	inline static typename thread_local_variable_auto<TTYPE, VCTDT, VRSSZ>::data _construct(TARGS ... pArgs);

	/**
		@brief		Ư�� ������ ���Ŀ� ���� ��ü ����� �Ŀ� ��ȯ
		@warning	����ڴ� �� �Լ��� ���� ȣ������ ����, TLVA ��ü�� ��� ��� �ʿ�
		*/
	template<typename TTYPE>
	inline static void _destroy(std::thread::id const& pThreadId, TTYPE* pData, bool pDoCallCtorDtorAlways /*= false*/);

private:
	template<typename TTYPE>
	inline static thread_local_variable_type_elem<TTYPE>* _get_type();

public:
	/**
		@brief		TLVM �� �����ϴ� ��� �ڿ��� ��ȯ
		@details	�ڿ� ��ȯ �� ������ ���� �ν��Ͻ��� ���� �޸� ������ �����ϱ� ���� �Ҹ��� ȣ���� �ʿ��ϴٸ� ȣ�� ����
		@warning	(������) ���μ��� ���� ������ ȣ�� �ʿ�
		*/
	static void clear();

};


#include "thread_local_variable_manager.inl"

