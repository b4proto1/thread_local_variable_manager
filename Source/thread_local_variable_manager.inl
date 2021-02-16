#pragma once


// thread_local_variable_type_pool_base

inline void* thread_local_variable_type_pool_base::construct(size_t pSize, bool pDoCallCtorDtorAlways, size_t pReserve, /*out*/ bool& pIsJustMade)
{
//	assert		1 <= pSize
//	assert		1 <= pReserve

	if (false == pDoCallCtorDtorAlways &&
		false == m_Pool_Obj.empty()
	)
	{
		void* aChunk1 = m_Pool_Obj.front();
		m_Pool_Obj.pop();
		return aChunk1;
	}
	
	if (true == m_Pool_Raw.empty()) {
		void* aChunk2 = malloc(pSize * pReserve);
		if (nullptr == aChunk2) {
			return nullptr;
		}

		m_List.push_back(aChunk2);

		for (size_t aIdx = 0 ; pReserve > aIdx ; ++aIdx) {
			m_Pool_Raw.push(static_cast<void*>(reinterpret_cast<char*>(aChunk2) + (pSize * aIdx)));
		}
	}

	if (true == m_Pool_Raw.empty()) {
		return nullptr;
	}

	pIsJustMade = true;

	void* aChunk3 = m_Pool_Raw.front();
	m_Pool_Raw.pop();
	return aChunk3;
}

inline void thread_local_variable_type_pool_base::destroy(void* pData, bool pDoCallCtorDtorAlways)
{
//	assert		nullptr != pData

	if (nullptr == pData) {
		return;
	}

	if (true == pDoCallCtorDtorAlways) {
		m_Pool_Raw.push(pData);
	}
	else {
		m_Pool_Obj.push(pData);
	}
}

inline void thread_local_variable_type_pool_base::clear()
{
	std::queue<void*> empty;
	std::swap(m_Pool_Raw, empty);

	m_List.clear();
}


// thread_local_variable_type_pool

template<typename TTYPE>
inline TTYPE* thread_local_variable_type_pool<TTYPE>::construct(bool pDoCallCtorDtorAlways, size_t pReserve, /*out*/ bool& pIsJustMade)
{
//	assert		1 <= pReserve

	return reinterpret_cast<TTYPE*>(base_type::construct(thread_local_variable_type_elem<TTYPE>::aligned_sizeof(), pDoCallCtorDtorAlways, pReserve, pIsJustMade));
}

template<typename TTYPE>
inline void thread_local_variable_type_pool<TTYPE>::destroy(TTYPE* pData, bool pDoCallCtorDtorAlways)
{
//	assert		nullptr != pData

	base_type::destroy(pData, pDoCallCtorDtorAlways);
}

template<typename TTYPE>
inline void thread_local_variable_type_pool<TTYPE>::clear()
{
	while (m_Pool_Obj.empty() == false) {
		TTYPE* aData = reinterpret_cast<TTYPE*>(m_Pool_Obj.front());
		aData->~TTYPE();	// �� �ش� ���Ŀ� ���� �Ҹ��ڸ� ��������� ȣ��
		m_Pool_Obj.pop();
	}

	base_type::clear();
}


// thread_local_variable_type_base

inline void thread_local_variable_type_base::clear()
{
	for (auto aElem : m_List) {
		if (nullptr == aElem.second) {
			continue;
		}

		aElem.second->clear();		// ???		// TID ���� ���� ���������� ���� ?!! �� tbb �����̳��̹Ƿ� ����ȭ ������ ������ �ʰ����� ��� ���� ��ü�� ȸ���� ���� ����

		std::free(aElem.second);
	}

	m_List.clear();
}


// thread_local_variable_type_elem

