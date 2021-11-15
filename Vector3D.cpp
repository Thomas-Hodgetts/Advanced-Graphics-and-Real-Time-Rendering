#include "Vector3D.h"

Vector3D::Vector3D()
{
	m_X = 0;
	m_Y = 0;
	m_Z = 0;
}

Vector3D::Vector3D(float X, float Y, float Z)
{
	m_X = X;
	m_Y = Y;
	m_Z = Z;
}

Vector3D::Vector3D(double X, double Y, double Z)
{
	m_X = X;
	m_Y = Y;
	m_Z = Z;
}


Vector3D::Vector3D(int X, int Y, int Z)
{
	m_X = X;
	m_Y = Y;
	m_Z = Z;
}

Vector3D::Vector3D(UINT X, UINT Y, UINT Z)
{
	m_X = X;
	m_Y = Y;
	m_Z = Z;
}

Vector3D::Vector3D(const Vector3D& vec)
{
	m_X = vec.m_X;
	m_Y = vec.m_Y;
	m_Z = vec.m_Z;
}

Vector3D::Vector3D(XMFLOAT3 XYZ)
{
	m_X = XYZ.x;
	m_Y = XYZ.y;
	m_Z = XYZ.z;
}

Vector3D::Vector3D(XMVECTOR XYZ)
{
	XMFLOAT3 xyz;
	XMStoreFloat3(&xyz, XYZ);
	m_X = xyz.x;
	m_Y = xyz.y;
	m_Z = xyz.z;
}

Vector3D::~Vector3D()
{

}

Vector3D Vector3D::CrossProductCalculation(const Vector3D& vec)
{
	XMFLOAT3 FinalCrossProduct;
	XMStoreFloat3(&FinalCrossProduct, XMVector3Cross(ConvertToXMvector3(*this), ConvertToXMvector3(vec )));
	return FinalCrossProduct;
}

Vector3D Vector3D::NormaliseVector3D(const Vector3D& vec)
{
	XMFLOAT3 FinalNorm;
	XMStoreFloat3(&FinalNorm, XMVector3Normalize(ConvertToXMvector3(vec)));
	return FinalNorm;
}

float Vector3D::ReturnMagnitude(const Vector3D& vec)
{
	if (((vec.m_X * vec.m_X) + (vec.m_Y * vec.m_Y) + (vec.m_Z * vec.m_Z)) == 0)
	{
		Debug d;
		d.OutputString("Cannot sqrt 0");
		return 0;
	}
	float finalMag = sqrtf((vec.m_X * vec.m_X) + (vec.m_Y * vec.m_Y) + (vec.m_Z * vec.m_Z));
	return finalMag;
}

float Vector3D::DotProduct(const Vector3D& vec)
{
	return (m_X * vec.m_X + m_Y * vec.m_Y + m_Z * vec.m_Z);
}



void Vector3D::Display()
{
	Debug::OutputString("Current Value is: X = " + std::to_string(m_X) + " , Y = " + std::to_string(m_Y) + " , Z = " + std::to_string(m_Z) + "\n");
}

Vector3D Vector3D::operator+(const Vector3D& vec)
{
	return Vector3D(m_X + vec.m_X, m_Y + vec.m_Y, m_Z + vec.m_Z);
}

Vector3D Vector3D::operator+(float value)
{
	return Vector3D(m_X + value, m_Y + value, m_Z + value);
}

Vector3D& Vector3D::operator+=(const Vector3D& vec)
{
	m_X += vec.m_X;
	m_Y += vec.m_Y;
	m_Z += vec.m_Z;
	return *this;
}

Vector3D& Vector3D::operator+=(float value)
{
	m_X += value;
	m_Y += value;
	m_Z += value;
	return *this;
}

Vector3D Vector3D::operator-(const Vector3D& vec)
{
	return Vector3D(m_X - vec.m_X, m_Y - vec.m_Y, m_Z - vec.m_Z);
}

Vector3D Vector3D::operator-(float value)
{
	return Vector3D(m_X - value, m_Y - value, m_Z - value);
}

Vector3D& Vector3D::operator-=(const Vector3D& vec)
{
	m_X -= vec.m_X;
	m_Y -= vec.m_Y;
	m_Z -= vec.m_Z;
	return *this;

}

Vector3D& Vector3D::operator-=(float value)
{
	m_X -= value;
	m_Y -= value;
	m_Z -= value;
	return *this;
}

Vector3D Vector3D::operator*(const Vector3D& vec)
{
	return Vector3D(m_X * vec.m_X, m_Y * vec.m_Y, m_Z * vec.m_Z);
}
Vector3D& Vector3D::operator*=(const Vector3D& vec)
{
	m_X *= vec.m_X;
	m_Y *= vec.m_Y;
	m_Z *= vec.m_Z;
	return *this;
}

Vector3D Vector3D::operator*(float value)
{
	return Vector3D(m_X * value, m_Y * value, m_Z * value);
}

Vector3D& Vector3D::operator*=(float value)
{
	m_X *= value;
	m_Y *= value;
	m_Z *= value;
	return *this;
}

Vector3D Vector3D::operator/(const Vector3D& vec)
{
	assert(vec.m_X != 0 && vec.m_Y != 0 && vec.m_Z != 0);
	return Vector3D(m_X / vec.m_X, m_Y / vec.m_Y, m_Z / vec.m_Z);
}
Vector3D& Vector3D::operator/=(const Vector3D& vec)
{
	assert(vec.m_X != 0 && vec.m_Y != 0 && vec.m_Z != 0);
	m_X /= vec.m_X;
	m_Y /= vec.m_Y;
	m_Z /= vec.m_Z;
	return *this;
}

Vector3D Vector3D::operator/(float value)
{
	assert(value != 0);
	return Vector3D(m_X / value, m_Y / value, m_Z / value);
}

Vector3D& Vector3D::operator/=(float value)
{
	assert(value != 0);
	m_X /= value;
	m_Y /= value;
	m_Z /= value;
	return *this;
}


Vector3D& Vector3D::operator=(const Vector3D& vec)
{
	m_X = vec.m_X;
	m_Y = vec.m_Y;
	m_Z = vec.m_Z;
	return *this;
}

bool& Vector3D::operator==(const Vector3D& vec)
{
	if (m_X == vec.m_X && m_Y == vec.m_Y && m_Z == vec.m_Z)
	{
		bool t = true;
		return t;
	}
	bool f = false;
	return f;
}

bool& Vector3D::operator!=(const Vector3D& vec)
{
	if (m_X != vec.m_X || m_Y != vec.m_Y || m_Z != vec.m_Z)
	{
		bool t = true;
		return t;
	}
	bool f = false;
	return f;
}

bool& Vector3D::operator>(const Vector3D& vec)
{
	if (m_X > vec.m_X && m_Y > vec.m_Y && m_Z > vec.m_Z)
	{
		bool t = true;
		return t;
	}
	bool f = false;
	return f;
}
bool& Vector3D::operator<(const Vector3D& vec)
{
	if (m_X < vec.m_X && m_Y < vec.m_Y && m_Z < vec.m_Z)
	{
		bool t = true;
		return t;
	}
	bool f = false;
	return f;
}