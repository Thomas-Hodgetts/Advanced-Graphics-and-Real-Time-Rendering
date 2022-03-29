#pragma once
#include "Transform.h"

class Bone
{
public:
	Bone();
	Bone(Transform* m_Transform);

private:

	Transform* m_Transform;
};

