// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
// Windows 헤더 파일
#include <windows.h>
// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>							/////
#include <map>								/////
#include <iostream>							/////

///// DX Header Files
#include <d2d1.h>							/////
#include <d2d1helper.h>						/////
#include <dwrite.h>							/////
#include <wincodec.h>						/////

///// trace
#include "Trace.h"							/////

#pragma comment(lib, "d2d1.lib")			/////
#pragma comment(lib, "dwrite.lib")			/////
#pragma comment(lib, "WindowsCodecs.lib")	/////

///// 속성 -> 링커 -> 입력 -> 추가 종속성 (d2d1.lib; dwrite.lib; WindowsCodecs.lib;)

///// 매크로 함수
#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) = NULL; } }	/////
