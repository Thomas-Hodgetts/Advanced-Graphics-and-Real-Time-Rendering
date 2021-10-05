#pragma once
#include <windows.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#include "Debug.h"

using namespace DirectX;


//2D Vector Class. Is the defualt class for objects in 2D space & for texture coordinates. Has basic math abilities as well as the ability to convert itself into a XMFLOAT2 or XMVector
class Vector2
{
public:
	//Default Constructr
	Vector2();
	Vector2(float U, float V);
	Vector2(double U, double V);
	Vector2(int U, int V);
	Vector2(UINT U, UINT V);
	Vector2(const Vector2& vec);
	Vector2(XMFLOAT2 UV);
	Vector2(XMVECTOR UV);

	~Vector2();

	///Operations///

	//addition
	Vector2 Vector2::operator+(const Vector2& vec);
	Vector2& Vector2::operator+=(const Vector2& vec);
	Vector2 Vector2::operator+(float value);
	Vector2& Vector2::operator+=(float value);

	//subtraction
	Vector2 Vector2::operator-(const Vector2& vec);
	Vector2& Vector2::operator-=(const Vector2& vec);
	Vector2 Vector2::operator-(float value);
	Vector2& Vector2::operator-=(float value);

	//multiplication

	Vector2 Vector2::operator*(const Vector2& vec);
	Vector2& Vector2::operator*=(const Vector2& vec);
	Vector2 Vector2::operator*(float value);
	Vector2& Vector2::operator*=(float value);

	//division
	Vector2 Vector2::operator/(const Vector2& vec);
	Vector2& Vector2::operator/=(const Vector2& vec);
	Vector2 Vector2::operator/(float value);
	Vector2& Vector2::operator/=(float value);


	//Equal to
	Vector2& Vector2::operator=(const Vector2& vec);
	bool& Vector2::operator==(const Vector2& vec);
	bool& Vector2::operator!=(const Vector2& vec);

	//Bigger & less than
	bool& Vector2::operator>(const Vector2& vec);
	bool& Vector2::operator<(const Vector2& vec);


	///Common Math ///
	//Converts a Vector3 into a XMFLOAT2
	static XMFLOAT2 ConvertToXMfloat2(const Vector2& vec) { return XMFLOAT2(vec.m_U, vec.m_V); }
	//Converts Local Vector3 into a XMFLOAT2.
	XMFLOAT2 ConvertToXMfloat2() { return XMFLOAT2(m_U, m_V); }
	//Converts Vector3 into a XMVECTOR
	static XMVECTOR ConvertToXMvector2(const Vector2& vec) { return XMLoadFloat2(&XMFLOAT2(vec.m_U, vec.m_V)); }
	//Converts Local Vector3 into a XMVECTOR
	XMVECTOR ConvertToXMvector2() { return XMLoadFloat2(&XMFLOAT2(m_U, m_V)); }
	//Converts a Vector

	//Rests the variable to the state that the default constructor would create the variable in
	void Vector2_Default() { m_U = 0; m_V = 0;}
	//Inverts the value of the vector
	void Invert() { m_U = -m_U; m_V = -m_V;}
	//Inverts the value of the vector
	Vector2 ReturnInvert() { return Vector2(-m_U, -m_V); }
	//Inverts the X value of the vector
	void InvertX() { m_U = -m_U; }
	//Inverts the Y value of the vector
	void InvertY() { m_V = -m_V; }
	//Returns a default Vector
	static Vector2 ZeroVector2() { return Vector2(0, 0); }
	//Returns an directional Vector2
	static Vector2 UpVector2() { return Vector2(0, 1); }
	//Returns an directional Vector2
	static Vector2 DownVector2() { return Vector2(0, -1); }
	//Returns an directional Vector2
	static Vector2 RightVector2() { return Vector2(1, 0); }
	//Returns an directional Vector2
	static Vector2 LeftVector2() { return Vector2(-1, 0); }

	//Debugging Functions

	//Displays current value in the output log of VS
	void Display();


	//Return Functions
	float ReturnU() { return m_U; };
	float ReturnV() { return m_V; };

	//Set Functions
	void SetU(float NewU) { m_U = NewU; };
	void SetV(float NewV) { m_V = NewV; };

private:
	float m_U, m_V;
};

