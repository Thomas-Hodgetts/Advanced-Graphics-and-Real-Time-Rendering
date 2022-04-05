#pragma once
#include "Bone.h"
#include <vector>

class Skeletal_Region
{
public:
	Skeletal_Region(std::vector<Bone> Bones);

	void ApplyAnimation();

private:

	Bone* m_Bones;

};

