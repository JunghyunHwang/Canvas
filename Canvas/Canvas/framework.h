// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>

// C RunTime Header Files:
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#include <math.h>
#include <vector>
#include <unordered_set>
#include <limits>
#include <intrin.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

#include "Object.h"

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

#define DEFAULT_OBJECT_CAPACITY (256)
#define MAX_LOADSTRING 100
#define SELECT_MARGIN (10)
#define SELECTED_RECT_MARGIN (5)

#define PRESSED(key) ((key) & 0x8000) > 0

enum class eMouseMode
{
	Select,
	Selected,
	Rect,

	Count
};

template<typename Interface>
inline void SafeRelease(Interface** interfaceToRelease)
{
	if (*interfaceToRelease != nullptr)
	{
		(*interfaceToRelease)->Release();
		*interfaceToRelease = nullptr;
	}
}
