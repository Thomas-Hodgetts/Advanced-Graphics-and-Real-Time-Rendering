#pragma once
#include "Object.h"
class ImaginaryObject : public Object
{
public:
	ImaginaryObject(Transform* transform) : m_Transform(transform) {}
	Transform* m_Transform;
};

