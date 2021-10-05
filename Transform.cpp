#include "Transform.h"

Transform::Transform()
{
}

Transform::Transform(Vector3D _position, Vector3D _rotation, Vector3D _scale)
{
	m_Position = _position;
	m_Rotation = _rotation;
	m_Scale = _scale;
}

Transform::~Transform()
{

}

Transform::Transform(XMFLOAT3 _position, XMFLOAT3 _rotation, XMFLOAT3 _scale)
{
	m_Position = _position;
	m_Rotation = _rotation;
	m_Scale = _scale;
}


void Transform::MoveForward()
{
	XMFLOAT3 position = m_Position.ConvertToXMfloat3();
	position.z += 0.02f;
	m_Position = position;
}

void Transform::MoveForward(float value)
{
	XMFLOAT3 position = m_Position.ConvertToXMfloat3();
	position.z += value;
	m_Position = position;
}

void Transform::MoveBackwards()
{
	XMFLOAT3 position = m_Position.ConvertToXMfloat3();
	position.z -= 0.02f;
	m_Position = position;
}

void Transform::MoveBackwards(float value)
{
	XMFLOAT3 position = m_Position.ConvertToXMfloat3();
	position.z +- value;
	m_Position = position;
}

void Transform::MoveRight()
{
	XMFLOAT3 position = m_Position.ConvertToXMfloat3();
	position.x += 0.02f;
	m_Position = position;
}

void Transform::MoveRight(float value)
{
	XMFLOAT3 position = m_Position.ConvertToXMfloat3();
	position.x += value;
	m_Position = position;
}

void Transform::MoveLeft()
{
	XMFLOAT3 position = m_Position.ConvertToXMfloat3();
	position.x -= 0.02f;
	m_Position = position;
}

void Transform::MoveLeft(float value)
{
	XMFLOAT3 position = m_Position.ConvertToXMfloat3();
	position.x + -value;
	m_Position = position;
}