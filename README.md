# Thread Local Variable Manager
[![Build Status](https://travis-ci.org/b4proto1/thread_local_variable_manager.svg?branch=master)](https://travis-ci.org/b4proto1/thread_local_variable_manager)

## Overview
Thread Local Variable Manager (이하 TLVM) 은 스레드 내에서 수시로 생성 / 해제되는 임시 변수들의 부하 절감 등을 목적으로 하는 메모리 풀로 아래와 같은 몇가지 특징을 가지고 있습니다.
- 기존의 지역 변수가 메모리 (stack (+ heap)) 를 매번 할당 / 반환하는데 비해, TLVM 은 메모리 풀의 형태로 할당 후에 사용이 끝나서 반환된 메모리를 필요 시 재사용함 (→ 메모리 할당 / 해제 부하 감소, 메모리 단편화 완화)
- 사용자는 오직 thread_local_variable_auto (이하 TLVA) 변수로 선언을 대체하는 것 이외에는 기존 지역 변수와 (거의) 동일한 방식으로 사용 가능
- 대부분의 데이터 형식을 사용 가능 (→ 일부 제한이 존재하며, 아래에 별도로 소개)
- 기존의 지역 변수와 마찬가지로 생성된 TLVA 변수는 생성 시의 스레드와 동일한 스레드 내에서만 사용되어야 하며, 동일한 스레드 내에서라면 생성 가능한 TLVA 변수의 개수와 위치에 제약이 없음
- TLVM 은 지정된 데이터 형식에 대한 메모리 청크를 Heap 기반으로 관리하므로 스레드 스택 (최대 약 1MB) 의 사용이 최소화됨<br>
따라서 기존에 스레드 스택의 크기 제한으로 인해 지역 변수로 선언하지 못했던 큰 사이즈의 데이터 형식들이나 스레드 스택 크기를 초과할 정도로 다수의 지역 변수 선언 시에도 기존보다 제약이 대폭 완화됨

<br>

## Quick Start
### Prerequisites:
- C++17 이상을 지원하는 C/C++ 컴파일러
  - msvc >= 19.0 (on Windows)
  - gcc >= 7.0 (on Linux)
  - clang >= 4.0 (on MacOS)

### Usage
아래는 TLVM 을 사용하는 단순한 예제로써, thread_local_variable_auto 클래스 템플릿의 첫 번째 파라메터로 원하는 형식을 지정하면 됩니다.
```cpp
#include <thread_local_variable_manager.h>

//thread_local_variable_auto<int> aVal01;		// ok
//thread_local_variable_auto<int> aVal01(100);		// ok
thread_local_variable_auto<int> aVal01 = 100;		// ok

if (!aVal01) {	// TLVA 변수 자체의 유효성 검사
	std::cout << "aVal01 is invalid!" << std::endl;
}
else {
	std::cout << "aVal01 : " << (*aVal01) << std::endl;

	(*aVal01) += 256;

	std::cout << "aVal01 : " << (*aVal01) << std::endl;
}
```

TLVM 은 C/C++ 의 기본 데이터 형식 (primitive type) 을 비롯하여 사용자 정의 형식, STL 컨테이너 등 대부분의 데이터 형식을 지정하여 사용할 수 있습니다.
```cpp
thread_local_variable_auto<std::vector<int>> aVec1;
if (0 >= aVec1->capacity()) {
	aVec1->reserve(100);
}
aVec1->clear();    // ※ TLVA 변수가 재활용된 것일 수 있으므로 초기화

for (auto idx = 0 ; 100 > idx ; ++idx) {
	aVec1->push_back(idx + 3);
}

for (auto elem : (*aVec1)) {
  std::cout << "aVec1 : elem : " << elem << std::endl;
}
```

### Interface
사용자가 직접적으로 사용하게 될 TLVA (thread_local_variable_auto) 클래스 템플릿의 인터페이스는 아래와 같습니다.<br>
자세한 내용은 [TLVM 레퍼런스 문서](Document/doxygen/html/index.html) (doxygen) 를 참고하시기 바랍니다.
  - thread_local_variable_auto
    - `TTYPE` : 사용자가 (지역 변수 대신) TLVA 로 생성하기를 원하는 데이터 형식
    - `VCTDT` : TLVA 변수의 생명 주기의 시작과 끝에 항상 생성자 / 소멸자를 호출하기를 원하는지 여부 (true : 일반적인 지역 변수와 동일하게 동작 / false : 해당 데이터 형식의 소멸자는 프로세스 종료 시에만 호출)
    - `VRSSZ` : TLVA 변수 생성 시 내부적으로 관리하는 메모리 풀의 자원이 고갈 시 마다 생성 (+ 준비) 할 메모리 청크의 개수
```cpp
template<typename TTYPE, bool VCTDT = false, size_t VRSSZ = 1>
class thread_local_variable_auto
{
  ...
  public:
  	template<typename ... TARGS>
	  thread_local_variable_auto(TARGS ... pArgs);
    ...
```

<br>

## Precautions
- TLVM 은 형식 파라메터에 `빈 형식` (0 >= sizeof(TTYPE)) 은 지원하지 않습니다.
- TLVM 은 형식 파라메터에 `배열 형식` 은 지원하지 않습니다. 배열 형식의 사용이 필요하시다면 `std::array` 혹은 `std::vector` 등의 표준 배열 형식을 대신 사용하시기 바랍니다.
  ```cpp
  //thread_local_variable_auto<int[10]> aVal1;            // error
  thread_local_variable_auto<std::array<int, 10>> aVal1;  // ok
  aVal1->at(2) = 765;
  (*aVal1)[5] = 2819;

  for (auto elem : (*aVal1)) {
      std::cout << "array : " << elem << std::endl;
  }
  ```
- TLVA 변수의 생성 방식은 템플릿 파라메터 중 2번째인 `VCTDT` 에 따라서 아래와 같이 2가지로 구분될 수 있으며, 각 생성 방식과 경우에 따라서 사용에 주의가 필요할 수 있습니다.
  - `VCTDT == false` (기본) : TLVA 가 가리키는 데이터 형식 인스턴스는 최초 할당 시에만 생성자가 호출되며, 이후로 프로세스가 종료되기 전에는 소멸자가 호출되지 않음<br>
  따라서 이 방식으로 생성된 인스턴스는 재사용된 것일 가능성이 있으며, 이 경우 사용자가 직접 초기화를 해주어야 하는지 확인이 필요함
    ```cpp
    thread_local_variable_auto<std::vector<int>> aVec1;    // VCTDT == false

    aVec1->clear();    // ※ TLVA 변수가 재활용된 것일 수 있으므로 초기화 → 아니면 메모리 고갈이 발생할 수 있음

    for (auto idx = 0 ; 100 > idx ; ++idx) {
	    aVec1->push_back(idx + 3);
    }
    ```
  - `VCTDT == true` : TLVA 가 가리키는 데이터 형식 인스턴스는 지역 변수의 생명 주기와 동일하게 해당 변수의 유효 스코프 내에서 매번 생성자 / 소멸자가 호출됨<br>
  변수를 선언할 때 항상 생성자가 호출되고, 변수의 생명 주기가 끝날 때 항상 소멸자가 호출되므로 (만일 해당 변수의 생성자에서 초기화가 되는 상황이라면) 사용자가 별도로 해당 변수를 사용하기 전에 초기화해줄 필요는 없음
    ```cpp
    thread_local_variable_auto<std::vector<int>, true> aVec1;    // VCTDT == true

    for (auto idx = 0 ; 100 > idx ; ++idx) {
	    aVec1->push_back(idx + 3);
    }
    ```
- TLVA 변수 생성 시 생성자에 인자를 지정하는 경우에는 상기에 설명했던 `VCTDT` 지정 방식 중 항시 `true` 상태가 강제되어 해당 TLVA 변수의 생명 주기에 따라서 항시 생성자 / 소멸자가 호출되도록 강제됨 (→ 이를 지키지 않을 경우 현재는 컴파일 에러가 발생하도록 설정됨)<br>
이것이 강제되는 이유는 (보통) 변수 생성 시 생성자에 인자를 지정하는 경우에 해당 인자값이 변수의 초기화에 중요한 영향을 주게되는 경우가 있는데, TLVM 의 특성상 `VCTDT == false` 인 상태로 할당되는 TLVA 변수가 재활용된 것일 경우에는 생성자가 호출되지 않으므로 사용자가 의도했던 변수의 상태 / 동작과 다를 수 있기 때문에 이로 인한 혼란을 사전에 방지하기 위함
  ```cpp
  //thread_local_variable_auto<std::vector<int>> aVec1;            // ok
  //thread_local_variable_auto<std::vector<int>> aVec1(10);        // error
  thread_local_variable_auto<std::vector<int>, true> aVec1(10);    // ok
  ...
  ```

<br>

## Tests
TLVA 에 대한 각종 테스트들은 [Test 폴더](Test/) 에서 확인할 수 있으며, 각 테스트의 목적은 아래와 같습니다.
- `test_00_basic_usage`
  : TLVA 의 다양한 사용 방식들을 확인하는 테스트
- `test_01_performance`
  : TLVA 와 (일반적인) 지역 변수의 성능을 비교하는 테스트
- `test_02_thread_safety`
  : TLVA 를 멀티 스레드 환경에서 사용 시의 안정성을 확인하는 테스트
  
