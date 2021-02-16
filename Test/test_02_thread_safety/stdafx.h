// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 특정 포함 파일이 들어 있는
// 포함 파일입니다.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#define NOMINMAX						// std::numeric_limits<T>::max() 와 충돌이 발생하여 min, max 비활성화	// https://andromedarabbit.net/%EB%A7%A4%ED%81%AC%EB%A1%9C%EC%97%90-%EB%8C%80%EC%B2%98%ED%95%98%EB%8A%94-%EC%9A%B0%EB%A6%AC%EC%9D%98-%EC%9E%90%EC%84%B8/
// Windows 헤더 파일
#include <windows.h>

// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// 여기서 프로그램에 필요한 추가 헤더를 참조합니다.
#include <thread>		// std::thread