template<typename TTYPE>
inline TTYPE* thread_local_variable_type_elem<TTYPE>::construct(bool pDoCallCtorDtorAlways, size_t pReserve, /*out*/ bool& pIsJustMade)
{
//	assert		1 <= pReserve
	
	auto const aThreadId = std::this_thread::get_id();
//	assert		std::thread::id() != aThreadId

	auto aIt = m_List.find(aThreadId);
	if (m_List.end() == aIt) {
		auto aPool1 = new(std::nothrow) thread_local_variable_type_pool<TTYPE>();
		if (nullptr == aPool1) {
			return nullptr;
		}

		auto aRetVal = m_List.insert(std::make_pair(aThreadId, aPool1));
		if (aRetVal.second == false) {
			return nullptr;
		}

		aIt = aRetVal.first;
	}

	auto* aPool2 = static_cast<thread_local_variable_type_pool<TTYPE>*>(aIt->second);
	auto* aData = aPool2->construct(pDoCallCtorDtorAlways, pReserve, pIsJustMade);
	if (nullptr == aData) {
		return nullptr;
	}

	return aData;
}

template<typename TTYPE>
inline void thread_local_variable_type_elem<TTYPE>::destroy(std::thread::id const& pThreadId, TTYPE* pData, bool pDoCallCtorDtorAlways)
{
//	assert		std::thread::id() != pThreadId
//	assert		nullptr != pData

	auto aIt = m_List.find(pThreadId);
	if (m_List.end() == aIt) {
//		assert
		return;
	}

	auto* aPool = static_cast<thread_local_variable_type_pool<TTYPE>*>(aIt->second);
//	assert	nullptr != aPool
	aPool->destroy(pData, pDoCallCtorDtorAlways);
}


// thread_local_variable_auto

template<typename TTYPE, bool VCTDT, size_t VRSSZ>
template<typename ... TARGS>
thread_local_variable_auto<TTYPE, VCTDT, VRSSZ>::thread_local_variable_auto(TARGS ... pArgs)
	: m_Data(thread_local_variable_manager::_construct<TTYPE, VCTDT, VRSSZ>(pArgs ...))
{
	
}

template<typename TTYPE, bool VCTDT, size_t VRSSZ>
thread_local_variable_auto<TTYPE, VCTDT, VRSSZ>::thread_local_variable_auto(TTYPE&& pArg)
	: m_Data(thread_local_variable_manager::_construct<TTYPE, VCTDT, VRSSZ>(std::forward<TTYPE>(pArg)))
{

}

template<typename TTYPE, bool VCTDT, size_t VRSSZ>
thread_local_variable_auto<TTYPE, VCTDT, VRSSZ>::~thread_local_variable_auto()
{
//	std::cout << "thread_local_variable_auto destructor" << std::endl;

	m_Data.release();
}


// thread_local_variable_auto::data

template<typename TTYPE, bool VCTDT, size_t VRSSZ>
thread_local_variable_auto<TTYPE, VCTDT, VRSSZ>::data::data()
	: m_ThreadId(std::thread::id())
	, m_Data(nullptr)
	, m_DoCallCtorDtorAlways(false)
{

}

template<typename TTYPE, bool VCTDT, size_t VRSSZ>
thread_local_variable_auto<TTYPE, VCTDT, VRSSZ>::data::data(std::thread::id const& pThreadId, TTYPE* pData, bool pDoCallCtorDtorAlways)
	: m_ThreadId(pThreadId)
	, m_Data(pData)
	, m_DoCallCtorDtorAlways(pDoCallCtorDtorAlways)
{
	// assert		std::thread::id() != pThreadId
	// assert		nullptr != pData
}

template<typename TTYPE, bool VCTDT, size_t VRSSZ>
thread_local_variable_auto<TTYPE, VCTDT, VRSSZ>::data::data(data&& pRv)
	: m_ThreadId(pRv.m_ThreadId)
	, m_Data(pRv.m_Data)
	, m_DoCallCtorDtorAlways(pRv.m_DoCallCtorDtorAlways)
{
	pRv.reset();
}

template<typename TTYPE, bool VCTDT, size_t VRSSZ>
void thread_local_variable_auto<TTYPE, VCTDT, VRSSZ>::data::release()
{
	if (nullptr == m_Data) {
		return;
	}
	
	thread_local_variable_manager::_destroy(
		m_ThreadId,
		m_Data,
		m_DoCallCtorDtorAlways
	);

	reset();
}


