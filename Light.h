#pragma once
#include "ImaginaryObject.h"
class Light : public ImaginaryObject
{
public:
	Light(Transform* trans) : ImaginaryObject(trans) {};
	~Light();

private:
};

