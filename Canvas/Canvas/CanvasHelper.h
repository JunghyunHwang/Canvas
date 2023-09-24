#pragma once

#define DEFAULT_OBJECT_CAPACITY (256)
#define OBJECT_MARGIN (10)
#define SELECTED_RECT_MARGIN (5)
#define NONE_POINT (-10.f)
#define RESIZING_RECTS_COUNT (8)
#define RESIZING_RECT_SIZE (4)
#define DEFAULT_STROKE_WIDTH (2)

#define PRESSED(key) ((key) & 0x8000)

#define SET_NONE_RECT(object)	object->mRect.left = NONE_POINT;\
								object->mRect.top = NONE_POINT;\
								object->mRect.right = NONE_POINT;\
								object->mRect.bottom = NONE_POINT;\

#define ADD_MARGIN_TO_RECT(rect, margin)	rect.left -= margin;\
											rect.top -= margin;\
											rect.right += margin;\
											rect.bottom += margin;\

#ifdef _DEBUG
#define DEBUG_BREAK(expression) \
if (!(expression))\
{\
    __debugbreak();\
}
#else
#define DEBUG_BREAK(expression)
#endif

enum class eMouseMode
{
	Select,
	Selected,
	Resize,
	Rect,
};

enum class eResizingDirection
{
	NorthWest,
	North,
	NorthEast,
	West,
	East,
	SouthWest,
	South,
	SouthEast,

	None
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