// thread_local_variable_manager

template<typename TTYPE, bool VCTDT, size_t VRSSZ, typename ... TARGS>
typename thread_local_variable_auto<TTYPE, VCTDT, VRSSZ>::data thread_local_variable_manager::_construct(TARGS ... pArgs)
{
	static_assert(VCTDT == true || 0 >= sizeof...(pArgs) || std::is_fundamental<TTYPE>::value == true,	"Invalid!");
	static_assert(VRSSZ >= EV_CONST_RESERVE_SIZE_MIN,	"Invalid!");
	static_assert(VRSSZ <= EV_CONST_RESERVE_SIZE_MAX,	"Invalid!");

	auto* aType = _get_type<TTYPE>();

	bool aDoCallCtorDtorAlways = (0 < sizeof...(pArgs))
		? (true)
		: (VCTDT);
	bool aIsJustMade = false;

//	auto* aElem2 = static_cast<thread_local_variable_type_elem<TTYPE>*>(aIt->second);
	auto* aData1 = aType->construct(aDoCallCtorDtorAlways, VRSSZ, aIsJustMade);
	if (nullptr == aData1) {
		return typename thread_local_variable_auto<TTYPE, VCTDT, VRSSZ>::data();
	}

	TTYPE* aData2 = nullptr;
	if (true == aDoCallCtorDtorAlways || true == aIsJustMade)
	{
		try
		{
			aData2 = new(aData1) TTYPE(pArgs ...);	// �� placement new �� noexcept �̹Ƿ� ���ܰ� �߻����� ������, TTYPE �� ������ ������ ���ܰ� �߻��� ���� ����
		}
		catch (...)
		{
			aType->destroy(std::this_thread::get_id(), aData1, true);
			return typename thread_local_variable_auto<TTYPE, VCTDT, VRSSZ>::data();
		}
	}
	else
	{
		aData2 = aData1;
	}

	if (nullptr == aData2) {
		return typename thread_local_variable_auto<TTYPE, VCTDT, VRSSZ>::data();
	}

	return typename thread_local_variable_auto<TTYPE, VCTDT, VRSSZ>::data(
		std::this_thread::get_id(),
		aData2,
		aDoCallCtorDtorAlways
	);
}

template<typename TTYPE>
void thread_local_variable_manager::_destroy(std::thread::id const& pThreadId, TTYPE* pData, bool pDoCallCtorDtorAlways)
{
//	assert		std::thread::id() != pThreadId
//	assert		nullptr != pData

	auto* aType = _get_type<TTYPE>();
//	assert		nullptr != aType

	if (true == pDoCallCtorDtorAlways) {
		pData->~TTYPE();	// �� �ش� ���Ŀ� ���� �Ҹ��ڸ� ��������� ȣ��
	}

	aType->destroy(pThreadId, pData, pDoCallCtorDtorAlways);
}

template<typename TTYPE>
thread_local_variable_type_elem<TTYPE>* thread_local_variable_manager::_get_type()
{
	static thread_local thread_local_variable_type_elem<TTYPE> aType;		// https://stackoverflow.com/questions/22794382/are-c11-thread-local-variables-automatically-static
	return &aType;
}

void thread_local_variable_manager::clear()
{


	// TODO>
		// (�ϴ�) ���� ����
		// thread_local_variable_type_elem<TTYPE> ���� �� ������ (?) ��� thread_local_variable_manager �� Ư�� (?) ��Ͽ� �߰� ������ �ʿ� ???
		// (Ȥ��) �� ������ ��� ���� ???
			// => (������) ���μ��� ���� ���� ȣ��� ��쿡�� ������ ���� ���� ??!!


	// 20201221 b4nfter		// ����
// 		for (auto aElem : m_List) {
// 			if (nullptr == aElem.second) {
// 				continue;
// 			}
// 
// 			aElem.second->clear();
// 
// 			std::free(aElem.second);
// 		}
// 
// 		m_List.clear();
}

