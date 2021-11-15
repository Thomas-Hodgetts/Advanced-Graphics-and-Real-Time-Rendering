#include "Vector2.h"

Vector2::Vector2()
{
	m_U = 0;
	m_V = 0;
}

Vector2::Vector2(float U, float V)
{
	m_U = U;
	m_V = V;
}

Vector2::Vector2(double U, double V)
{
	m_U = U;
	m_V = V;
}

Vector2::Vector2(int U, int V)
{
	m_U = U;
	m_V = V;
}

Vector2::Vector2(UINT U, UINT V)
{
	m_U = U;
	m_V = V;
}

Vector2::Vector2(const Vector2& vec)
{
	m_U = vec.m_U;
	m_V = vec.m_V;
}

Vector2::Vector2(XMFLOAT2 UV)
{
	m_U = UV.x;
	m_V = UV.y;
}

Vector2::Vector2(XMVECTOR UV)
{
	XMFLOAT2 xy;
	XMStoreFloat2(&xy, UV);
	m_U = xy.x;
	m_V = xy.y;
}

Vector2::~Vector2()
{
}


Vector2 Vector2::operator+(const Vector2& vec)
{
	return Vector2(m_U + vec.m_U, m_V + vec.m_V);
}

Vector2 Vector2::operator+(float value)
{
	return Vector2(m_U + value, m_V + value);
}

Vector2& Vector2::operator+=(const Vector2& vec)
{
	m_U += vec.m_U;
	m_V += vec.m_V;
	return *this;
}

Vector2& Vector2::operator+=(float value)
{
	m_U += value;
	m_V += value;
	return *this;
}

Vector2 Vector2::operator-(const Vector2& vec)
{
	return Vector2(m_U - vec.m_U, m_V - vec.m_V);
}

Vector2 Vector2::operator-(float value)
{
	return Vector2(m_U - value, m_V - value);
}

Vector2& Vector2::operator-=(const Vector2& vec)
{
	m_U -= vec.m_U;
	m_V -= vec.m_V;
	return *this;

}

Vector2& Vector2::operator-=(float value)
{
	m_U -= value;
	m_V -= value;
	return *this;
}

Vector2 Vector2::operator*(const Vector2& vec)
{
	return Vector2(m_U * vec.m_U, m_V * vec.m_V);
}
Vector2& Vector2::operator*=(const Vector2& vec)
{
	m_U *= vec.m_U;
	m_V *= vec.m_V;
	return *this;
}

Vector2 Vector2::operator*(float value)
{
	return Vector2(m_U * value, m_V * value);
}

Vector2& Vector2::operator*=(float value)
{
	m_U *= value;
	m_V *= value;
	return *this;
}

Vector2 Vector2::operator/(const Vector2& vec)
{
	assert(vec.m_U != 0 && vec.m_V != 0);
	return Vector2(m_U / vec.m_U, m_V / vec.m_V);
}
Vector2& Vector2::operator/=(const Vector2& vec)
{
	assert(vec.m_U != 0 && vec.m_V != 0);
	m_U /= vec.m_U;
	m_V /= vec.m_V;
	return *this;
}

Vector2 Vector2::operator/(float value)
{
	assert(value != 0);
	return Vector2(m_U / value, m_V / value);
}

Vector2& Vector2::operator/=(float value)
{
	assert(value != 0);
	m_U /= value;
	m_V /= value;
	return *this;
}


Vector2& Vector2::operator=(const Vector2& vec)
{
	m_U = vec.m_U;
	m_V = vec.m_V;
	return *this;
}

bool& Vector2::operator==(const Vector2& vec)
{
	if (m_U == vec.m_U && m_V == vec.m_V)
	{
		bool t = true;
		return t;
	}
	bool f = false;
	return f;
}

bool& Vector2::operator!=(const Vector2& vec)
{
	if (m_U != vec.m_U || m_V != vec.m_V)
	{
		bool t = true;
		return t;
	}
	bool f = false;
	return f;
}

bool& Vector2::operator>(const Vector2& vec)
{
	if (m_U > vec.m_U && m_V > vec.m_V)
	{
		bool t = true;
		return t;
	}
	bool f = false;
	return f;
}
bool& Vector2::operator<(const Vector2& vec)
{
	if (m_U < vec.m_U && m_V < vec.m_V)
	{
		bool t = true;
		return t;
	}
	bool f = false;
	return f;
}

void Vector2::Display()
{
	Debug::OutputString("Current Value is: U = " + std::to_string(m_U) + " , V = " + std::to_string(m_V));
}
