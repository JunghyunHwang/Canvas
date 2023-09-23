#pragma once

#define DEFAULT_OBJECT_CAPACITY (256)
#define OBJECT_MARGIN (10)
#define SELECTED_RECT_MARGIN (5)
#define NONE_POINT (-10.f)
#define RESIZING_RECTS_COUNT (8)
#define RESIZING_RECT_SIZE (4)

#define PRESSED(key) ((key) & 0x8000)

#define SET_RECT_BY_OBJ_POINTER(rect, object)	rect.left = object->mLeftTop.x;\
												rect.top = object->mLeftTop.y;\
												rect.right = object->mRightBottom.x;\
												rect.bottom = object->mRightBottom.y;\

#define SET_NONE_RECT(object)	object->mLeftTop.x = NONE_POINT;\
								object->mLeftTop.y = NONE_POINT;\
								object->mRightBottom.x = NONE_POINT;\
								object->mRightBottom.y = NONE_POINT;\

#define ADD_MARGIN_TO_RECT(rect, margin)	rect.left -= margin;\
											rect.top -= margin;\
											rect.right += margin;\
											rect.bottom += margin;\

#define DEBUG_BREAK(expression)	if (!(expression)) {\
									__debugbreak();\
								}\

enum class eMouseMode
{
	Select,
	Selected,
	Resize,
	Rect,

	Count
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
