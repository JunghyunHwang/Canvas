#pragma once

#define DEFAULT_OBJECT_CAPACITY (256)
#define OBJECT_MARGIN (10)
#define SELECTED_RECT_MARGIN (5)
#define NONE_POINT (-10.f)

#define PRESSED(key) ((key) & 0x8000)

#define SET_RECT_BY_OBJ_POINTER(rect, object)	rect.left = object->mLeftTop.x;\
												rect.top = object->mLeftTop.y;\
												rect.right = object->mRightBottom.x;\
												rect.bottom = object->mRightBottom.y;\

#define SET_NONE_RECT(object)	object->mLeftTop.x = NONE_POINT;\
								object->mLeftTop.y = NONE_POINT;\
								object->mRightBottom.x = NONE_POINT;\
								object->mRightBottom.y = NONE_POINT;\

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