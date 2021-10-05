#pragma once
#include <directxmath.h>
#include <d3d11_1.h>
#include <windows.h>
#include "Debug.h"

using namespace DirectX;

//3D Vector Class. Is the defualt class for objects in 3D space. Has basic math abilities as well as the ability to convert itself into a XMFLOAT3 or XMVector3
class Vector3D
{
public:
	//Default Constructr
	Vector3D();
	Vector3D(float X, float Y, float Z);
	Vector3D(double X, double Y, double Z);
	Vector3D(int X, int Y, int Z);
	Vector3D(UINT X, UINT Y, UINT Z);
	Vector3D(const Vector3D& vec);
	Vector3D(XMFLOAT3 XYZ);
	Vector3D(XMVECTOR XYZ);

	~Vector3D();

	//Operators

	//addition
	Vector3D Vector3D::operator+(const Vector3D& vec);
	Vector3D& Vector3D::operator+=(const Vector3D& vec);
	Vector3D Vector3D::operator+(float value);
	Vector3D& Vector3D::operator+=(float value);
	
	//subtraction
	Vector3D Vector3D::operator-(const Vector3D& vec);
	Vector3D& Vector3D::operator-=(const Vector3D& vec);
	Vector3D Vector3D::operator-(float value);
	Vector3D& Vector3D::operator-=(float value);

	//multiplication

	Vector3D Vector3D::operator*(const Vector3D& vec);
	Vector3D& Vector3D::operator*=(const Vector3D& vec);
	Vector3D Vector3D::operator*(float value);
	Vector3D& Vector3D::operator*=(float value);

	//division
	Vector3D Vector3D::operator/(const Vector3D& vec);
	Vector3D& Vector3D::operator/=(const Vector3D& vec);
	Vector3D Vector3D::operator/(float value);
	Vector3D& Vector3D::operator/=(float value);


	//Equal to
	Vector3D& Vector3D::operator=(const Vector3D& vec);
	bool& Vector3D::operator==(const Vector3D& vec);
	bool& Vector3D::operator!=(const Vector3D& vec);

	//Bigger & less than
	bool& Vector3D::operator>(const Vector3D& vec);
	bool& Vector3D::operator<(const Vector3D& vec);



	//Common math

	//Converts a Vector3 into a XMFLOAT3
	static XMFLOAT3 ConvertToXMfloat3(const Vector3D& vec) { return XMFLOAT3(vec.m_X, vec.m_Y, vec.m_Z); }
	//Converts Local Vector3 into a XMFLOAT3.
	XMFLOAT3 ConvertToXMfloat3() { return XMFLOAT3(m_X, m_Y, m_Z); }
	//Converts Vector3 into a XMVECTOR
	static XMVECTOR ConvertToXMvector3(const Vector3D& vec){ return XMLoadFloat3(&XMFLOAT3(vec.m_X, vec.m_Y, vec.m_Z)); }
	//Converts Local Vector3 into a XMVECTOR
	XMVECTOR ConvertToXMvector3(){ return XMLoadFloat3(&XMFLOAT3(m_X, m_Y, m_Z)); }
	//Converts a Vector

	//Performs a cross product calculation and returns the value as a XMFLOAT3
	Vector3D CrossProductCalculation(const Vector3D& vec);
	//Performs a dot product calculation and returns it as a float
	float DotProduct(const Vector3D& vec);
	//Normalised a Vector that is passed through
	Vector3D NormaliseVector3D(const Vector3D& vec);
	//Rests the variable to the state that the default constructor would create the variable in
	void Vector3D_Default() { m_X = 0; m_Y = 0; m_Z = 0; }
	//Inverts the value of the vector
	void Invert() { m_X = -m_X; m_Y = -m_Y; m_Z = -m_Z;}
	//Inverts the value of the vector
	Vector3D ReturnInvert() { return Vector3D(-m_X, -m_Y, -m_Z); }
	//Inverts the X value of the vector
	void InvertX() { m_X = -m_X;}
	//Inverts the Y value of the vector
	void InvertY() {m_Y = -m_Y;}
	//Inverts the Z value of the vector
	void InvertZ() {m_Z = -m_Z;}
	//Returns a default Vector
	static Vector3D ZeroVector3D() { return Vector3D(0, 0, 0); }
	//Returns an directional Vector3D
	static Vector3D UpVector3D() { return Vector3D(0, 1, 0); }
	//Returns an directional Vector3D
	static Vector3D DownVector3D() { return Vector3D(0, -1, 0); }
	//Returns an directional Vector3D
	static Vector3D RightVector3D() { return Vector3D(1, 0, 0); }
	//Returns an directional Vector3D
	static Vector3D LeftVector3D() { return Vector3D(-1, 0, 0); }
	//Returns the Magnitude of a given vector
	float ReturnMagnitude(const Vector3D& vec);

	//Debugging Functions

	//Displays current value in the output log of VS
	void Display();

	//Return Functions
	float ReturnX() { return m_X; };
	float ReturnY() { return m_Y; };
	float ReturnZ() { return m_Z; };

	//Set Functions
	void SetX(float NewX) { m_X = NewX; };
	void SetY(float NewY) { m_Y = NewY; };
	void SetZ(float NewZ) { m_Z = NewZ; };

private:
	
	float m_X, m_Y, m_Z;
};

