#pragma once
#include "Transform.h"
#include <vector>

class KeyFrame
{
public:
	KeyFrame(Transform pos);

private:
	
	XMFLOAT4X4 m_Keyframe;

};

